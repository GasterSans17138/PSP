#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "ShooterCharacter.h"
#include "ShooterSquadComponent.h"
#include "BrainComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "Kismet/GameplayStatics.h"
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

APawn* AShooterAIController::AcquirePlayerTarget()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	CombatTarget = PlayerPawn;
	return PlayerPawn;
}

bool AShooterAIController::MoveToCombatTarget(float AcceptanceRadius, bool bCanStrafe)
EPathFollowingRequestResult::Type AShooterAIController::MoveToCombatTarget(float AcceptanceRadius, bool bCanStrafe)
{
	APawn* TargetPawn = CombatTarget.Get();
	if (!IsValid(TargetPawn))
	{
		TargetPawn = AcquirePlayerTarget();
	}

	if (!IsValid(TargetPawn))
	{
		return false;
		return EPathFollowingRequestResult::Failed;
	}

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(TargetPawn);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetCanStrafe(bCanStrafe);

	MoveTo(MoveRequest);
	return true;
	return MoveTo(MoveRequest);
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
