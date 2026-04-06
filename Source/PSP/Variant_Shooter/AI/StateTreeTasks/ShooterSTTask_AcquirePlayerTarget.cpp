#include "ShooterSTTask_AcquirePlayerTarget.h"
#include "ShooterAIController.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus UShooterSTTask_AcquirePlayerTarget::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);

	if (AShooterAIController* Controller = Cast<AShooterAIController>(Context.GetOwner()))
	{
		Controller->AcquirePlayerTarget();
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Failed;
}
