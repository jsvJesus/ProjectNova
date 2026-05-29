#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/PNInteractableInterface.h"
#include "Inventory/PNInventoryTypes.h"
#include "PNWorldItemActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UPNItemDataAsset;
class UPNItemInstance;
class UPNInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPNWorldItemChangedSignature);

UCLASS()
class PROJECTNOVA_API APNWorldItemActor : public AActor, public IPNInteractableInterface
{
	GENERATED_BODY()

public:
	APNWorldItemActor();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Item|Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Item|Components")
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Item|Setup")
	TObjectPtr<UPNItemDataAsset> DefaultItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Item|Setup", meta = (ClampMin = "1"))
	int32 DefaultQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Item|Setup")
	bool bInitializeOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Item|Setup")
	bool bDestroyWhenPickedUp = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Item|Physics")
	bool bSimulatePhysicsOnSpawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Item|Interaction", meta = (ClampMin = "10.0"))
	float InteractionRadius = 120.0f;

	UPROPERTY(BlueprintAssignable, Category = "World Item")
	FPNWorldItemChangedSignature OnWorldItemChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Item|Runtime")
	TObjectPtr<UPNItemInstance> ItemInstance = nullptr;

	UPROPERTY(Transient)
	bool bConsumed = false;

public:
	UFUNCTION(BlueprintCallable, Category = "World Item")
	void InitializeFromData(UPNItemDataAsset* InItemData, int32 InQuantity = 1);

	UFUNCTION(BlueprintCallable, Category = "World Item")
	void InitializeFromInstance(UPNItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Category = "World Item")
	bool PickupToInventory(UPNInventoryComponent* TargetInventory);

	UFUNCTION(BlueprintCallable, Category = "World Item")
	void RefreshVisual();

	UFUNCTION(BlueprintCallable, Category = "World Item")
	void SetQuantity(int32 NewQuantity);

	UFUNCTION(BlueprintCallable, Category = "World Item")
	void SetWorldItemPhysicsEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "World Item")
	UPNItemInstance* GetItemInstance() const;

	UFUNCTION(BlueprintPure, Category = "World Item")
	UPNItemDataAsset* GetItemData() const;

	UFUNCTION(BlueprintPure, Category = "World Item")
	int32 GetQuantity() const;

	UFUNCTION(BlueprintPure, Category = "World Item")
	bool IsValidWorldItem() const;

	UFUNCTION(BlueprintPure, Category = "World Item")
	bool CanPickup() const;

public:
	virtual FText GetInteractionDisplayName_Implementation(APawn* InteractingPawn) const override;
	virtual FText GetInteractionActionText_Implementation(APawn* InteractingPawn) const override;
	virtual bool CanInteract_Implementation(APawn* InteractingPawn) const override;
	virtual bool Interact_Implementation(APawn* InteractingPawn) override;

protected:
	UPNItemInstance* DuplicateInstanceForWorld(UPNItemInstance* SourceInstance);
	void ApplyInteractionRadius();
	void DeactivateWorldItem();
	void BroadcastWorldItemChanged();
};