#include "ShooterAICharacter.h"
#include "ShooterSquadComponent.h"
#include "ShooterAIController.h"

AShooterAICharacter::AShooterAICharacter()
{
	SquadComponent = CreateDefaultSubobject<UShooterSquadComponent>(TEXT("SquadComponent"));
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AShooterAIController::StaticClass();
}

void AShooterAICharacter::RefreshSquadOrder()
{
	if (IsValid(SquadComponent))
	{
		CachedOrder = SquadComponent->GetCurrentOrder();
	}
}
