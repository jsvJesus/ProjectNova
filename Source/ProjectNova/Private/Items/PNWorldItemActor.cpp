#include "Items/PNWorldItemActor.h"

#include "Characters/PNBaseCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Items/PNInventoryComponent.h"
#include "Items/PNItemDataAsset.h"
#include "Items/PNItemInstance.h"

APNWorldItemActor::APNWorldItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetReplicates(true);
	SetReplicateMovement(true);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	MeshComponent->SetSimulatePhysics(false);

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(RootComponent);
	PickupSphere->SetSphereRadius(InteractionRadius);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionObjectType(ECC_WorldDynamic);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupSphere->SetGenerateOverlapEvents(true);
}

void APNWorldItemActor::BeginPlay()
{
	Super::BeginPlay();

	ApplyInteractionRadius();

	if (bInitializeOnBeginPlay && !ItemInstance && DefaultItemData)
	{
		InitializeFromData(DefaultItemData, DefaultQuantity);
	}
	else
	{
		RefreshVisual();
	}

	SetWorldItemPhysicsEnabled(bSimulatePhysicsOnSpawn);
}

void APNWorldItemActor::InitializeFromData(UPNItemDataAsset* InItemData, int32 InQuantity)
{
	if (!InItemData)
	{
		ItemInstance = nullptr;
		RefreshVisual();
		BroadcastWorldItemChanged();
		return;
	}

	UPNItemInstance* NewInstance = NewObject<UPNItemInstance>(this);
	if (!NewInstance)
	{
		ItemInstance = nullptr;
		RefreshVisual();
		BroadcastWorldItemChanged();
		return;
	}

	NewInstance->Initialize(InItemData, FMath::Max(1, InQuantity));
	ItemInstance = NewInstance;

	DefaultItemData = InItemData;
	DefaultQuantity = FMath::Max(1, InQuantity);

	RefreshVisual();
	BroadcastWorldItemChanged();
}

void APNWorldItemActor::InitializeFromInstance(UPNItemInstance* InItemInstance)
{
	ItemInstance = DuplicateInstanceForWorld(InItemInstance);

	if (ItemInstance)
	{
		DefaultItemData = ItemInstance->GetItemData();
		DefaultQuantity = ItemInstance->Quantity;
	}
	else
	{
		DefaultItemData = nullptr;
		DefaultQuantity = 1;
	}

	RefreshVisual();
	BroadcastWorldItemChanged();
}

bool APNWorldItemActor::PickupToInventory(UPNInventoryComponent* TargetInventory)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!TargetInventory || !CanPickup())
	{
		return false;
	}

	FPNInventoryAddItemResult AddResult = TargetInventory->AddItem(ItemInstance, true, true);

	if (AddResult.AddedQuantity <= 0)
	{
		return false;
	}

	if (AddResult.RemainingQuantity <= 0)
	{
		if (bDestroyWhenPickedUp)
		{
			Destroy();
		}
		else
		{
			ItemInstance = nullptr;
			RefreshVisual();
			BroadcastWorldItemChanged();
		}

		return true;
	}

	ItemInstance->SetQuantityClamped(AddResult.RemainingQuantity);
	DefaultQuantity = ItemInstance->Quantity;

	RefreshVisual();
	BroadcastWorldItemChanged();

	return true;
}

void APNWorldItemActor::RefreshVisual()
{
	UPNItemDataAsset* Data = GetItemData();

	if (!MeshComponent)
	{
		return;
	}

	if (!Data)
	{
		MeshComponent->SetStaticMesh(nullptr);
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		return;
	}

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	UStaticMesh* LoadedStaticMesh = Data->Visual.StaticMesh.LoadSynchronous();
	MeshComponent->SetStaticMesh(LoadedStaticMesh);
}

void APNWorldItemActor::SetQuantity(int32 NewQuantity)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ItemInstance)
	{
		return;
	}

	ItemInstance->SetQuantityClamped(NewQuantity);
	DefaultQuantity = ItemInstance->Quantity;

	if (ItemInstance->IsEmpty())
	{
		if (bDestroyWhenPickedUp)
		{
			Destroy();
			return;
		}

		ItemInstance = nullptr;
	}

	RefreshVisual();
	BroadcastWorldItemChanged();
}

void APNWorldItemActor::SetWorldItemPhysicsEnabled(bool bEnabled)
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetSimulatePhysics(bEnabled);
	MeshComponent->SetEnableGravity(bEnabled);
}

UPNItemInstance* APNWorldItemActor::GetItemInstance() const
{
	return ItemInstance;
}

UPNItemDataAsset* APNWorldItemActor::GetItemData() const
{
	if (ItemInstance)
	{
		return ItemInstance->GetItemData();
	}

	return DefaultItemData.Get();
}

int32 APNWorldItemActor::GetQuantity() const
{
	return ItemInstance ? ItemInstance->Quantity : 0;
}

bool APNWorldItemActor::IsValidWorldItem() const
{
	return ItemInstance && ItemInstance->IsValidItem();
}

bool APNWorldItemActor::CanPickup() const
{
	return IsValidWorldItem();
}

FText APNWorldItemActor::GetInteractionDisplayName_Implementation(APawn* InteractingPawn) const
{
	UPNItemDataAsset* Data = GetItemData();

	if (!Data)
	{
		return FText::FromString(TEXT("Unknown Item"));
	}

	return Data->GetItemName();
}

FText APNWorldItemActor::GetInteractionActionText_Implementation(APawn* InteractingPawn) const
{
	return FText::FromString(TEXT("Pick Up"));
}

bool APNWorldItemActor::CanInteract_Implementation(APawn* InteractingPawn) const
{
	if (!InteractingPawn)
	{
		return false;
	}

	return CanPickup();
}

bool APNWorldItemActor::Interact_Implementation(APawn* InteractingPawn)
{
	if (!HasAuthority())
	{
		return false;
	}

	APNBaseCharacter* Character = Cast<APNBaseCharacter>(InteractingPawn);
	if (!Character)
	{
		return false;
	}

	UPNInventoryComponent* TargetInventory = Character->GetInventoryComponent();
	if (!TargetInventory)
	{
		return false;
	}

	return PickupToInventory(TargetInventory);
}

UPNItemInstance* APNWorldItemActor::DuplicateInstanceForWorld(UPNItemInstance* SourceInstance)
{
	if (!SourceInstance || !SourceInstance->GetItemData())
	{
		return nullptr;
	}

	UPNItemInstance* NewInstance = NewObject<UPNItemInstance>(this);
	if (!NewInstance)
	{
		return nullptr;
	}

	NewInstance->Initialize(SourceInstance->GetItemData(), SourceInstance->Quantity);

	NewInstance->CurrentDurability = SourceInstance->CurrentDurability;
	NewInstance->CurrentBatteryCharge = SourceInstance->CurrentBatteryCharge;
	NewInstance->RemainingShelfLifeSeconds = SourceInstance->RemainingShelfLifeSeconds;
	NewInstance->AmmoInMagazine = SourceInstance->AmmoInMagazine;
	NewInstance->bInitialized = true;
	NewInstance->SetQuantityClamped(SourceInstance->Quantity);

	return NewInstance;
}

void APNWorldItemActor::ApplyInteractionRadius()
{
	if (!PickupSphere)
	{
		return;
	}

	PickupSphere->SetSphereRadius(FMath::Max(10.0f, InteractionRadius));
}

void APNWorldItemActor::BroadcastWorldItemChanged()
{
	OnWorldItemChanged.Broadcast();
}