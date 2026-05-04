#pragma once

#include "CoreMinimal.h"
#include "PNCharacterStatsTypes.generated.h"

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNCharacterAttributeStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float Endurance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float StaminaRegen = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float HealthRegen = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float Bleeding = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float WoundHealing = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float MaxWeight = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float Telepathy = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float PsiProtection = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float Burn = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float ThermalProtection = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float ChemicalBurn = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float ChemicalProtection = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float ElectricShock = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float ElectricProtection = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float Radiation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float RadiationProtection = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float Satiety = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Attributes")
	float BulletResistance = 0.0f;

	void ResetToZero()
	{
		Endurance = 0.0f;
		StaminaRegen = 0.0f;
		Health = 0.0f;
		HealthRegen = 0.0f;
		Bleeding = 0.0f;
		WoundHealing = 0.0f;
		MaxWeight = 0.0f;
		Telepathy = 0.0f;
		PsiProtection = 0.0f;
		Burn = 0.0f;
		ThermalProtection = 0.0f;
		ChemicalBurn = 0.0f;
		ChemicalProtection = 0.0f;
		ElectricShock = 0.0f;
		ElectricProtection = 0.0f;
		Radiation = 0.0f;
		RadiationProtection = 0.0f;
		Satiety = 0.0f;
		BulletResistance = 0.0f;
	}

	void Add(const FPNCharacterAttributeStats& Other)
	{
		Endurance += Other.Endurance;
		StaminaRegen += Other.StaminaRegen;
		Health += Other.Health;
		HealthRegen += Other.HealthRegen;
		Bleeding += Other.Bleeding;
		WoundHealing += Other.WoundHealing;
		MaxWeight += Other.MaxWeight;
		Telepathy += Other.Telepathy;
		PsiProtection += Other.PsiProtection;
		Burn += Other.Burn;
		ThermalProtection += Other.ThermalProtection;
		ChemicalBurn += Other.ChemicalBurn;
		ChemicalProtection += Other.ChemicalProtection;
		ElectricShock += Other.ElectricShock;
		ElectricProtection += Other.ElectricProtection;
		Radiation += Other.Radiation;
		RadiationProtection += Other.RadiationProtection;
		Satiety += Other.Satiety;
		BulletResistance += Other.BulletResistance;
	}

	void ClampFinal()
	{
		Health = FMath::Max(1.0f, Health);
		Endurance = FMath::Max(1.0f, Endurance);
		StaminaRegen = FMath::Max(0.0f, StaminaRegen);
		HealthRegen = FMath::Max(0.0f, HealthRegen);
		WoundHealing = FMath::Max(0.0f, WoundHealing);
		MaxWeight = FMath::Max(0.0f, MaxWeight);

		PsiProtection = FMath::Max(0.0f, PsiProtection);
		ThermalProtection = FMath::Max(0.0f, ThermalProtection);
		ChemicalProtection = FMath::Max(0.0f, ChemicalProtection);
		ElectricProtection = FMath::Max(0.0f, ElectricProtection);
		RadiationProtection = FMath::Max(0.0f, RadiationProtection);
		BulletResistance = FMath::Max(0.0f, BulletResistance);
	}
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNCharacterCurrentStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Current")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Current")
	float Stamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Current")
	float Hunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Current")
	float Thirst = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Current")
	float Radiation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Current")
	float Toxicity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Current")
	float Psy = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Current")
	float Bleeding = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Current")
	float Wounds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Current")
	float Burn = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Current")
	float ChemicalBurn = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Current")
	float ElectricShock = 0.0f;

	void Clamp(const FPNCharacterAttributeStats& FinalStats)
	{
		Health = FMath::Clamp(Health, 0.0f, FMath::Max(1.0f, FinalStats.Health));
		Stamina = FMath::Clamp(Stamina, 0.0f, FMath::Max(1.0f, FinalStats.Endurance));

		Hunger = FMath::Clamp(Hunger, 0.0f, 100.0f);
		Thirst = FMath::Clamp(Thirst, 0.0f, 100.0f);

		Radiation = FMath::Clamp(Radiation, 0.0f, 100.0f);
		Toxicity = FMath::Clamp(Toxicity, 0.0f, 100.0f);
		Psy = FMath::Clamp(Psy, 0.0f, 100.0f);

		Bleeding = FMath::Clamp(Bleeding, 0.0f, 100.0f);
		Wounds = FMath::Clamp(Wounds, 0.0f, 100.0f);
		Burn = FMath::Clamp(Burn, 0.0f, 100.0f);
		ChemicalBurn = FMath::Clamp(ChemicalBurn, 0.0f, 100.0f);
		ElectricShock = FMath::Clamp(ElectricShock, 0.0f, 100.0f);
	}
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNCharacterStatsSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.05"))
	float ServerTickInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float SprintStaminaDrainPerSecond = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float HungerDrainPerSecond = 0.015f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float ThirstDrainPerSecond = 0.025f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float LowHungerDamagePerSecond = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float LowThirstDamagePerSecond = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float RadiationDamageThreshold = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float ToxicityDamageThreshold = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float PsyDamageThreshold = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float RadiationDamagePerSecond = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float ToxicityDamagePerSecond = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float PsyDamagePerSecond = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float BleedingDamagePerSecond = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float BurnDamagePerSecond = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float ChemicalBurnDamagePerSecond = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats|Settings", meta = (ClampMin = "0.0"))
	float ElectricShockDamagePerSecond = 0.5f;
};