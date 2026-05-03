#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PNInteractableInterface.generated.h"

class APawn;

UINTERFACE(BlueprintType)
class PROJECTNOVA_API UPNInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTNOVA_API IPNInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionDisplayName(APawn* InteractingPawn) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionActionText(APawn* InteractingPawn) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(APawn* InteractingPawn) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool Interact(APawn* InteractingPawn);
};