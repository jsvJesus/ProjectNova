#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/PNHUDTypes.h"
#include "PNHUDLayoutWidgets.generated.h"

class UPNItemDataAsset;

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

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory")
	FPNHUDInventoryPanelData InventoryData;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetInventoryData(const FPNHUDInventoryPanelData& InInventoryData);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const FPNHUDInventoryPanelData& GetInventoryData() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetColumns() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetRows() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetSlotSize() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void BP_OnInventoryDataUpdated(const FPNHUDInventoryPanelData& InInventoryData);
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