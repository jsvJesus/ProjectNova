#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/PNInventoryActionTypes.h"
#include "PNInventoryActionComponent.generated.h"

class APNWorldItemActor;
class UPNEquipmentComponent;
class UPNInventoryComponent;
class UPNItemDataAsset;
class UPNItemInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPNInventoryActionCompletedSignature, const FPNInventoryActionResponse&, Response);

UCLASS(ClassGroup = (ProjectNova), meta = (BlueprintSpawnableComponent))
class PROJECTNOVA_API UPNInventoryActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPNInventoryActionComponent();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory Action")
	FPNInventoryActionCompletedSignature OnInventoryActionCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Action|Drop")
	TSubclassOf<APNWorldItemActor> WorldItemActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Action|Drop", meta = (ClampMin = "10.0"))
	float DropForwardDistance = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Action|Drop", meta = (ClampMin = "0.0"))
	float DropUpOffset = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Action|Debug")
	bool bDebugInventoryActions = true;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory Action")
	FPNInventoryActionResponse ExecuteInventoryAction(const FPNInventoryActionRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action")
	void RequestInventoryAction(const FPNInventoryActionRequest& Request);

	UFUNCTION(Server, Reliable)
	void Server_ExecuteInventoryAction(FPNInventoryActionRequest Request);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Use")
	void RequestUseInventoryItem(FPNInventoryGridPosition InventoryPosition);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Use")
	void RequestUseQuickSlotItem(int32 QuickSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Equip")
	void RequestEquipInventoryItem(FPNInventoryGridPosition InventoryPosition, EPNEquipmentSlot TargetSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Equip")
	void RequestUnequipItem(EPNEquipmentSlot SourceSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Move")
	void RequestMoveInventoryItem(FPNInventoryGridPosition FromPosition, FPNInventoryGridPosition ToPosition, bool bRotated = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Move")
	void RequestMoveInventoryItemToQuickSlot(FPNInventoryGridPosition FromPosition, int32 QuickSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Move")
	void RequestMoveQuickSlotItemToInventory(int32 QuickSlotIndex, FPNInventoryGridPosition ToPosition, bool bRotated = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Stack")
	void RequestSplitInventoryStack(FPNInventoryGridPosition FromPosition, FPNInventoryGridPosition ToPosition, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Stack")
	void RequestSplitInventoryStackHalf(FPNInventoryGridPosition FromPosition, FPNInventoryGridPosition ToPosition);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Stack")
	void RequestMergeInventoryStacks(FPNInventoryGridPosition FromPosition, FPNInventoryGridPosition ToPosition, int32 Quantity = -1);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Drop")
	void RequestDropInventoryItem(FPNInventoryGridPosition InventoryPosition, int32 Quantity = -1);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Drop")
	void RequestDropQuickSlotItem(int32 QuickSlotIndex, int32 Quantity = -1);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Drop")
	void RequestDropEquipmentItem(EPNEquipmentSlot SourceSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Drop")
	void RequestDropEquipmentInternalItem(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Move")
	void RequestMoveEquipmentItemToQuickSlot(EPNEquipmentSlot SourceSlot, int32 QuickSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Move")
	void RequestMoveEquipmentInternalItemToQuickSlot(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex, int32 QuickSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Move")
	void RequestMoveQuickSlotItemToEquipment(int32 QuickSlotIndex, EPNEquipmentSlot TargetSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Move")
	void RequestMoveQuickSlotItemToEquipmentInternal(int32 QuickSlotIndex, EPNEquipmentInternalContainer Container, int32 InternalSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Rotate")
	void RequestRotateInventoryItem(FPNInventoryGridPosition InventoryPosition);

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Inspect")
	FPNInventoryActionResponse InspectTarget(const FPNInventoryActionTarget& Target) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Context Menu")
	TArray<FPNInventoryContextMenuEntry> GetContextMenuActions(const FPNInventoryActionTarget& Target) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory Action|Context Menu")
	bool BuildContextActionRequest(const FPNInventoryActionTarget& Target, EPNInventoryActionType ActionType, FPNInventoryActionRequest& OutRequest) const;

protected:
	FPNInventoryActionResponse HandleUseAction(const FPNInventoryActionRequest& Request);
	FPNInventoryActionResponse HandleEquipAction(const FPNInventoryActionRequest& Request);
	FPNInventoryActionResponse HandleUnequipAction(const FPNInventoryActionRequest& Request);
	FPNInventoryActionResponse HandleMoveAction(const FPNInventoryActionRequest& Request);
	FPNInventoryActionResponse HandleMergeStackAction(const FPNInventoryActionRequest& Request);
	FPNInventoryActionResponse HandleSplitStackAction(const FPNInventoryActionRequest& Request);
	FPNInventoryActionResponse HandleDropAction(const FPNInventoryActionRequest& Request);
	FPNInventoryActionResponse HandleRotateAction(const FPNInventoryActionRequest& Request);
	FPNInventoryActionResponse HandleInspectAction(const FPNInventoryActionRequest& Request) const;

	UPNInventoryComponent* GetOwnerInventoryComponent() const;
	UPNEquipmentComponent* GetOwnerEquipmentComponent() const;

	bool HasActionAuthority() const;

	UPNItemInstance* GetInventoryItemAtTarget(const FPNInventoryActionTarget& Target) const;
	UPNItemDataAsset* GetItemDataFromTarget(const FPNInventoryActionTarget& Target) const;

	int32 GetQuantityFromTarget(const FPNInventoryActionTarget& Target) const;

	bool FindBestEquipmentSlotForItemData(UPNItemDataAsset* ItemData, EPNEquipmentSlot& OutSlot) const;

	FPNInventoryContextMenuEntry MakeContextMenuEntry(
		const FPNInventoryActionTarget& Target,
		EPNInventoryActionType ActionType,
		const FText& DisplayName,
		bool bEnabled,
		int32 SortPriority,
		bool bRequiresDestination = false,
		bool bRequiresQuantity = false,
		bool bDestructive = false,
		EPNInventoryActionResult DisabledReason = EPNInventoryActionResult::UnknownError
	) const;

	int32 ResolveInventoryQuantity(UPNItemInstance* SourceItem, const FPNInventoryActionRequest& Request) const;
	int32 ResolveQuickSlotQuantity(const FPNInventoryQuickSlotEntry& SlotEntry, const FPNInventoryActionRequest& Request) const;

	bool SpawnDroppedWorldItem(UPNItemInstance* DroppedInstance);

	FPNInventoryActionResponse MakeResponse(
		const FPNInventoryActionRequest& Request,
		EPNInventoryActionResult Result,
		bool bSuccess = false,
		UPNItemDataAsset* ItemData = nullptr,
		int32 Quantity = 0
	) const;

	void BroadcastActionCompleted(const FPNInventoryActionResponse& Response);
};