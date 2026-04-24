#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "ShooterSTTask_PeekFromCover.generated.h"

class AShooterAIController;
class AShooterAICharacter;
struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;

USTRUCT(BlueprintType)
struct FShooterSTTask_PeekFromCoverInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Peek", meta = (ClampMin = 0, Units = "cm"))
	float MoveAcceptanceRadius = 75.0f;

	UPROPERTY(EditAnywhere, Category = "Peek", meta = (ClampMin = 0, Units = "s"))
	float RepathInterval = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Peek", meta = (ClampMin = 0, Units = "s"))
	float PeekDuration = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Peek", meta = (ClampMin = 0, Units = "cm"))
	float FireStartDistance = 180.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float PeekStartTime = -1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasSeenTargetDuringPeek = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float StateEnterTime = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float LastMoveRequestTime = -1000.0f;
};

USTRUCT(meta = (DisplayName = "Shooter: Peek From Cover"))
struct PSP_API FShooterSTTask_PeekFromCover : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FShooterSTTask_PeekFromCoverInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	AShooterAIController* GetController(FStateTreeExecutionContext& Context) const;
	AShooterAICharacter* GetAICharacter(FStateTreeExecutionContext& Context) const;
};