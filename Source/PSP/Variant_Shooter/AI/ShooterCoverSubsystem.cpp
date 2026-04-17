#include "ShooterCoverSubsystem.h"
#include "ShooterCoverPoint.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

void UShooterCoverSubsystem::RegisterCoverPoint(AShooterCoverPoint* CoverPoint)
{
	if (!IsValid(CoverPoint))
	{
		return;
	}

	CoverPoints.RemoveAll([](const TObjectPtr<AShooterCoverPoint>& Point)
	{
		return !IsValid(Point);
	});

	if (!CoverPoints.Contains(CoverPoint))
	{
		CoverPoints.Add(CoverPoint);
	}
}

void UShooterCoverSubsystem::UnregisterCoverPoint(AShooterCoverPoint* CoverPoint)
{
	if (!IsValid(CoverPoint))
	{
		return;
	}

	CoverPoints.Remove(CoverPoint);
}

AShooterCoverPoint* UShooterCoverSubsystem::FindBestCover(AActor* Requester, AActor* ThreatActor, float MaxSearchDistance)
{
	if (!IsValid(Requester) || !IsValid(ThreatActor))
	{
		return nullptr;
	}

	CoverPoints.RemoveAll([](const TObjectPtr<AShooterCoverPoint>& Point)
	{
		return !IsValid(Point);
	});

	AShooterCoverPoint* BestCover = nullptr;
	float BestScore = -FLT_MAX;

	for (AShooterCoverPoint* CoverPoint : CoverPoints)
	{
		if (!IsValid(CoverPoint) || !CoverPoint->CanBeUsedBy(Requester))
		{
			continue;
		}

		const float DistToRequester = FVector::Dist(Requester->GetActorLocation(), CoverPoint->GetCoverLocation());
		if (DistToRequester > MaxSearchDistance)
		{
			continue;
		}

		if (!IsCoverProtectedFromThreat(CoverPoint, ThreatActor))
		{
			continue;
		}

		const float Score = ScoreCover(CoverPoint, Requester, ThreatActor);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestCover = CoverPoint;
		}
	}

	return BestCover;
}

bool UShooterCoverSubsystem::ReserveCover(AShooterCoverPoint* CoverPoint, AActor* Occupant)
{
	return IsValid(CoverPoint) && CoverPoint->Reserve(Occupant);
}

void UShooterCoverSubsystem::ReleaseCover(AShooterCoverPoint* CoverPoint, AActor* Occupant)
{
	if (IsValid(CoverPoint))
	{
		CoverPoint->Release(Occupant);
	}
}

void UShooterCoverSubsystem::CleanupInvalidCoverPoints()
{
	CoverPoints.RemoveAll([](const TObjectPtr<AShooterCoverPoint>& Point)
	{
		return !IsValid(Point);
	});
}

bool UShooterCoverSubsystem::IsCoverProtectedFromThreat(const AShooterCoverPoint* CoverPoint, const AActor* ThreatActor) const
{
	if (!IsValid(CoverPoint) || !IsValid(ThreatActor))
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CoverPoint);
	Params.AddIgnoredActor(ThreatActor);

	const FVector Start = ThreatActor->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
	const FVector End = CoverPoint->GetCoverLocation() + FVector(0.0f, 0.0f, 60.0f);

	const bool bBlockingHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	// Good cover if visibility is blocked before reaching the cover point.
	return bBlockingHit && Hit.GetActor() != CoverPoint;
}

float UShooterCoverSubsystem::ScoreCover(const AShooterCoverPoint* CoverPoint, const AActor* Requester, const AActor* ThreatActor) const
{
	if (!IsValid(CoverPoint) || !IsValid(Requester) || !IsValid(ThreatActor))
	{
		return -FLT_MAX;
	}

	const float DistToRequester = FVector::Dist(Requester->GetActorLocation(), CoverPoint->GetCoverLocation());
	const float DistToThreat = FVector::Dist(ThreatActor->GetActorLocation(), CoverPoint->GetCoverLocation());

	// Prefer closer cover for requester, but not too close to threat.
	float Score = 0.0f;
	Score -= DistToRequester * 1.0f;
	Score += FMath::Clamp(DistToThreat, 0.0f, 3000.0f) * 0.2f;

	return Score;
}