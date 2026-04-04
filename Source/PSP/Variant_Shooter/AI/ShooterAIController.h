#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ShooterAIController.generated.h"

class UStateTreeAIComponent;
class UShooterSquadComponent;

/**
 * AI controller preconfigured with a StateTree AI component.
 * Use this controller as the brain for AShooterAICharacter pawns.
 */
UCLASS()
class PSP_API AShooterAIController : public AAIController
{
	GENERATED_BODY()

public:

	AShooterAIController();

protected:

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/** If true, starts the AI brain on possess. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	bool bStartLogicOnPossess = true;

	/** State Tree runtime component for decision making. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;

public:

	UFUNCTION(BlueprintPure, Category = "AI|Squad")
	UShooterSquadComponent* GetControlledSquadComponent() const;
};
