#pragma once

#include "CoreMinimal.h"
#include "Items/PNInventoryTypes.h"
#include "PNEquipmentTypes.generated.h"

class UPNItemDataAsset;

UENUM(BlueprintType)
enum class EPNEquipmentSlot : uint8
{
	None             UMETA(DisplayName = "None"),

	Helmet           UMETA(DisplayName = "Helmet"),
	Gloves           UMETA(DisplayName = "Gloves"),
	Armor            UMETA(DisplayName = "Armor"),
	Backpack         UMETA(DisplayName = "Backpack"),

	PrimaryWeapon1   UMETA(DisplayName = "Primary Weapon 1"),
	PrimaryWeapon2   UMETA(DisplayName = "Primary Weapon 2"),
	Sidearm          UMETA(DisplayName = "Sidearm"),
	Knife            UMETA(DisplayName = "Knife")
};

UENUM(BlueprintType)
enum class EPNEquipmentInternalContainer : uint8
{
	None   UMETA(DisplayName = "None"),

	Helmet UMETA(DisplayName = "Helmet Internal"),
	Armor  UMETA(DisplayName = "Armor Internal")
};

UENUM(BlueprintType)
enum class EPNEquipmentOperationResult : uint8
{
	Success                    UMETA(DisplayName = "Success"),

	InvalidEquipment            UMETA(DisplayName = "Invalid Equipment"),
	InvalidInventory            UMETA(DisplayName = "Invalid Inventory"),
	InvalidSlot                 UMETA(DisplayName = "Invalid Slot"),
	InvalidInternalContainer    UMETA(DisplayName = "Invalid Internal Container"),
	InvalidInternalSlotIndex    UMETA(DisplayName = "Invalid Internal Slot Index"),
	InvalidItem                 UMETA(DisplayName = "Invalid Item"),

	SlotEmpty                   UMETA(DisplayName = "Slot Empty"),
	SlotOccupied                UMETA(DisplayName = "Slot Occupied"),
	TopSlotRequired             UMETA(DisplayName = "Top Slot Required"),
	InternalSlotLocked          UMETA(DisplayName = "Internal Slot Locked"),
	InternalItemsNotEmpty       UMETA(DisplayName = "Internal Items Not Empty"),

	ItemTypeNotAllowed          UMETA(DisplayName = "Item Type Not Allowed"),

	InventoryRemoveFailed       UMETA(DisplayName = "Inventory Remove Failed"),
	InventoryAddFailed          UMETA(DisplayName = "Inventory Add Failed"),

	NoSpace                     UMETA(DisplayName = "No Space"),
	OverWeight                  UMETA(DisplayName = "Over Weight"),

	UnknownError                UMETA(DisplayName = "Unknown Error")
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNEquipmentSlotEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EPNEquipmentSlot Slot = EPNEquipmentSlot::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FPNRepItemInstanceData InstanceData;

	bool IsOccupied() const
	{
		return Slot != EPNEquipmentSlot::None && InstanceData.IsValid();
	}

	bool IsEmpty() const
	{
		return !IsOccupied();
	}
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNEquipmentInternalSlotEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EPNEquipmentInternalContainer Container = EPNEquipmentInternalContainer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment", meta = (ClampMin = "0"))
	int32 SlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FPNRepItemInstanceData InstanceData;

	bool IsOccupied() const
	{
		return Container != EPNEquipmentInternalContainer::None && InstanceData.IsValid();
	}

	bool IsEmpty() const
	{
		return !IsOccupied();
	}
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNEquipmentOperationResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Result")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Result")
	EPNEquipmentOperationResult Result = EPNEquipmentOperationResult::UnknownError;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Result")
	EPNEquipmentSlot Slot = EPNEquipmentSlot::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Result")
	EPNEquipmentInternalContainer InternalContainer = EPNEquipmentInternalContainer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Result")
	int32 InternalSlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Result")
	TObjectPtr<UPNItemDataAsset> ItemData = nullptr;
};