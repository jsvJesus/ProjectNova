#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PNInventoryTypes.h"
#include "PNInventoryComponent.generated.h"

class UPNItemDataAsset;
class UPNItemInstance;
class UPNCharacterStatsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPNInventoryChangedSignature);

UCLASS(ClassGroup = (ProjectNova), meta = (BlueprintSpawnableComponent))
class PROJECTNOVA_API UPNInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPNInventoryComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(ReplicatedUsing = OnRep_Settings, EditAnywhere, BlueprintReadOnly, Category = "Inventory|Settings")
	FPNInventorySettings Settings;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FPNInventoryChangedSignature OnInventoryChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedItems, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Replication")
	TArray<FPNRepInventoryItemEntry> ReplicatedItems;

	UPROPERTY(ReplicatedUsing = OnRep_QuickSlots, EditAnywhere, BlueprintReadOnly, Category = "Inventory|Quick Slots", meta = (ClampMin = "1", ClampMax = "12"))
	int32 QuickSlotCount = 6;

	UPROPERTY(ReplicatedUsing = OnRep_QuickSlots, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Quick Slots")
	TArray<FPNInventoryQuickSlotEntry> QuickSlots;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	TArray<FPNInventorySlot> Slots;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	TArray<FPNInventoryItemEntry> Items;

	bool bRebuildingFromReplication = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventory(const FPNInventorySettings& InSettings);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RebuildSlots();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UPNItemInstance* CreateItemInstance(UPNItemDataAsset* ItemData, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FPNInventoryAddItemResult AddItem(UPNItemInstance* ItemInstance, bool bAllowStack = true, bool bAutoRotate = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FPNInventoryAddItemResult AddItemAtPosition(UPNItemInstance* ItemInstance, FPNInventoryGridPosition Position, bool bRotated = false, bool bAllowStack = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FPNInventoryMoveItemResult MoveItem(UPNItemInstance* ItemInstance, FPNInventoryGridPosition NewPosition, bool bRotated = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FPNInventoryMoveItemResult MoveItemFromPosition(FPNInventoryGridPosition OldPosition, FPNInventoryGridPosition NewPosition, bool bRotated = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestMoveItemFromPosition(FPNInventoryGridPosition OldPosition, FPNInventoryGridPosition NewPosition, bool bRotated = false);

	UFUNCTION(Server, Reliable)
	void Server_MoveItemFromPosition(FPNInventoryGridPosition OldPosition, FPNInventoryGridPosition NewPosition, bool bRotated);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FPNInventoryRemoveItemResult RemoveItemInstance(UPNItemInstance* ItemInstance, int32 QuantityToRemove = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FPNInventoryRemoveItemResult RemoveItemAtPosition(FPNInventoryGridPosition Position, int32 QuantityToRemove = 1);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FPNInventorySlot>& GetSlots() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FPNInventoryItemEntry>& GetItems() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FPNRepInventoryItemEntry>& GetReplicatedItems() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetColumns() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetRows() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetSlotCount() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetMaxWeight() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool UsesWeightLimit() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool CanReceiveItems() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool CanRemoveItems() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsValidGridPosition(FPNInventoryGridPosition Position) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsAreaInside(FPNInventoryGridPosition Position, FPNInventoryItemSize Size) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsAreaFree(FPNInventoryGridPosition Position, FPNInventoryItemSize Size) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 PositionToIndex(FPNInventoryGridPosition Position) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FPNInventoryGridPosition IndexToPosition(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UPNItemInstance* GetItemAtPosition(FPNInventoryGridPosition Position) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool FindFreePositionForItem(UPNItemInstance* ItemInstance, bool bAutoRotate, FPNInventoryGridPosition& OutPosition, bool& bOutRotated) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Quick Slots")
	void InitializeQuickSlots();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Quick Slots")
	FPNInventoryAddItemResult AddItemToQuickSlot(UPNItemInstance* ItemInstance, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Quick Slots")
	FPNInventoryAddItemResult MoveItemFromInventoryToQuickSlot(FPNInventoryGridPosition InventoryPosition, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Quick Slots")
	FPNInventoryAddItemResult MoveQuickSlotToInventory(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Quick Slots")
	FPNInventoryRemoveItemResult RemoveItemFromQuickSlot(int32 SlotIndex, int32 QuantityToRemove = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Quick Slots")
	void ClearQuickSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Inventory|Quick Slots")
	bool IsValidQuickSlotIndex(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Quick Slots")
	bool IsQuickSlotOccupied(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Quick Slots")
	bool CanPlaceItemDataIntoQuickSlot(UPNItemDataAsset* ItemData) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Quick Slots")
	FPNInventoryQuickSlotEntry GetQuickSlotEntry(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Quick Slots")
	UPNItemDataAsset* GetQuickSlotItemData(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Quick Slots")
	const TArray<FPNInventoryQuickSlotEntry>& GetQuickSlots() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Use Item")
	FPNInventoryUseItemResponse UseItemFromInventoryPosition(FPNInventoryGridPosition InventoryPosition);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Use Item")
	FPNInventoryUseItemResponse UseItemFromQuickSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Use Item")
	void RequestUseItemFromInventoryPosition(FPNInventoryGridPosition InventoryPosition);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Use Item")
	void RequestUseItemFromQuickSlot(int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_UseItemFromInventoryPosition(FPNInventoryGridPosition InventoryPosition);

	UFUNCTION(Server, Reliable)
	void Server_UseItemFromQuickSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Inventory|Use Item")
	bool CanUseItemData(UPNItemDataAsset* ItemData) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Use Item")
	bool IsConsumableItemData(UPNItemDataAsset* ItemData) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Debug")
	bool bDebugInventoryReplication = true;

	UFUNCTION(BlueprintPure, Category = "Inventory|Debug")
	int32 GetInventoryItemCount() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Debug")
	FString GetInventoryDebugString() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Debug")
	void PrintInventoryDebug() const;

protected:
	UFUNCTION()
	void OnRep_Settings();

	UFUNCTION()
	void OnRep_ReplicatedItems();

	bool HasInventoryAuthority() const;
	void ClampSettings();

	void SyncReplicatedItemsFromRuntime();
	void RebuildRuntimeItemsFromReplication();

	bool CanFitWeight(UPNItemInstance* ItemInstance, int32 QuantityToAdd) const;
	bool TryStackItem(UPNItemInstance* SourceItem, int32& InOutRemainingQuantity, FPNInventoryAddItemResult& InOutResult);
	bool IsAreaFreeInternal(FPNInventoryGridPosition Position, FPNInventoryItemSize Size, const UPNItemInstance* IgnoredItem) const;

	int32 FindEntryIndexByItem(const UPNItemInstance* ItemInstance) const;
	int32 FindEntryIndexByPosition(FPNInventoryGridPosition Position) const;

	FPNInventoryItemSize BuildItemSize(const UPNItemInstance* ItemInstance, bool bRotated) const;

	UPNItemInstance* DuplicateItemInstance(UPNItemInstance* SourceItem, int32 Quantity);
	void AddItemEntry(UPNItemInstance* ItemInstance, FPNInventoryGridPosition Position, bool bRotated);
	void RemoveItemEntryByIndex(int32 EntryIndex);
	void MarkItemArea(const FPNInventoryItemEntry& Entry);
	void BroadcastInventoryChanged();

	UFUNCTION()
	void OnRep_QuickSlots();

	void BuildDefaultQuickSlots();

	int32 FindQuickSlotArrayIndex(int32 SlotIndex) const;
	int32 FindOrCreateQuickSlotArrayIndex(int32 SlotIndex);

	void SetQuickSlotDataInternal(int32 SlotIndex, const FPNRepItemInstanceData& InstanceData);
	void ClearQuickSlotDataInternal(int32 SlotIndex);

	UPNCharacterStatsComponent* GetOwnerCharacterStatsComponent() const;

	bool ApplyConsumableEffects(const FPNRepItemInstanceData& InstanceData);
	float RollStatRange(float MinValue, float MaxValue) const;

	EPNInventoryUseItemResult ConvertRemoveResultToUseResult(EPNInventoryOperationResult RemoveResult) const;
};