#include "ShooterSTTask_CombatBySquadOrder.h"
#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "ShooterSquadTypes.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "ShooterSquadComponent.h"

AShooterAIController* FShooterSTTask_CombatBySquadOrder::GetController(FStateTreeExecutionContext& Context) const
{
	return Cast<AShooterAIController>(Context.GetOwner());
}

AShooterAICharacter* FShooterSTTask_CombatBySquadOrder::GetAICharacter(FStateTreeExecutionContext& Context) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		return Cast<AShooterAICharacter>(Controller->GetPawn());
	}

	return nullptr;
}

EStateTreeRunStatus FShooterSTTask_CombatBySquadOrder::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

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

	// Broadcast the current combat target to the squad so everyone can derive consistent orders.
	if (AICharacter->GetCachedOrder().TargetActor == nullptr)
	{
		if (UShooterSquadComponent* SquadComp = Controller->GetControlledSquadComponent())
		{
			SquadComp->BroadcastTarget(Controller->GetCombatTarget());
			AICharacter->RefreshSquadOrder();
			AICharacter->RefreshAIState();
		}
	}

	Controller->SetFireEnabled(false);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FShooterSTTask_CombatBySquadOrder::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

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

	// Refresh state + current squad order every tick.
	AICharacter->RefreshAIState();
	AICharacter->RefreshSquadOrder();

	UShooterSquadComponent* SquadComp = Controller->GetControlledSquadComponent();
	if (IsValid(SquadComp))
	{
		SquadComp->BroadcastTarget(Controller->GetCombatTarget());
		AICharacter->RefreshSquadOrder();
	}

	const FShooterSquadOrder& Order = AICharacter->GetCachedOrder();

	AActor* TargetActor = Order.TargetActor;
	if (!IsValid(TargetActor))
	{
		TargetActor = Controller->GetCombatTarget();
	}

	if (!IsValid(TargetActor))
	{
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	Controller->SetFocus(TargetActor);

	const APawn* SelfPawn = Controller->GetPawn();
	if (!IsValid(SelfPawn))
	{
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Failed;
	}

	const float DistanceToTarget = FVector::Dist(SelfPawn->GetActorLocation(), TargetActor->GetActorLocation());
	const bool bCanFire = DistanceToTarget <= InstanceData.FireDistance;

	if (bCanFire)
	{
		Controller->SetFireEnabled(true);
	}
	else
	{
		Controller->SetFireEnabled(false);
	}

	// Preferred squad move behavior.
	const bool bHasMoveLocation = !Order.MoveLocation.IsNearlyZero();
	if (bHasMoveLocation)
	{
		Controller->MoveToLocation(Order.MoveLocation, InstanceData.MoveAcceptanceRadius, true, true, false, true, nullptr, true);
	}
	else if (InstanceData.bFallbackToDirectTargetMove)
	{
		Controller->MoveToCombatTarget(InstanceData.MoveAcceptanceRadius, true);
	}

	return EStateTreeRunStatus::Running;
}

void FShooterSTTask_CombatBySquadOrder::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		Controller->SetFireEnabled(false);
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}