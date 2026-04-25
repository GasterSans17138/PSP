#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterCoverPoint.generated.h"

class UArrowComponent;
class USceneComponent;

UCLASS()
class PSP_API AShooterCoverPoint : public AActor
{
	GENERATED_BODY()

public:
	AShooterCoverPoint();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "Cover")
	FVector GetCoverLocation() const;

	UFUNCTION(BlueprintPure, Category = "Cover")
	FVector GetPeekLocation() const;

	UFUNCTION(BlueprintPure, Category = "Cover")
	FVector GetCoverForward() const;

	UFUNCTION(BlueprintPure, Category = "Cover")
	bool IsOccupied() const { return CurrentOccupant.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Cover")
	AActor* GetCurrentOccupant() const { return CurrentOccupant.Get(); }

	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool Reserve(AActor* Occupant);

	UFUNCTION(BlueprintCallable, Category = "Cover")
	void Release(AActor* Occupant);

	UFUNCTION(BlueprintPure, Category = "Cover")
	bool CanBeUsedBy(AActor* Requester) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> CoverForwardArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> PeekArrow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover", meta = (ClampMin = 0, Units = "cm"))
	float StandOffsetFromWall = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover")
	bool bEnabled = true;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentOccupant;
};