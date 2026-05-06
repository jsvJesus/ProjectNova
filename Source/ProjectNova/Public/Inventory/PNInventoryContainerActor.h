#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/PNInteractableInterface.h"
#include "Inventory/PNInventoryTypes.h"
#include "PNInventoryContainerActor.generated.h"

class UPNInventoryComponent;
class UStaticMeshComponent;

UCLASS()
class PROJECTNOVA_API APNInventoryContainerActor : public AActor, public IPNInteractableInterface
{
	GENERATED_BODY()

public:
	APNInventoryContainerActor();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Container|Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Container|Components")
	TObjectPtr<UPNInventoryComponent> InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container|Setup")
	EPNInventoryType ContainerType = EPNInventoryType::LootContainer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container|Setup")
	FPNInventorySettings ContainerSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container|Setup")
	bool bInitializeInventoryOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container|Interaction")
	FText DisplayName = FText::FromString(TEXT("Container"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container|Interaction")
	FText ActionText = FText::FromString(TEXT("Open"));

public:
	UFUNCTION(BlueprintPure, Category = "Inventory Container")
	UPNInventoryComponent* GetInventoryComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory Container")
	void InitializeContainerInventory();

public:
	virtual FText GetInteractionDisplayName_Implementation(APawn* InteractingPawn) const override;
	virtual FText GetInteractionActionText_Implementation(APawn* InteractingPawn) const override;
	virtual bool CanInteract_Implementation(APawn* InteractingPawn) const override;
	virtual bool Interact_Implementation(APawn* InteractingPawn) override;
};