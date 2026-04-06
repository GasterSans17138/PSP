#include "ShooterAICharacter.h"
#include "ShooterSquadComponent.h"

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
