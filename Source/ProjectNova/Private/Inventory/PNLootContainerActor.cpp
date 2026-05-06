#include "Inventory/PNLootContainerActor.h"

#include "Inventory/PNInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

APNLootContainerActor::APNLootContainerActor()
{
	ContainerType = EPNInventoryType::LootContainer;

	DisplayName = FText::FromString(TEXT("Loot Container"));
	ActionText = FText::FromString(TEXT("Open"));

	ContainerSettings.InventoryType = EPNInventoryType::LootContainer;
	ContainerSettings.GridSize.Columns = 8;
	ContainerSettings.GridSize.Rows = 6;
	ContainerSettings.bUseWeightLimit = false;
	ContainerSettings.MaxWeight = 0.0f;
	ContainerSettings.bAllowItemRotation = true;
	ContainerSettings.bAllowStacking = true;

	// Важно: лут-контейнер только отдаёт предметы.
	// Игрок не может положить предмет внутрь мешка/сундука/стеллажа.
	ContainerSettings.bCanReceiveItems = false;
	ContainerSettings.bCanRemoveItems = true;
	ContainerSettings.bCanDropItems = false;
	ContainerSettings.bCanTradeItems = false;
}

void APNLootContainerActor::BeginPlay()
{
	Super::BeginPlay();

	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &APNLootContainerActor::HandleLootInventoryChanged);
	}

	if (HasAuthority())
	{
		HandleLootInventoryChanged();
	}
}

void APNLootContainerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APNLootContainerActor, bLootRespawnBlocked);
}

bool APNLootContainerActor::HasLootItems() const
{
	return InventoryComponent && InventoryComponent->GetInventoryItemCount() > 0;
}

bool APNLootContainerActor::IsLootRespawnBlocked() const
{
	return bLootRespawnBlocked;
}

void APNLootContainerActor::StartLootRespawnCooldown()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bRespawnWhenEmpty)
	{
		bLootRespawnBlocked = true;
		return;
	}

	bLootRespawnBlocked = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LootRespawnTimerHandle);

		if (LootRespawnSeconds <= 0.0f)
		{
			FinishLootRespawnCooldown();
			return;
		}

		World->GetTimerManager().SetTimer(
			LootRespawnTimerHandle,
			this,
			&APNLootContainerActor::FinishLootRespawnCooldown,
			LootRespawnSeconds,
			false
		);
	}
}

void APNLootContainerActor::FinishLootRespawnCooldown()
{
	if (!HasAuthority())
	{
		return;
	}

	bLootRespawnBlocked = false;

	// Пока LootTable нет — сюда в Blueprint можно временно заспавнить предметы руками.
	// Позже Stage LootTable будет заполнять InventoryComponent на сервере.
	BP_RespawnLoot();

	HandleLootInventoryChanged();
}

bool APNLootContainerActor::CanInteract_Implementation(APawn* InteractingPawn) const
{
	if (!Super::CanInteract_Implementation(InteractingPawn))
	{
		return false;
	}

	if (bLootRespawnBlocked)
	{
		return false;
	}

	if (!bCanOpenEmptyLootContainer && !HasLootItems())
	{
		return false;
	}

	return true;
}

bool APNLootContainerActor::Interact_Implementation(APawn* InteractingPawn)
{
	if (!CanInteract_Implementation(InteractingPawn))
	{
		return false;
	}

	return Super::Interact_Implementation(InteractingPawn);
}

void APNLootContainerActor::HandleLootInventoryChanged()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bRespawnWhenEmpty)
	{
		return;
	}

	if (bLootRespawnBlocked)
	{
		return;
	}

	if (!HasLootItems())
	{
		StartLootRespawnCooldown();
	}
}

void APNLootContainerActor::OnRep_LootRespawnState()
{
}