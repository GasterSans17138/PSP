#pragma once

#include "CoreMinimal.h"
#include "ShooterCharacter.h"
#include "ShooterSquadTypes.h"
#include "ShooterAICharacter.generated.h"

class UShooterSquadComponent;
class AShooterAIController;
class AShooterWeapon;

UCLASS()
class PSP_API AShooterAICharacter : public AShooterCharacter
{
	GENERATED_BODY()

public:
	AShooterAICharacter();

	UFUNCTION(BlueprintCallable, Category = "AI|Squad")
	void RefreshSquadOrder();

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void RefreshAIState();

	UFUNCTION(BlueprintPure, Category = "AI|Squad")
	const FShooterSquadOrder& GetCachedOrder() const { return CachedOrder; }

	UFUNCTION(BlueprintPure, Category = "Weapons")
	AShooterWeapon* GetCurrentWeaponActor() const { return CurrentWeapon.Get(); }

	virtual void PlayFiringMontage(UAnimMontage* Montage) override;
	virtual FVector GetWeaponTargetLocation() override;

protected:
	virtual bool UsesFirstPersonPresentation() const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bHasWeapon = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bHasWeaponTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bHasCombatTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bOrderIsTakeCover = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bOrderIsPeek = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	EShooterTacticalOrder CurrentTacticalOrder = EShooterTacticalOrder::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	FVector CurrentSquadMoveLocation = FVector::ZeroVector;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UShooterSquadComponent> SquadComponent;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Squad")
	FShooterSquadOrder CachedOrder;
};