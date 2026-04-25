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

	CleanupInvalidCoverPoints();

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

AShooterCoverPoint* UShooterCoverSubsystem::FindBestCoverNearLocation(
	AActor* Requester,
	AActor* ThreatActor,
	const FVector& PreferredLocation,
	EShooterTacticalOrder BaseTacticalOrder,
	float MaxSearchDistance)
{
	if (!IsValid(Requester) || !IsValid(ThreatActor))
	{
		return nullptr;
	}

	CleanupInvalidCoverPoints();

	AShooterCoverPoint* BestCoverWithPeekLOS = nullptr;
	float BestCoverWithPeekLOSScore = -FLT_MAX;

	AShooterCoverPoint* BestProtectedCover = nullptr;
	float BestProtectedCoverScore = -FLT_MAX;

	for (AShooterCoverPoint* CoverPoint : CoverPoints)
	{
		if (!IsValid(CoverPoint) || !CoverPoint->CanBeUsedBy(Requester))
		{
			continue;
		}

		const float DistToRequester = FVector::Dist(
			Requester->GetActorLocation(),
			CoverPoint->GetCoverLocation());

		if (DistToRequester > MaxSearchDistance)
		{
			continue;
		}

		if (IsTooCloseToOccupiedCover(CoverPoint, Requester, 220.0f))
		{
			continue;
		}

		// Priority 1: cover must protect from player.
		if (!IsCoverProtectedFromThreat(CoverPoint, ThreatActor))
		{
			continue;
		}

		// Priority 2: score role/position quality.
		const float Score = ScoreCoverNearLocation(
			CoverPoint,
			Requester,
			ThreatActor,
			PreferredLocation,
			BaseTacticalOrder);

		// Fallback candidate: protected cover, even if peek has no LOS.
		if (Score > BestProtectedCoverScore)
		{
			BestProtectedCoverScore = Score;
			BestProtectedCover = CoverPoint;
		}

		// Preferred candidate: protected cover + peek can see player.
		if (HasPeekLineOfSightToThreat(CoverPoint, ThreatActor))
		{
			if (Score > BestCoverWithPeekLOSScore)
			{
				BestCoverWithPeekLOSScore = Score;
				BestCoverWithPeekLOS = CoverPoint;
			}
		}
	}

	// Priority 3: among role-valid protected covers, prefer one with peek LOS.
	if (IsValid(BestCoverWithPeekLOS))
	{
		return BestCoverWithPeekLOS;
	}

	// Priority 4: fallback to protected cover even without peek LOS.
	return BestProtectedCover;
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

bool UShooterCoverSubsystem::IsCoverProtectedFromThreat(
	const AShooterCoverPoint* CoverPoint,
	const AActor* ThreatActor) const
{
	if (!IsValid(CoverPoint) || !IsValid(ThreatActor))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CoverPoint);
	Params.AddIgnoredActor(ThreatActor);

	const FVector Start = ThreatActor->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
	const FVector End = CoverPoint->GetCoverLocation() + FVector(0.0f, 0.0f, 60.0f);

	return World->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params);
}

bool UShooterCoverSubsystem::IsCoverStillValidAgainstThreat(
	const AShooterCoverPoint* CoverPoint,
	const AActor* ThreatActor) const
{
	return IsCoverProtectedFromThreat(CoverPoint, ThreatActor);
}

bool UShooterCoverSubsystem::IsTooCloseToOccupiedCover(
	const AShooterCoverPoint* CandidateCover,
	const AActor* Requester,
	float MinDistance) const
{
	if (!IsValid(CandidateCover) || !IsValid(Requester))
	{
		return false;
	}

	for (const AShooterCoverPoint* OtherCover : CoverPoints)
	{
		if (!IsValid(OtherCover) || OtherCover == CandidateCover)
		{
			continue;
		}

		AActor* Occupant = OtherCover->GetCurrentOccupant();
		if (!IsValid(Occupant) || Occupant == Requester)
		{
			continue;
		}

		const float Dist = FVector::Dist(
			CandidateCover->GetCoverLocation(),
			OtherCover->GetCoverLocation());

		if (Dist < MinDistance)
		{
			return true;
		}
	}

	return false;
}

float UShooterCoverSubsystem::ScoreCoverNearLocation(
	const AShooterCoverPoint* CoverPoint,
	const AActor* Requester,
	const AActor* ThreatActor,
	const FVector& PreferredLocation,
	EShooterTacticalOrder BaseTacticalOrder) const
{
	if (!IsValid(CoverPoint) || !IsValid(Requester) || !IsValid(ThreatActor))
	{
		return -FLT_MAX;
	}

	const FVector CoverLocation = CoverPoint->GetCoverLocation();

	const float DistToRequester = FVector::Dist(Requester->GetActorLocation(), CoverLocation);
	const float DistToThreat = FVector::Dist(ThreatActor->GetActorLocation(), CoverLocation);
	const float DistToPreferred = FVector::Dist(PreferredLocation, CoverLocation);

	float DesiredThreatDistance = 1000.0f;
	float PreferredWeight = 1.2f;
	float RequesterWeight = 0.35f;
	float DesiredDistanceWeight = 0.45f;

	switch (BaseTacticalOrder)
	{
	case EShooterTacticalOrder::Push:
		DesiredThreatDistance = 420.0f;
		PreferredWeight = 1.3f;
		RequesterWeight = 0.25f;
		DesiredDistanceWeight = 0.55f;
		break;

	case EShooterTacticalOrder::Suppress:
		DesiredThreatDistance = 1100.0f;
		PreferredWeight = 1.25f;
		RequesterWeight = 0.25f;
		DesiredDistanceWeight = 0.45f;
		break;

	case EShooterTacticalOrder::FlankLeft:
	case EShooterTacticalOrder::FlankRight:
		DesiredThreatDistance = 850.0f;
		PreferredWeight = 5.0f;
		RequesterWeight = 0.05f;
		DesiredDistanceWeight = 0.35f;
		break;

	case EShooterTacticalOrder::Hold:
		DesiredThreatDistance = 850.0f;
		PreferredWeight = 1.1f;
		RequesterWeight = 0.3f;
		DesiredDistanceWeight = 0.45f;
		break;

	default:
		break;
	}

	const float DistToDesiredThreatDistance = FMath::Abs(DistToThreat - DesiredThreatDistance);

	float Score = 0.0f;

	Score -= DistToPreferred * PreferredWeight;
	Score -= DistToRequester * RequesterWeight;
	Score -= DistToDesiredThreatDistance * DesiredDistanceWeight;

	// Important: prefer covers whose peek point can actually see the player.
	// If no such cover exists, normal protected cover scoring still works.
	Score += GetPeekLineOfSightBonus(CoverPoint, ThreatActor, BaseTacticalOrder);

	return Score;
}

bool UShooterCoverSubsystem::HasPeekLineOfSightToThreat(
	const AShooterCoverPoint* CoverPoint,
	const AActor* ThreatActor) const
{
	if (!IsValid(CoverPoint) || !IsValid(ThreatActor))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CoverPoint);
	Params.AddIgnoredActor(ThreatActor);

	const FVector Start = CoverPoint->GetPeekLocation() + FVector(0.0f, 0.0f, 60.0f);
	const FVector End = ThreatActor->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);

	const bool bBlockingHit = World->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params);

	if (!bBlockingHit)
	{
		return true;
	}

	return Hit.GetActor() == ThreatActor;
}

float UShooterCoverSubsystem::GetPeekLineOfSightBonus(
	const AShooterCoverPoint* CoverPoint,
	const AActor* ThreatActor,
	EShooterTacticalOrder BaseTacticalOrder) const
{
	if (!HasPeekLineOfSightToThreat(CoverPoint, ThreatActor))
	{
		return 0.0f;
	}

	switch (BaseTacticalOrder)
	{
	case EShooterTacticalOrder::Suppress:
		return 2500.0f;

	case EShooterTacticalOrder::Push:
		return 2200.0f;

	case EShooterTacticalOrder::FlankLeft:
	case EShooterTacticalOrder::FlankRight:
		return 3000.0f;

	case EShooterTacticalOrder::Hold:
		return 1800.0f;

	default:
		return 2000.0f;
	}
}