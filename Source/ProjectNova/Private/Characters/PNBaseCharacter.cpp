#include "Characters/PNBaseCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Inventory/PNInventoryActionComponent.h"
#include "Equipment/PNEquipmentComponent.h"
#include "Equipment/PNEquipmentVisualComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Inventory/PNInventoryComponent.h"
#include "Items/PNQuickSlotComponent.h"
#include "Net/UnrealNetwork.h"
#include "Stats/PNCharacterStatsComponent.h"
#include "Inventory/PNInventoryContainerComponent.h"
#include "Items/PNItemDataAsset.h"
#include "UI/PNPlayerHUDComponent.h"

APNBaseCharacter::APNBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	InventoryComponent = CreateDefaultSubobject<UPNInventoryComponent>(TEXT("InventoryComponent"));
	if (InventoryComponent)
	{
		InventoryComponent->SetIsReplicated(true);
	}

	PlayerHUDComponent = CreateDefaultSubobject<UPNPlayerHUDComponent>(TEXT("PlayerHUDComponent"));

	EquipmentComponent = CreateDefaultSubobject<UPNEquipmentComponent>(TEXT("EquipmentComponent"));
	if (EquipmentComponent)
	{
		EquipmentComponent->SetIsReplicated(true);
	}

	EquipmentVisualComponent = CreateDefaultSubobject<UPNEquipmentVisualComponent>(TEXT("EquipmentVisualComponent"));
	if (EquipmentVisualComponent)
	{
		EquipmentVisualComponent->SetIsReplicated(true);
	}

	CharacterStatsComponent = CreateDefaultSubobject<UPNCharacterStatsComponent>(TEXT("CharacterStatsComponent"));
	if (CharacterStatsComponent)
	{
		CharacterStatsComponent->SetIsReplicated(true);
	}

	QuickSlotComponent = CreateDefaultSubobject<UPNQuickSlotComponent>(TEXT("QuickSlotComponent"));
	if (QuickSlotComponent)
	{
		QuickSlotComponent->SetIsReplicated(true);
	}

	InventoryActionComponent = CreateDefaultSubobject<UPNInventoryActionComponent>(TEXT("InventoryActionComponent"));
	if (InventoryActionComponent)
	{
		InventoryActionComponent->SetIsReplicated(true);
	}

	InventoryContainerComponent = CreateDefaultSubobject<UPNInventoryContainerComponent>(TEXT("InventoryContainerComponent"));
	if (InventoryContainerComponent)
	{
		InventoryContainerComponent->SetIsReplicated(true);
	}

	GetMesh()->SetIsReplicated(true);
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetOnlyOwnerSee(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ThirdPersonHeadMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonHeadMeshComponent"));
	ThirdPersonHeadMeshComponent->SetupAttachment(GetMesh());
	ThirdPersonHeadMeshComponent->SetIsReplicated(true);
	ThirdPersonHeadMeshComponent->SetOwnerNoSee(true);
	ThirdPersonHeadMeshComponent->SetOnlyOwnerSee(false);
	ThirdPersonHeadMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ThirdPersonLegsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonLegsMeshComponent"));
	ThirdPersonLegsMeshComponent->SetupAttachment(GetMesh());
	ThirdPersonLegsMeshComponent->SetIsReplicated(true);
	ThirdPersonLegsMeshComponent->SetOwnerNoSee(true);
	ThirdPersonLegsMeshComponent->SetOnlyOwnerSee(false);
	ThirdPersonLegsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ThirdPersonHandsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonHandsMeshComponent"));
	ThirdPersonHandsMeshComponent->SetupAttachment(GetMesh());
	ThirdPersonHandsMeshComponent->SetIsReplicated(true);
	ThirdPersonHandsMeshComponent->SetOwnerNoSee(true);
	ThirdPersonHandsMeshComponent->SetOnlyOwnerSee(false);
	ThirdPersonHandsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

void APNBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetupThirdPersonMeshVisibility();
	ApplyModularCharacterMeshes();
	ApplyMovementSpeed();

	if (PlayerHUDComponent)
	{
		PlayerHUDComponent->SetOwnerCharacter(this);
		PlayerHUDComponent->RefreshHUDData();
	}

	if (EquipmentComponent)
	{
		EquipmentComponent->OnEquipmentChanged.AddUniqueDynamic(this, &APNBaseCharacter::HandleEquipmentChanged);
	}

	if (HasAuthority())
	{
		RefreshEquipmentInventories();
	}
}

void APNBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APNBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APNBaseCharacter, ThirdPersonMasterBodyMeshAsset);
	DOREPLIFETIME(APNBaseCharacter, ThirdPersonHeadMeshAsset);
	DOREPLIFETIME(APNBaseCharacter, ThirdPersonLegsMeshAsset);
	DOREPLIFETIME(APNBaseCharacter, ThirdPersonHandsMeshAsset);
	DOREPLIFETIME(APNBaseCharacter, bIsSprinting);
	DOREPLIFETIME(APNBaseCharacter, bIsDead);
}

float APNBaseCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!HasAuthority())
	{
		return AppliedDamage;
	}

	if (bIsDead)
	{
		return AppliedDamage;
	}

	if (AppliedDamage <= 0.0f)
	{
		return AppliedDamage;
	}

	if (CharacterStatsComponent)
	{
		return CharacterStatsComponent->ApplyDamage(AppliedDamage, EventInstigator, DamageCauser);
	}

	return AppliedDamage;
}

UPNInventoryComponent* APNBaseCharacter::GetInventoryComponent() const
{
	return InventoryComponent;
}

UPNPlayerHUDComponent* APNBaseCharacter::GetPlayerHUDComponent() const
{
	return PlayerHUDComponent;
}

UPNInventoryComponent* APNBaseCharacter::GetBackpackInventoryComponent() const
{
	return nullptr;
}

UPNInventoryComponent* APNBaseCharacter::GetVestInventoryComponent() const
{
	return nullptr;
}

bool APNBaseCharacter::HasActiveBackpackInventory() const
{
	return false;
}

bool APNBaseCharacter::HasActiveVestInventory() const
{
	return false;
}

void APNBaseCharacter::RefreshEquipmentInventories()
{
	if (!HasAuthority())
	{
		return;
	}

	ApplyPlayerInventoryCapacityFromEquipment();
}

void APNBaseCharacter::HandleEquipmentChanged()
{
	if (!HasAuthority())
	{
		return;
	}

	RefreshEquipmentInventories();
}

void APNBaseCharacter::ApplyPlayerInventoryCapacityFromEquipment()
{
	if (!InventoryComponent)
	{
		return;
	}

	UPNItemDataAsset* BackpackData = EquipmentComponent
		? EquipmentComponent->GetEquippedItemData(EPNEquipmentSlot::Backpack)
		: nullptr;

	const int32 BackpackExtraSlots = BackpackData
		? FMath::Max(0, BackpackData->BackpackStats.MaxSlots)
		: 0;

	float NewMaxWeight = DefaultBaseInventoryMaxWeight;

	if (CharacterStatsComponent)
	{
		NewMaxWeight = CharacterStatsComponent->GetMaxWeight();
	}
	else if (BackpackData)
	{
		NewMaxWeight += FMath::Max(0.0f, BackpackData->BackpackStats.MaxWeight);
	}

	InventoryComponent->SetPlayerInventoryCapacity(
		DefaultBaseInventorySlots,
		BackpackExtraSlots,
		DefaultInventoryColumns,
		NewMaxWeight,
		DefaultInventoryMaxVisualSlots
	);
}

UPNInventoryActionComponent* APNBaseCharacter::GetInventoryActionComponent() const
{
	return InventoryActionComponent;
}

UPNInventoryContainerComponent* APNBaseCharacter::GetInventoryContainerComponent() const
{
	return InventoryContainerComponent;
}

UPNEquipmentComponent* APNBaseCharacter::GetEquipmentComponent() const
{
	return EquipmentComponent;
}

UPNEquipmentVisualComponent* APNBaseCharacter::GetEquipmentVisualComponent() const
{
	return EquipmentVisualComponent;
}

UPNCharacterStatsComponent* APNBaseCharacter::GetCharacterStatsComponent() const
{
	return CharacterStatsComponent;
}

UPNQuickSlotComponent* APNBaseCharacter::GetQuickSlotComponent() const
{
	return QuickSlotComponent;
}

USkeletalMeshComponent* APNBaseCharacter::GetThirdPersonHeadMeshComponent() const
{
	return ThirdPersonHeadMeshComponent;
}

USkeletalMeshComponent* APNBaseCharacter::GetThirdPersonLegsMeshComponent() const
{
	return ThirdPersonLegsMeshComponent;
}

USkeletalMeshComponent* APNBaseCharacter::GetThirdPersonHandsMeshComponent() const
{
	return ThirdPersonHandsMeshComponent;
}

void APNBaseCharacter::SetModularCharacterMeshes(
	USkeletalMesh* NewMasterBodyMesh,
	USkeletalMesh* NewHeadMesh,
	USkeletalMesh* NewLegsMesh,
	USkeletalMesh* NewHandsMesh
)
{
	if (!HasAuthority())
	{
		return;
	}

	ThirdPersonMasterBodyMeshAsset = NewMasterBodyMesh;
	ThirdPersonHeadMeshAsset = NewHeadMesh;
	ThirdPersonLegsMeshAsset = NewLegsMesh;
	ThirdPersonHandsMeshAsset = NewHandsMesh;

	ApplyModularCharacterMeshes();
}

void APNBaseCharacter::ApplyModularCharacterMeshes()
{
	if (GetMesh())
	{
		GetMesh()->SetSkeletalMesh(ThirdPersonMasterBodyMeshAsset);
	}

	if (ThirdPersonHeadMeshComponent)
	{
		ThirdPersonHeadMeshComponent->SetSkeletalMesh(ThirdPersonHeadMeshAsset);
		ThirdPersonHeadMeshComponent->SetLeaderPoseComponent(GetMesh());
	}

	if (ThirdPersonLegsMeshComponent)
	{
		ThirdPersonLegsMeshComponent->SetSkeletalMesh(ThirdPersonLegsMeshAsset);
		ThirdPersonLegsMeshComponent->SetLeaderPoseComponent(GetMesh());
	}

	if (ThirdPersonHandsMeshComponent)
	{
		ThirdPersonHandsMeshComponent->SetSkeletalMesh(ThirdPersonHandsMeshAsset);
		ThirdPersonHandsMeshComponent->SetLeaderPoseComponent(GetMesh());
	}

	SetupThirdPersonMeshVisibility();
}

bool APNBaseCharacter::IsSprinting() const
{
	return bIsSprinting;
}

bool APNBaseCharacter::IsDead() const
{
	return bIsDead;
}

void APNBaseCharacter::StartSprint()
{
	if (bIsDead || bIsCrouched)
	{
		return;
	}

	if (CharacterStatsComponent && !CharacterStatsComponent->CanSprint())
	{
		return;
	}

	if (HasAuthority())
	{
		SetSprintingInternal(true);
		return;
	}

	Server_SetSprinting(true);
}

void APNBaseCharacter::StopSprint()
{
	if (HasAuthority())
	{
		SetSprintingInternal(false);
		return;
	}

	Server_SetSprinting(false);
}

void APNBaseCharacter::Die(AController* KillerController)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	bIsSprinting = false;

	if (CharacterStatsComponent)
	{
		CharacterStatsComponent->SetSprintDrainEnabled(false);
	}

	ApplyMovementSpeed();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	OnRep_IsDead();
}

void APNBaseCharacter::Revive()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsDead = false;
	bIsSprinting = false;

	if (CharacterStatsComponent)
	{
		CharacterStatsComponent->ResetForRespawn();
		CharacterStatsComponent->SetSprintDrainEnabled(false);
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	ApplyMovementSpeed();

	OnRep_IsDead();
}

void APNBaseCharacter::Server_SetSprinting_Implementation(bool bNewSprinting)
{
	if (bNewSprinting && CharacterStatsComponent && !CharacterStatsComponent->CanSprint())
	{
		bNewSprinting = false;
	}

	SetSprintingInternal(bNewSprinting);
}

void APNBaseCharacter::SetSprintingInternal(bool bNewSprinting)
{
	if (bIsDead || bIsCrouched)
	{
		bNewSprinting = false;
	}

	if (bNewSprinting && CharacterStatsComponent && !CharacterStatsComponent->CanSprint())
	{
		bNewSprinting = false;
	}

	if (bIsSprinting == bNewSprinting)
	{
		if (CharacterStatsComponent)
		{
			CharacterStatsComponent->SetSprintDrainEnabled(bIsSprinting);
		}

		return;
	}

	bIsSprinting = bNewSprinting;

	if (CharacterStatsComponent)
	{
		CharacterStatsComponent->SetSprintDrainEnabled(bIsSprinting);
	}

	ApplyMovementSpeed();

	OnRep_IsSprinting();
}

void APNBaseCharacter::ApplyMovementSpeed()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	if (bIsDead)
	{
		GetCharacterMovement()->MaxWalkSpeed = 0.0f;
		GetCharacterMovement()->MaxWalkSpeedCrouched = 0.0f;
		return;
	}

	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

	if (bIsSprinting && !bIsCrouched)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APNBaseCharacter::SetupThirdPersonMeshVisibility()
{
	if (GetMesh())
	{
		GetMesh()->SetOwnerNoSee(true);
		GetMesh()->SetOnlyOwnerSee(false);
	}

	if (ThirdPersonHeadMeshComponent)
	{
		ThirdPersonHeadMeshComponent->SetOwnerNoSee(true);
		ThirdPersonHeadMeshComponent->SetOnlyOwnerSee(false);
	}

	if (ThirdPersonLegsMeshComponent)
	{
		ThirdPersonLegsMeshComponent->SetOwnerNoSee(true);
		ThirdPersonLegsMeshComponent->SetOnlyOwnerSee(false);
	}

	if (ThirdPersonHandsMeshComponent)
	{
		ThirdPersonHandsMeshComponent->SetOwnerNoSee(true);
		ThirdPersonHandsMeshComponent->SetOnlyOwnerSee(false);
	}
}

void APNBaseCharacter::OnRep_ModularMeshes()
{
	ApplyModularCharacterMeshes();
}

void APNBaseCharacter::OnRep_IsSprinting()
{
	ApplyMovementSpeed();
}

void APNBaseCharacter::OnRep_IsDead()
{
	ApplyMovementSpeed();
}