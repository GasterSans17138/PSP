#pragma once

#include "CoreMinimal.h"
#include "ShooterCharacter.h"
#include "ShooterSquadTypes.h"
#include "ShooterAICharacter.generated.h"

class UShooterSquadComponent;
class AShooterAIController;
class AShooterWeapon;

/**
 * AI version of the shooter character with built-in squad component.
 * State Tree logic can use this class as context and read simple state variables.
 */
UCLASS()
class PSP_API AShooterAICharacter : public AShooterCharacter
{
	GENERATED_BODY()

public:

	AShooterAICharacter();

	UFUNCTION(BlueprintCallable, Category = "AI|Squad")
	void RefreshSquadOrder();

	/** Refreshes lightweight AI state variables consumed by State Tree enter conditions. */
	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void RefreshAIState();

	UFUNCTION(BlueprintPure, Category = "AI|Squad")
	const FShooterSquadOrder& GetCachedOrder() const { return CachedOrder; }

	/** True if this AI currently has an equipped weapon. */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	bool HasWeaponState() const { return bHasWeapon; }

	/** Current equipped weapon, if any. */
	UFUNCTION(BlueprintPure, Category = "Weapons")
	AShooterWeapon* GetCurrentWeaponActor() const { return CurrentWeapon.Get(); }

	/** Read by State Tree enter conditions. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bHasWeapon = false;

	/** Read by State Tree enter conditions. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bHasWeaponTarget = false;

	/** Read by State Tree enter conditions. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bHasCombatTarget = false;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UShooterSquadComponent> SquadComponent;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Squad")
	FShooterSquadOrder CachedOrder;

	
};