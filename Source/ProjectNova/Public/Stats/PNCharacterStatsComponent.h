#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Stats/PNCharacterStatsTypes.h"
#include "PNCharacterStatsComponent.generated.h"

class UPNItemDataAsset;
class UPNEquipmentComponent;
class UPNInventoryComponent;
class AController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPNCharacterFloatStatChangedSignature, float, OldValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPNCharacterStatsChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPNCharacterDeathSignature);

UCLASS(ClassGroup = (ProjectNova), meta = (BlueprintSpawnableComponent))
class PROJECTNOVA_API UPNCharacterStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPNCharacterStatsComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Character Stats")
	FPNCharacterStatsChangedSignature OnStatsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Character Stats")
	FPNCharacterFloatStatChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Character Stats")
	FPNCharacterFloatStatChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Character Stats")
	FPNCharacterDeathSignature OnDeath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Stats|Debug")
	bool bDebugStatsReplication = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Stats|Settings")
	FPNCharacterStatsSettings Settings;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_BaseStats, EditAnywhere, BlueprintReadOnly, Category = "Character Stats|Base")
	FPNCharacterAttributeStats BaseStats;

	UPROPERTY(ReplicatedUsing = OnRep_EquipmentModifiers, VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats|Modifiers")
	FPNCharacterAttributeStats EquipmentModifiers;

	UPROPERTY(ReplicatedUsing = OnRep_FinalStats, VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats|Final")
	FPNCharacterAttributeStats FinalStats;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentStats, VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats|Current")
	FPNCharacterCurrentStats CurrentStats;

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats|State")
	bool bIsDead = false;

	bool bSprintDrainEnabled = false;
	float ServerTickAccumulator = 0.0f;

public:
	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void InitializeStats();

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void RecalculateEquipmentModifiers();

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	float ApplyDamage(float DamageAmount, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	float Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	bool ConsumeStamina(float StaminaAmount);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	float RestoreStamina(float StaminaAmount);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void SetSprintDrainEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void AddHunger(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void AddThirst(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void AddRadiation(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void AddToxicity(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void AddPsy(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void AddBleeding(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void AddWounds(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void AddBurn(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void AddChemicalBurn(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void AddElectricShock(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Character Stats")
	void ResetForRespawn();

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	const FPNCharacterAttributeStats& GetBaseStats() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	const FPNCharacterAttributeStats& GetEquipmentModifiers() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	const FPNCharacterAttributeStats& GetFinalStats() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	const FPNCharacterCurrentStats& GetCurrentStats() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	float GetStamina() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	float GetMaxWeight() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	bool CanSprint() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats|Debug")
	FString GetStatsDebugString() const;

	UFUNCTION(BlueprintCallable, Category = "Character Stats|Debug")
	void PrintStatsDebug() const;

protected:
	UFUNCTION()
	void OnRep_BaseStats();

	UFUNCTION()
	void OnRep_EquipmentModifiers();

	UFUNCTION()
	void OnRep_FinalStats();

	UFUNCTION()
	void OnRep_CurrentStats();

	UFUNCTION()
	void OnRep_IsDead();

	UFUNCTION()
	void HandleEquipmentChanged();

	bool HasStatsAuthority() const;

	void ServerTickStats(float DeltaTime);
	void ClampAllStats();

	void BuildFinalStats();
	void ApplyStatsToLinkedComponents();

	void AddItemDataModifiers(UPNItemDataAsset* ItemData);
	void AddItemAttributeModifiers(const struct FPNItemAttributeModifiers& ItemModifiers);
	void AddProtectionFromItem(UPNItemDataAsset* ItemData);

	void SetHealthInternal(float NewHealth, bool bBroadcast);
	void SetStaminaInternal(float NewStamina, bool bBroadcast);
	void SetDeadInternal(bool bNewDead, AController* KillerController);

	void BroadcastStatsChanged();
};