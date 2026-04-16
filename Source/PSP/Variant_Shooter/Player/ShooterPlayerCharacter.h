#pragma once

#include "CoreMinimal.h"
#include "ShooterCharacter.h"
#include "ShooterPlayerCharacter.generated.h"

UCLASS()
class PSP_API AShooterPlayerCharacter : public AShooterCharacter
{
	GENERATED_BODY()

protected:
	virtual bool UsesFirstPersonPresentation() const override;
};