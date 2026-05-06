#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Inventory/PNLootTableTypes.h"
#include "PNLootTableDataAsset.generated.h"

class UPNItemDataAsset;
class UPNItemInstance;

UCLASS(BlueprintType)
class PROJECTNOVA_API UPNLootTableDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table|Main")
	FText LootTableName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table|Main", meta = (MultiLine = "true"))
	FText Description;

	// Общий шанс, что таблица вообще что-то создаст.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table|Rules", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float TableChancePercent = 100.0f;

	// Защита от переполнения. Например, холодильник не должен генерить 50 стаков.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table|Rules", meta = (ClampMin = "0"))
	int32 MaxGeneratedStacks = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table|Rules")
	TArray<FPNLootTableCategoryRule> Categories;

public:
	UFUNCTION(BlueprintCallable, Category = "Loot Table")
	TArray<UPNItemInstance*> GenerateLoot(UObject* Outer) const;

	UFUNCTION(BlueprintCallable, Category = "Loot Table")
	TArray<UPNItemInstance*> GenerateLootWithSeed(UObject* Outer, int32 Seed) const;

	UFUNCTION(BlueprintPure, Category = "Loot Table")
	bool DoesItemMatchSpawnCategory(UPNItemDataAsset* ItemData, EPNLootSpawnCategory Category) const;

protected:
	TArray<UPNItemInstance*> GenerateLootInternal(UObject* Outer, FRandomStream& RandomStream) const;

	const FPNLootTableEntry* PickEntryFromCategory(const FPNLootTableCategoryRule& CategoryRule, FRandomStream& RandomStream) const;

	UPNItemInstance* BuildLootItemInstance(UObject* Outer, const FPNLootTableEntry& Entry, FRandomStream& RandomStream) const;

	bool RollChance(float ChancePercent, FRandomStream& RandomStream) const;

	int32 RollInt(int32 MinValue, int32 MaxValue, FRandomStream& RandomStream) const;

	float RollFloat(float MinValue, float MaxValue, FRandomStream& RandomStream) const;

	bool IsPrimaryWeaponCategory(EPNItemCategory Category) const;

	bool IsPistolCategory(EPNItemCategory Category) const;
};