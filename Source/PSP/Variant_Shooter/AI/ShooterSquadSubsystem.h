#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ShooterSquadTypes.h"
#include "ShooterSquadSubsystem.generated.h"

class UShooterSquadComponent;

USTRUCT()
struct FShooterSquadRuntime
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UShooterSquadComponent>> Members;

	UPROPERTY()
	TObjectPtr<AActor> SharedTarget = nullptr;
};

/**
 * World subsystem storing squad membership and shared target data.
 * Shooter AI State Trees can pull orders from it every tick.
 */
UCLASS()
class PSP_API UShooterSquadSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	/** Registers an AI unit in a squad. */
	void RegisterMember(FName SquadId, UShooterSquadComponent* Member);

	/** Unregisters an AI unit from a squad. */
	void UnregisterMember(FName SquadId, UShooterSquadComponent* Member);

	/** Updates the shared squad target. */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	void SetSquadTarget(FName SquadId, AActor* NewTarget);

	/** Builds an order for a unit based on the shared target and role. */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	FShooterSquadOrder BuildOrder(FName SquadId, const UShooterSquadComponent* Requester) const;

private:

	UPROPERTY(Transient)
	TMap<FName, FShooterSquadRuntime> Squads;

	void CleanupNullMembers(FShooterSquadRuntime& Squad) const;

	EShooterTacticalOrder ComputeTacticalOrder(
		const TArray<TObjectPtr<UShooterSquadComponent>>& Members,
		const UShooterSquadComponent* Requester) const;

	FVector ComputeMoveLocation(
		const UShooterSquadComponent* Requester,
		const AActor* TargetActor,
		EShooterTacticalOrder TacticalOrder) const;
};
