#include "ShooterAICharacter.h"
#include "ShooterSquadComponent.h"
#include "ShooterAIController.h"
#include "ShooterCoverPoint.h"
#include "GameFramework/CharacterMovementComponent.h"

AShooterAICharacter::AShooterAICharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SquadComponent = CreateDefaultSubobject<UShooterSquadComponent>(TEXT("SquadComponent"));
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = true;
		MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	}
}

void AShooterAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SuppressionValue > 0.0f)
	{
		SuppressionValue = FMath::Max(0.0f, SuppressionValue - (SuppressionDecayPerSecond * DeltaTime));
	}
}

void AShooterAICharacter::RefreshSquadOrder()
{
	if (IsValid(SquadComponent))
	{
		CachedOrder = SquadComponent->GetCurrentOrder();
	}
	else
	{
		CachedOrder = FShooterSquadOrder();
	}

	CurrentTacticalOrder = CachedOrder.TacticalOrder;
	CurrentSquadMoveLocation = CachedOrder.MoveLocation;

	bOrderIsTakeCover = (CurrentTacticalOrder == EShooterTacticalOrder::TakeCover);
	bOrderIsPeek = (CurrentTacticalOrder == EShooterTacticalOrder::Peek);
	bOrderIsThrowGrenade = (CurrentTacticalOrder == EShooterTacticalOrder::ThrowGrenade);
}

void AShooterAICharacter::RefreshAIState()
{
	bHasWeapon = (CurrentWeapon != nullptr);

	const AShooterAIController* AIController = Cast<AShooterAIController>(GetController());

	if (AIController)
	{
		bHasCombatTarget = IsValid(AIController->GetCombatTarget());
		bHasWeaponTarget = IsValid(AIController->GetWeaponTarget());
	}
	else
	{
		bHasCombatTarget = false;
		bHasWeaponTarget = false;
	}

	if (IsValid(SquadComponent))
	{
		FShooterSquadMemberRuntimeState RuntimeState;
		RuntimeState.bHasWeapon = bHasWeapon;
		RuntimeState.bHasLineOfSight = false;
		RuntimeState.bReachedTacticalMoveLocation = false;
		RuntimeState.bIsPeeking = false;
		RuntimeState.DistanceToTarget = 0.0f;

		if (AIController)
		{
			if (AActor* Target = AIController->GetCombatTarget())
			{
				RuntimeState.bHasLineOfSight = AIController->HasLineOfSightToActor(Target);
				RuntimeState.DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
			}

			if (const AShooterCoverPoint* CurrentCover = AIController->GetCurrentCoverPoint())
			{
				const float DistToCover = FVector::Dist(GetActorLocation(), CurrentCover->GetCoverLocation());
				const float CoverReachedTolerance = 450.0f;
				RuntimeState.bReachedTacticalMoveLocation = DistToCover <= CoverReachedTolerance;
			}
			else if (!CurrentSquadMoveLocation.IsNearlyZero())
			{
				const float DistToMove = FVector::Dist(GetActorLocation(), CurrentSquadMoveLocation);
				const float ReachedTolerance = 450.0f;
				RuntimeState.bReachedTacticalMoveLocation = DistToMove <= ReachedTolerance;
			}
		}

		if (AIController)
		{
			RuntimeState.bIsPeeking =
				AIController->GetCoverCombatPhase() == EShooterCoverCombatPhase::Peek;
		}

		SquadComponent->SetRuntimeState(RuntimeState);
	}

	RefreshSquadOrder();
}

bool AShooterAICharacter::UsesFirstPersonPresentation() const
{
	return false;
}

void AShooterAICharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	if (!Montage)
	{
		return;
	}

	if (USkeletalMeshComponent* ThirdPersonMesh = GetMesh())
	{
		if (UAnimInstance* AnimInstance = ThirdPersonMesh->GetAnimInstance())
		{
			AnimInstance->Montage_Play(Montage);
		}
	}
}

FVector AShooterAICharacter::GetWeaponTargetLocation()
{
	if (const AShooterAIController* AIController = Cast<AShooterAIController>(GetController()))
	{
		if (AActor* CombatTarget = AIController->GetCombatTarget())
		{
			return CombatTarget->GetActorLocation();
		}

		if (CachedOrder.TargetActor)
		{
			return CachedOrder.TargetActor->GetActorLocation();
		}
	}

	return GetActorLocation() + (GetActorForwardVector() * MaxAimDistance);
}

float AShooterAICharacter::TakeDamage(
	float Damage,
	struct FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (AppliedDamage > 0.0f)
	{
		AddSuppression(35.0f);
	}

	return AppliedDamage;
}

void AShooterAICharacter::AddSuppression(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	SuppressionValue = FMath::Clamp(SuppressionValue + Amount, 0.0f, MaxSuppressionValue);
}

void AShooterAICharacter::ClearSuppression()
{
	SuppressionValue = 0.0f;
}

float AShooterAICharacter::GetSuppressionAlpha() const
{
	if (MaxSuppressionValue <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return FMath::Clamp(SuppressionValue / MaxSuppressionValue, 0.0f, 1.0f);
}