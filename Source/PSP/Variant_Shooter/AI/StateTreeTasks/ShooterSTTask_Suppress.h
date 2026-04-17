#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "ShooterSTTask_Suppress.generated.h"

class AShooterAIController;
class AShooterAICharacter;
struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;

USTRUCT(BlueprintType)
struct FShooterSTTask_SuppressInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float MinFireDistance = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float MaxFireDistance = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float MoveAcceptanceRadius = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "s"))
	float InitialReactionDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "s"))
	float BurstDuration = 1.2f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "s"))
	float PauseBetweenBursts = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAcquireTargetIfMissing = true;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bRequireLineOfSightToFire = true;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float RecomputeIfTargetMovedDistance = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "s"))
	float RepathInterval = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "s"))
	float LoseSightRepositionDelay = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = 0, Units = "cm"))
	float LastSeenAcceptanceRadius = 120.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FVector LockedMoveLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FVector LastTargetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FVector LastSeenTargetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasLastSeenTargetLocation = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float TimeSinceLostSight = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float StateEnterTime = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float LastMoveRequestTime = -1000.0f;
};

USTRUCT(meta = (DisplayName = "Shooter: Suppress"))
struct PSP_API FShooterSTTask_Suppress : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FShooterSTTask_SuppressInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	AShooterAIController* GetController(FStateTreeExecutionContext& Context) const;
	AShooterAICharacter* GetAICharacter(FStateTreeExecutionContext& Context) const;
};