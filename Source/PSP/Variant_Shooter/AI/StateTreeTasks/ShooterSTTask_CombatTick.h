#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "ShooterSTTask_CombatTick.generated.h"

class AShooterAIController;
struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;

USTRUCT(BlueprintType)
struct FShooterSTTask_CombatTickInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAcquireTargetOnEnter = true;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float MoveAcceptanceRadius = 550.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float FireDistance = 1200.0f;
};

/**
 * Native C++ StateTree combat task (Enter + Tick + Exit).
 */
USTRUCT(meta = (DisplayName = "Shooter: Combat Loop"))
struct PSP_API FShooterSTTask_CombatTick : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FShooterSTTask_CombatTickInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:

	AShooterAIController* GetController(FStateTreeExecutionContext& Context) const;
};