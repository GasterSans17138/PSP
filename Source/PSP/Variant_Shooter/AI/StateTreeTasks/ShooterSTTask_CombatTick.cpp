#include "ShooterSTTask_CombatTick.h"
#include "ShooterAIController.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Pawn.h"

AShooterAIController* FShooterSTTask_CombatTick::GetController(FStateTreeExecutionContext& Context) const
{
	return Cast<AShooterAIController>(Context.GetOwner());
}

EStateTreeRunStatus FShooterSTTask_CombatTick::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AShooterAIController* Controller = GetController(Context);
	if (!IsValid(Controller))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.bAcquireTargetOnEnter)
	{
		Controller->AcquirePlayerTarget();
	}

	Controller->SetFireEnabled(false);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FShooterSTTask_CombatTick::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AShooterAIController* Controller = GetController(Context);
	if (!IsValid(Controller))
	{
		return EStateTreeRunStatus::Failed;
	}

	Controller->UpdateFocusOnCombatTarget();
	Controller->MoveToCombatTarget(InstanceData.MoveAcceptanceRadius, true);

	const APawn* SelfPawn = Controller->GetPawn();
	const APawn* TargetPawn = Controller->GetCombatTarget();

	const bool bCanFire = IsValid(SelfPawn)
		&& IsValid(TargetPawn)
		&& FVector::Dist(SelfPawn->GetActorLocation(), TargetPawn->GetActorLocation()) <= InstanceData.FireDistance;

	Controller->SetFireEnabled(bCanFire);
	return EStateTreeRunStatus::Running;
}

void FShooterSTTask_CombatTick::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		Controller->SetFireEnabled(false);
	}
}