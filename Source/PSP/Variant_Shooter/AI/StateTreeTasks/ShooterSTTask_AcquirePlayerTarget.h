#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBlueprintBase.h"
#include "ShooterSTTask_AcquirePlayerTarget.generated.h"

struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;

/**
 * C++ State Tree task: acquires player target once on state enter.
 */
UCLASS(DisplayName = "Shooter: Acquire Player Target")
class PSP_API UShooterSTTask_AcquirePlayerTarget : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
};
