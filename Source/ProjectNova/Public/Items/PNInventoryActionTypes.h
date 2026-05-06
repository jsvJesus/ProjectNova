#pragma once

#include "CoreMinimal.h"
#include "Equipment/PNEquipmentTypes.h"
#include "Items/PNInventoryTypes.h"
#include "PNInventoryActionTypes.generated.h"

class UPNItemDataAsset;

UENUM(BlueprintType)
enum class EPNInventoryActionType : uint8
{
	None        UMETA(DisplayName = "None"),

	Use         UMETA(DisplayName = "Use"),
	Equip       UMETA(DisplayName = "Equip"),
	Unequip     UMETA(DisplayName = "Unequip"),
	Drop        UMETA(DisplayName = "Drop"),
	Move        UMETA(DisplayName = "Move"),
	MergeStack  UMETA(DisplayName = "Merge Stack"),
	SplitStack  UMETA(DisplayName = "Split Stack"),
	Rotate      UMETA(DisplayName = "Rotate"),
	Inspect     UMETA(DisplayName = "Inspect")
};

UENUM(BlueprintType)
enum class EPNInventoryActionContainer : uint8
{
	None              UMETA(DisplayName = "None"),

	Inventory         UMETA(DisplayName = "Inventory"),
	QuickSlot         UMETA(DisplayName = "Quick Slot"),
	EquipmentSlot     UMETA(DisplayName = "Equipment Slot"),
	EquipmentInternal UMETA(DisplayName = "Equipment Internal Slot"),
	World             UMETA(DisplayName = "World")
};

UENUM(BlueprintType)
enum class EPNInventoryActionResult : uint8
{
	Success                 UMETA(DisplayName = "Success"),

	InvalidActionComponent  UMETA(DisplayName = "Invalid Action Component"),
	InvalidOwner            UMETA(DisplayName = "Invalid Owner"),
	InvalidInventory        UMETA(DisplayName = "Invalid Inventory"),
	InvalidEquipment        UMETA(DisplayName = "Invalid Equipment"),

	InvalidAction           UMETA(DisplayName = "Invalid Action"),
	InvalidSource           UMETA(DisplayName = "Invalid Source"),
	InvalidDestination      UMETA(DisplayName = "Invalid Destination"),
	InvalidQuantity         UMETA(DisplayName = "Invalid Quantity"),
	InvalidItem             UMETA(DisplayName = "Invalid Item"),

	SourceEmpty             UMETA(DisplayName = "Source Empty"),
	DestinationOccupied     UMETA(DisplayName = "Destination Occupied"),

	ItemNotUsable           UMETA(DisplayName = "Item Not Usable"),
	ItemNotConsumable       UMETA(DisplayName = "Item Not Consumable"),

	UseFailed               UMETA(DisplayName = "Use Failed"),
	EquipFailed             UMETA(DisplayName = "Equip Failed"),
	UnequipFailed           UMETA(DisplayName = "Unequip Failed"),
	DropFailed              UMETA(DisplayName = "Drop Failed"),
	MoveFailed              UMETA(DisplayName = "Move Failed"),
	MergeFailed             UMETA(DisplayName = "Merge Failed"),
	SplitFailed             UMETA(DisplayName = "Split Failed"),
	RotateFailed            UMETA(DisplayName = "Rotate Failed"),

	NotImplemented          UMETA(DisplayName = "Not Implemented"),
	UnknownError            UMETA(DisplayName = "Unknown Error")
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryActionTarget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	EPNInventoryActionContainer Container = EPNInventoryActionContainer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	FPNInventoryGridPosition InventoryPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	int32 QuickSlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	EPNEquipmentSlot EquipmentSlot = EPNEquipmentSlot::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	EPNEquipmentInternalContainer InternalContainer = EPNEquipmentInternalContainer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	int32 InternalSlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	bool bRotated = false;

	bool IsInventory() const
	{
		return Container == EPNInventoryActionContainer::Inventory;
	}

	bool IsQuickSlot() const
	{
		return Container == EPNInventoryActionContainer::QuickSlot;
	}

	bool IsEquipmentSlot() const
	{
		return Container == EPNInventoryActionContainer::EquipmentSlot;
	}

	bool IsEquipmentInternal() const
	{
		return Container == EPNInventoryActionContainer::EquipmentInternal;
	}
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryActionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	EPNInventoryActionType ActionType = EPNInventoryActionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	FPNInventoryActionTarget Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	FPNInventoryActionTarget Destination;

	// -1 = весь стак.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	int32 Quantity = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	bool bHalfStack = false;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryActionResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	EPNInventoryActionResult Result = EPNInventoryActionResult::UnknownError;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	EPNInventoryActionType ActionType = EPNInventoryActionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	FPNInventoryActionTarget Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	FPNInventoryActionTarget Destination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	TObjectPtr<UPNItemDataAsset> ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Action")
	FText ItemDescription;
};