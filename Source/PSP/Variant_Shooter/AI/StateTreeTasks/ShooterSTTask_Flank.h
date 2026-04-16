#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "ShooterSTTask_Flank.generated.h"

class AShooterAIController;
class AShooterAICharacter;
struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;

USTRUCT(BlueprintType)
struct FShooterSTTask_FlankInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float MinFireDistance = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float MaxFireDistance = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float MoveAcceptanceRadius = 125.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "s"))
	float InitialReactionDelay = 0.45f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "s"))
	float BurstDuration = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "s"))
	float PauseBetweenBursts = 0.45f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAcquireTargetIfMissing = true;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float RecomputeIfTargetMovedDistance = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "s"))
	float RepathInterval = 0.75f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bRequireLineOfSightToFire = true;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FVector LockedMoveLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FVector LastTargetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float StateEnterTime = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float LastMoveRequestTime = -1000.0f;
};

USTRUCT(meta = (DisplayName = "Shooter: Flank"))
struct PSP_API FShooterSTTask_Flank : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FShooterSTTask_FlankInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	AShooterAIController* GetController(FStateTreeExecutionContext& Context) const;
	AShooterAICharacter* GetAICharacter(FStateTreeExecutionContext& Context) const;
};