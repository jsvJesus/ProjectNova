#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Inventory/PNInventoryTypes.h"
#include "PNLootContainerProfileDataAsset.generated.h"

class UPNLootTableDataAsset;
class UStaticMesh;

UENUM(BlueprintType)
enum class EPNLootContainerProfileType : uint8
{
	None                UMETA(DisplayName = "None"),

	Fridge              UMETA(DisplayName = "Fridge"),
	KitchenCabinet      UMETA(DisplayName = "Kitchen Cabinet"),
	MedicineCabinet     UMETA(DisplayName = "Medicine Cabinet"),
	CivilianCabinet     UMETA(DisplayName = "Civilian Cabinet"),
	CivilianShelf       UMETA(DisplayName = "Civilian Shelf"),

	WorkshopShelf       UMETA(DisplayName = "Workshop Shelf"),
	ToolBox             UMETA(DisplayName = "Tool Box"),
	IndustrialCrate     UMETA(DisplayName = "Industrial Crate"),

	MilitaryCrate       UMETA(DisplayName = "Military Crate"),
	WeaponCrate         UMETA(DisplayName = "Weapon Crate"),
	AmmoBox             UMETA(DisplayName = "Ammo Box"),
	GearCrate           UMETA(DisplayName = "Gear Crate"),

	BackpackCache       UMETA(DisplayName = "Backpack Cache"),
	ArmorCache          UMETA(DisplayName = "Armor Cache"),

	TrashPile           UMETA(DisplayName = "Trash Pile"),
	StashBag            UMETA(DisplayName = "Stash Bag"),
	Custom              UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNLootContainerRespawnProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot Profile|Respawn")
	bool bSpawnLootOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot Profile|Respawn")
	bool bClearInventoryBeforeLootRespawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot Profile|Respawn")
	bool bRespawnWhenEmpty = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot Profile|Respawn", meta = (ClampMin = "0.0"))
	float LootRespawnSeconds = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot Profile|Respawn")
	bool bCanOpenEmptyLootContainer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot Profile|Random")
	bool bUseFixedLootSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot Profile|Random", meta = (EditCondition = "bUseFixedLootSeed"))
	int32 FixedLootSeed = 1337;
};

UCLASS(BlueprintType)
class PROJECTNOVA_API UPNLootContainerProfileDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPNLootContainerProfileDataAsset();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Profile|Main")
	EPNLootContainerProfileType ProfileType = EPNLootContainerProfileType::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Profile|Main")
	FText ProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Profile|Main", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Profile|Interaction")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Profile|Interaction")
	FText ActionText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Profile|Visual")
	TObjectPtr<UStaticMesh> ContainerMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Profile|Inventory")
	EPNInventoryType ContainerType = EPNInventoryType::LootContainer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Profile|Inventory")
	FPNInventorySettings ContainerSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Profile|Loot Table")
	TObjectPtr<UPNLootTableDataAsset> LootTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Profile|Respawn")
	FPNLootContainerRespawnProfile RespawnProfile;

public:
	UFUNCTION(BlueprintCallable, Category = "Loot Profile")
	void ApplyDefaultPreset();

	UFUNCTION(BlueprintPure, Category = "Loot Profile")
	bool IsValidLootProfile() const;

protected:
	void ApplyLootOnlyRules();

	void SetProfileInventorySize(int32 Columns, int32 Rows);
};