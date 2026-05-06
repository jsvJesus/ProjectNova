#include "Inventory/PNInventoryContainerActor.h"

#include "Characters/PNBaseCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Inventory/PNInventoryComponent.h"
#include "Inventory/PNInventoryContainerComponent.h"

APNInventoryContainerActor::APNInventoryContainerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

	InventoryComponent = CreateDefaultSubobject<UPNInventoryComponent>(TEXT("InventoryComponent"));
	if (InventoryComponent)
	{
		InventoryComponent->SetIsReplicated(true);
	}

	ContainerSettings.InventoryType = EPNInventoryType::LootContainer;
	ContainerSettings.GridSize.Columns = 8;
	ContainerSettings.GridSize.Rows = 6;
	ContainerSettings.bUseWeightLimit = false;
	ContainerSettings.MaxWeight = 0.0f;
	ContainerSettings.bAllowItemRotation = true;
	ContainerSettings.bAllowStacking = true;
	ContainerSettings.bCanReceiveItems = true;
	ContainerSettings.bCanRemoveItems = true;
	ContainerSettings.bCanDropItems = false;
	ContainerSettings.bCanTradeItems = false;
}

void APNInventoryContainerActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bInitializeInventoryOnBeginPlay)
	{
		InitializeContainerInventory();
	}
}

UPNInventoryComponent* APNInventoryContainerActor::GetInventoryComponent() const
{
	return InventoryComponent;
}

void APNInventoryContainerActor::InitializeContainerInventory()
{
	if (!HasAuthority() || !InventoryComponent)
	{
		return;
	}

	FPNInventorySettings SettingsToApply = ContainerSettings;
	SettingsToApply.InventoryType = ContainerType;

	InventoryComponent->InitializeInventory(SettingsToApply);
}

FText APNInventoryContainerActor::GetInteractionDisplayName_Implementation(APawn* InteractingPawn) const
{
	return DisplayName;
}

FText APNInventoryContainerActor::GetInteractionActionText_Implementation(APawn* InteractingPawn) const
{
	return ActionText;
}

bool APNInventoryContainerActor::CanInteract_Implementation(APawn* InteractingPawn) const
{
	if (!InteractingPawn || !InventoryComponent)
	{
		return false;
	}

	return true;
}

bool APNInventoryContainerActor::Interact_Implementation(APawn* InteractingPawn)
{
	if (!InteractingPawn || !InventoryComponent)
	{
		return false;
	}

	APNBaseCharacter* Character = Cast<APNBaseCharacter>(InteractingPawn);
	if (!Character)
	{
		return false;
	}

	UPNInventoryContainerComponent* ContainerComponent = Character->GetInventoryContainerComponent();
	if (!ContainerComponent)
	{
		return false;
	}

	ContainerComponent->RequestOpenContainer(InventoryComponent);
	return true;
}