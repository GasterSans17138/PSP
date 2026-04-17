#include "ShooterSTTask_Push.h"
#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "ShooterSquadComponent.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Pawn.h"

AShooterAIController* FShooterSTTask_Push::GetController(FStateTreeExecutionContext& Context) const
{
	return Cast<AShooterAIController>(Context.GetOwner());
}

AShooterAICharacter* FShooterSTTask_Push::GetAICharacter(FStateTreeExecutionContext& Context) const
{
	if (AShooterAIController* C = GetController(Context))
	{
		return Cast<AShooterAICharacter>(C->GetPawn());
	}
	return nullptr;
}

EStateTreeRunStatus FShooterSTTask_Push::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult&) const
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

	if (AActor* Target = C->GetCombatTarget())
	{
		Data.LastSeenTargetLocation = Target->GetActorLocation();
		Data.LastTargetLocation = Target->GetActorLocation();
		Data.bHasLastSeenTargetLocation = true;
	}

	C->SetFireEnabled(false);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FShooterSTTask_Push::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	AShooterAIController* C = GetController(Context);
	AShooterAICharacter* AIChar = GetAICharacter(Context);
	if (!IsValid(C) || !IsValid(AIChar))
	{
		return EStateTreeRunStatus::Failed;
	}

	// Reacquire target if needed
	if (Data.bAcquireTargetIfMissing && !IsValid(C->GetCombatTarget()))
	{
		C->AcquirePlayerTarget();
	}

	// Refresh AI + squad
	AIChar->RefreshAIState();
	AIChar->RefreshSquadOrder();

	if (!AIChar->bOrderIsPush)
	{
		C->SetFireEnabled(false);
		return EStateTreeRunStatus::Succeeded;
	}
	
	if (UShooterSquadComponent* SquadComp = C->GetControlledSquadComponent())
	{
		SquadComp->BroadcastTarget(C->GetCombatTarget());
		AIChar->RefreshSquadOrder();
	}

	// Get target
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

	// -----------------------------
	//  LOCKED MOVE LOCATION FIX
	// -----------------------------
	const bool bTargetMovedEnough =
		FVector::DistSquared(TargetLoc, Data.LastTargetLocation) >
		FMath::Square(Data.RecomputeIfTargetMovedDistance);

	if (bTargetMovedEnough)
	{
		Data.LockedMoveLocation = AIChar->GetCachedOrder().MoveLocation;
		Data.LastTargetLocation = TargetLoc;
	}

	// -----------------------------
	//  LOS + MEMORY
	// -----------------------------
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

	// -----------------------------
	//  LOST SIGHT → PUSH LAST SEEN
	// -----------------------------
	const bool bShouldPushLastSeen =
		!bHasLOS &&
		Data.bHasLastSeenTargetLocation &&
		Data.TimeSinceLostSight >= Data.LoseSightRepositionDelay;

	if (bShouldPushLastSeen)
	{
		if (Time - Data.LastMoveRequestTime >= Data.RepathInterval)
		{
			if (C->MoveToTacticalLocation(Data.LastSeenTargetLocation, 100.f, true))
			{
				Data.LastMoveRequestTime = Time;
			}
		}

		C->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	// -----------------------------
	//  NORMAL PUSH (STABLE)
	// -----------------------------
	const float DistanceToTarget = FVector::Dist(SelfLoc, TargetLoc);

	// Stop push if too close
	const bool bTooCloseToPush = DistanceToTarget <= Data.StopPushDistance;

	if (bTooCloseToPush)
	{
		C->StopMovement();
	}
	else
	{
		const FVector PushMoveLocation = Data.LockedMoveLocation;

		if (!PushMoveLocation.IsNearlyZero())
		{
			if (Time - Data.LastMoveRequestTime >= Data.RepathInterval)
			{
				if (C->MoveToTacticalLocation(PushMoveLocation, Data.MoveAcceptanceRadius, true))
				{
					Data.LastMoveRequestTime = Time;
				}
			}
		}
		else
		{
			if (Time - Data.LastMoveRequestTime >= Data.RepathInterval)
			{
				if (C->MoveToCombatTarget(Data.MoveAcceptanceRadius, true))
				{
					Data.LastMoveRequestTime = Time;
				}
			}
		}
	}

	// -----------------------------
	//  FIRE LOGIC
	// -----------------------------
	const float Dist = FVector::Dist(SelfLoc, TargetLoc);
	const bool bInRange = Dist >= Data.MinFireDistance && Dist <= Data.MaxFireDistance;

	const float Elapsed = Time - Data.StateEnterTime;

	bool bFire = false;
	if (Elapsed >= Data.InitialReactionDelay && bInRange && bHasLOS)
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

void FShooterSTTask_Push::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult&) const
{
	if (AShooterAIController* C = GetController(Context))
	{
		C->SetFireEnabled(false);
		C->ClearFocus(EAIFocusPriority::Gameplay);
	}
}