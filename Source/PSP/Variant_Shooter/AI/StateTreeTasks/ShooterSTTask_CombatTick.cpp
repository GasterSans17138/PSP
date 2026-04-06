#include "ShooterSTTask_CombatTick.h"
#include "ShooterAIController.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Pawn.h"

EStateTreeRunStatus UShooterSTTask_CombatTick::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);

	AShooterAIController* Controller = Cast<AShooterAIController>(Context.GetOwner());
	if (!IsValid(Controller))
	{
		return EStateTreeRunStatus::Failed;
	}

	Controller->UpdateFocusOnCombatTarget();
	Controller->MoveToCombatTarget(MoveAcceptanceRadius, true);

	const APawn* SelfPawn = Controller->GetPawn();
	const APawn* TargetPawn = Controller->GetCombatTarget();

	const bool bCanFire = IsValid(SelfPawn)
		&& IsValid(TargetPawn)
		&& FVector::Dist(SelfPawn->GetActorLocation(), TargetPawn->GetActorLocation()) <= FireDistance;

	Controller->SetFireEnabled(bCanFire);
	return EStateTreeRunStatus::Running;
}

void UShooterSTTask_CombatTick::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::ExitState(Context, Transition);

	if (AShooterAIController* Controller = Cast<AShooterAIController>(Context.GetOwner()))
	{
		Controller->SetFireEnabled(false);
	}
}
