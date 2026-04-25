#include "ShooterSTTask_PeekFromCover.h"
#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "ShooterCoverPoint.h"
#include "StateTreeExecutionContext.h"
#include "ShooterCoverSubsystem.h"

AShooterAIController* FShooterSTTask_PeekFromCover::GetController(FStateTreeExecutionContext& Context) const
{
	return Cast<AShooterAIController>(Context.GetOwner());
}

AShooterAICharacter* FShooterSTTask_PeekFromCover::GetAICharacter(FStateTreeExecutionContext& Context) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		return Cast<AShooterAICharacter>(Controller->GetPawn());
	}
	return nullptr;
}

EStateTreeRunStatus FShooterSTTask_PeekFromCover::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult&) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	AShooterAIController* Controller = GetController(Context);
	if (!IsValid(Controller) || !IsValid(Controller->GetCurrentCoverPoint()))
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.StateEnterTime = Controller->GetWorld()->GetTimeSeconds();
	Data.LastMoveRequestTime = -1000.0f;
	Data.PeekStartTime = -1.0f;
	Data.bHasSeenTargetDuringPeek = false;

	Controller->SetFireEnabled(false);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FShooterSTTask_PeekFromCover::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	AShooterAIController* Controller = GetController(Context);
	AShooterAICharacter* AIChar = GetAICharacter(Context);
	if (!IsValid(Controller) || !IsValid(AIChar))
	{
		return EStateTreeRunStatus::Failed;
	}

	AIChar->RefreshAIState();

	if (!AIChar->bOrderIsPeek)
	{
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Succeeded;
	}

	AShooterCoverPoint* CoverPoint = Controller->GetCurrentCoverPoint();
	if (!IsValid(CoverPoint))
	{
		return EStateTreeRunStatus::Failed;
	}

	AActor* ThreatActor = AIChar->GetCachedOrder().TargetActor;
	if (!IsValid(ThreatActor))
	{
		ThreatActor = Controller->GetCombatTarget();
	}

	if (!IsValid(ThreatActor))
	{
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	const FShooterSquadOrder& CachedOrder = AIChar->GetCachedOrder();

	float EffectivePeekDuration = Data.PeekDuration;
	float EffectiveBurstDuration = 0.4f;
	float EffectiveBurstPause = 0.3f;

	switch (CachedOrder.BaseTacticalOrder)
	{
	case EShooterTacticalOrder::Suppress:
		EffectivePeekDuration = Data.SuppressPeekDuration;
		EffectiveBurstDuration = Data.SuppressBurstDuration;
		EffectiveBurstPause = Data.SuppressBurstPause;
		break;

	case EShooterTacticalOrder::Push:
		EffectivePeekDuration = Data.PushPeekDuration;
		EffectiveBurstDuration = Data.PushBurstDuration;
		EffectiveBurstPause = Data.PushBurstPause;
		break;

	case EShooterTacticalOrder::FlankLeft:
	case EShooterTacticalOrder::FlankRight:
		EffectivePeekDuration = Data.FlankPeekDuration;
		EffectiveBurstDuration = Data.FlankBurstDuration;
		EffectiveBurstPause = Data.FlankBurstPause;
		break;

	case EShooterTacticalOrder::Hold:
		EffectivePeekDuration = Data.HoldPeekDuration;
		EffectiveBurstDuration = Data.HoldBurstDuration;
		EffectiveBurstPause = Data.HoldBurstPause;
		break;

	default:
		break;
	}

	const float Time = Controller->GetWorld()->GetTimeSeconds();
	const FVector SelfLocation = Controller->GetPawn()->GetActorLocation();
	const FVector PeekLocation = CoverPoint->GetPeekLocation();
	const float DistToPeek = FVector::Dist(SelfLocation, PeekLocation);

	const bool bCloseEnoughToPeek = DistToPeek <= Data.FireStartDistance;

	if (!bCloseEnoughToPeek)
	{
		Data.PeekStartTime = -1.0f;
		Data.bHasSeenTargetDuringPeek = false;

		if ((Time - Data.LastMoveRequestTime) >= Data.RepathInterval)
		{
			if (Controller->MoveToPeekLocation(Data.MoveAcceptanceRadius, true))
			{
				Data.LastMoveRequestTime = Time;
			}
		}

		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	Controller->StopMovement();
	Controller->SetFocus(ThreatActor);

	if (Data.PeekStartTime < 0.0f)
	{
		Data.PeekStartTime = Time;
	}

	const bool bHasLOS = Controller->HasLineOfSightToActor(ThreatActor);

	if (bHasLOS && !Data.bHasSeenTargetDuringPeek)
	{
		Data.bHasSeenTargetDuringPeek = true;
		Data.PeekStartTime = Time;
	}

	const float PeekElapsed = Time - Data.PeekStartTime;

	bool bShouldFire = false;

	if (bHasLOS)
	{
		const float CycleDuration = EffectiveBurstDuration + EffectiveBurstPause;

		if (CycleDuration <= KINDA_SMALL_NUMBER)
		{
			bShouldFire = true;
		}
		else
		{
			const float CycleTime = FMath::Fmod(PeekElapsed, CycleDuration);
			bShouldFire = CycleTime <= EffectiveBurstDuration;
		}
	}

	Controller->SetFireEnabled(bShouldFire);

	if (PeekElapsed >= EffectivePeekDuration)
	{
		Controller->SetFireEnabled(false);
		Controller->SetCoverCombatPhase(EShooterCoverCombatPhase::ReturnToCover);

		AIChar->RefreshAIState();

		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FShooterSTTask_PeekFromCover::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult&) const
{
	AShooterAIController* Controller = GetController(Context);

	UE_LOG(LogTemp, Warning, TEXT("[Peek][Exit] Pawn=%s Cover=%s"),
		*GetNameSafe(Controller ? Controller->GetPawn() : nullptr),
		*GetNameSafe(Controller ? Controller->GetCurrentCoverPoint() : nullptr));

	if (Controller)
	{
		Controller->SetFireEnabled(false);
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}