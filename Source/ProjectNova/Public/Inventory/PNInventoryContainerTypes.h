#pragma once

#include "CoreMinimal.h"
#include "Inventory/PNInventoryTypes.h"
#include "PNInventoryContainerTypes.generated.h"

class UPNInventoryComponent;
class UPNItemDataAsset;

UENUM(BlueprintType)
enum class EPNInventoryContainerOperationResult : uint8
{
	Success                    UMETA(DisplayName = "Success"),
	PartialTransfer            UMETA(DisplayName = "Partial Transfer"),

	InvalidContainerComponent  UMETA(DisplayName = "Invalid Container Component"),
	InvalidOwner               UMETA(DisplayName = "Invalid Owner"),
	InvalidOwnerInventory      UMETA(DisplayName = "Invalid Owner Inventory"),
	InvalidSourceInventory     UMETA(DisplayName = "Invalid Source Inventory"),
	InvalidTargetInventory     UMETA(DisplayName = "Invalid Target Inventory"),
	InvalidContainerInventory  UMETA(DisplayName = "Invalid Container Inventory"),

	InvalidItem                UMETA(DisplayName = "Invalid Item"),
	SourceEmpty                UMETA(DisplayName = "Source Empty"),
	InvalidQuantity            UMETA(DisplayName = "Invalid Quantity"),

	SourceRemoveFailed         UMETA(DisplayName = "Source Remove Failed"),
	TargetAddFailed            UMETA(DisplayName = "Target Add Failed"),
	RollbackFailed             UMETA(DisplayName = "Rollback Failed"),

	SameInventory              UMETA(DisplayName = "Same Inventory"),
	ContainerNotOpen           UMETA(DisplayName = "Container Not Open"),
	NotAuthority               UMETA(DisplayName = "Not Authority"),

	UnknownError               UMETA(DisplayName = "Unknown Error")
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryContainerOpenResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	EPNInventoryContainerOperationResult Result = EPNInventoryContainerOperationResult::UnknownError;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	TObjectPtr<UPNInventoryComponent> OwnerInventory = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	TObjectPtr<UPNInventoryComponent> ContainerInventory = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	EPNInventoryType ContainerType = EPNInventoryType::None;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryContainerTransferRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	TObjectPtr<UPNInventoryComponent> SourceInventory = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	TObjectPtr<UPNInventoryComponent> TargetInventory = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	FPNInventoryGridPosition SourcePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	FPNInventoryGridPosition TargetPosition;

	// -1 = весь стак.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	int32 Quantity = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	bool bHalfStack = false;

	// false = положить автоматически в свободное место.
	// true = положить в TargetPosition.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	bool bUseTargetPosition = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	bool bTargetRotated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	bool bAllowStack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	bool bAutoRotate = true;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNInventoryContainerTransferResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	bool bPartial = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	EPNInventoryContainerOperationResult Result = EPNInventoryContainerOperationResult::UnknownError;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	TObjectPtr<UPNInventoryComponent> SourceInventory = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	TObjectPtr<UPNInventoryComponent> TargetInventory = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	FPNInventoryGridPosition SourcePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	FPNInventoryGridPosition TargetPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	int32 RequestedQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	int32 MovedQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	int32 RemainingQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Container")
	TObjectPtr<UPNItemDataAsset> ItemData = nullptr;
};