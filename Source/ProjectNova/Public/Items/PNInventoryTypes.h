#pragma once

#include "CoreMinimal.h"
#include "PNInventoryTypes.generated.h"

class UPNItemDataAsset;
class UPNItemInstance;

UENUM(BlueprintType)
enum class EPNInventoryType : uint8
{
	None          UMETA(DisplayName = "None"),

	Inventory     UMETA(DisplayName = "Player Inventory"),
	Backpack      UMETA(DisplayName = "Backpack"),
	Vest          UMETA(DisplayName = "Vest"),
	LootContainer UMETA(DisplayName = "Loot Container"),
	SafeLocker    UMETA(DisplayName = "Safe Locker"),
	DeadBody      UMETA(DisplayName = "Dead Body"),
	TradeShop     UMETA(DisplayName = "Trade / Shop")
};

UENUM(BlueprintType)
enum class EPNInventoryOperationResult : uint8
{
	Success                   UMETA(DisplayName = "Success"),

	InvalidInventory          UMETA(DisplayName = "Invalid Inventory"),
	InvalidItem               UMETA(DisplayName = "Invalid Item"),
	InvalidSlot               UMETA(DisplayName = "Invalid Slot"),
	InvalidQuantity           UMETA(DisplayName = "Invalid Quantity"),

	NoSpace                   UMETA(DisplayName = "No Space"),
	OverWeight                UMETA(DisplayName = "Over Weight"),
	OutOfBounds               UMETA(DisplayName = "Out Of Bounds"),
	SlotOccupied              UMETA(DisplayName = "Slot Occupied"),
	SlotEmpty                 UMETA(DisplayName = "Slot Empty"),

	StackMismatch             UMETA(DisplayName = "Stack Mismatch"),
	StackFull                 UMETA(DisplayName = "Stack Full"),
	NotEnoughQuantity         UMETA(DisplayName = "Not Enough Quantity"),

	InventoryTypeNotAllowed   UMETA(DisplayName = "Inventory Type Not Allowed"),
	ItemTypeNotAllowed        UMETA(DisplayName = "Item Type Not Allowed"),

	SameSlot                  UMETA(DisplayName = "Same Slot"),
	UnknownError              UMETA(DisplayName = "Unknown Error")
};

UENUM(BlueprintType)
enum class EPNInventorySlotState : uint8
{
	Empty     UMETA(DisplayName = "Empty"),
	Occupied  UMETA(DisplayName = "Occupied"),
	Blocked   UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryGridPosition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Grid", meta = (ClampMin = "0"))
	int32 X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Grid", meta = (ClampMin = "0"))
	int32 Y = 0;

	bool operator==(const FPNInventoryGridPosition& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}

	bool operator!=(const FPNInventoryGridPosition& Other) const
	{
		return !(*this == Other);
	}
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryGridSize
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Grid", meta = (ClampMin = "1"))
	int32 Columns = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Grid", meta = (ClampMin = "1"))
	int32 Rows = 5;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryItemSize
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item Size", meta = (ClampMin = "1"))
	int32 Width = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item Size", meta = (ClampMin = "1"))
	int32 Height = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item Size")
	bool bRotated = false;

	int32 GetFinalWidth() const
	{
		return bRotated ? Height : Width;
	}

	int32 GetFinalHeight() const
	{
		return bRotated ? Width : Height;
	}
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNRepItemInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Replication")
	TObjectPtr<UPNItemDataAsset> ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Replication", meta = (ClampMin = "0"))
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Replication", meta = (ClampMin = "0.0"))
	float CurrentDurability = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Replication", meta = (ClampMin = "0.0"))
	float CurrentBatteryCharge = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Replication", meta = (ClampMin = "0.0"))
	float RemainingShelfLifeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Replication", meta = (ClampMin = "0"))
	int32 AmmoInMagazine = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Replication")
	bool bInitialized = false;

	bool IsValid() const
	{
		return ItemData != nullptr && Quantity > 0;
	}
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Slot")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Slot")
	FPNInventoryGridPosition Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Slot")
	EPNInventorySlotState State = EPNInventorySlotState::Empty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Slot")
	TObjectPtr<UPNItemInstance> ItemInstance = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Slot")
	bool bRootSlot = false;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryItemEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	TObjectPtr<UPNItemInstance> ItemInstance = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	FPNInventoryGridPosition Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	FPNInventoryItemSize Size;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	bool bRotated = false;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNRepInventoryItemEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Replication")
	FPNRepItemInstanceData InstanceData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Replication")
	FPNInventoryGridPosition Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Replication")
	FPNInventoryItemSize Size;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Replication")
	bool bRotated = false;

	bool IsValid() const
	{
		return InstanceData.IsValid();
	}
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryQuickSlotEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Quick Slot")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Quick Slot")
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
struct PROJECTNOVA_API FPNInventorySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	EPNInventoryType InventoryType = EPNInventoryType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	FPNInventoryGridSize GridSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	bool bUseWeightLimit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings", meta = (ClampMin = "0.0"))
	float MaxWeight = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	bool bAllowItemRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	bool bAllowStacking = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	bool bCanReceiveItems = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	bool bCanRemoveItems = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	bool bCanDropItems = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	bool bCanTradeItems = false;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryAddItemResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	EPNInventoryOperationResult Result = EPNInventoryOperationResult::UnknownError;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	FPNInventoryGridPosition Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	int32 AddedQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	int32 RemainingQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	TObjectPtr<UPNItemInstance> TargetItemInstance = nullptr;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryMoveItemResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	EPNInventoryOperationResult Result = EPNInventoryOperationResult::UnknownError;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	FPNInventoryGridPosition OldPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	FPNInventoryGridPosition NewPosition;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryRemoveItemResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	EPNInventoryOperationResult Result = EPNInventoryOperationResult::UnknownError;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	int32 RemovedQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Result")
	TObjectPtr<UPNItemInstance> RemovedItemInstance = nullptr;
};