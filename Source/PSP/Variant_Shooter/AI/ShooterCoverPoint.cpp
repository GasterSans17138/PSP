#include "ShooterCoverPoint.h"
#include "ShooterCoverSubsystem.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

AShooterCoverPoint::AShooterCoverPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	CoverForwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("CoverForwardArrow"));
	CoverForwardArrow->SetupAttachment(RootComponent);
	CoverForwardArrow->ArrowSize = 1.25f;
	CoverForwardArrow->SetRelativeLocation(FVector::ZeroVector);

	PeekArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("PeekArrow"));
	PeekArrow->SetupAttachment(RootComponent);
	PeekArrow->ArrowSize = 1.0f;
	PeekArrow->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f));
}

void AShooterCoverPoint::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UShooterCoverSubsystem* CoverSubsystem = World->GetSubsystem<UShooterCoverSubsystem>())
		{
			CoverSubsystem->RegisterCoverPoint(this);
		}
	}
}

void AShooterCoverPoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UShooterCoverSubsystem* CoverSubsystem = World->GetSubsystem<UShooterCoverSubsystem>())
		{
			CoverSubsystem->UnregisterCoverPoint(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

FVector AShooterCoverPoint::GetCoverLocation() const
{
	return GetActorLocation() - (GetCoverForward() * StandOffsetFromWall);
}

FVector AShooterCoverPoint::GetCoverForward() const
{
	return CoverForwardArrow ? CoverForwardArrow->GetForwardVector() : GetActorForwardVector();
}

FVector AShooterCoverPoint::GetPeekLocation() const
{
	return PeekArrow ? PeekArrow->GetComponentLocation() : GetCoverLocation();
}

bool AShooterCoverPoint::Reserve(AActor* Occupant)
{
	if (!bEnabled || !IsValid(Occupant))
	{
		return false;
	}

	if (CurrentOccupant.IsValid() && CurrentOccupant.Get() != Occupant)
	{
		return false;
	}

	CurrentOccupant = Occupant;
	return true;
}

void AShooterCoverPoint::Release(AActor* Occupant)
{
	if (!CurrentOccupant.IsValid())
	{
		return;
	}

	if (!Occupant || CurrentOccupant.Get() == Occupant)
	{
		CurrentOccupant.Reset();
	}
}

bool AShooterCoverPoint::CanBeUsedBy(AActor* Requester) const
{
	if (!bEnabled || !IsValid(Requester))
	{
		return false;
	}

	return !CurrentOccupant.IsValid() || CurrentOccupant.Get() == Requester;
}