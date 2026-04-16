#include "ShooterAICharacter.h"
#include "ShooterSquadComponent.h"
#include "ShooterAIController.h"
#include "ShooterWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"

AShooterAICharacter::AShooterAICharacter()
{
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

	bOrderIsPush = (CurrentTacticalOrder == EShooterTacticalOrder::Push);
	bOrderIsSuppress = (CurrentTacticalOrder == EShooterTacticalOrder::Suppress);
	bOrderIsFlank =
		(CurrentTacticalOrder == EShooterTacticalOrder::FlankLeft) ||
		(CurrentTacticalOrder == EShooterTacticalOrder::FlankRight);
	bOrderIsHold = (CurrentTacticalOrder == EShooterTacticalOrder::Hold);
}

void AShooterAICharacter::RefreshAIState()
{
	bHasWeapon = (CurrentWeapon != nullptr);

	if (const AShooterAIController* AIController = Cast<AShooterAIController>(GetController()))
	{
		bHasCombatTarget = IsValid(AIController->GetCombatTarget());
		bHasWeaponTarget = IsValid(AIController->GetWeaponTarget());
	}
	else
	{
		bHasCombatTarget = false;
		bHasWeaponTarget = false;
	}

	RefreshSquadOrder();
}

bool AShooterAICharacter::UsesFirstPersonPresentation() const
{
	return false;
}

void AShooterAICharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	Super::AttachWeaponMeshes(Weapon);
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