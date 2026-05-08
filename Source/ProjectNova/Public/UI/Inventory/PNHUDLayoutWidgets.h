#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/PNHUDTypes.h"
#include "PNHUDLayoutWidgets.generated.h"

class UBorder;
class UCanvasPanel;
class UHorizontalBox;
class UImage;
class UOverlay;
class UPNItemDataAsset;
class USizeBox;
class UTextBlock;
class UTexture2D;
class UUniformGridPanel;
class UVerticalBox;
class UWidget;

UENUM(BlueprintType)
enum class EPNInventoryHUDPage : uint8
{
	None          UMETA(DisplayName = "None"),
	Customization UMETA(DisplayName = "Customization"),
	Inventory     UMETA(DisplayName = "Inventory"),
	Craft         UMETA(DisplayName = "Craft"),
	Missions      UMETA(DisplayName = "Missions"),
	Map           UMETA(DisplayName = "Map"),
	Options       UMETA(DisplayName = "Options")
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDPlayerInfoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Player Info")
	FText PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Player Info")
	FText RankText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Player Info")
	FText ClanName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Player Info")
	int32 GameDollars = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Player Info")
	int32 GameCoins = 0;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDInventoryGridSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory Slot")
	EPNHUDInventoryPanel Panel = EPNHUDInventoryPanel::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory Slot")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory Slot")
	FPNInventoryGridPosition Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory Slot")
	bool bUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory Slot")
	bool bOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory Slot")
	bool bRootItemSlot = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory Slot")
	FPNHUDInventoryItemData OccupyingItem;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDInventoryItemVisualData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory Item")
	FPNHUDInventoryItemData ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory Item")
	FVector2D PixelPosition = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory Item")
	FVector2D PixelSize = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory Item")
	int32 Layer = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPNHUDNavigationPageRequestedSignature, EPNInventoryHUDPage, Page);

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNHUDLayoutWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPNHUDLayoutWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HUD")
	FPNPlayerHUDSnapshot CachedHUDData;

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	virtual void SetHUDData(const FPNPlayerHUDSnapshot& InHUDData);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	virtual void SetLayoutVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category = "HUD")
	const FPNPlayerHUDSnapshot& GetCachedHUDData() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnHUDDataUpdated(const FPNPlayerHUDSnapshot& HUDData);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnLayoutVisibilityChanged(bool bVisible);
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNNavigationLayoutWidget : public UPNHUDLayoutWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Navigation")
	FPNHUDNavigationPageRequestedSignature OnNavigationPageRequested;

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Navigation")
	EPNInventoryHUDPage ActivePage = EPNInventoryHUDPage::Inventory;

public:
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void RequestPage(EPNInventoryHUDPage Page);

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void SetActivePage(EPNInventoryHUDPage Page);

	UFUNCTION(BlueprintPure, Category = "Navigation")
	EPNInventoryHUDPage GetActivePage() const;

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void RequestCustomizationPage();

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void RequestInventoryPage();

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void RequestCraftPage();

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void RequestMissionsPage();

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void RequestMapPage();

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void RequestOptionsPage();

	UFUNCTION(BlueprintImplementableEvent, Category = "Navigation")
	void BP_OnActivePageChanged(EPNInventoryHUDPage Page);
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNInventoryGridWidget : public UPNHUDLayoutWidget
{
	GENERATED_BODY()

public:
	UPNInventoryGridWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory")
	FPNHUDInventoryPanelData InventoryData;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Native Render")
	bool bBuildNativeGrid = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Native Render")
	bool bPreviewInDesigner = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Header")
	FText DisplayTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Header")
	TSoftObjectPtr<UTexture2D> DisplayIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid", meta = (ClampMin = "1"))
	int32 MaxSupportedSlotCount = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid", meta = (ClampMin = "1"))
	int32 DefaultVisualColumns = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid")
	bool bUseInventoryColumnsWhenActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid", meta = (ClampMin = "8.0"))
	float SlotSize = 64.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid", meta = (ClampMin = "0.0"))
	float SlotPadding = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid", meta = (ClampMin = "0.0"))
	float HeaderHeight = 34.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid", meta = (ClampMin = "0.0"))
	float RootPadding = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Preview", meta = (ClampMin = "0"))
	int32 PreviewUnlockedSlots = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Preview", meta = (ClampMin = "0"))
	int32 PreviewUsedSlots = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Textures")
	TSoftObjectPtr<UTexture2D> RootBackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Textures")
	TSoftObjectPtr<UTexture2D> HeaderBackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Textures")
	TSoftObjectPtr<UTexture2D> SlotUnlockedTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Textures")
	TSoftObjectPtr<UTexture2D> SlotLockedTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Textures")
	TSoftObjectPtr<UTexture2D> SlotOccupiedTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Colors")
	FLinearColor RootBackgroundColor = FLinearColor(0.015f, 0.016f, 0.018f, 0.92f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Colors")
	FLinearColor HeaderBackgroundColor = FLinearColor(0.025f, 0.028f, 0.032f, 0.96f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Colors")
	FLinearColor SlotUnlockedColor = FLinearColor(0.035f, 0.04f, 0.045f, 0.92f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Colors")
	FLinearColor SlotLockedColor = FLinearColor(0.006f, 0.007f, 0.008f, 0.22f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Colors")
	FLinearColor SlotOccupiedColor = FLinearColor(0.08f, 0.09f, 0.10f, 0.96f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Colors")
	FLinearColor TextColor = FLinearColor(0.88f, 0.92f, 0.94f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Colors")
	FLinearColor CounterTextColor = FLinearColor(0.72f, 0.76f, 0.78f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Style", meta = (ClampMin = "0.0"))
	float HeaderAccentWidth = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Style", meta = (ClampMin = "1"))
	int32 TitleFontSize = 18;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Style", meta = (ClampMin = "1"))
	int32 CounterFontSize = 18;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Style", meta = (ClampMin = "0.0"))
	float HeaderBottomSpacing = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Style", meta = (ClampMin = "0.0"))
	float GridFramePadding = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Style")
	FLinearColor HeaderAccentColor = FLinearColor(0.96f, 0.28f, 0.24f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Style")
	FLinearColor GridFrameColor = FLinearColor(0.18f, 0.18f, 0.18f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Style")
	FLinearColor GridBackgroundColor = FLinearColor(0.28f, 0.28f, 0.28f, 1.0f);

protected:
	UPROPERTY(Transient)
	TObjectPtr<USizeBox> RootSizeBox = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> RootBorder = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> RootVerticalBox = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> HeaderBox = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UImage> HeaderIconImage = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HeaderTitleText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HeaderCounterText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USizeBox> GridSizeBox = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UOverlay> GridOverlay = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUniformGridPanel> SlotGridPanel = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> ItemCanvasPanel = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> HeaderBorder = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> GridBorder = nullptr;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void SetInventoryData(const FPNHUDInventoryPanelData& InInventoryData);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Native Render")
	void RefreshNativeGrid();

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const FPNHUDInventoryPanelData& GetInventoryData() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetColumns() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetRows() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetSlotSize() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Header")
	FText GetDisplayTitle() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Header")
	FText GetSlotCounterText() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Header")
	int32 GetUnlockedSlotCount() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Header")
	int32 GetMaxVisualSlotCount() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Header")
	int32 GetOccupiedItemCount() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Header")
	int32 GetUsedSlotAreaCount() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	int32 GetVisualColumns() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	int32 GetVisualRows() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	FVector2D GetVisualGridSize() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	TArray<FPNHUDInventoryGridSlotData> GetVisualGridSlots() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	TArray<FPNHUDInventoryItemVisualData> GetVisualItemData() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	bool IsInventoryUnlocked() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void BP_OnInventoryDataUpdated(const FPNHUDInventoryPanelData& InInventoryData);

protected:
	void BuildPreviewInventoryData();
	void BuildNativeGridRoot();
	void UpdateNativeHeader();
	void RebuildNativeSlots();
	void RebuildNativeItems();
	void ApplyTextureToImage(UImage* TargetImage, UTexture2D* Texture, const FVector2D& ImageSize) const;
	void ApplyTextureToBorder(UBorder* TargetBorder, UTexture2D* Texture, const FLinearColor& Color, const FVector2D& ImageSize) const;

	bool FindItemAtPosition(const FPNInventoryGridPosition& Position, FPNHUDInventoryItemData& OutItem, bool& bOutRootSlot) const;
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNInventoryGridSlotWidget : public UPNHUDLayoutWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory Slot")
	FPNHUDInventoryGridSlotData SlotData;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
	void SetSlotData(const FPNHUDInventoryGridSlotData& InSlotData);

	UFUNCTION(BlueprintPure, Category = "Inventory Slot")
	const FPNHUDInventoryGridSlotData& GetSlotData() const;

	UFUNCTION(BlueprintPure, Category = "Inventory Slot")
	bool IsUnlocked() const;

	UFUNCTION(BlueprintPure, Category = "Inventory Slot")
	bool IsOccupied() const;

	UFUNCTION(BlueprintPure, Category = "Inventory Slot")
	bool IsRootItemSlot() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Slot")
	void BP_OnSlotDataUpdated(const FPNHUDInventoryGridSlotData& InSlotData);
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNInventoryItemWidget : public UPNHUDLayoutWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory Item")
	FPNHUDInventoryItemData InventoryItemData;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory Item")
	void SetInventoryItemData(const FPNHUDInventoryItemData& InItemData);

	UFUNCTION(BlueprintPure, Category = "Inventory Item")
	const FPNHUDInventoryItemData& GetInventoryItemData() const;

	UFUNCTION(BlueprintPure, Category = "Inventory Item")
	FPNHUDItemViewData GetItemViewData() const;

	UFUNCTION(BlueprintPure, Category = "Inventory Item")
	UPNItemDataAsset* GetItemData() const;

	UFUNCTION(BlueprintPure, Category = "Inventory Item")
	FText GetItemName() const;

	UFUNCTION(BlueprintPure, Category = "Inventory Item")
	int32 GetQuantity() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Item")
	void BP_OnInventoryItemDataUpdated(const FPNHUDInventoryItemData& InItemData);
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNEquipmentLayoutWidget : public UPNHUDLayoutWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Equipment")
	FPNHUDEquipmentData EquipmentData;

public:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetEquipmentData(const FPNHUDEquipmentData& InEquipmentData);

	UFUNCTION(BlueprintPure, Category = "Equipment")
	const FPNHUDEquipmentData& GetEquipmentData() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Equipment")
	void BP_OnEquipmentDataUpdated(const FPNHUDEquipmentData& InEquipmentData);
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNEquipmentSlotWidget : public UPNHUDLayoutWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Equipment Slot")
	FPNHUDEquipmentSlotData EquipmentSlotData;

public:
	UFUNCTION(BlueprintCallable, Category = "Equipment Slot")
	void SetEquipmentSlotData(const FPNHUDEquipmentSlotData& InSlotData);

	UFUNCTION(BlueprintPure, Category = "Equipment Slot")
	const FPNHUDEquipmentSlotData& GetEquipmentSlotData() const;

	UFUNCTION(BlueprintPure, Category = "Equipment Slot")
	UPNItemDataAsset* GetItemData() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Equipment Slot")
	void BP_OnEquipmentSlotDataUpdated(const FPNHUDEquipmentSlotData& InSlotData);
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNInternalEquipmentSlotWidget : public UPNHUDLayoutWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Internal Equipment Slot")
	FPNHUDInternalEquipmentSlotData InternalSlotData;

public:
	UFUNCTION(BlueprintCallable, Category = "Internal Equipment Slot")
	void SetInternalEquipmentSlotData(const FPNHUDInternalEquipmentSlotData& InSlotData);

	UFUNCTION(BlueprintPure, Category = "Internal Equipment Slot")
	const FPNHUDInternalEquipmentSlotData& GetInternalEquipmentSlotData() const;

	UFUNCTION(BlueprintPure, Category = "Internal Equipment Slot")
	UPNItemDataAsset* GetItemData() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Internal Equipment Slot")
	void BP_OnInternalEquipmentSlotDataUpdated(const FPNHUDInternalEquipmentSlotData& InSlotData);
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNContainerLayoutWidget : public UPNHUDLayoutWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Container")
	FPNHUDContainerData ContainerData;

public:
	UFUNCTION(BlueprintCallable, Category = "Container")
	void SetContainerData(const FPNHUDContainerData& InContainerData);

	UFUNCTION(BlueprintPure, Category = "Container")
	const FPNHUDContainerData& GetContainerData() const;

	UFUNCTION(BlueprintPure, Category = "Container")
	bool IsContainerOpen() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Container")
	void BP_OnContainerDataUpdated(const FPNHUDContainerData& InContainerData);
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNQuickSlotLayoutWidget : public UPNHUDLayoutWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Quick Slots")
	TArray<FPNHUDQuickSlotData> QuickSlotsData;

public:
	UFUNCTION(BlueprintCallable, Category = "Quick Slots")
	void SetQuickSlotsData(const TArray<FPNHUDQuickSlotData>& InQuickSlotsData);

	UFUNCTION(BlueprintPure, Category = "Quick Slots")
	const TArray<FPNHUDQuickSlotData>& GetQuickSlotsData() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Quick Slots")
	void BP_OnQuickSlotsDataUpdated(const TArray<FPNHUDQuickSlotData>& InQuickSlotsData);
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNQuickSlotWidget : public UPNHUDLayoutWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Quick Slot")
	FPNHUDQuickSlotData QuickSlotData;

public:
	UFUNCTION(BlueprintCallable, Category = "Quick Slot")
	void SetQuickSlotData(const FPNHUDQuickSlotData& InQuickSlotData);

	UFUNCTION(BlueprintPure, Category = "Quick Slot")
	const FPNHUDQuickSlotData& GetQuickSlotData() const;

	UFUNCTION(BlueprintPure, Category = "Quick Slot")
	UPNItemDataAsset* GetItemData() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Quick Slot")
	void BP_OnQuickSlotDataUpdated(const FPNHUDQuickSlotData& InQuickSlotData);
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNPlayerStatsLayoutWidget : public UPNHUDLayoutWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Player Stats")
	FPNHUDCharacterStatsData StatsData;

public:
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void SetStatsData(const FPNHUDCharacterStatsData& InStatsData);

	UFUNCTION(BlueprintPure, Category = "Player Stats")
	const FPNHUDCharacterStatsData& GetStatsData() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Stats")
	void BP_OnStatsDataUpdated(const FPNHUDCharacterStatsData& InStatsData);
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTNOVA_API UPNPlayerInfoLayoutWidget : public UPNHUDLayoutWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Player Info")
	FPNHUDPlayerInfoData PlayerInfoData;

public:
	UFUNCTION(BlueprintCallable, Category = "Player Info")
	void SetPlayerInfoData(const FPNHUDPlayerInfoData& InPlayerInfoData);

	UFUNCTION(BlueprintPure, Category = "Player Info")
	const FPNHUDPlayerInfoData& GetPlayerInfoData() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Info")
	void BP_OnPlayerInfoDataUpdated(const FPNHUDPlayerInfoData& InPlayerInfoData);
};