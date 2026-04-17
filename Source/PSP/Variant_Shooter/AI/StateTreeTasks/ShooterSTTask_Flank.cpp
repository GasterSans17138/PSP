#include "ShooterSTTask_Flank.h"
#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "ShooterSquadComponent.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Pawn.h"

AShooterAIController* FShooterSTTask_Flank::GetController(FStateTreeExecutionContext& Context) const
{
	return Cast<AShooterAIController>(Context.GetOwner());
}

AShooterAICharacter* FShooterSTTask_Flank::GetAICharacter(FStateTreeExecutionContext& Context) const
{
	if (AShooterAIController* C = GetController(Context))
	{
		return Cast<AShooterAICharacter>(C->GetPawn());
	}
	return nullptr;
}

EStateTreeRunStatus FShooterSTTask_Flank::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult&) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	AShooterAIController* C = GetController(Context);
	AShooterAICharacter* AIChar = GetAICharacter(Context);
	if (!IsValid(C) || !IsValid(AIChar))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (Data.bAcquireTargetIfMissing && !IsValid(C->GetCombatTarget()))
	{
		C->AcquirePlayerTarget();
	}

	AIChar->RefreshAIState();
	AIChar->RefreshSquadOrder();

	if (UShooterSquadComponent* SquadComp = C->GetControlledSquadComponent())
	{
		SquadComp->BroadcastTarget(C->GetCombatTarget());
		AIChar->RefreshSquadOrder();
	}

	Data.StateEnterTime = C->GetWorld()->GetTimeSeconds();
	Data.LastMoveRequestTime = -1000.f;
	Data.TimeSinceLostSight = 0.f;
	Data.bHasLastSeenTargetLocation = false;
	Data.LockedMoveLocation = AIChar->GetCachedOrder().MoveLocation;

	if (AActor* Target = AIChar->GetCachedOrder().TargetActor)
	{
		Data.LastTargetLocation = Target->GetActorLocation();
		Data.LastSeenTargetLocation = Target->GetActorLocation();
		Data.bHasLastSeenTargetLocation = true;
	}

	C->SetFireEnabled(false);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FShooterSTTask_Flank::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	AShooterAIController* C = GetController(Context);
	AShooterAICharacter* AIChar = GetAICharacter(Context);
	if (!IsValid(C) || !IsValid(AIChar))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (Data.bAcquireTargetIfMissing && !IsValid(C->GetCombatTarget()))
	{
		C->AcquirePlayerTarget();
	}

	AIChar->RefreshAIState();
	AIChar->RefreshSquadOrder();

	if (!AIChar->bOrderIsFlank)
	{
		C->SetFireEnabled(false);
		return EStateTreeRunStatus::Succeeded;
	}

	if (UShooterSquadComponent* SquadComp = C->GetControlledSquadComponent())
	{
		SquadComp->BroadcastTarget(C->GetCombatTarget());
		AIChar->RefreshSquadOrder();
	}

	AActor* Target = AIChar->GetCachedOrder().TargetActor;
	if (!IsValid(Target))
	{
		Target = C->GetCombatTarget();
	}

	if (!IsValid(Target))
	{
		C->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	const APawn* SelfPawn = C->GetPawn();
	if (!IsValid(SelfPawn))
	{
		C->SetFireEnabled(false);
		return EStateTreeRunStatus::Failed;
	}

	C->SetFocus(Target);

	const float Time = C->GetWorld()->GetTimeSeconds();
	const FVector TargetLoc = Target->GetActorLocation();
	const FVector SelfLoc = SelfPawn->GetActorLocation();

	const bool bTargetMovedEnough =
		FVector::DistSquared(TargetLoc, Data.LastTargetLocation) >
		FMath::Square(Data.RecomputeIfTargetMovedDistance);

	if (bTargetMovedEnough)
	{
		Data.LockedMoveLocation = AIChar->GetCachedOrder().MoveLocation;
		Data.LastTargetLocation = TargetLoc;
	}

	bool bHasLOS = !Data.bRequireLineOfSightToFire || C->HasLineOfSightToActor(Target);

	if (bHasLOS)
	{
		Data.TimeSinceLostSight = 0.f;
		Data.LastSeenTargetLocation = TargetLoc;
		Data.bHasLastSeenTargetLocation = true;
	}
	else
	{
		Data.TimeSinceLostSight += DeltaTime;
	}

	const float DistanceToMoveLocation = FVector::Dist(SelfLoc, Data.LockedMoveLocation);
	const float ReachedTolerance = FMath::Max(Data.MoveAcceptanceRadius * 3.0f, 300.0f);

	const bool bReachedFlankPoint =
		Data.LockedMoveLocation.IsNearlyZero() ||
		(DistanceToMoveLocation <= ReachedTolerance);

	// Lost sight logic
	if (!bHasLOS && Data.bHasLastSeenTargetLocation)
	{
		if (Data.TimeSinceLostSight < Data.FlankHoldTimeBeforeReposition)
		{
			C->StopMovement();
			C->SetFireEnabled(false);
			return EStateTreeRunStatus::Running;
		}

		if (Time - Data.LastMoveRequestTime >= Data.RepathInterval)
		{
			if (C->MoveToTacticalLocation(Data.LastSeenTargetLocation, 120.f, true))
			{
				Data.LastMoveRequestTime = Time;
			}
		}

		C->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	// Normal flank movement
	if (!bReachedFlankPoint && (Time - Data.LastMoveRequestTime) >= Data.RepathInterval)
	{
		if (C->MoveToTacticalLocation(Data.LockedMoveLocation, Data.MoveAcceptanceRadius, true))
		{
			Data.LastMoveRequestTime = Time;
		}
	}
	else if (bReachedFlankPoint)
	{
		C->StopMovement();
	}

	const float Dist = FVector::Dist(SelfLoc, TargetLoc);
	const bool bInRange = Dist >= Data.MinFireDistance && Dist <= Data.MaxFireDistance;

	const float Elapsed = Time - Data.StateEnterTime;

	bool bFire = false;
	if (Elapsed >= Data.InitialReactionDelay && bInRange && bHasLOS && bReachedFlankPoint)
	{
		const float Phase = Elapsed - Data.InitialReactionDelay;
		const float Cycle = Data.BurstDuration + Data.PauseBetweenBursts;

		if (Cycle > KINDA_SMALL_NUMBER)
		{
			bFire = FMath::Fmod(Phase, Cycle) <= Data.BurstDuration;
		}
	}

	C->SetFireEnabled(bFire);
	return EStateTreeRunStatus::Running;
}

void FShooterSTTask_Flank::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult&) const
{
	if (AShooterAIController* C = GetController(Context))
	{
		C->SetFireEnabled(false);
		C->ClearFocus(EAIFocusPriority::Gameplay);
	}
}