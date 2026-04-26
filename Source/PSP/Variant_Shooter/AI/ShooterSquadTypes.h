#pragma once

#include "CoreMinimal.h"
#include "ShooterSquadTypes.generated.h"

UENUM(BlueprintType)
enum class EShooterSquadRole : uint8
{
	Assaulter  UMETA(DisplayName = "Assaulter"),
	Flanker    UMETA(DisplayName = "Flanker"),
	Suppressor UMETA(DisplayName = "Suppressor"),
	Breacher   UMETA(DisplayName = "Breacher")
};

UENUM(BlueprintType)
enum class EShooterTacticalOrder : uint8
{
	None       UMETA(DisplayName = "None"),

	// Base role orders
	Hold       UMETA(DisplayName = "Hold"),
	Push       UMETA(DisplayName = "Push"),
	Suppress   UMETA(DisplayName = "Suppress"),
	FlankLeft  UMETA(DisplayName = "Flank Left"),
	FlankRight UMETA(DisplayName = "Flank Right"),
	Regroup    UMETA(DisplayName = "Regroup"),

	// Cover combat orders
	TakeCover  UMETA(DisplayName = "Take Cover"),
	Peek       UMETA(DisplayName = "Peek"),
	ThrowGrenade UMETA(DisplayName = "Throw Grenade")
};

USTRUCT(BlueprintType)
struct FShooterSquadOrder
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bHasTarget = false;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FVector MoveLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FVector AttackLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bReachedMoveLocation = false;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bHasSquadCoverFire = false;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	EShooterSquadRole RecommendedRole = EShooterSquadRole::Assaulter;

	// Current executable order used by State Tree: TakeCover / Peek.
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	EShooterTacticalOrder TacticalOrder = EShooterTacticalOrder::None;

	// Role/placement order used to choose tactical cover and peek behavior.
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	EShooterTacticalOrder BaseTacticalOrder = EShooterTacticalOrder::None;
};