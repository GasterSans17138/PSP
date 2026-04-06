#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBlueprintBase.h"
#include "ShooterSTTask_CombatTick.generated.h"

struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;

/**
 * C++ State Tree task: moves toward target and fires when in range.
 */
UCLASS(DisplayName = "Shooter: Combat Tick")
class PSP_API UShooterSTTask_CombatTick : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

protected:

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float MoveAcceptanceRadius = 550.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float FireDistance = 1200.0f;
};
