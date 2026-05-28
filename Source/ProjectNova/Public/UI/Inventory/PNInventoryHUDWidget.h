#pragma once

#include "CoreMinimal.h"
#include "UI/Inventory/PNHUDLayoutWidgets.h"
#include "UI/PNPlayerHUDWidget.h"
#include "PNInventoryHUDWidget.generated.h"

class UWidget;
class UButton;
class UImage;
class UOverlay;
class USizeBox;
class UTextBlock;
class UTexture2D;
class UDragDropOperation;
class UPNInventoryActionComponent;
class UPNInventoryDragDropOperation;

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNInventoryHUDWidget : public UPNPlayerHUDWidget
{
	GENERATED_BODY()

public:
	UPNInventoryHUDWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual bool NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation
	) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD")
	EPNInventoryHUDPage ActivePage = EPNInventoryHUDPage::Inventory;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD")
	TObjectPtr<UPNNavigationLayoutWidget> NavigationLayout = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD")
	TObjectPtr<UPNInventoryGridWidget> InventoryLayout = nullptr;

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

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Slots")
	TObjectPtr<USizeBox> Equip_PrimaryWeapon1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Slots")
	TObjectPtr<USizeBox> Equip_PrimaryWeapon2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Slots")
	TObjectPtr<USizeBox> Equip_Sidearm = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Slots")
	TObjectPtr<USizeBox> Equip_Knife = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Slots")
	TObjectPtr<USizeBox> Equip_Helmet = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Slots")
	TObjectPtr<USizeBox> Equip_Armor = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Slots")
	TObjectPtr<USizeBox> Equip_Gloves = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Slots")
	TObjectPtr<USizeBox> Equip_Backpack = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Internal Slots")
	TObjectPtr<USizeBox> HelmetInternal_0 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Internal Slots")
	TObjectPtr<USizeBox> HelmetInternal_1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Internal Slots")
	TObjectPtr<USizeBox> HelmetInternal_2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Internal Slots")
	TObjectPtr<USizeBox> HelmetInternal_3 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Internal Slots")
	TObjectPtr<USizeBox> ArmorInternal_0 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Internal Slots")
	TObjectPtr<USizeBox> ArmorInternal_1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Internal Slots")
	TObjectPtr<USizeBox> ArmorInternal_2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "ProjectNova|Inventory HUD|Equipment Internal Slots")
	TObjectPtr<USizeBox> ArmorInternal_3 = nullptr;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD|Equipment Style")
	FVector2D EquipmentPrimaryWeaponSlotSize = FVector2D(256.0f, 128.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD|Equipment Style")
	FVector2D EquipmentDefaultSlotSize = FVector2D(128.0f, 128.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD|Equipment Style")
	FVector2D EquipmentInternalSlotSize = FVector2D(64.0f, 64.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD|Equipment Style", meta = (ClampMin = "1"))
	int32 EquipmentWeaponLabelFontSize = 14;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD|Equipment Style")
	FLinearColor EquipmentWeaponLabelColor = FLinearColor(0.88f, 0.92f, 0.94f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD|Equipment Textures")
	TSoftObjectPtr<UTexture2D> EquipmentSlotBackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD|Equipment Textures")
	TSoftObjectPtr<UTexture2D> EquipmentSlotHoverTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD|Equipment Textures")
	TSoftObjectPtr<UTexture2D> EquipmentItemBackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD|Equipment Textures")
	TSoftObjectPtr<UTexture2D> EquipmentInternalSlotBackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD|Equipment Textures")
	TSoftObjectPtr<UTexture2D> EquipmentInternalSlotHoverTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD|Equipment Textures")
	TSoftObjectPtr<UTexture2D> EquipmentInternalItemBackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|Inventory HUD|Equipment Textures")
	TSoftObjectPtr<UTexture2D> EquipmentInternalSlotLockedTexture;

	UFUNCTION(BlueprintImplementableEvent, Category = "ProjectNova|Inventory HUD")
	void BP_OnActivePageChanged(EPNInventoryHUDPage NewPage);

	void ApplySharedInventoryGridStyle();

protected:
	void BindNavigation();
	void UnbindNavigation();

	void PushHUDDataToLayouts();
	void PushPlayerInfoToLayout();

	void ApplyPageVisibility();

	void SetWidgetVisible(UWidget* Widget, bool bVisible) const;
	void SetLayoutVisible(UPNHUDLayoutWidget* Layout, bool bVisible) const;

	FPNHUDPlayerInfoData BuildPlayerInfoData() const;

	void PushEquipmentDataToBoundSlots();

	void BuildEquipmentSlotWidget(
		USizeBox* TargetSizeBox,
		EPNEquipmentSlot EquipmentSlot,
		const FPNHUDEquipmentSlotData& SlotData
	);

	void BuildInternalEquipmentSlotWidget(
		USizeBox* TargetSizeBox,
		EPNEquipmentInternalContainer InternalContainer,
		int32 InternalSlotIndex,
		const FPNHUDInternalEquipmentSlotData& SlotData
	);

	FPNHUDEquipmentSlotData FindHUDSlotData(EPNEquipmentSlot EquipmentSlot) const;

	FPNHUDInternalEquipmentSlotData FindHUDInternalSlotData(
		EPNEquipmentInternalContainer InternalContainer,
		int32 InternalSlotIndex
	) const;

	FText GetWeaponSlotLabel(EPNEquipmentSlot EquipmentSlot) const;

	bool IsWeaponEquipmentSlot(EPNEquipmentSlot EquipmentSlot) const;

	void ApplyEquipmentButtonStyle(
		UButton* TargetButton,
		bool bOccupied,
		bool bHoveredSlot,
		bool bLocked,
		bool bInternalSlot,
		const FVector2D& ImageSize
	) const;

	void ApplyTextureToEquipmentImage(
		UImage* TargetImage,
		UTexture2D* Texture,
		const FVector2D& ImageSize
	) const;

	FVector2D GetEquipmentSlotRenderSize(EPNEquipmentSlot EquipmentSlot) const;

	UFUNCTION()
	void HandleNavigationPageRequested(EPNInventoryHUDPage RequestedPage);

	bool FindEquipmentSlotUnderCursor(const FVector2D& ScreenSpacePosition, EPNEquipmentSlot& OutEquipmentSlot) const;
	bool TryHandleInventoryDropToEquipmentSlot(UPNInventoryDragDropOperation* DragOperation, EPNEquipmentSlot TargetSlot);
	UPNInventoryActionComponent* GetOwnerInventoryActionComponent() const;
};