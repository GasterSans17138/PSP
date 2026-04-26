#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "ShooterSTTask_ThrowGrenade.generated.h"

class AShooterAIController;
class AShooterAICharacter;

USTRUCT(BlueprintType)
struct FShooterSTTask_ThrowGrenadeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Grenade")
	TSubclassOf<AActor> GrenadeClass;

	UPROPERTY(EditAnywhere, Category = "Grenade", meta = (ClampMin = 0, Units = "cm"))
	float SpawnForwardOffset = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Grenade", meta = (ClampMin = 0, Units = "cm"))
	float SpawnUpOffset = 120.0f;

	UPROPERTY(EditAnywhere, Category = "Grenade", meta = (ClampMin = 0, Units = "cm"))
	float TargetShortOffset = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Grenade", meta = (ClampMin = 0, Units = "cm/s"))
	float ThrowSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Grenade", meta = (ClampMin = 0, Units = "cm/s"))
	float MaxThrowSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, Category = "Grenade", meta = (ClampMin = 0, Units = "cm/s"))
	float ThrowUpBoost = 900.0f;

	UPROPERTY(EditAnywhere, Category = "Grenade", meta = (ClampMin = 0, Units = "s"))
	float ThrowDelay = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Grenade|Safety")
	bool bCheckThrowPathBeforeThrow = true;

	UPROPERTY(EditAnywhere, Category = "Grenade|Safety", meta = (ClampMin = 0.1f, Units = "s"))
	float ThrowPathCheckTime = 0.65f;

	UPROPERTY(EditAnywhere, Category = "Grenade|Safety", meta = (ClampMin = 1.0f, Units = "cm"))
	float ThrowPathProjectileRadius = 12.0f;

	UPROPERTY(EditAnywhere, Category = "Grenade|Safety", meta = (ClampMin = 1.0f))
	float ThrowPathSimFrequency = 15.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float StateEnterTime = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bGrenadeThrown = false;
};

USTRUCT(meta = (DisplayName = "Shooter: Throw Grenade"))
struct PSP_API FShooterSTTask_ThrowGrenade : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FShooterSTTask_ThrowGrenadeInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	AShooterAIController* GetController(FStateTreeExecutionContext& Context) const;
	AShooterAICharacter* GetAICharacter(FStateTreeExecutionContext& Context) const;

	bool SpawnGrenade(AShooterAIController* Controller, AShooterAICharacter* AIChar, const FInstanceDataType& Data) const;
};