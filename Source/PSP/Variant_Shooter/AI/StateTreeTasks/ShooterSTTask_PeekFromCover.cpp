#include "ShooterSTTask_PeekFromCover.h"
#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "ShooterCoverPoint.h"
#include "StateTreeExecutionContext.h"

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
	AShooterAICharacter* AIChar = GetAICharacter(Context);

	UE_LOG(LogTemp, Warning, TEXT("[Peek][Enter] Pawn=%s Controller=%s AIChar=%s Cover=%s"),
		*GetNameSafe(Controller ? Controller->GetPawn() : nullptr),
		*GetNameSafe(Controller),
		*GetNameSafe(AIChar),
		*GetNameSafe(Controller ? Controller->GetCurrentCoverPoint() : nullptr));

	if (!IsValid(Controller) || !IsValid(Controller->GetCurrentCoverPoint()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Peek][Enter] FAILED - invalid controller or no current cover"));
		return EStateTreeRunStatus::Failed;
	}

	Data.StateEnterTime = Controller->GetWorld()->GetTimeSeconds();
	Data.LastMoveRequestTime = -1000.0f;
	Controller->SetFireEnabled(false);

	UE_LOG(LogTemp, Warning, TEXT("[Peek][Enter] OK - StateEnterTime=%.3f"),
		Data.StateEnterTime);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FShooterSTTask_PeekFromCover::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	AShooterAIController* Controller = GetController(Context);
	AShooterAICharacter* AIChar = GetAICharacter(Context);
	if (!IsValid(Controller) || !IsValid(AIChar))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Peek][Tick] FAILED - invalid controller or AIChar"));
		return EStateTreeRunStatus::Failed;
	}

	AIChar->RefreshAIState();

	UE_LOG(LogTemp, Warning, TEXT("[Peek][Tick] Pawn=%s Order=%d bOrderIsPeek=%d bOrderIsTakeCover=%d CurrentCover=%s"),
		*GetNameSafe(Controller->GetPawn()),
		(int32)AIChar->CurrentTacticalOrder,
		AIChar->bOrderIsPeek ? 1 : 0,
		AIChar->bOrderIsTakeCover ? 1 : 0,
		*GetNameSafe(Controller->GetCurrentCoverPoint()));

	if (!AIChar->bOrderIsPeek)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Peek][Tick] EXIT -> Succeeded because bOrderIsPeek is false"));
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Succeeded;
	}

	AShooterCoverPoint* CoverPoint = Controller->GetCurrentCoverPoint();
	if (!IsValid(CoverPoint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Peek][Tick] FAILED - no current cover point"));
		return EStateTreeRunStatus::Failed;
	}

	AActor* ThreatActor = AIChar->GetCachedOrder().TargetActor;
	if (!IsValid(ThreatActor))
	{
		ThreatActor = Controller->GetCombatTarget();
	}

	UE_LOG(LogTemp, Warning, TEXT("[Peek][Tick] ThreatActor=%s"),
		*GetNameSafe(ThreatActor));

	if (!IsValid(ThreatActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Peek][Tick] RUNNING - no threat actor"));
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	const float Time = Controller->GetWorld()->GetTimeSeconds();
	const FVector SelfLocation = Controller->GetPawn()->GetActorLocation();
	const FVector PeekLocation = CoverPoint->GetPeekLocation();
	const float DistToPeek = FVector::Dist(SelfLocation, PeekLocation);
	const float Elapsed = Time - Data.StateEnterTime;

	UE_LOG(LogTemp, Warning, TEXT("[Peek][Tick] Time=%.3f Enter=%.3f Elapsed=%.3f DistToPeek=%.2f PeekLocation=%s"),
		Time,
		Data.StateEnterTime,
		Elapsed,
		DistToPeek,
		*PeekLocation.ToString());

	if (DistToPeek > Data.MoveAcceptanceRadius)
	{
		if ((Time - Data.LastMoveRequestTime) >= Data.RepathInterval)
		{
			const bool bMoveStarted = Controller->MoveToPeekLocation(Data.MoveAcceptanceRadius, true);

			UE_LOG(LogTemp, Warning, TEXT("[Peek][Tick] MoveToPeekLocation=%d LastMoveRequestTime(before)=%.3f"),
				bMoveStarted ? 1 : 0,
				Data.LastMoveRequestTime);

			if (bMoveStarted)
			{
				Data.LastMoveRequestTime = Time;
			}
		}

		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	Controller->StopMovement();
	Controller->SetFocus(ThreatActor);

	const bool bHasLOS = Controller->HasLineOfSightToActor(ThreatActor);
	Controller->SetFireEnabled(bHasLOS);

	UE_LOG(LogTemp, Warning, TEXT("[Peek][Tick] Reached peek spot - bHasLOS=%d Fire=%d"),
		bHasLOS ? 1 : 0,
		bHasLOS ? 1 : 0);

	if (Elapsed >= Data.PeekDuration)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Peek][Tick] EXIT -> Succeeded because PeekDuration reached (Elapsed=%.3f / Duration=%.3f)"),
			Elapsed,
			Data.PeekDuration);

		Controller->SetFireEnabled(false);
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