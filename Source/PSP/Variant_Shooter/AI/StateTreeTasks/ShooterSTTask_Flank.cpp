#include "ShooterSTTask_Flank.h"
#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "ShooterSquadComponent.h"
#include "ShooterSquadTypes.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Pawn.h"

AShooterAIController* FShooterSTTask_Flank::GetController(FStateTreeExecutionContext& Context) const
{
	return Cast<AShooterAIController>(Context.GetOwner());
}

AShooterAICharacter* FShooterSTTask_Flank::GetAICharacter(FStateTreeExecutionContext& Context) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		return Cast<AShooterAICharacter>(Controller->GetPawn());
	}
	return nullptr;
}

EStateTreeRunStatus FShooterSTTask_Flank::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AShooterAIController* Controller = GetController(Context);
	AShooterAICharacter* AICharacter = GetAICharacter(Context);
	if (!IsValid(Controller) || !IsValid(AICharacter))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.bAcquireTargetIfMissing && !IsValid(Controller->GetCombatTarget()))
	{
		Controller->AcquirePlayerTarget();
	}

	AICharacter->RefreshAIState();
	AICharacter->RefreshSquadOrder();

	if (UShooterSquadComponent* SquadComp = Controller->GetControlledSquadComponent())
	{
		SquadComp->BroadcastTarget(Controller->GetCombatTarget());
		AICharacter->RefreshSquadOrder();
	}

	InstanceData.StateEnterTime = Controller->GetWorld()->GetTimeSeconds();
	InstanceData.LastMoveRequestTime = -1000.0f;
	InstanceData.LockedMoveLocation = AICharacter->GetCachedOrder().MoveLocation;

	if (AActor* Target = AICharacter->GetCachedOrder().TargetActor)
	{
		InstanceData.LastTargetLocation = Target->GetActorLocation();
	}

	Controller->SetFireEnabled(false);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FShooterSTTask_Flank::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AShooterAIController* Controller = GetController(Context);
	AShooterAICharacter* AICharacter = GetAICharacter(Context);
	if (!IsValid(Controller) || !IsValid(AICharacter))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.bAcquireTargetIfMissing && !IsValid(Controller->GetCombatTarget()))
	{
		Controller->AcquirePlayerTarget();
	}

	AICharacter->RefreshAIState();
	AICharacter->RefreshSquadOrder();

	if (UShooterSquadComponent* SquadComp = Controller->GetControlledSquadComponent())
	{
		SquadComp->BroadcastTarget(Controller->GetCombatTarget());
		AICharacter->RefreshSquadOrder();
	}

	AActor* TargetActor = AICharacter->GetCachedOrder().TargetActor;
	if (!IsValid(TargetActor))
	{
		TargetActor = Controller->GetCombatTarget();
	}
	if (!IsValid(TargetActor))
	{
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	const APawn* SelfPawn = Controller->GetPawn();
	if (!IsValid(SelfPawn))
	{
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Failed;
	}

	Controller->SetFocus(TargetActor);

	const float TimeSeconds = Controller->GetWorld()->GetTimeSeconds();
	const FVector CurrentTargetLocation = TargetActor->GetActorLocation();

	const bool bTargetMovedEnough =
		FVector::DistSquared(CurrentTargetLocation, InstanceData.LastTargetLocation) >
		FMath::Square(InstanceData.RecomputeIfTargetMovedDistance);

	if (bTargetMovedEnough)
	{
		InstanceData.LockedMoveLocation = AICharacter->GetCachedOrder().MoveLocation;
		InstanceData.LastTargetLocation = CurrentTargetLocation;
	}

	const float DistanceToMoveLocation = FVector::Dist(SelfPawn->GetActorLocation(), InstanceData.LockedMoveLocation);
	bool bReachedFlankPoint =
		InstanceData.LockedMoveLocation.IsNearlyZero() ||
		(DistanceToMoveLocation <= InstanceData.MoveAcceptanceRadius);

	if (!bReachedFlankPoint &&
		(TimeSeconds - InstanceData.LastMoveRequestTime) >= InstanceData.RepathInterval)
	{
		if (Controller->MoveToTacticalLocation(InstanceData.LockedMoveLocation, InstanceData.MoveAcceptanceRadius, true))
		{
			InstanceData.LastMoveRequestTime = TimeSeconds;
		}
	}

	const float DistanceToTarget = FVector::Dist(SelfPawn->GetActorLocation(), CurrentTargetLocation);
	const bool bInFireRange =
		DistanceToTarget >= InstanceData.MinFireDistance &&
		DistanceToTarget <= InstanceData.MaxFireDistance;

	bool bHasLOS = true;
	if (InstanceData.bRequireLineOfSightToFire)
	{
		bHasLOS = Controller->HasLineOfSightToActor(TargetActor);
	}

	const float ElapsedSinceEnter = TimeSeconds - InstanceData.StateEnterTime;

	bReachedFlankPoint = true; // FIX TEMP
	
	bool bShouldFire = false;
	if (bReachedFlankPoint && ElapsedSinceEnter >= InstanceData.InitialReactionDelay && bInFireRange && bHasLOS)
	{
		const float PhaseTime = ElapsedSinceEnter - InstanceData.InitialReactionDelay;
		const float CycleDuration = InstanceData.BurstDuration + InstanceData.PauseBetweenBursts;

		if (CycleDuration > KINDA_SMALL_NUMBER)
		{
			const float CycleTime = FMath::Fmod(PhaseTime, CycleDuration);
			bShouldFire = (CycleTime <= InstanceData.BurstDuration);
		}
	}

	Controller->SetFireEnabled(bShouldFire);
	return EStateTreeRunStatus::Running;
}

void FShooterSTTask_Flank::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		Controller->SetFireEnabled(false);
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}