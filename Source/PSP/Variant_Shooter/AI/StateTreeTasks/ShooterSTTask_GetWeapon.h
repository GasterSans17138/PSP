#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "ShooterSTTask_GetWeapon.generated.h"

class AShooterAIController;
struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;

USTRUCT(BlueprintType)
struct FShooterSTTask_GetWeaponInstanceData
{
	GENERATED_BODY()

	/** Radius used to search for a weapon pickup around the AI pawn. */
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (ClampMin = 0, Units = "cm"))
	float SearchRadius = 30000.0f;

	/** Acceptance radius used while moving toward the pickup. */
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (ClampMin = 0, Units = "cm"))
	float MoveAcceptanceRadius = 100.0f;

	/** If true, re-searches a pickup when current target becomes invalid/hidden. */
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bRetrySearchIfTargetInvalid = true;
};

/**
 * Native C++ StateTree task:
 * If unarmed, find a weapon pickup and move to it until overlap grants a weapon.
 */
USTRUCT(meta = (DisplayName = "Shooter: Get Weapon"))
struct PSP_API FShooterSTTask_GetWeapon : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FShooterSTTask_GetWeaponInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:

	AShooterAIController* GetController(FStateTreeExecutionContext& Context) const;
};