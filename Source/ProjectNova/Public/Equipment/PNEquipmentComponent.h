#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Equipment/PNEquipmentTypes.h"
#include "Inventory/PNInventoryTypes.h"
#include "Items/PNItemTypes.h"
#include "PNEquipmentComponent.generated.h"

class UPNInventoryComponent;
class UPNItemDataAsset;
class UPNItemInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPNEquipmentChangedSignature);

UCLASS(ClassGroup = (ProjectNova), meta = (BlueprintSpawnableComponent))
class PROJECTNOVA_API UPNEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPNEquipmentComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FPNEquipmentChangedSignature OnEquipmentChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Debug")
	bool bDebugEquipmentReplication = true;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentSlots, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Replication")
	TArray<FPNEquipmentSlotEntry> EquipmentSlots;

	UPROPERTY(ReplicatedUsing = OnRep_HelmetInternalSlots, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Replication")
	TArray<FPNEquipmentInternalSlotEntry> HelmetInternalSlots;

	UPROPERTY(ReplicatedUsing = OnRep_ArmorInternalSlots, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Replication")
	TArray<FPNEquipmentInternalSlotEntry> ArmorInternalSlots;

public:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void InitializeEquipment();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	FPNEquipmentOperationResponse EquipFromInventoryToSlot(FPNInventoryGridPosition InventoryPosition, EPNEquipmentSlot TargetSlot);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	FPNEquipmentOperationResponse UnequipSlotToInventory(EPNEquipmentSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	FPNEquipmentOperationResponse InsertFromInventoryToInternalSlot(FPNInventoryGridPosition InventoryPosition, EPNEquipmentInternalContainer Container, int32 InternalSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	FPNEquipmentOperationResponse RemoveInternalSlotToInventory(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Direct")
	FPNEquipmentOperationResponse EquipItemInstanceToSlot(UPNItemInstance* ItemInstance, EPNEquipmentSlot TargetSlot);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Direct")
	FPNEquipmentOperationResponse InsertItemInstanceToInternalSlot(UPNItemInstance* ItemInstance, EPNEquipmentInternalContainer Container, int32 InternalSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Direct")
	UPNItemInstance* RemoveEquipmentSlotAsItemInstance(EPNEquipmentSlot Slot, FPNEquipmentOperationResponse& OutResponse);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Direct")
	UPNItemInstance* RemoveInternalSlotAsItemInstance(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex, FPNEquipmentOperationResponse& OutResponse);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void RequestEquipFromInventoryToSlot(FPNInventoryGridPosition InventoryPosition, EPNEquipmentSlot TargetSlot);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void RequestUnequipSlotToInventory(EPNEquipmentSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void RequestInsertFromInventoryToInternalSlot(FPNInventoryGridPosition InventoryPosition, EPNEquipmentInternalContainer Container, int32 InternalSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void RequestRemoveInternalSlotToInventory(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_EquipFromInventoryToSlot(FPNInventoryGridPosition InventoryPosition, EPNEquipmentSlot TargetSlot);

	UFUNCTION(Server, Reliable)
	void Server_UnequipSlotToInventory(EPNEquipmentSlot Slot);

	UFUNCTION(Server, Reliable)
	void Server_InsertFromInventoryToInternalSlot(FPNInventoryGridPosition InventoryPosition, EPNEquipmentInternalContainer Container, int32 InternalSlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_RemoveInternalSlotToInventory(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex);

	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsEquipmentSlotValid(EPNEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsEquipmentSlotOccupied(EPNEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsInternalSlotOccupied(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	int32 GetInternalSlotCount(EPNEquipmentInternalContainer Container) const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	int32 GetUnlockedInternalSlotCount(EPNEquipmentInternalContainer Container) const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	FPNEquipmentSlotEntry GetEquipmentSlotEntry(EPNEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	FPNEquipmentInternalSlotEntry GetInternalSlotEntry(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	UPNItemDataAsset* GetEquippedItemData(EPNEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	UPNItemDataAsset* GetInternalSlotItemData(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	const TArray<FPNEquipmentSlotEntry>& GetEquipmentSlots() const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	const TArray<FPNEquipmentInternalSlotEntry>& GetHelmetInternalSlots() const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	const TArray<FPNEquipmentInternalSlotEntry>& GetArmorInternalSlots() const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool CanEquipItemToSlot(UPNItemInstance* ItemInstance, EPNEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool CanInsertItemIntoInternalSlot(UPNItemInstance* ItemInstance, EPNEquipmentInternalContainer Container, int32 InternalSlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Equipment|Debug")
	FString GetEquipmentDebugString() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment|Debug")
	void PrintEquipmentDebug() const;

protected:
	UFUNCTION()
	void OnRep_EquipmentSlots();

	UFUNCTION()
	void OnRep_HelmetInternalSlots();

	UFUNCTION()
	void OnRep_ArmorInternalSlots();

	bool HasEquipmentAuthority() const;

	UPNInventoryComponent* GetOwnerInventoryComponent() const;

	void BuildDefaultEquipmentSlots();
	void BuildInternalSlots(EPNEquipmentInternalContainer Container, int32 SlotCount);

	int32 FindEquipmentSlotIndex(EPNEquipmentSlot Slot) const;
	int32 FindOrCreateEquipmentSlotIndex(EPNEquipmentSlot Slot);

	TArray<FPNEquipmentInternalSlotEntry>* GetMutableInternalSlots(EPNEquipmentInternalContainer Container);
	const TArray<FPNEquipmentInternalSlotEntry>* GetConstInternalSlots(EPNEquipmentInternalContainer Container) const;

	int32 FindInternalSlotIndex(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex) const;
	int32 FindOrCreateInternalSlotIndex(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex);

	void SetEquipmentSlotItem(EPNEquipmentSlot Slot, const FPNRepItemInstanceData& InstanceData);
	void ClearEquipmentSlot(EPNEquipmentSlot Slot);

	void SetInternalSlotItem(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex, const FPNRepItemInstanceData& InstanceData);
	void ClearInternalSlot(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex);

	bool HasAnyInternalItemsForTopSlot(EPNEquipmentSlot Slot) const;

	UPNItemDataAsset* GetEquippedContainerData(EPNEquipmentInternalContainer Container) const;

	bool IsPrimaryWeaponCategory(EPNItemCategory Category) const;
	bool IsSidearmCategory(EPNItemCategory Category) const;

	bool HasLinkedInventoryItemsForSlot(EPNEquipmentSlot Slot) const;

	EPNEquipmentOperationResult ConvertInventoryAddFail(EPNInventoryOperationResult InventoryResult) const;
	EPNEquipmentOperationResult ConvertInventoryRemoveFail(EPNInventoryOperationResult InventoryResult) const;

	void BroadcastEquipmentChanged();
};