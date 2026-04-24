#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ShooterCoverSubsystem.generated.h"

class AShooterCoverPoint;

UCLASS()
class PSP_API UShooterCoverSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterCoverPoint(AShooterCoverPoint* CoverPoint);
	void UnregisterCoverPoint(AShooterCoverPoint* CoverPoint);

	UFUNCTION(BlueprintCallable, Category="Cover")
	AShooterCoverPoint* FindBestCover(AActor* Requester, AActor* ThreatActor, float MaxSearchDistance = 2500.0f);

	UFUNCTION(BlueprintCallable, Category="Cover")
	bool ReserveCover(AShooterCoverPoint* CoverPoint, AActor* Occupant);

	UFUNCTION(BlueprintCallable, Category="Cover")
	void ReleaseCover(AShooterCoverPoint* CoverPoint, AActor* Occupant);

	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool IsCoverStillValidAgainstThreat(const AShooterCoverPoint* CoverPoint, const AActor* ThreatActor) const;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AShooterCoverPoint>> CoverPoints;

	void CleanupInvalidCoverPoints();
	bool IsCoverProtectedFromThreat(const AShooterCoverPoint* CoverPoint, const AActor* ThreatActor) const;
	float ScoreCover(const AShooterCoverPoint* CoverPoint, const AActor* Requester, const AActor* ThreatActor) const;
};