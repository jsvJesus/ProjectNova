#pragma once

#include "CoreMinimal.h"
#include "PNBaseCharacter.h"
#include "PNPlayerCharacter.generated.h"

class UCameraComponent;

UCLASS()
class PROJECTNOVA_API APNPlayerCharacter : public APNBaseCharacter
{
	GENERATED_BODY()

public:
	APNPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

public:
	UFUNCTION(BlueprintPure, Category = "Camera")
	UCameraComponent* GetFirstPersonCameraComponent() const;
};