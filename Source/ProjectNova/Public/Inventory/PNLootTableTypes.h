#pragma once

#include "CoreMinimal.h"
#include "Items/PNItemTypes.h"
#include "PNLootTableTypes.generated.h"

class UPNItemDataAsset;

UENUM(BlueprintType)
enum class EPNLootSpawnCategory : uint8
{
	None                    UMETA(DisplayName = "None"),

	// Human / civilian containers
	Household               UMETA(DisplayName = "Household"),
	Kitchen                 UMETA(DisplayName = "Kitchen"),
	Medical                 UMETA(DisplayName = "Medical"),
	Workshop                UMETA(DisplayName = "Workshop"),
	Industrial              UMETA(DisplayName = "Industrial"),
	Military                UMETA(DisplayName = "Military"),

	// Direct item buckets
	FoodWaterMedicine       UMETA(DisplayName = "Food / Water / Medicine"),
	Food                    UMETA(DisplayName = "Food"),
	Water                   UMETA(DisplayName = "Water"),
	Medicine                UMETA(DisplayName = "Medicine"),

	AmmoAndMagazines        UMETA(DisplayName = "Ammo / Magazines"),
	AllWeapons              UMETA(DisplayName = "All Weapons"),
	PrimaryWeapons          UMETA(DisplayName = "Primary Weapons"),
	Pistols                 UMETA(DisplayName = "Pistols"),
	Melee                   UMETA(DisplayName = "Melee"),

	Armor                   UMETA(DisplayName = "Armor"),
	Helmet                  UMETA(DisplayName = "Helmet"),
	Backpack                UMETA(DisplayName = "Backpack"),
	AllGear                 UMETA(DisplayName = "All Gear"),

	WeaponAttachments       UMETA(DisplayName = "Weapon Attachments"),
	ArmorAttachments        UMETA(DisplayName = "Armor Attachments"),

	CraftItems              UMETA(DisplayName = "Craft Items"),
	Components              UMETA(DisplayName = "Components"),
	Resources               UMETA(DisplayName = "Resources"),
	Recipes                 UMETA(DisplayName = "Recipes"),
	UsableDevices           UMETA(DisplayName = "Usable Devices"),
	BuildParts              UMETA(DisplayName = "Build Parts"),

	MiscItems               UMETA(DisplayName = "Misc Items")
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNLootRuntimeRollSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Runtime", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinDurabilityPercent = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Runtime", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxDurabilityPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Runtime", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinBatteryPercent = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Runtime", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxBatteryPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Runtime", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinShelfLifePercent = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Runtime", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxShelfLifePercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Runtime")
	bool bRandomizeMagazineAmmo = true;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNLootTableEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Entry")
	TObjectPtr<UPNItemDataAsset> ItemData = nullptr;

	// Вес внутри категории. Чем выше — тем чаще выбирается.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Entry", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;

	// Дополнительный шанс конкретного предмета после выбора категории.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Entry", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float ChancePercent = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Entry", meta = (ClampMin = "1"))
	int32 MinQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Entry", meta = (ClampMin = "1"))
	int32 MaxQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Entry")
	bool bClampQuantityToMaxStack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Entry")
	FPNLootRuntimeRollSettings RuntimeRollSettings;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNLootTableCategoryRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Category")
	EPNLootSpawnCategory SpawnCategory = EPNLootSpawnCategory::None;

	// Общий шанс запуска этой категории.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Category", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float CategoryChancePercent = 100.0f;

	// Сколько раз категория попробует выбрать предмет.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Category", meta = (ClampMin = "0"))
	int32 MinRolls = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Category", meta = (ClampMin = "0"))
	int32 MaxRolls = 1;

	// true = предмет должен подходить под SpawnCategory по ItemType/ItemCategory.
	// false = можно руками положить любой ItemData.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Category")
	bool bEnforceCategoryFilter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Category")
	TArray<FPNLootTableEntry> Entries;
};