#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "ShooterCharacter.h"
#include "ShooterPickup.h"
#include "ShooterSquadComponent.h"
#include "BrainComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "ShooterCoverPoint.h"

AShooterAIController::AShooterAIController()
{
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));
	bAttachToPawn = true;
}

void AShooterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	RefreshControlledAIState();

	if (bStartLogicOnPossess && BrainComponent && !BrainComponent->IsRunning())
	{
		BrainComponent->StartLogic();
	}
}

void AShooterAIController::OnUnPossess()
{
	if (BrainComponent && BrainComponent->IsRunning())
	{
		BrainComponent->StopLogic(TEXT("UnPossess"));
	}

	CombatTarget = nullptr;
	WeaponTarget = nullptr;

	Super::OnUnPossess();
}

UShooterSquadComponent* AShooterAIController::GetControlledSquadComponent() const
{
	if (const AShooterAICharacter* ShooterCharacter = Cast<AShooterAICharacter>(GetPawn()))
	{
		return ShooterCharacter->FindComponentByClass<UShooterSquadComponent>();
	}

	return nullptr;
}

APawn* AShooterAIController::AcquirePlayerTarget()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	CombatTarget = PlayerPawn;
	RefreshControlledAIState();
	return PlayerPawn;
}

bool AShooterAIController::MoveToCombatTarget(float AcceptanceRadius, bool bCanStrafe)
{
	APawn* TargetPawn = CombatTarget.Get();
	if (!IsValid(TargetPawn))
	{
		TargetPawn = AcquirePlayerTarget();
	}

	if (!IsValid(TargetPawn))
	{
		RefreshControlledAIState();
		return false;
	}

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(TargetPawn);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetCanStrafe(bCanStrafe);

	MoveTo(MoveRequest);
	RefreshControlledAIState();
	return true;
}

void AShooterAIController::SetFireEnabled(bool bEnabled)
{
	AShooterCharacter* Shooter = Cast<AShooterCharacter>(GetPawn());
	if (!IsValid(Shooter))
	{
		return;
	}

	if (bEnabled)
	{
		Shooter->DoStartFiring();
	}
	else
	{
		Shooter->DoStopFiring();
	}
}

void AShooterAIController::UpdateFocusOnCombatTarget()
{
	if (APawn* TargetPawn = CombatTarget.Get())
	{
		SetFocus(TargetPawn);
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);
	}
}

bool AShooterAIController::HasUsableWeapon() const
{
	const AShooterAICharacter* ShooterAI = Cast<AShooterAICharacter>(GetPawn());
	return IsValid(ShooterAI) && ShooterAI->GetCurrentWeaponActor() != nullptr;
}

AActor* AShooterAIController::FindBestWeaponPickup(float SearchRadius)
{
	WeaponTarget = nullptr;

	APawn* SelfPawn = GetPawn();
	if (!IsValid(SelfPawn))
	{
		RefreshControlledAIState();
		return nullptr;
	}

	// If already armed, no need to look for a pickup.
	if (const AShooterAICharacter* ShooterAI = Cast<AShooterAICharacter>(SelfPawn))
	{
		if (ShooterAI->GetCurrentWeaponActor() != nullptr)
		{
			RefreshControlledAIState();
			return nullptr;
		}
	}

	TArray<AActor*> FoundPickups;
	UGameplayStatics::GetAllActorsOfClass(this, AShooterPickup::StaticClass(), FoundPickups);

	AActor* BestPickup = nullptr;
	float BestDistSq = FLT_MAX;
	const FVector SelfLocation = SelfPawn->GetActorLocation();
	const float MaxDistSq = FMath::Square(SearchRadius);

	for (AActor* Candidate : FoundPickups)
	{
		if (!IsValid(Candidate))
		{
			continue;
		}

		// Hidden pickup = already taken / unavailable.
		if (Candidate->IsHidden())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(SelfLocation, Candidate->GetActorLocation());
		if (DistSq > MaxDistSq)
		{
			continue;
		}

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestPickup = Candidate;
		}
	}

	WeaponTarget = BestPickup;
	RefreshControlledAIState();
	return BestPickup;
}

bool AShooterAIController::MoveToWeaponTarget(float AcceptanceRadius)
{
	AActor* TargetActor = WeaponTarget.Get();
	if (!IsValid(TargetActor))
	{
		RefreshControlledAIState();
		return false;
	}

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(TargetActor);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetCanStrafe(false);

	MoveTo(MoveRequest);
	RefreshControlledAIState();
	return true;
}

void AShooterAIController::ClearWeaponTarget()
{
	WeaponTarget = nullptr;
	RefreshControlledAIState();
}

void AShooterAIController::RefreshControlledAIState()
{
	if (AShooterAICharacter* ShooterAI = Cast<AShooterAICharacter>(GetPawn()))
	{
		ShooterAI->RefreshAIState();
	}
}

bool AShooterAIController::HasLineOfSightToActor(AActor* TargetActor) const
{
	const APawn* SelfPawn = GetPawn();
	if (!IsValid(SelfPawn) || !IsValid(TargetActor))
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(SelfPawn);
	QueryParams.AddIgnoredActor(TargetActor);

	const FVector Start = SelfPawn->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
	const FVector End = TargetActor->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);

	const bool bBlockingHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		QueryParams);

	if (!bBlockingHit)
	{
		return true;
	}

	return Hit.GetActor() == TargetActor;
}

bool AShooterAIController::MoveToTacticalLocation(const FVector& Location, float AcceptanceRadius, bool bCanStrafe)
{
	if (Location.IsNearlyZero())
	{
		return false;
	}

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(Location);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetCanStrafe(bCanStrafe);

	MoveTo(MoveRequest);
	return true;
}

void AShooterAIController::SetCurrentCoverPoint(AShooterCoverPoint* NewCoverPoint)
{
	CurrentCoverPoint = NewCoverPoint;
}

void AShooterAIController::ClearCurrentCoverPoint()
{
	CurrentCoverPoint.Reset();
}

bool AShooterAIController::MoveToCoverPoint(float AcceptanceRadius, bool bCanStrafe)
{
	if (!CurrentCoverPoint.IsValid())
	{
		return false;
	}

	return MoveToTacticalLocation(CurrentCoverPoint->GetCoverLocation(), AcceptanceRadius, bCanStrafe);
}