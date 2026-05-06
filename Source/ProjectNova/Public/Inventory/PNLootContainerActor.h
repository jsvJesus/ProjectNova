#pragma once

#include "CoreMinimal.h"
#include "Inventory/PNInventoryContainerActor.h"
#include "PNLootContainerActor.generated.h"

class UPNLootContainerProfileDataAsset;
class UPNLootTableDataAsset;

UCLASS()
class PROJECTNOVA_API APNLootContainerActor : public APNInventoryContainerActor
{
	GENERATED_BODY()

public:
	APNLootContainerActor();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Container|Profile")
	TObjectPtr<UPNLootContainerProfileDataAsset> ContainerProfile = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Container|Profile")
	bool bApplyProfileOnConstruction = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Container|Profile")
	bool bApplyProfileOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Container|Loot Table")
	TObjectPtr<UPNLootTableDataAsset> LootTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Container|Loot Table")
	bool bSpawnLootOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Container|Loot Table")
	bool bClearInventoryBeforeLootRespawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Container|Loot Table")
	bool bUseFixedLootSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Container|Loot Table", meta = (EditCondition = "bUseFixedLootSeed"))
	int32 FixedLootSeed = 1337;

	UPROPERTY(ReplicatedUsing = OnRep_LootRespawnState, VisibleAnywhere, BlueprintReadOnly, Category = "Loot Container|Respawn")
	bool bLootRespawnBlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Container|Respawn")
	bool bRespawnWhenEmpty = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Container|Respawn", meta = (ClampMin = "0.0"))
	float LootRespawnSeconds = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Container|Respawn")
	bool bCanOpenEmptyLootContainer = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Loot Container|Profile")
	bool ApplyContainerProfile(bool bReinitializeInventory = false);

	UFUNCTION(BlueprintPure, Category = "Loot Container|Profile")
	UPNLootContainerProfileDataAsset* GetContainerProfile() const;

	UFUNCTION(BlueprintPure, Category = "Loot Container")
	bool HasLootItems() const;

	UFUNCTION(BlueprintPure, Category = "Loot Container")
	bool IsLootRespawnBlocked() const;

	UFUNCTION(BlueprintCallable, Category = "Loot Container|Respawn")
	void StartLootRespawnCooldown();

	UFUNCTION(BlueprintCallable, Category = "Loot Container|Respawn")
	void FinishLootRespawnCooldown();

	UFUNCTION(BlueprintCallable, Category = "Loot Container|Loot Table")
	int32 RespawnLootFromTable();

	UFUNCTION(BlueprintImplementableEvent, Category = "Loot Container|Respawn")
	void BP_RespawnLoot();

	UFUNCTION(BlueprintImplementableEvent, Category = "Loot Container|Respawn")
	void BP_OnLootRespawned(int32 SpawnedStacks);

public:
	virtual bool CanInteract_Implementation(APawn* InteractingPawn) const override;
	virtual bool Interact_Implementation(APawn* InteractingPawn) override;

protected:
	UPROPERTY()
	FTimerHandle LootRespawnTimerHandle;

	UPROPERTY()
	bool bIsRespawningLoot = false;

	UFUNCTION()
	void HandleLootInventoryChanged();

	UFUNCTION()
	void OnRep_LootRespawnState();
};