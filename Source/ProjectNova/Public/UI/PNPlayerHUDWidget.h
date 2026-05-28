#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/PNHUDTypes.h"
#include "PNPlayerHUDWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UHorizontalBox;
class UImage;
class UPNItemDataAsset;
class UPNPlayerHUDComponent;
class UProgressBar;
class UScrollBox;
class UTextBlock;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPNPlayerHUDWidgetDataUpdatedSignature, const FPNPlayerHUDSnapshot&, HUDData);

USTRUCT()
struct PROJECTNOVA_API FPNHUDStatWidgetRefs
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LabelText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ValueText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> ProgressBar = nullptr;
};

USTRUCT()
struct PROJECTNOVA_API FPNHUDQuickSlotWidgetRefs
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UBorder> RootBorder = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> IndexText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UImage> IconImage = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ItemText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> QuantityText = nullptr;
};

USTRUCT()
struct PROJECTNOVA_API FPNHUDInventoryPanelWidgetRefs
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UBorder> RootBorder = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HeaderText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WeightText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> ItemList = nullptr;
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPNPlayerHUDWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "ProjectNova|HUD")
	FPNPlayerHUDWidgetDataUpdatedSignature OnHUDDataUpdated;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|HUD|Layout")
	bool bBuildNativeLayout = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProjectNova|HUD|Layout")
	bool bShowInventoryOnStart = false;

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "ProjectNova|HUD")
	TObjectPtr<UPNPlayerHUDComponent> HUDComponent = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "ProjectNova|HUD")
	FPNPlayerHUDSnapshot CachedHUDData;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootCanvas = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> StatsBox = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> QuickSlotsBox = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> InventoryRootBorder = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> InventoryPanelsBox = nullptr;

	UPROPERTY(Transient)
	FPNHUDStatWidgetRefs HealthRefs;

	UPROPERTY(Transient)
	FPNHUDStatWidgetRefs StaminaRefs;

	UPROPERTY(Transient)
	FPNHUDStatWidgetRefs HungerRefs;

	UPROPERTY(Transient)
	FPNHUDStatWidgetRefs ThirstRefs;

	UPROPERTY(Transient)
	FPNHUDStatWidgetRefs WeightRefs;

	UPROPERTY(Transient)
	FPNHUDStatWidgetRefs RadiationRefs;

	UPROPERTY(Transient)
	FPNHUDStatWidgetRefs ToxicityRefs;

	UPROPERTY(Transient)
	FPNHUDStatWidgetRefs PsyRefs;

	UPROPERTY(Transient)
	TArray<FPNHUDQuickSlotWidgetRefs> QuickSlotWidgets;

	UPROPERTY(Transient)
	FPNHUDInventoryPanelWidgetRefs MainInventoryRefs;

	UPROPERTY(Transient)
	FPNHUDInventoryPanelWidgetRefs ContainerInventoryRefs;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "ProjectNova|HUD")
	bool bInventoryVisible = false;

public:
	virtual void InitializeWithHUDComponent(UPNPlayerHUDComponent* InHUDComponent);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|HUD")
	virtual void SetHUDData(const FPNPlayerHUDSnapshot& InHUDData);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|HUD")
	virtual void RefreshFromHUDComponent();

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|HUD")
	virtual void SetInventoryVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|HUD")
	virtual void ToggleInventoryVisible();

	UFUNCTION(BlueprintPure, Category = "ProjectNova|HUD")
	bool IsInventoryVisible() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|HUD")
	const FPNPlayerHUDSnapshot& GetHUDData() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|HUD")
	UPNPlayerHUDComponent* GetHUDComponent() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "ProjectNova|HUD")
	void BP_OnHUDDataUpdated(const FPNPlayerHUDSnapshot& HUDData);

	UFUNCTION(BlueprintImplementableEvent, Category = "ProjectNova|HUD")
	void BP_OnInventoryVisibilityChanged(bool bVisible);

protected:
	void BindToHUDComponent();
	void UnbindFromHUDComponent();

	void RebuildNativeWidgetTree();
	void BuildStatsPanel();
	void BuildQuickSlotsPanel();
	void BuildInventoryPanel();

	FPNHUDStatWidgetRefs CreateStatRow(UVerticalBox* Parent, const FText& Label);
	FPNHUDInventoryPanelWidgetRefs CreateInventoryPanel(UHorizontalBox* Parent, const FText& Header);

	void UpdateNativeLayout();
	void UpdateStatsPanel();
	void UpdateQuickSlotsPanel();
	void UpdateInventoryPanel(const FPNHUDInventoryPanelData& Data, FPNHUDInventoryPanelWidgetRefs& Refs, bool bForceVisible);
	void AddInventoryItemRow(UScrollBox* ItemList, const FPNHUDInventoryItemData& ItemData);

	void SetStatWidget(FPNHUDStatWidgetRefs& Refs, const FPNHUDValuePercent& Value);
	void SetQuickSlotWidget(FPNHUDQuickSlotWidgetRefs& Refs, const FPNHUDQuickSlotData& QuickSlotData);

	FText MakeValueText(const FPNHUDValuePercent& Value) const;
	FText MakeItemText(const FPNHUDItemViewData& Item) const;
	FString GetItemNameString(const FPNHUDItemViewData& Item) const;

	void SetItemIcon(UImage* Image, const FPNHUDItemViewData& Item) const;

	UFUNCTION()
	void HandleHUDDataChanged(const FPNPlayerHUDSnapshot& HUDData);
};