#pragma once

#include "CoreMinimal.h"
#include "Equipment/PNEquipmentTypes.h"
#include "Inventory/PNInventoryTypes.h"
#include "PNHUDTypes.generated.h"

class UPNItemDataAsset;
class UTexture2D;

UENUM(BlueprintType)
enum class EPNHUDInventoryPanel : uint8
{
	None            UMETA(DisplayName = "None"),
	MainInventory   UMETA(DisplayName = "Main Inventory"),
	OpenedContainer UMETA(DisplayName = "Opened Container")
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDValuePercent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	float Current = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	float Max = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	float Percent = 0.0f;

	void Set(float InCurrent, float InMax)
	{
		Current = InCurrent;
		Max = InMax;
		Percent = Max > 0.0f ? FMath::Clamp(Current / Max, 0.0f, 1.0f) : 0.0f;
	}
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDItemViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Item")
	bool bValid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Item")
	TObjectPtr<UPNItemDataAsset> ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Item")
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Item")
	float TotalWeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Item")
	float DurabilityPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Item")
	float BatteryPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Item")
	float ExpirationPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Item")
	int32 AmmoInMagazine = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Item")
	bool bBroken = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Item")
	bool bExpired = false;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDInventoryItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	FPNHUDItemViewData Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	FPNInventoryGridPosition Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	FPNInventoryItemSize Size;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	bool bRotated = false;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDInventoryPanelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	EPNHUDInventoryPanel Panel = EPNHUDInventoryPanel::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	EPNInventoryType InventoryType = EPNInventoryType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	FText DisplayTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	TSoftObjectPtr<UTexture2D> DisplayIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	int32 Columns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	int32 Rows = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	int32 SlotCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	int32 MaxVisualSlotCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	bool bUsesWeightLimit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	bool bCanReceiveItems = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	bool bCanRemoveItems = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	FPNHUDValuePercent Weight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Inventory")
	TArray<FPNHUDInventoryItemData> Items;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDQuickSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Quick Slot")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Quick Slot")
	bool bValidSlot = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Quick Slot")
	bool bOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Quick Slot")
	bool bSelected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Quick Slot")
	FPNHUDItemViewData Item;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDEquipmentSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Equipment")
	EPNEquipmentSlot Slot = EPNEquipmentSlot::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Equipment")
	bool bOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Equipment")
	FPNHUDItemViewData Item;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDInternalEquipmentSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Equipment")
	EPNEquipmentInternalContainer Container = EPNEquipmentInternalContainer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Equipment")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Equipment")
	bool bUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Equipment")
	bool bOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Equipment")
	FPNHUDItemViewData Item;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDEquipmentData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Equipment")
	TArray<FPNHUDEquipmentSlotData> EquipmentSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Equipment")
	TArray<FPNHUDInternalEquipmentSlotData> HelmetInternalSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Equipment")
	TArray<FPNHUDInternalEquipmentSlotData> ArmorInternalSlots;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDCharacterStatsData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent Hunger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent Thirst;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent Weight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent Radiation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent Toxicity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent Psy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent Bleeding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent Wounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent Burn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent ChemicalBurn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	FPNHUDValuePercent ElectricShock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	bool bIsDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	bool bIsSprinting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Stats")
	bool bCanSprint = false;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNHUDContainerData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Container")
	bool bIsOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Container")
	EPNInventoryType OpenedContainerType = EPNInventoryType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Container")
	FPNHUDInventoryPanelData Inventory;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNPlayerHUDSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bHasOwnerCharacter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bHasInventory = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bHasStats = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bHasEquipment = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bHasQuickSlots = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bHasContainerComponent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FPNHUDCharacterStatsData Stats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FPNHUDInventoryPanelData MainInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TArray<FPNHUDQuickSlotData> QuickSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FPNHUDEquipmentData Equipment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FPNHUDContainerData Container;
};