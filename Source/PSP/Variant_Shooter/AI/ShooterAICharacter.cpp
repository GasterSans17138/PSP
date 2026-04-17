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
	bOrderIsTakeCover = (CurrentTacticalOrder == EShooterTacticalOrder::TakeCover);
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

	if (IsValid(SquadComponent))
	{
		FShooterSquadMemberRuntimeState RuntimeState;
		RuntimeState.bHasWeapon = bHasWeapon;
		RuntimeState.bHasLineOfSight = false;
		RuntimeState.bReachedTacticalMoveLocation = false;
		RuntimeState.DistanceToTarget = 0.0f;

		if (const AShooterAIController* AIController = Cast<AShooterAIController>(GetController()))
		{
			if (AActor* Target = AIController->GetCombatTarget())
			{
				RuntimeState.bHasLineOfSight = AIController->HasLineOfSightToActor(Target);
				RuntimeState.DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
			}
		}

		if (!CurrentSquadMoveLocation.IsNearlyZero())
		{
			const float DistToMove = FVector::Dist(GetActorLocation(), CurrentSquadMoveLocation);
			const float ReachedTolerance = 450.0f;
			RuntimeState.bReachedTacticalMoveLocation = DistToMove <= ReachedTolerance;
		}
		else
		{
			RuntimeState.bReachedTacticalMoveLocation = false;
		}

		SquadComponent->SetRuntimeState(RuntimeState);
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