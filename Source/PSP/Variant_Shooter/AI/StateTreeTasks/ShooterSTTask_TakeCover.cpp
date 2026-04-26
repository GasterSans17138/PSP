#include "ShooterSTTask_TakeCover.h"
#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "ShooterCoverPoint.h"
#include "ShooterCoverSubsystem.h"
#include "StateTreeExecutionContext.h"
#include "ShooterSquadComponent.h"
#include "Engine/World.h"

AShooterAIController* FShooterSTTask_TakeCover::GetController(FStateTreeExecutionContext& Context) const
{
	return Cast<AShooterAIController>(Context.GetOwner());
}

AShooterAICharacter* FShooterSTTask_TakeCover::GetAICharacter(FStateTreeExecutionContext& Context) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		return Cast<AShooterAICharacter>(Controller->GetPawn());
	}
	return nullptr;
}

EStateTreeRunStatus FShooterSTTask_TakeCover::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult&) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	AShooterAIController* Controller = GetController(Context);
	AShooterAICharacter* AIChar = GetAICharacter(Context);

	if (!IsValid(Controller) || !IsValid(AIChar))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!IsValid(Controller->GetCombatTarget()))
	{
		Controller->AcquirePlayerTarget();
	}

	AIChar->RefreshAIState();

	if (UShooterSquadComponent* SquadComp = Controller->GetControlledSquadComponent())
	{
		SquadComp->BroadcastTarget(Controller->GetCombatTarget());
		AIChar->RefreshSquadOrder();
	}

	Data.LastMoveRequestTime = -1000.0f;
	Data.CoverReachedTime = -1.0f;
	Data.LastCoverEvaluationTargetLocation = FVector::ZeroVector;
	Data.bHasCoverEvaluationTargetLocation = false;

	if (Controller->GetCoverCombatPhase() == EShooterCoverCombatPhase::None)
	{
		Controller->SetCoverCombatPhase(EShooterCoverCombatPhase::TakingCover);
	}

	Controller->SetFireEnabled(false);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FShooterSTTask_TakeCover::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	AShooterAIController* Controller = GetController(Context);
	AShooterAICharacter* AIChar = GetAICharacter(Context);
	if (!IsValid(Controller) || !IsValid(AIChar))
	{
		return EStateTreeRunStatus::Failed;
	}

	// Resolve threat FIRST, so RefreshAIState/RefreshSquadOrder use the latest target/LOS.
	AActor* ThreatActor = AIChar->GetCachedOrder().TargetActor;
	if (!IsValid(ThreatActor))
	{
		ThreatActor = Controller->GetCombatTarget();
	}

	if (!IsValid(ThreatActor))
	{
		Controller->AcquirePlayerTarget();
		ThreatActor = Controller->GetCombatTarget();
	}

	AIChar->RefreshAIState();

	if (AIChar->CurrentTacticalOrder != EShooterTacticalOrder::TakeCover)
	{
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Succeeded;
	}

	if (!IsValid(ThreatActor))
	{
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	UWorld* World = Controller->GetWorld();
	if (!World)
	{
		return EStateTreeRunStatus::Failed;
	}

	UShooterCoverSubsystem* CoverSubsystem = World->GetSubsystem<UShooterCoverSubsystem>();
	if (!CoverSubsystem)
	{
		return EStateTreeRunStatus::Failed;
	}

	AShooterCoverPoint* CurrentCover = Controller->GetCurrentCoverPoint();

	const FVector TargetLocation = ThreatActor->GetActorLocation();

	bool bShouldReevaluateCover = false;

	if (!Data.bHasCoverEvaluationTargetLocation)
	{
		Data.LastCoverEvaluationTargetLocation = TargetLocation;
		Data.bHasCoverEvaluationTargetLocation = true;
	}
	else
	{
		const float TargetMovedDist = FVector::Dist(TargetLocation, Data.LastCoverEvaluationTargetLocation);
		if (TargetMovedDist >= Data.ReevaluateCoverIfTargetMovedDistance)
		{
			bShouldReevaluateCover = true;
		}
	}

	if (IsValid(CurrentCover))
	{
		const bool bCurrentCoverStillValid =
			CoverSubsystem->IsCoverStillValidAgainstThreat(CurrentCover, ThreatActor);

		if (!bCurrentCoverStillValid)
		{
			bShouldReevaluateCover = true;
		}
	}

	if (bShouldReevaluateCover)
	{
		if (IsValid(CurrentCover))
		{
			CoverSubsystem->ReleaseCover(CurrentCover, Controller->GetPawn());
			Controller->ClearCurrentCoverPoint();
			CurrentCover = nullptr;
		}

		Data.CoverReachedTime = -1.0f;
		Data.LastCoverEvaluationTargetLocation = TargetLocation;
		Data.bHasCoverEvaluationTargetLocation = true;

		Controller->SetCoverCombatPhase(EShooterCoverCombatPhase::TakingCover);
	}

	if (!IsValid(CurrentCover))
	{
		const FShooterSquadOrder& CachedOrder = AIChar->GetCachedOrder();

		const FVector PreferredCoverLocation = !CachedOrder.MoveLocation.IsNearlyZero()
			? CachedOrder.MoveLocation
			: Controller->GetPawn()->GetActorLocation();

		AShooterCoverPoint* BestCover = CoverSubsystem->FindBestCoverNearLocation(
			Controller->GetPawn(),
			ThreatActor,
			PreferredCoverLocation,
			CachedOrder.BaseTacticalOrder,
			Data.SearchRadius);

		if (IsValid(BestCover) && CoverSubsystem->ReserveCover(BestCover, Controller->GetPawn()))
		{
			Controller->SetCurrentCoverPoint(BestCover);
			CurrentCover = BestCover;
		}
	}

	if (!IsValid(CurrentCover))
	{
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	const float Time = World->GetTimeSeconds();
	const FVector SelfLocation = Controller->GetPawn()->GetActorLocation();
	const FVector CoverLocation = CurrentCover->GetCoverLocation();
	const float DistToCover = FVector::Dist(SelfLocation, CoverLocation);

	const bool bCloseEnoughToCover = DistToCover <= Data.CoverValidationDistance;

	if (!bCloseEnoughToCover)
	{
		Data.CoverReachedTime = -1.0f;

		if ((Time - Data.LastMoveRequestTime) >= Data.RepathInterval)
		{
			if (Controller->MoveToCoverPoint(Data.MoveAcceptanceRadius, true))
			{
				Data.LastMoveRequestTime = Time;
			}
		}

		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	Controller->StopMovement();
	Controller->SetFireEnabled(false);

	if (Data.CoverReachedTime < 0.0f)
	{
		Data.CoverReachedTime = Time;
	}

	const float TimeInCover = Time - Data.CoverReachedTime;

	const float SuppressionAlpha = AIChar->GetSuppressionAlpha();
	const float EffectiveCoverHoldDuration =
		Data.CoverHoldDuration + (Data.ExtraCoverHoldDurationWhenSuppressed * SuppressionAlpha);

	if (TimeInCover < EffectiveCoverHoldDuration)
	{
		return EStateTreeRunStatus::Running;
	}

	// Cover wait finished: next state must be Peek.
	Controller->SetCoverCombatPhase(EShooterCoverCombatPhase::Peek);

	AIChar->RefreshAIState();

	return EStateTreeRunStatus::Succeeded;
}

void FShooterSTTask_TakeCover::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult&) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		// Keep the reserved/current cover so Peek can use it next.
		Controller->SetFireEnabled(false);
	}
}