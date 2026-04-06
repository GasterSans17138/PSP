#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBlueprintBase.h"
#include "ShooterSTTask_CombatTick.generated.h"

struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;

/**
 * Single C++ State Tree combat task.
 * - Enter: optionally acquires player target and resets fire.
 * - Tick: updates focus, moves and toggles firing by distance.
 * - Exit: always stops firing.
 */
UCLASS(DisplayName = "Shooter: Combat Loop")
class PSP_API UShooterSTTask_CombatTick : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

protected:

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAcquireTargetOnEnter = true;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float MoveAcceptanceRadius = 550.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float FireDistance = 1200.0f;
};
