#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ShooterSquadTypes.h"
#include "ShooterCoverSubsystem.generated.h"

class AShooterCoverPoint;

UCLASS()
class PSP_API UShooterCoverSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterCoverPoint(AShooterCoverPoint* CoverPoint);
	void UnregisterCoverPoint(AShooterCoverPoint* CoverPoint);

	UFUNCTION(BlueprintCallable, Category = "Cover")
	AShooterCoverPoint* FindBestCoverNearLocation(
		AActor* Requester,
		AActor* ThreatActor,
		const FVector& PreferredLocation,
		EShooterTacticalOrder BaseTacticalOrder,
		float MaxSearchDistance = 2500.0f);

	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool ReserveCover(AShooterCoverPoint* CoverPoint, AActor* Occupant);

	UFUNCTION(BlueprintCallable, Category = "Cover")
	void ReleaseCover(AShooterCoverPoint* CoverPoint, AActor* Occupant);

	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool IsCoverStillValidAgainstThreat(const AShooterCoverPoint* CoverPoint, const AActor* ThreatActor) const;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AShooterCoverPoint>> CoverPoints;

	void CleanupInvalidCoverPoints();

	bool IsCoverProtectedFromThreat(
		const AShooterCoverPoint* CoverPoint,
		const AActor* ThreatActor) const;

	bool IsTooCloseToOccupiedCover(
		const AShooterCoverPoint* CandidateCover,
		const AActor* Requester,
		float MinDistance) const;

	float ScoreCoverNearLocation(
		const AShooterCoverPoint* CoverPoint,
		const AActor* Requester,
		const AActor* ThreatActor,
		const FVector& PreferredLocation,
		EShooterTacticalOrder BaseTacticalOrder) const;

	bool HasPeekLineOfSightToThreat(
		const AShooterCoverPoint* CoverPoint,
		const AActor* ThreatActor) const;

	float GetPeekLineOfSightBonus(
		const AShooterCoverPoint* CoverPoint,
		const AActor* ThreatActor,
		EShooterTacticalOrder BaseTacticalOrder) const;
};