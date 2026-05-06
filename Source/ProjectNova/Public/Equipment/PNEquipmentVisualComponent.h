#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Equipment/PNEquipmentTypes.h"
#include "PNEquipmentVisualComponent.generated.h"

class APNBaseCharacter;
class UPNEquipmentComponent;
class UPNItemDataAsset;
class USkeletalMeshComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPNEquipmentVisualsChangedSignature);

UCLASS(ClassGroup = (ProjectNova), meta = (BlueprintSpawnableComponent))
class PROJECTNOVA_API UPNEquipmentVisualComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPNEquipmentVisualComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Equipment Visuals")
	FPNEquipmentVisualsChangedSignature OnEquipmentVisualsChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Debug")
	bool bDebugEquipmentVisuals = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Sockets")
	FName HelmetSocketName = TEXT("head_equipment_socket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Sockets")
	FName ArmorSocketName = TEXT("spine_armor_socket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Sockets")
	FName BackpackSocketName = TEXT("backpack_socket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Sockets")
	FName GlovesSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Sockets")
	FName PrimaryWeapon1SocketName = TEXT("primary_weapon_1_socket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Sockets")
	FName PrimaryWeapon2SocketName = TEXT("primary_weapon_2_socket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Sockets")
	FName SidearmSocketName = TEXT("sidearm_socket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Sockets")
	FName KnifeSocketName = TEXT("knife_socket");

protected:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Helmet")
	TObjectPtr<USkeletalMeshComponent> HelmetSkeletalVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Helmet")
	TObjectPtr<UStaticMeshComponent> HelmetStaticVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Armor")
	TObjectPtr<USkeletalMeshComponent> ArmorSkeletalVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Armor")
	TObjectPtr<UStaticMeshComponent> ArmorStaticVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Backpack")
	TObjectPtr<USkeletalMeshComponent> BackpackSkeletalVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Backpack")
	TObjectPtr<UStaticMeshComponent> BackpackStaticVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Gloves")
	TObjectPtr<USkeletalMeshComponent> GlovesSkeletalVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Gloves")
	TObjectPtr<UStaticMeshComponent> GlovesStaticVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Weapons")
	TObjectPtr<USkeletalMeshComponent> PrimaryWeapon1SkeletalVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Weapons")
	TObjectPtr<UStaticMeshComponent> PrimaryWeapon1StaticVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Weapons")
	TObjectPtr<USkeletalMeshComponent> PrimaryWeapon2SkeletalVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Weapons")
	TObjectPtr<UStaticMeshComponent> PrimaryWeapon2StaticVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Weapons")
	TObjectPtr<USkeletalMeshComponent> SidearmSkeletalVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Weapons")
	TObjectPtr<UStaticMeshComponent> SidearmStaticVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Weapons")
	TObjectPtr<USkeletalMeshComponent> KnifeSkeletalVisualComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals|Weapons")
	TObjectPtr<UStaticMeshComponent> KnifeStaticVisualComponent;

public:
	UFUNCTION(BlueprintCallable, Category = "Equipment Visuals")
	void RefreshEquipmentVisuals();

	UFUNCTION(BlueprintCallable, Category = "Equipment Visuals")
	void ClearAllEquipmentVisuals();

	UFUNCTION(BlueprintPure, Category = "Equipment Visuals")
	USkeletalMeshComponent* GetSkeletalVisualComponent(EPNEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "Equipment Visuals")
	UStaticMeshComponent* GetStaticVisualComponent(EPNEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "Equipment Visuals|Debug")
	FString GetEquipmentVisualDebugString() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment Visuals|Debug")
	void PrintEquipmentVisualDebug() const;

protected:
	UFUNCTION()
	void HandleEquipmentChanged();

	APNBaseCharacter* GetOwnerCharacter() const;
	UPNEquipmentComponent* GetOwnerEquipmentComponent() const;

	void CreateVisualComponents();
	void DestroyVisualComponents();

	USkeletalMeshComponent* CreateSkeletalVisualComponent(FName ComponentName);
	UStaticMeshComponent* CreateStaticVisualComponent(FName ComponentName);

	void RefreshSlotVisual(EPNEquipmentSlot Slot, UPNItemDataAsset* ItemData);
	void ClearSlotVisual(EPNEquipmentSlot Slot);

	void ApplySkeletalVisual(EPNEquipmentSlot Slot, UPNItemDataAsset* ItemData);
	void ApplyStaticVisual(EPNEquipmentSlot Slot, UPNItemDataAsset* ItemData);

	void ClearSkeletalVisual(USkeletalMeshComponent* Component);
	void ClearStaticVisual(UStaticMeshComponent* Component);

	FName GetAttachSocketForSlot(EPNEquipmentSlot Slot, UPNItemDataAsset* ItemData) const;
	bool ShouldUseLeaderPose(EPNEquipmentSlot Slot) const;

	void BroadcastVisualsChanged();
};