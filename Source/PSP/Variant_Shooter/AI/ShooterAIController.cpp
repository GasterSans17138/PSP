#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "ShooterSquadComponent.h"
#include "BrainComponent.h"
#include "Components/StateTreeAIComponent.h"

AShooterAIController::AShooterAIController()
{
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));
	bAttachToPawn = true;
}

void AShooterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

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
