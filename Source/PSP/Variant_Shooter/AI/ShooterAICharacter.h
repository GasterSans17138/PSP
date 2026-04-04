#pragma once

#include "CoreMinimal.h"
#include "ShooterCharacter.h"
#include "ShooterSquadTypes.h"
#include "ShooterAICharacter.generated.h"

class UShooterSquadComponent;

/**
 * AI version of the shooter character with built-in squad component.
 * State Tree logic can use this class as context and call RefreshSquadOrder.
 */
UCLASS()
class PSP_API AShooterAICharacter : public AShooterCharacter
{
	GENERATED_BODY()

public:

	AShooterAICharacter();

	UFUNCTION(BlueprintCallable, Category = "AI|Squad")
	void RefreshSquadOrder();

	UFUNCTION(BlueprintPure, Category = "AI|Squad")
	const FShooterSquadOrder& GetCachedOrder() const { return CachedOrder; }

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UShooterSquadComponent> SquadComponent;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Squad")
	FShooterSquadOrder CachedOrder;
};
