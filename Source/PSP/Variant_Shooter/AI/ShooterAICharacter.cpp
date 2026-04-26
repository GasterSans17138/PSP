#include "ShooterAICharacter.h"
#include "ShooterSquadComponent.h"
#include "ShooterAIController.h"
#include "ShooterCoverPoint.h"
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