#include "ShooterSTTask_Hold.h"
#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "ShooterSquadComponent.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Pawn.h"

AShooterAIController* FShooterSTTask_Hold::GetController(FStateTreeExecutionContext& Context) const
{
	return Cast<AShooterAIController>(Context.GetOwner());
}

AShooterAICharacter* FShooterSTTask_Hold::GetAICharacter(FStateTreeExecutionContext& Context) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		return Cast<AShooterAICharacter>(Controller->GetPawn());
	}
	return nullptr;
}

EStateTreeRunStatus FShooterSTTask_Hold::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	AShooterAIController* Controller = GetController(Context);
	AShooterAICharacter* AICharacter = GetAICharacter(Context);
	if (!IsValid(Controller) || !IsValid(AICharacter))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		InstanceData.bAcquireTargetIfMissing && !IsValid(Controller->GetCombatTarget()))
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

	Controller->SetFireEnabled(false);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FShooterSTTask_Hold::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
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

	if (UShooterSquadComponent* SquadComp = Controller->GetControlledSquadComponent())
	{
		SquadComp->BroadcastTarget(Controller->GetCombatTarget());
		AICharacter->RefreshSquadOrder();
	}

	AActor* TargetActor = Controller->GetCombatTarget();
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

	const float DistanceToTarget = FVector::Dist(SelfPawn->GetActorLocation(), TargetActor->GetActorLocation());
	const bool bInFireRange =
		DistanceToTarget >= InstanceData.MinFireDistance &&
		DistanceToTarget <= InstanceData.MaxFireDistance;

	const float TimeSeconds = Controller->GetWorld()->GetTimeSeconds();
	const float PhaseTime = TimeSeconds - InstanceData.InitialReactionDelay;

	bool bShouldFire = false;
	if (TimeSeconds >= InstanceData.InitialReactionDelay && bInFireRange)
	{
		const float CycleDuration = InstanceData.BurstDuration + InstanceData.PauseBetweenBursts;
		if (CycleDuration > KINDA_SMALL_NUMBER)
		{
			const float CycleTime = FMath::Fmod(PhaseTime, CycleDuration);
			bShouldFire = (CycleTime <= InstanceData.BurstDuration);
		}
	}

	if (DistanceToTarget > InstanceData.MaxHoldDistanceFromTarget)
	{
		Controller->MoveToCombatTarget(InstanceData.RepositionAcceptanceRadius, true);
	}

	Controller->SetFireEnabled(bShouldFire);
	return EStateTreeRunStatus::Running;
}

void FShooterSTTask_Hold::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		Controller->SetFireEnabled(false);
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}