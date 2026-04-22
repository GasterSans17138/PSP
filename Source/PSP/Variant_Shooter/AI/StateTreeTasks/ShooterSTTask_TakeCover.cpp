#include "ShooterSTTask_TakeCover.h"
#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "ShooterCoverPoint.h"
#include "ShooterCoverSubsystem.h"
#include "StateTreeExecutionContext.h"
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

EStateTreeRunStatus FShooterSTTask_TakeCover::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult&) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	AShooterAIController* Controller = GetController(Context);
	if (!IsValid(Controller))
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.LastMoveRequestTime = -1000.0f;
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

	if (!IsValid(CurrentCover))
	{
		AShooterCoverPoint* BestCover = CoverSubsystem->FindBestCover(Controller->GetPawn(), ThreatActor, Data.SearchRadius);
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

	if (DistToCover > Data.MoveAcceptanceRadius)
	{
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

	// Arrived at cover
	Controller->StopMovement();
	Controller->SetFireEnabled(false);
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