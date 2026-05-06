#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/PNInventoryContainerTypes.h"
#include "UI/PNHUDTypes.h"
#include "PNPlayerHUDComponent.generated.h"

class APNBaseCharacter;
class UPNCharacterStatsComponent;
class UPNEquipmentComponent;
class UPNInventoryComponent;
class UPNInventoryContainerComponent;
class UPNItemDataAsset;
class UPNItemInstance;
class UPNQuickSlotComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPNPlayerHUDDataChangedSignature, const FPNPlayerHUDSnapshot&, HUDData);

UCLASS(ClassGroup = (ProjectNova), meta = (BlueprintSpawnableComponent))
class PROJECTNOVA_API UPNPlayerHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPNPlayerHUDComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Player HUD")
	FPNPlayerHUDDataChangedSignature OnHUDDataChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player HUD")
	bool bRefreshOnBeginPlay = true;

protected:
	UPROPERTY(Transient)
	TObjectPtr<APNBaseCharacter> OwnerCharacter = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPNInventoryComponent> BoundOpenedContainerInventory = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player HUD")
	FPNPlayerHUDSnapshot CachedHUDData;

public:
	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Player HUD")
	void SetOwnerCharacter(APNBaseCharacter* InOwnerCharacter);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Player HUD")
	void RefreshHUDData();

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Player HUD")
	const FPNPlayerHUDSnapshot& GetCachedHUDData() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Player HUD")
	APNBaseCharacter* GetOwnerProjectNovaCharacter() const;

protected:
	void ResolveOwnerCharacter();

	void BindToOwnerComponents();
	void UnbindFromOwnerComponents();

	void BindInventory(UPNInventoryComponent* InventoryComponent);
	void UnbindInventory(UPNInventoryComponent* InventoryComponent);

	void BindOpenedContainerInventory(UPNInventoryComponent* InventoryComponent);
	void UnbindOpenedContainerInventory();

	FPNPlayerHUDSnapshot BuildHUDSnapshot() const;

	FPNHUDCharacterStatsData BuildStatsData() const;

	FPNHUDInventoryPanelData BuildInventoryPanelData(
		UPNInventoryComponent* InventoryComponent,
		EPNHUDInventoryPanel Panel
	) const;

	FPNHUDItemViewData BuildItemViewDataFromRep(const FPNRepItemInstanceData& InstanceData) const;
	FPNHUDItemViewData BuildItemViewDataFromInstance(const UPNItemInstance* ItemInstance) const;

	FPNHUDQuickSlotData BuildQuickSlotData(
		const FPNInventoryQuickSlotEntry& QuickSlotEntry,
		int32 SelectedQuickSlotIndex
	) const;

	FPNHUDEquipmentData BuildEquipmentData() const;
	FPNHUDContainerData BuildContainerData() const;

	float GetTotalCarriedWeight() const;

	float CalculateDurabilityPercent(UPNItemDataAsset* ItemData, float CurrentDurability) const;
	float CalculateBatteryPercent(UPNItemDataAsset* ItemData, float CurrentBatteryCharge) const;
	float CalculateExpirationPercent(UPNItemDataAsset* ItemData, float RemainingShelfLifeSeconds) const;

	UFUNCTION()
	void HandleHUDSourceChanged();

	UFUNCTION()
	void HandleFloatStatChanged(float OldValue, float NewValue);

	UFUNCTION()
	void HandleQuickSlotSelected(int32 SlotIndex, UPNItemDataAsset* ItemData);

	UFUNCTION()
	void HandleContainerOpened(const FPNInventoryContainerOpenResponse& Response);

	UFUNCTION()
	void HandleContainerClosed();

	UFUNCTION()
	void HandleContainerTransferCompleted(const FPNInventoryContainerTransferResponse& Response);
};