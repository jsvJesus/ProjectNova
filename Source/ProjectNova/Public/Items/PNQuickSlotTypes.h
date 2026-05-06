#pragma once

#include "CoreMinimal.h"
#include "Inventory/PNInventoryTypes.h"
#include "PNQuickSlotTypes.generated.h"

class UPNItemDataAsset;

UENUM(BlueprintType)
enum class EPNQuickSlotOperationResult : uint8
{
	Success                    UMETA(DisplayName = "Success"),

	InvalidComponent            UMETA(DisplayName = "Invalid Component"),
	InvalidInventory            UMETA(DisplayName = "Invalid Inventory"),
	InvalidCharacterStats       UMETA(DisplayName = "Invalid Character Stats"),
	InvalidSlotIndex            UMETA(DisplayName = "Invalid Slot Index"),
	InvalidItem                 UMETA(DisplayName = "Invalid Item"),

	SlotEmpty                   UMETA(DisplayName = "Slot Empty"),
	SlotOccupied                UMETA(DisplayName = "Slot Occupied"),

	ItemTypeNotAllowed          UMETA(DisplayName = "Item Type Not Allowed"),
	ItemNotConsumable           UMETA(DisplayName = "Item Not Consumable"),

	InventoryRemoveFailed       UMETA(DisplayName = "Inventory Remove Failed"),
	InventoryAddFailed          UMETA(DisplayName = "Inventory Add Failed"),

	UnknownError                UMETA(DisplayName = "Unknown Error")
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNQuickSlotEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quick Slot")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quick Slot")
	FPNRepItemInstanceData InstanceData;

	bool IsValidSlot() const
	{
		return SlotIndex != INDEX_NONE;
	}

	bool IsOccupied() const
	{
		return IsValidSlot() && InstanceData.IsValid();
	}

	bool IsEmpty() const
	{
		return !IsOccupied();
	}
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNQuickSlotOperationResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quick Slot|Result")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quick Slot|Result")
	EPNQuickSlotOperationResult Result = EPNQuickSlotOperationResult::UnknownError;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quick Slot|Result")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quick Slot|Result")
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quick Slot|Result")
	TObjectPtr<UPNItemDataAsset> ItemData = nullptr;
};