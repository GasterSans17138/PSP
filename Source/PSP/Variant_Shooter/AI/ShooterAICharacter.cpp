#include "ShooterAICharacter.h"
#include "ShooterSquadComponent.h"
#include "ShooterAIController.h"
#include "ShooterWeapon.h"

AShooterAICharacter::AShooterAICharacter()
{
	SquadComponent = CreateDefaultSubobject<UShooterSquadComponent>(TEXT("SquadComponent"));
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AShooterAICharacter::RefreshSquadOrder()
{
	if (IsValid(SquadComponent))
	{
		CachedOrder = SquadComponent->GetCurrentOrder();
	}
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
}