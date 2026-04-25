#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "ShooterSTTask_TakeCover.generated.h"

class AShooterAIController;
class AShooterAICharacter;
class AShooterCoverPoint;
struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;

USTRUCT(BlueprintType)
struct FShooterSTTask_TakeCoverInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Cover", meta = (ClampMin = 0, Units = "cm"))
	float SearchRadius = 2500.0f;

	UPROPERTY(EditAnywhere, Category = "Cover", meta = (ClampMin = 0, Units = "cm"))
	float MoveAcceptanceRadius = 75.0f;

	UPROPERTY(EditAnywhere, Category = "Cover", meta = (ClampMin = 0, Units = "cm"))
	float CoverValidationDistance = 180.0f;

	UPROPERTY(EditAnywhere, Category = "Cover", meta = (ClampMin = 0, Units = "s"))
	float RepathInterval = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Cover", meta = (ClampMin = 0, Units = "s"))
	float CoverHoldDuration = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Cover", meta = (ClampMin = 0, Units = "cm"))
	float ReevaluateCoverIfTargetMovedDistance = 600.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FVector LastCoverEvaluationTargetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasCoverEvaluationTargetLocation = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float LastMoveRequestTime = -1000.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float CoverReachedTime = -1.0f;
};

USTRUCT(meta=(DisplayName="Shooter: Take Cover"))
struct PSP_API FShooterSTTask_TakeCover : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FShooterSTTask_TakeCoverInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	AShooterAIController* GetController(FStateTreeExecutionContext& Context) const;
	AShooterAICharacter* GetAICharacter(FStateTreeExecutionContext& Context) const;
};