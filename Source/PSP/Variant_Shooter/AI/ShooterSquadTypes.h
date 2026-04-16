#pragma once

#include "CoreMinimal.h"
#include "ShooterSquadTypes.generated.h"

UENUM(BlueprintType)
enum class EShooterSquadRole : uint8
{
	Assaulter UMETA(DisplayName = "Assaulter"),
	Flanker UMETA(DisplayName = "Flanker"),
	Suppressor UMETA(DisplayName = "Suppressor"),
	Breacher UMETA(DisplayName = "Breacher")
};

UENUM(BlueprintType)
enum class EShooterTacticalOrder : uint8
{
	None UMETA(DisplayName = "None"),
	Hold UMETA(DisplayName = "Hold"),
	Push UMETA(DisplayName = "Push"),
	Suppress UMETA(DisplayName = "Suppress"),
	FlankLeft UMETA(DisplayName = "Flank Left"),
	FlankRight UMETA(DisplayName = "Flank Right"),
	Regroup UMETA(DisplayName = "Regroup")
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
	EShooterSquadRole RecommendedRole = EShooterSquadRole::Assaulter;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	EShooterTacticalOrder TacticalOrder = EShooterTacticalOrder::None;
};
