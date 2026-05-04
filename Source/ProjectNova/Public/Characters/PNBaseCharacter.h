#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PNBaseCharacter.generated.h"

class UPNInventoryComponent;
class UPNEquipmentComponent;
class UPNCharacterStatsComponent;
class UPNQuickSlotComponent;
class USkeletalMesh;
class USkeletalMeshComponent;

UCLASS()
class PROJECTNOVA_API APNBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APNBaseCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPNInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPNEquipmentComponent> EquipmentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPNCharacterStatsComponent> CharacterStatsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPNQuickSlotComponent> QuickSlotComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modular Character")
	TObjectPtr<USkeletalMeshComponent> ThirdPersonHeadMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modular Character")
	TObjectPtr<USkeletalMeshComponent> ThirdPersonLegsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modular Character")
	TObjectPtr<USkeletalMeshComponent> ThirdPersonHandsMeshComponent;

	UPROPERTY(ReplicatedUsing = OnRep_ModularMeshes, EditAnywhere, BlueprintReadOnly, Category = "Modular Character")
	TObjectPtr<USkeletalMesh> ThirdPersonMasterBodyMeshAsset = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_ModularMeshes, EditAnywhere, BlueprintReadOnly, Category = "Modular Character")
	TObjectPtr<USkeletalMesh> ThirdPersonHeadMeshAsset = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_ModularMeshes, EditAnywhere, BlueprintReadOnly, Category = "Modular Character")
	TObjectPtr<USkeletalMesh> ThirdPersonLegsMeshAsset = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_ModularMeshes, EditAnywhere, BlueprintReadOnly, Category = "Modular Character")
	TObjectPtr<USkeletalMesh> ThirdPersonHandsMeshAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float WalkSpeed = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float SprintSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float CrouchSpeed = 180.0f;

	UPROPERTY(ReplicatedUsing = OnRep_IsSprinting, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly, Category = "State")
	bool bIsDead = false;

public:
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UPNInventoryComponent* GetInventoryComponent() const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	UPNEquipmentComponent* GetEquipmentComponent() const;

	UFUNCTION(BlueprintPure, Category = "Character Stats")
	UPNCharacterStatsComponent* GetCharacterStatsComponent() const;

	UFUNCTION(BlueprintPure, Category = "Quick Slots")
	UPNQuickSlotComponent* GetQuickSlotComponent() const;

	UFUNCTION(BlueprintPure, Category = "Modular Character")
	USkeletalMeshComponent* GetThirdPersonHeadMeshComponent() const;

	UFUNCTION(BlueprintPure, Category = "Modular Character")
	USkeletalMeshComponent* GetThirdPersonLegsMeshComponent() const;

	UFUNCTION(BlueprintPure, Category = "Modular Character")
	USkeletalMeshComponent* GetThirdPersonHandsMeshComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Modular Character")
	void SetModularCharacterMeshes(
		USkeletalMesh* NewMasterBodyMesh,
		USkeletalMesh* NewHeadMesh,
		USkeletalMesh* NewLegsMesh,
		USkeletalMesh* NewHandsMesh
	);

	UFUNCTION(BlueprintCallable, Category = "Modular Character")
	void ApplyModularCharacterMeshes();

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSprinting() const;

	UFUNCTION(BlueprintPure, Category = "State")
	bool IsDead() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	UFUNCTION(BlueprintCallable, Category = "State")
	virtual void Die(AController* KillerController);

	UFUNCTION(BlueprintCallable, Category = "State")
	virtual void Revive();

	UFUNCTION(Server, Reliable)
	void Server_SetSprinting(bool bNewSprinting);

protected:
	void SetSprintingInternal(bool bNewSprinting);
	void ApplyMovementSpeed();
	void SetupThirdPersonMeshVisibility();

	UFUNCTION()
	void OnRep_ModularMeshes();

	UFUNCTION()
	void OnRep_IsSprinting();

	UFUNCTION()
	void OnRep_IsDead();
};