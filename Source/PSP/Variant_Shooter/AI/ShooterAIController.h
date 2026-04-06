#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ShooterAIController.generated.h"

class UStateTreeAIComponent;
class UShooterSquadComponent;
class AShooterCharacter;

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

	/** Finds and stores the player pawn as current combat target. */
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	APawn* AcquirePlayerTarget();

	/** Returns current combat target pawn. */
	UFUNCTION(BlueprintPure, Category = "AI|Combat")
	APawn* GetCombatTarget() const { return CombatTarget.Get(); }

	/** Moves toward combat target using AI MoveTo. */
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	bool MoveToCombatTarget(float AcceptanceRadius = 550.0f, bool bCanStrafe = true);
	EPathFollowingRequestResult::Type MoveToCombatTarget(float AcceptanceRadius = 550.0f, bool bCanStrafe = true);

	/** Starts or stops firing on the controlled shooter character. */
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void SetFireEnabled(bool bEnabled);

	/** Aims focus at current combat target. */
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void UpdateFocusOnCombatTarget();

protected:

	UPROPERTY(BlueprintReadOnly, Category = "AI|Combat")
	TWeakObjectPtr<APawn> CombatTarget;
};
