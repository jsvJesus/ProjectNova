#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/PNInventoryTypes.h"
#include "Items/PNItemTypes.h"
#include "Items/PNQuickSlotTypes.h"
#include "PNQuickSlotComponent.generated.h"

class UPNInventoryComponent;
class UPNItemDataAsset;
class UPNCharacterStatsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPNQuickSlotsChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPNQuickSlotSelectedSignature, int32, SlotIndex, UPNItemDataAsset*, ItemData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPNQuickSlotUseAnimationSignature, EPNAnimType, UseAnimType, UPNItemDataAsset*, ItemData);

UCLASS(ClassGroup = (ProjectNova), meta = (BlueprintSpawnableComponent))
class PROJECTNOVA_API UPNQuickSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPNQuickSlotComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Quick Slots")
	FPNQuickSlotsChangedSignature OnQuickSlotsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Quick Slots")
	FPNQuickSlotSelectedSignature OnQuickSlotSelected;

	UPROPERTY(BlueprintAssignable, Category = "Quick Slots")
	FPNQuickSlotUseAnimationSignature OnFirstPersonUseAnimationRequested;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quick Slots|Debug")
	bool bDebugQuickSlots = true;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_SelectedQuickSlotIndex, VisibleAnywhere, BlueprintReadOnly, Category = "Quick Slots|Replication")
	int32 SelectedQuickSlotIndex = INDEX_NONE;

public:
	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	FPNQuickSlotOperationResponse AssignFromInventoryPosition(FPNInventoryGridPosition InventoryPosition, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	FPNQuickSlotOperationResponse ClearQuickSlotToInventory(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	FPNQuickSlotOperationResponse SelectQuickSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	FPNQuickSlotOperationResponse ActivateQuickSlot(int32 SlotIndex, bool bDoubleClick);

	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	FPNQuickSlotOperationResponse UseQuickSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	FPNQuickSlotOperationResponse UseInventoryConsumable(FPNInventoryGridPosition InventoryPosition);

	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	void RequestAssignFromInventoryPosition(FPNInventoryGridPosition InventoryPosition, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	void RequestClearQuickSlotToInventory(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	void RequestSelectQuickSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	void RequestActivateQuickSlot(int32 SlotIndex, bool bDoubleClick);

	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	void RequestUseQuickSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	void RequestUseInventoryConsumable(FPNInventoryGridPosition InventoryPosition);

	UFUNCTION(Server, Reliable)
	void Server_AssignFromInventoryPosition(FPNInventoryGridPosition InventoryPosition, int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_ClearQuickSlotToInventory(int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_SelectQuickSlot(int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_ActivateQuickSlot(int32 SlotIndex, bool bDoubleClick);

	UFUNCTION(Server, Reliable)
	void Server_UseQuickSlot(int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_UseInventoryConsumable(FPNInventoryGridPosition InventoryPosition);

	UFUNCTION(Client, Reliable)
	void Client_PlayFirstPersonUseAnimation(EPNAnimType UseAnimType, UPNItemDataAsset* ItemData);

	UFUNCTION(BlueprintPure, Category = "Quick Slots")
	bool IsValidQuickSlotIndex(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Quick Slots")
	bool IsQuickSlotOccupied(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Quick Slots")
	bool IsQuickSlotSelected(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Quick Slots")
	bool IsConsumableItemData(UPNItemDataAsset* ItemData) const;

	UFUNCTION(BlueprintPure, Category = "Quick Slots")
	FPNInventoryQuickSlotEntry GetQuickSlotEntry(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Quick Slots")
	FPNInventoryQuickSlotEntry GetSelectedQuickSlotEntry() const;

	UFUNCTION(BlueprintPure, Category = "Quick Slots")
	UPNItemDataAsset* GetQuickSlotItemData(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Quick Slots")
	UPNItemDataAsset* GetSelectedQuickSlotItemData() const;

	UFUNCTION(BlueprintPure, Category = "Quick Slots")
	int32 GetSelectedQuickSlotIndex() const;

	UFUNCTION(BlueprintPure, Category = "Quick Slots")
	const TArray<FPNInventoryQuickSlotEntry>& GetQuickSlots() const;

	UFUNCTION(BlueprintPure, Category = "Quick Slots|Debug")
	FString GetQuickSlotsDebugString() const;

	UFUNCTION(BlueprintCallable, Category = "Quick Slots|Debug")
	void PrintQuickSlotsDebug() const;

protected:
	UFUNCTION()
	void OnRep_SelectedQuickSlotIndex();

	UFUNCTION()
	void HandleInventoryChanged();

	bool HasQuickSlotAuthority() const;

	UPNInventoryComponent* GetOwnerInventoryComponent() const;
	UPNCharacterStatsComponent* GetOwnerCharacterStatsComponent() const;

	bool ApplyConsumableEffects(const FPNRepItemInstanceData& InstanceData);
	float RollStatRange(float MinValue, float MaxValue) const;

	EPNQuickSlotOperationResult ConvertInventoryAddFail(EPNInventoryOperationResult InventoryResult) const;
	EPNQuickSlotOperationResult ConvertInventoryRemoveFail(EPNInventoryOperationResult InventoryResult) const;

	void BroadcastQuickSlotsChanged();
	void BroadcastSelectedQuickSlot();
};