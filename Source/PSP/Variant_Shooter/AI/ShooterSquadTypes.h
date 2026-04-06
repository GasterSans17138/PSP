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

USTRUCT(BlueprintType)
struct FShooterSquadOrder
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bHasTarget = false;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FVector AttackLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	EShooterSquadRole RecommendedRole = EShooterSquadRole::Assaulter;
};
