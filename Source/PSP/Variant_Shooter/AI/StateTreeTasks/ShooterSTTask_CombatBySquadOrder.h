#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "ShooterSTTask_CombatBySquadOrder.generated.h"

class AShooterAIController;
class AShooterAICharacter;
struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;

USTRUCT(BlueprintType)
struct FShooterSTTask_CombatBySquadOrderInstanceData
{
	GENERATED_BODY()

	/** Distance at which the AI starts firing. */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float FireDistance = 1400.0f;

	/** Acceptance radius when moving to squad move location. */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float MoveAcceptanceRadius = 150.0f;

	/** If true, reacquires player target when missing. */
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAcquireTargetIfMissing = true;

	/** If true, will fallback to direct combat target move when squad order has no valid move location. */
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bFallbackToDirectTargetMove = true;
};

/**
 * Native C++ StateTree combat task driven by squad order.
 * Moves toward CachedOrder.MoveLocation, keeps focus on target, and fires when in range.
 */
USTRUCT(meta = (DisplayName = "Shooter: Combat By Squad Order"))
struct PSP_API FShooterSTTask_CombatBySquadOrder : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FShooterSTTask_CombatBySquadOrderInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:

	AShooterAIController* GetController(FStateTreeExecutionContext& Context) const;
	AShooterAICharacter* GetAICharacter(FStateTreeExecutionContext& Context) const;
};