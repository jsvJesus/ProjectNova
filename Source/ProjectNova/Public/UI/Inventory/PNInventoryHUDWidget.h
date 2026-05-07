#pragma once

#include "CoreMinimal.h"
#include "UI/Inventory/PNHUDLayoutWidgets.h"
#include "UI/PNPlayerHUDWidget.h"
#include "PNInventoryHUDWidget.generated.h"

class UWidget;

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNInventoryHUDWidget : public UPNPlayerHUDWidget
{
	GENERATED_BODY()

public:
	UPNInventoryHUDWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD")
	EPNInventoryHUDPage ActivePage = EPNInventoryHUDPage::Inventory;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD")
	TObjectPtr<UPNNavigationLayoutWidget> NavigationLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD")
	TObjectPtr<UPNInventoryGridWidget> InventoryLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD")
	TObjectPtr<UPNInventoryGridWidget> VestInventoryLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD")
	TObjectPtr<UPNInventoryGridWidget> BackpackInventoryLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD")
	TObjectPtr<UPNEquipmentLayoutWidget> EquipmentLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD")
	TObjectPtr<UPNContainerLayoutWidget> ContainerLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD")
	TObjectPtr<UPNQuickSlotLayoutWidget> QuickSlotLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD")
	TObjectPtr<UPNPlayerStatsLayoutWidget> PlayerStatsLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD")
	TObjectPtr<UPNPlayerInfoLayoutWidget> PlayerInfoLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Pages")
	TObjectPtr<UWidget> CustomizationLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Pages")
	TObjectPtr<UWidget> CraftLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Pages")
	TObjectPtr<UWidget> MissionsLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Pages")
	TObjectPtr<UWidget> MapLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Pages")
	TObjectPtr<UWidget> OptionsLayout = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD")
	FPNHUDPlayerInfoData CachedPlayerInfoData;

public:
	virtual void SetHUDData(const FPNPlayerHUDSnapshot& InHUDData) override;
	virtual void SetInventoryVisible(bool bVisible) override;

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Inventory HUD")
	void SetActivePage(EPNInventoryHUDPage NewPage);

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Inventory HUD")
	EPNInventoryHUDPage GetActivePage() const;

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Inventory HUD")
	void RefreshAllLayouts();

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Inventory HUD")
	const FPNHUDPlayerInfoData& GetPlayerInfoData() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "ProjectNova|Inventory HUD")
	void BP_OnActivePageChanged(EPNInventoryHUDPage NewPage);

protected:
	void BindNavigation();
	void UnbindNavigation();

	void PushHUDDataToLayouts();
	void PushPlayerInfoToLayout();

	void ApplyPageVisibility();

	void SetWidgetVisible(UWidget* Widget, bool bVisible) const;
	void SetLayoutVisible(UPNHUDLayoutWidget* Layout, bool bVisible) const;

	FPNHUDPlayerInfoData BuildPlayerInfoData() const;

	UFUNCTION()
	void HandleNavigationPageRequested(EPNInventoryHUDPage RequestedPage);
};