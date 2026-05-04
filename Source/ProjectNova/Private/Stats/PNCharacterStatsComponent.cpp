#include "Stats/PNCharacterStatsComponent.h"

#include "Characters/PNBaseCharacter.h"
#include "Engine/Engine.h"
#include "Equipment/PNEquipmentComponent.h"
#include "Items/PNInventoryComponent.h"
#include "Items/PNItemDataAsset.h"
#include "Items/PNItemStats.h"
#include "Net/UnrealNetwork.h"

UPNCharacterStatsComponent::UPNCharacterStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SetIsReplicatedByDefault(true);
}

void UPNCharacterStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeStats();

	if (HasStatsAuthority())
	{
		if (APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner()))
		{
			if (UPNEquipmentComponent* EquipmentComponent = OwnerCharacter->GetEquipmentComponent())
			{
				EquipmentComponent->OnEquipmentChanged.AddDynamic(this, &UPNCharacterStatsComponent::HandleEquipmentChanged);
			}
		}

		RecalculateEquipmentModifiers();
	}
}

void UPNCharacterStatsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner()))
	{
		if (UPNEquipmentComponent* EquipmentComponent = OwnerCharacter->GetEquipmentComponent())
		{
			EquipmentComponent->OnEquipmentChanged.RemoveDynamic(this, &UPNCharacterStatsComponent::HandleEquipmentChanged);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UPNCharacterStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!HasStatsAuthority())
	{
		return;
	}

	if (bIsDead)
	{
		return;
	}

	ServerTickAccumulator += DeltaTime;

	const float SafeInterval = FMath::Max(0.05f, Settings.ServerTickInterval);
	if (ServerTickAccumulator < SafeInterval)
	{
		return;
	}

	const float TickDelta = ServerTickAccumulator;
	ServerTickAccumulator = 0.0f;

	ServerTickStats(TickDelta);
}

void UPNCharacterStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPNCharacterStatsComponent, BaseStats);
	DOREPLIFETIME(UPNCharacterStatsComponent, EquipmentModifiers);
	DOREPLIFETIME(UPNCharacterStatsComponent, FinalStats);
	DOREPLIFETIME(UPNCharacterStatsComponent, CurrentStats);
	DOREPLIFETIME(UPNCharacterStatsComponent, bIsDead);
}

void UPNCharacterStatsComponent::InitializeStats()
{
	if (!HasStatsAuthority())
	{
		return;
	}

	BuildFinalStats();

	CurrentStats.Health = FMath::Clamp(CurrentStats.Health, 0.0f, FinalStats.Health);
	CurrentStats.Stamina = FMath::Clamp(CurrentStats.Stamina, 0.0f, FinalStats.Endurance);

	if (CurrentStats.Health <= 0.0f)
	{
		CurrentStats.Health = FinalStats.Health;
	}

	if (CurrentStats.Stamina <= 0.0f)
	{
		CurrentStats.Stamina = FinalStats.Endurance;
	}

	ClampAllStats();
	ApplyStatsToLinkedComponents();
	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::RecalculateEquipmentModifiers()
{
	if (!HasStatsAuthority())
	{
		return;
	}

	EquipmentModifiers.ResetToZero();

	const APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		BuildFinalStats();
		ClampAllStats();
		BroadcastStatsChanged();
		return;
	}

	const UPNEquipmentComponent* EquipmentComponent = OwnerCharacter->GetEquipmentComponent();
	if (!EquipmentComponent)
	{
		BuildFinalStats();
		ClampAllStats();
		BroadcastStatsChanged();
		return;
	}

	for (const FPNEquipmentSlotEntry& SlotEntry : EquipmentComponent->GetEquipmentSlots())
	{
		AddItemDataModifiers(SlotEntry.InstanceData.ItemData);
	}

	for (const FPNEquipmentInternalSlotEntry& InternalEntry : EquipmentComponent->GetHelmetInternalSlots())
	{
		AddItemDataModifiers(InternalEntry.InstanceData.ItemData);
	}

	for (const FPNEquipmentInternalSlotEntry& InternalEntry : EquipmentComponent->GetArmorInternalSlots())
	{
		AddItemDataModifiers(InternalEntry.InstanceData.ItemData);
	}

	BuildFinalStats();
	ClampAllStats();
	ApplyStatsToLinkedComponents();
	BroadcastStatsChanged();
}

float UPNCharacterStatsComponent::ApplyDamage(float DamageAmount, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasStatsAuthority())
	{
		return 0.0f;
	}

	if (bIsDead)
	{
		return 0.0f;
	}

	const float SafeDamage = FMath::Max(0.0f, DamageAmount);
	if (SafeDamage <= 0.0f)
	{
		return 0.0f;
	}

	const float OldHealth = CurrentStats.Health;
	const float NewHealth = FMath::Max(0.0f, CurrentStats.Health - SafeDamage);

	SetHealthInternal(NewHealth, true);

	if (CurrentStats.Health <= 0.0f)
	{
		SetDeadInternal(true, EventInstigator);
	}

	return OldHealth - CurrentStats.Health;
}

float UPNCharacterStatsComponent::Heal(float HealAmount)
{
	if (!HasStatsAuthority())
	{
		return 0.0f;
	}

	if (bIsDead)
	{
		return 0.0f;
	}

	const float SafeHeal = FMath::Max(0.0f, HealAmount);
	if (SafeHeal <= 0.0f)
	{
		return 0.0f;
	}

	const float OldHealth = CurrentStats.Health;
	const float NewHealth = FMath::Min(FinalStats.Health, CurrentStats.Health + SafeHeal);

	SetHealthInternal(NewHealth, true);

	return CurrentStats.Health - OldHealth;
}

bool UPNCharacterStatsComponent::ConsumeStamina(float StaminaAmount)
{
	if (!HasStatsAuthority())
	{
		return false;
	}

	const float SafeAmount = FMath::Max(0.0f, StaminaAmount);
	if (SafeAmount <= 0.0f)
	{
		return true;
	}

	if (CurrentStats.Stamina < SafeAmount)
	{
		SetStaminaInternal(0.0f, true);
		return false;
	}

	SetStaminaInternal(CurrentStats.Stamina - SafeAmount, true);
	return true;
}

float UPNCharacterStatsComponent::RestoreStamina(float StaminaAmount)
{
	if (!HasStatsAuthority())
	{
		return 0.0f;
	}

	const float SafeAmount = FMath::Max(0.0f, StaminaAmount);
	if (SafeAmount <= 0.0f)
	{
		return 0.0f;
	}

	const float OldStamina = CurrentStats.Stamina;
	const float NewStamina = FMath::Min(FinalStats.Endurance, CurrentStats.Stamina + SafeAmount);

	SetStaminaInternal(NewStamina, true);

	return CurrentStats.Stamina - OldStamina;
}

void UPNCharacterStatsComponent::SetSprintDrainEnabled(bool bEnabled)
{
	if (!HasStatsAuthority())
	{
		return;
	}

	if (bEnabled && !CanSprint())
	{
		bSprintDrainEnabled = false;
		return;
	}

	bSprintDrainEnabled = bEnabled;
}

void UPNCharacterStatsComponent::AddHunger(float Delta)
{
	if (!HasStatsAuthority())
	{
		return;
	}

	CurrentStats.Hunger = FMath::Clamp(CurrentStats.Hunger + Delta, 0.0f, 100.0f);
	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::AddThirst(float Delta)
{
	if (!HasStatsAuthority())
	{
		return;
	}

	CurrentStats.Thirst = FMath::Clamp(CurrentStats.Thirst + Delta, 0.0f, 100.0f);
	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::AddRadiation(float Delta)
{
	if (!HasStatsAuthority())
	{
		return;
	}

	const float Protection = FMath::Max(0.0f, FinalStats.RadiationProtection);
	const float EffectiveDelta = Delta > 0.0f ? Delta * FMath::Clamp(1.0f - Protection / 100.0f, 0.0f, 1.0f) : Delta;

	CurrentStats.Radiation = FMath::Clamp(CurrentStats.Radiation + EffectiveDelta, 0.0f, 100.0f);
	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::AddToxicity(float Delta)
{
	if (!HasStatsAuthority())
	{
		return;
	}

	const float Protection = FMath::Max(0.0f, FinalStats.ChemicalProtection);
	const float EffectiveDelta = Delta > 0.0f ? Delta * FMath::Clamp(1.0f - Protection / 100.0f, 0.0f, 1.0f) : Delta;

	CurrentStats.Toxicity = FMath::Clamp(CurrentStats.Toxicity + EffectiveDelta, 0.0f, 100.0f);
	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::AddPsy(float Delta)
{
	if (!HasStatsAuthority())
	{
		return;
	}

	const float Protection = FMath::Max(0.0f, FinalStats.PsiProtection);
	const float EffectiveDelta = Delta > 0.0f ? Delta * FMath::Clamp(1.0f - Protection / 100.0f, 0.0f, 1.0f) : Delta;

	CurrentStats.Psy = FMath::Clamp(CurrentStats.Psy + EffectiveDelta, 0.0f, 100.0f);
	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::AddBleeding(float Delta)
{
	if (!HasStatsAuthority())
	{
		return;
	}

	CurrentStats.Bleeding = FMath::Clamp(CurrentStats.Bleeding + Delta, 0.0f, 100.0f);
	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::AddWounds(float Delta)
{
	if (!HasStatsAuthority())
	{
		return;
	}

	CurrentStats.Wounds = FMath::Clamp(CurrentStats.Wounds + Delta, 0.0f, 100.0f);
	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::AddBurn(float Delta)
{
	if (!HasStatsAuthority())
	{
		return;
	}

	const float Protection = FMath::Max(0.0f, FinalStats.ThermalProtection);
	const float EffectiveDelta = Delta > 0.0f ? Delta * FMath::Clamp(1.0f - Protection / 100.0f, 0.0f, 1.0f) : Delta;

	CurrentStats.Burn = FMath::Clamp(CurrentStats.Burn + EffectiveDelta, 0.0f, 100.0f);
	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::AddChemicalBurn(float Delta)
{
	if (!HasStatsAuthority())
	{
		return;
	}

	const float Protection = FMath::Max(0.0f, FinalStats.ChemicalProtection);
	const float EffectiveDelta = Delta > 0.0f ? Delta * FMath::Clamp(1.0f - Protection / 100.0f, 0.0f, 1.0f) : Delta;

	CurrentStats.ChemicalBurn = FMath::Clamp(CurrentStats.ChemicalBurn + EffectiveDelta, 0.0f, 100.0f);
	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::AddElectricShock(float Delta)
{
	if (!HasStatsAuthority())
	{
		return;
	}

	const float Protection = FMath::Max(0.0f, FinalStats.ElectricProtection);
	const float EffectiveDelta = Delta > 0.0f ? Delta * FMath::Clamp(1.0f - Protection / 100.0f, 0.0f, 1.0f) : Delta;

	CurrentStats.ElectricShock = FMath::Clamp(CurrentStats.ElectricShock + EffectiveDelta, 0.0f, 100.0f);
	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::ResetForRespawn()
{
	if (!HasStatsAuthority())
	{
		return;
	}

	bIsDead = false;
	bSprintDrainEnabled = false;

	CurrentStats.Health = FinalStats.Health;
	CurrentStats.Stamina = FinalStats.Endurance;
	CurrentStats.Hunger = 100.0f;
	CurrentStats.Thirst = 100.0f;
	CurrentStats.Radiation = 0.0f;
	CurrentStats.Toxicity = 0.0f;
	CurrentStats.Psy = 0.0f;
	CurrentStats.Bleeding = 0.0f;
	CurrentStats.Wounds = 0.0f;
	CurrentStats.Burn = 0.0f;
	CurrentStats.ChemicalBurn = 0.0f;
	CurrentStats.ElectricShock = 0.0f;

	ClampAllStats();
	BroadcastStatsChanged();
}

const FPNCharacterAttributeStats& UPNCharacterStatsComponent::GetBaseStats() const
{
	return BaseStats;
}

const FPNCharacterAttributeStats& UPNCharacterStatsComponent::GetEquipmentModifiers() const
{
	return EquipmentModifiers;
}

const FPNCharacterAttributeStats& UPNCharacterStatsComponent::GetFinalStats() const
{
	return FinalStats;
}

const FPNCharacterCurrentStats& UPNCharacterStatsComponent::GetCurrentStats() const
{
	return CurrentStats;
}

float UPNCharacterStatsComponent::GetHealth() const
{
	return CurrentStats.Health;
}

float UPNCharacterStatsComponent::GetMaxHealth() const
{
	return FMath::Max(1.0f, FinalStats.Health);
}

float UPNCharacterStatsComponent::GetHealthPercent() const
{
	return FMath::Clamp(CurrentStats.Health / GetMaxHealth(), 0.0f, 1.0f);
}

float UPNCharacterStatsComponent::GetStamina() const
{
	return CurrentStats.Stamina;
}

float UPNCharacterStatsComponent::GetMaxStamina() const
{
	return FMath::Max(1.0f, FinalStats.Endurance);
}

float UPNCharacterStatsComponent::GetStaminaPercent() const
{
	return FMath::Clamp(CurrentStats.Stamina / GetMaxStamina(), 0.0f, 1.0f);
}

float UPNCharacterStatsComponent::GetMaxWeight() const
{
	return FMath::Max(0.0f, FinalStats.MaxWeight);
}

bool UPNCharacterStatsComponent::IsDead() const
{
	return bIsDead;
}

bool UPNCharacterStatsComponent::CanSprint() const
{
	return !bIsDead && CurrentStats.Stamina > 1.0f;
}

FString UPNCharacterStatsComponent::GetStatsDebugString() const
{
	const AActor* OwnerActor = GetOwner();

	const FString NetSide = OwnerActor && OwnerActor->HasAuthority()
		? TEXT("SERVER")
		: TEXT("CLIENT");

	const FString OwnerName = OwnerActor
		? OwnerActor->GetName()
		: TEXT("NoOwner");

	return FString::Printf(
		TEXT("[%s] CharacterStats: %s\n")
		TEXT("HP: %.1f / %.1f | ST: %.1f / %.1f | Dead:%s\n")
		TEXT("Hunger: %.1f | Thirst: %.1f | Rad: %.1f | Toxic: %.1f | Psy: %.1f\n")
		TEXT("Bleed: %.1f | Wounds: %.1f | Burn: %.1f | Chem: %.1f | Elec: %.1f\n")
		TEXT("Weight: %.1f | HPRegen: %.1f | STRegen: %.1f | BulletRes: %.1f\n")
		TEXT("Protections: Rad %.1f | Psi %.1f | Thermal %.1f | Chem %.1f | Elec %.1f"),
		*NetSide,
		*OwnerName,
		CurrentStats.Health,
		FinalStats.Health,
		CurrentStats.Stamina,
		FinalStats.Endurance,
		bIsDead ? TEXT("Yes") : TEXT("No"),
		CurrentStats.Hunger,
		CurrentStats.Thirst,
		CurrentStats.Radiation,
		CurrentStats.Toxicity,
		CurrentStats.Psy,
		CurrentStats.Bleeding,
		CurrentStats.Wounds,
		CurrentStats.Burn,
		CurrentStats.ChemicalBurn,
		CurrentStats.ElectricShock,
		FinalStats.MaxWeight,
		FinalStats.HealthRegen,
		FinalStats.StaminaRegen,
		FinalStats.BulletResistance,
		FinalStats.RadiationProtection,
		FinalStats.PsiProtection,
		FinalStats.ThermalProtection,
		FinalStats.ChemicalProtection,
		FinalStats.ElectricProtection
	);
}

void UPNCharacterStatsComponent::PrintStatsDebug() const
{
	const FString DebugText = GetStatsDebugString();

	UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugText);

	if (GEngine)
	{
		const int32 DebugKey = GetOwner()
			? static_cast<int32>(GetOwner()->GetUniqueID()) + 20000
			: INDEX_NONE;

		GEngine->AddOnScreenDebugMessage(
			DebugKey,
			5.0f,
			FColor::Orange,
			DebugText
		);
	}
}

void UPNCharacterStatsComponent::OnRep_BaseStats()
{
	OnStatsChanged.Broadcast();
}

void UPNCharacterStatsComponent::OnRep_EquipmentModifiers()
{
	OnStatsChanged.Broadcast();
}

void UPNCharacterStatsComponent::OnRep_FinalStats()
{
	OnStatsChanged.Broadcast();

	if (bDebugStatsReplication)
	{
		PrintStatsDebug();
	}
}

void UPNCharacterStatsComponent::OnRep_CurrentStats()
{
	OnStatsChanged.Broadcast();

	if (bDebugStatsReplication)
	{
		PrintStatsDebug();
	}
}

void UPNCharacterStatsComponent::OnRep_IsDead()
{
	OnStatsChanged.Broadcast();

	if (bIsDead)
	{
		OnDeath.Broadcast();
	}

	if (bDebugStatsReplication)
	{
		PrintStatsDebug();
	}
}

void UPNCharacterStatsComponent::HandleEquipmentChanged()
{
	if (!HasStatsAuthority())
	{
		return;
	}

	RecalculateEquipmentModifiers();
}

bool UPNCharacterStatsComponent::HasStatsAuthority() const
{
	const AActor* OwnerActor = GetOwner();
	return !OwnerActor || OwnerActor->HasAuthority();
}

void UPNCharacterStatsComponent::ServerTickStats(float DeltaTime)
{
	const float SafeDeltaTime = FMath::Max(0.0f, DeltaTime);

	if (SafeDeltaTime <= 0.0f)
	{
		return;
	}

	if (bSprintDrainEnabled)
	{
		const bool bHasStamina = ConsumeStamina(Settings.SprintStaminaDrainPerSecond * SafeDeltaTime);

		if (!bHasStamina)
		{
			bSprintDrainEnabled = false;

			if (APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner()))
			{
				OwnerCharacter->StopSprint();
			}
		}
	}
	else
	{
		RestoreStamina(FinalStats.StaminaRegen * SafeDeltaTime);
	}

	const float SatietyProtection = FMath::Clamp(FinalStats.Satiety / 100.0f, -1.0f, 0.8f);
	const float HungerDrain = FMath::Max(0.0f, Settings.HungerDrainPerSecond * (1.0f - SatietyProtection));
	const float ThirstDrain = FMath::Max(0.0f, Settings.ThirstDrainPerSecond);

	CurrentStats.Hunger = FMath::Clamp(CurrentStats.Hunger - HungerDrain * SafeDeltaTime, 0.0f, 100.0f);
	CurrentStats.Thirst = FMath::Clamp(CurrentStats.Thirst - ThirstDrain * SafeDeltaTime, 0.0f, 100.0f);

	if (FinalStats.Radiation > 0.0f)
	{
		AddRadiation(FinalStats.Radiation * SafeDeltaTime);
	}

	if (FinalStats.Bleeding > 0.0f)
	{
		AddBleeding(FinalStats.Bleeding * SafeDeltaTime);
	}

	if (FinalStats.Burn > 0.0f)
	{
		AddBurn(FinalStats.Burn * SafeDeltaTime);
	}

	if (FinalStats.ChemicalBurn > 0.0f)
	{
		AddChemicalBurn(FinalStats.ChemicalBurn * SafeDeltaTime);
	}

	if (FinalStats.ElectricShock > 0.0f)
	{
		AddElectricShock(FinalStats.ElectricShock * SafeDeltaTime);
	}

	if (FinalStats.WoundHealing > 0.0f)
	{
		CurrentStats.Wounds = FMath::Max(0.0f, CurrentStats.Wounds - FinalStats.WoundHealing * SafeDeltaTime);
	}

	if (FinalStats.HealthRegen > 0.0f && CurrentStats.Health > 0.0f)
	{
		Heal(FinalStats.HealthRegen * SafeDeltaTime);
	}

	float EnvironmentDamage = 0.0f;

	if (CurrentStats.Hunger <= 0.0f)
	{
		EnvironmentDamage += Settings.LowHungerDamagePerSecond * SafeDeltaTime;
	}

	if (CurrentStats.Thirst <= 0.0f)
	{
		EnvironmentDamage += Settings.LowThirstDamagePerSecond * SafeDeltaTime;
	}

	if (CurrentStats.Radiation >= Settings.RadiationDamageThreshold)
	{
		EnvironmentDamage += Settings.RadiationDamagePerSecond * SafeDeltaTime;
	}

	if (CurrentStats.Toxicity >= Settings.ToxicityDamageThreshold)
	{
		EnvironmentDamage += Settings.ToxicityDamagePerSecond * SafeDeltaTime;
	}

	if (CurrentStats.Psy >= Settings.PsyDamageThreshold)
	{
		EnvironmentDamage += Settings.PsyDamagePerSecond * SafeDeltaTime;
	}

	if (CurrentStats.Bleeding > 0.0f)
	{
		EnvironmentDamage += Settings.BleedingDamagePerSecond * (CurrentStats.Bleeding / 100.0f) * SafeDeltaTime;
	}

	if (CurrentStats.Burn > 0.0f)
	{
		EnvironmentDamage += Settings.BurnDamagePerSecond * (CurrentStats.Burn / 100.0f) * SafeDeltaTime;
	}

	if (CurrentStats.ChemicalBurn > 0.0f)
	{
		EnvironmentDamage += Settings.ChemicalBurnDamagePerSecond * (CurrentStats.ChemicalBurn / 100.0f) * SafeDeltaTime;
	}

	if (CurrentStats.ElectricShock > 0.0f)
	{
		EnvironmentDamage += Settings.ElectricShockDamagePerSecond * (CurrentStats.ElectricShock / 100.0f) * SafeDeltaTime;
	}

	if (EnvironmentDamage > 0.0f)
	{
		ApplyDamage(EnvironmentDamage, nullptr, GetOwner());
	}

	ClampAllStats();
	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::ClampAllStats()
{
	FinalStats.ClampFinal();
	CurrentStats.Clamp(FinalStats);
}

void UPNCharacterStatsComponent::BuildFinalStats()
{
	FinalStats = BaseStats;
	FinalStats.Add(EquipmentModifiers);
	FinalStats.ClampFinal();
}

void UPNCharacterStatsComponent::ApplyStatsToLinkedComponents()
{
	APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	UPNInventoryComponent* InventoryComponent = OwnerCharacter->GetInventoryComponent();
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->Settings.MaxWeight = GetMaxWeight();
}

void UPNCharacterStatsComponent::AddItemDataModifiers(UPNItemDataAsset* ItemData)
{
	if (!ItemData)
	{
		return;
	}

	if (ItemData->ItemType == EPNItemType::IT_Backpack)
	{
		EquipmentModifiers.MaxWeight += FMath::Max(0.0f, ItemData->BackpackStats.MaxWeight);
	}

	if (ItemData->ItemType == EPNItemType::IT_Armor
		|| ItemData->ItemType == EPNItemType::IT_HArmor
		|| ItemData->ItemType == EPNItemType::IT_Gloves)
	{
		AddItemAttributeModifiers(ItemData->ArmorStats.AttributeModifiers);
		AddProtectionFromItem(ItemData);
		return;
	}

	if (ItemData->ItemType == EPNItemType::IT_Armor_ATTM)
	{
		AddItemAttributeModifiers(ItemData->ArmorAttachmentStats.AttributeModifiers);
		AddProtectionFromItem(ItemData);

		if (ItemData->ItemCategory == EPNItemCategory::Plate)
		{
			EquipmentModifiers.BulletResistance += ItemData->ArmorPlateStats.ShieldPercent;

			if (ItemData->ArmorPlateStats.BulletResistance.bEnabled)
			{
				EquipmentModifiers.BulletResistance += ItemData->ArmorPlateStats.BulletResistance.Value;
			}
		}
	}
}

void UPNCharacterStatsComponent::AddItemAttributeModifiers(const FPNItemAttributeModifiers& ItemModifiers)
{
	if (ItemModifiers.Endurance.bEnabled)
	{
		EquipmentModifiers.Endurance += ItemModifiers.Endurance.Value;
	}

	if (ItemModifiers.StaminaRegen.bEnabled)
	{
		EquipmentModifiers.StaminaRegen += ItemModifiers.StaminaRegen.Value;
	}

	if (ItemModifiers.Health.bEnabled)
	{
		EquipmentModifiers.Health += ItemModifiers.Health.Value;
	}

	if (ItemModifiers.HealthRegen.bEnabled)
	{
		EquipmentModifiers.HealthRegen += ItemModifiers.HealthRegen.Value;
	}

	if (ItemModifiers.Bleeding.bEnabled)
	{
		EquipmentModifiers.Bleeding += ItemModifiers.Bleeding.Value;
	}

	if (ItemModifiers.WoundHealing.bEnabled)
	{
		EquipmentModifiers.WoundHealing += ItemModifiers.WoundHealing.Value;
	}

	if (ItemModifiers.MaxWeight.bEnabled)
	{
		EquipmentModifiers.MaxWeight += ItemModifiers.MaxWeight.Value;
	}

	if (ItemModifiers.Telepathy.bEnabled)
	{
		EquipmentModifiers.Telepathy += ItemModifiers.Telepathy.Value;
	}

	if (ItemModifiers.PsiProtection.bEnabled)
	{
		EquipmentModifiers.PsiProtection += ItemModifiers.PsiProtection.Value;
	}

	if (ItemModifiers.Burn.bEnabled)
	{
		EquipmentModifiers.Burn += ItemModifiers.Burn.Value;
	}

	if (ItemModifiers.ThermalProtection.bEnabled)
	{
		EquipmentModifiers.ThermalProtection += ItemModifiers.ThermalProtection.Value;
	}

	if (ItemModifiers.ChemicalBurn.bEnabled)
	{
		EquipmentModifiers.ChemicalBurn += ItemModifiers.ChemicalBurn.Value;
	}

	if (ItemModifiers.ChemicalProtection.bEnabled)
	{
		EquipmentModifiers.ChemicalProtection += ItemModifiers.ChemicalProtection.Value;
	}

	if (ItemModifiers.ElectricShock.bEnabled)
	{
		EquipmentModifiers.ElectricShock += ItemModifiers.ElectricShock.Value;
	}

	if (ItemModifiers.ElectricProtection.bEnabled)
	{
		EquipmentModifiers.ElectricProtection += ItemModifiers.ElectricProtection.Value;
	}

	if (ItemModifiers.Radiation.bEnabled)
	{
		EquipmentModifiers.Radiation += ItemModifiers.Radiation.Value;
	}

	if (ItemModifiers.RadiationProtection.bEnabled)
	{
		EquipmentModifiers.RadiationProtection += ItemModifiers.RadiationProtection.Value;
	}

	if (ItemModifiers.Satiety.bEnabled)
	{
		EquipmentModifiers.Satiety += ItemModifiers.Satiety.Value;
	}

	if (ItemModifiers.BulletResistance.bEnabled)
	{
		EquipmentModifiers.BulletResistance += ItemModifiers.BulletResistance.Value;
	}
}

void UPNCharacterStatsComponent::AddProtectionFromItem(UPNItemDataAsset* ItemData)
{
	if (!ItemData)
	{
		return;
	}

	if (ItemData->ItemType == EPNItemType::IT_Armor
		|| ItemData->ItemType == EPNItemType::IT_HArmor
		|| ItemData->ItemType == EPNItemType::IT_Gloves)
	{
		EquipmentModifiers.BulletResistance += FMath::Max(0.0f, ItemData->ArmorStats.Protection.DamageMax);
		return;
	}

	if (ItemData->ItemType == EPNItemType::IT_Armor_ATTM)
	{
		if (ItemData->ArmorAttachmentStats.bProvidesProtection)
		{
			EquipmentModifiers.BulletResistance += FMath::Max(0.0f, ItemData->ArmorAttachmentStats.Protection.DamageMax);
		}
	}
}

void UPNCharacterStatsComponent::SetHealthInternal(float NewHealth, bool bBroadcast)
{
	const float OldHealth = CurrentStats.Health;
	CurrentStats.Health = FMath::Clamp(NewHealth, 0.0f, FinalStats.Health);

	if (bBroadcast && !FMath::IsNearlyEqual(OldHealth, CurrentStats.Health))
	{
		OnHealthChanged.Broadcast(OldHealth, CurrentStats.Health);
		BroadcastStatsChanged();
	}
}

void UPNCharacterStatsComponent::SetStaminaInternal(float NewStamina, bool bBroadcast)
{
	const float OldStamina = CurrentStats.Stamina;
	CurrentStats.Stamina = FMath::Clamp(NewStamina, 0.0f, FinalStats.Endurance);

	if (bBroadcast && !FMath::IsNearlyEqual(OldStamina, CurrentStats.Stamina))
	{
		OnStaminaChanged.Broadcast(OldStamina, CurrentStats.Stamina);
		BroadcastStatsChanged();
	}
}

void UPNCharacterStatsComponent::SetDeadInternal(bool bNewDead, AController* KillerController)
{
	if (bIsDead == bNewDead)
	{
		return;
	}

	bIsDead = bNewDead;
	bSprintDrainEnabled = false;

	if (bIsDead)
	{
		CurrentStats.Health = 0.0f;

		OnDeath.Broadcast();

		if (APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner()))
		{
			OwnerCharacter->Die(KillerController);
		}
	}

	BroadcastStatsChanged();
}

void UPNCharacterStatsComponent::BroadcastStatsChanged()
{
	OnStatsChanged.Broadcast();

	if (bDebugStatsReplication)
	{
		PrintStatsDebug();
	}
}