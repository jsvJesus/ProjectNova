#include "Items/PNInventoryActionComponent.h"

#include "Characters/PNBaseCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Equipment/PNEquipmentComponent.h"
#include "Items/PNInventoryComponent.h"
#include "Items/PNItemDataAsset.h"
#include "Items/PNItemInstance.h"
#include "Items/PNWorldItemActor.h"

UPNInventoryActionComponent::UPNInventoryActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPNInventoryActionComponent::BeginPlay()
{
	Super::BeginPlay();
}

FPNInventoryActionResponse UPNInventoryActionComponent::ExecuteInventoryAction(const FPNInventoryActionRequest& Request)
{
	if (!HasActionAuthority())
	{
		FPNInventoryActionResponse Response = MakeResponse(Request, EPNInventoryActionResult::InvalidActionComponent);
		BroadcastActionCompleted(Response);
		return Response;
	}

	FPNInventoryActionResponse Response;

	switch (Request.ActionType)
	{
	case EPNInventoryActionType::Use:
		Response = HandleUseAction(Request);
		break;

	case EPNInventoryActionType::Equip:
		Response = HandleEquipAction(Request);
		break;

	case EPNInventoryActionType::Unequip:
		Response = HandleUnequipAction(Request);
		break;

	case EPNInventoryActionType::Move:
		Response = HandleMoveAction(Request);
		break;

	case EPNInventoryActionType::MergeStack:
		Response = HandleMergeStackAction(Request);
		break;

	case EPNInventoryActionType::SplitStack:
		Response = HandleSplitStackAction(Request);
		break;

	case EPNInventoryActionType::Drop:
		Response = HandleDropAction(Request);
		break;

	case EPNInventoryActionType::Rotate:
		Response = HandleRotateAction(Request);
		break;

	case EPNInventoryActionType::Inspect:
		Response = HandleInspectAction(Request);
		break;

	default:
		Response = MakeResponse(Request, EPNInventoryActionResult::InvalidAction);
		break;
	}

	BroadcastActionCompleted(Response);
	return Response;
}

void UPNInventoryActionComponent::RequestInventoryAction(const FPNInventoryActionRequest& Request)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		ExecuteInventoryAction(Request);
		return;
	}

	Server_ExecuteInventoryAction(Request);
}

void UPNInventoryActionComponent::Server_ExecuteInventoryAction_Implementation(FPNInventoryActionRequest Request)
{
	ExecuteInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestUseInventoryItem(FPNInventoryGridPosition InventoryPosition)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Use;
	Request.Source.Container = EPNInventoryActionContainer::Inventory;
	Request.Source.InventoryPosition = InventoryPosition;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestUseQuickSlotItem(int32 QuickSlotIndex)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Use;
	Request.Source.Container = EPNInventoryActionContainer::QuickSlot;
	Request.Source.QuickSlotIndex = QuickSlotIndex;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestEquipInventoryItem(FPNInventoryGridPosition InventoryPosition, EPNEquipmentSlot TargetSlot)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Equip;
	Request.Source.Container = EPNInventoryActionContainer::Inventory;
	Request.Source.InventoryPosition = InventoryPosition;
	Request.Destination.Container = EPNInventoryActionContainer::EquipmentSlot;
	Request.Destination.EquipmentSlot = TargetSlot;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestUnequipItem(EPNEquipmentSlot SourceSlot)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Unequip;
	Request.Source.Container = EPNInventoryActionContainer::EquipmentSlot;
	Request.Source.EquipmentSlot = SourceSlot;
	Request.Destination.Container = EPNInventoryActionContainer::Inventory;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestMoveInventoryItem(FPNInventoryGridPosition FromPosition, FPNInventoryGridPosition ToPosition, bool bRotated)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Move;
	Request.Source.Container = EPNInventoryActionContainer::Inventory;
	Request.Source.InventoryPosition = FromPosition;
	Request.Destination.Container = EPNInventoryActionContainer::Inventory;
	Request.Destination.InventoryPosition = ToPosition;
	Request.Destination.bRotated = bRotated;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestMoveInventoryItemToQuickSlot(FPNInventoryGridPosition FromPosition, int32 QuickSlotIndex)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Move;
	Request.Source.Container = EPNInventoryActionContainer::Inventory;
	Request.Source.InventoryPosition = FromPosition;
	Request.Destination.Container = EPNInventoryActionContainer::QuickSlot;
	Request.Destination.QuickSlotIndex = QuickSlotIndex;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestMoveQuickSlotItemToInventory(int32 QuickSlotIndex, FPNInventoryGridPosition ToPosition, bool bRotated)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Move;
	Request.Source.Container = EPNInventoryActionContainer::QuickSlot;
	Request.Source.QuickSlotIndex = QuickSlotIndex;
	Request.Destination.Container = EPNInventoryActionContainer::Inventory;
	Request.Destination.InventoryPosition = ToPosition;
	Request.Destination.bRotated = bRotated;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestSplitInventoryStack(FPNInventoryGridPosition FromPosition, FPNInventoryGridPosition ToPosition, int32 Quantity)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::SplitStack;
	Request.Source.Container = EPNInventoryActionContainer::Inventory;
	Request.Source.InventoryPosition = FromPosition;
	Request.Destination.Container = EPNInventoryActionContainer::Inventory;
	Request.Destination.InventoryPosition = ToPosition;
	Request.Quantity = Quantity;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestSplitInventoryStackHalf(FPNInventoryGridPosition FromPosition, FPNInventoryGridPosition ToPosition)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::SplitStack;
	Request.Source.Container = EPNInventoryActionContainer::Inventory;
	Request.Source.InventoryPosition = FromPosition;
	Request.Destination.Container = EPNInventoryActionContainer::Inventory;
	Request.Destination.InventoryPosition = ToPosition;
	Request.Quantity = -1;
	Request.bHalfStack = true;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestMergeInventoryStacks(FPNInventoryGridPosition FromPosition, FPNInventoryGridPosition ToPosition, int32 Quantity)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::MergeStack;
	Request.Source.Container = EPNInventoryActionContainer::Inventory;
	Request.Source.InventoryPosition = FromPosition;
	Request.Destination.Container = EPNInventoryActionContainer::Inventory;
	Request.Destination.InventoryPosition = ToPosition;
	Request.Quantity = Quantity;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestDropInventoryItem(FPNInventoryGridPosition InventoryPosition, int32 Quantity)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Drop;
	Request.Source.Container = EPNInventoryActionContainer::Inventory;
	Request.Source.InventoryPosition = InventoryPosition;
	Request.Destination.Container = EPNInventoryActionContainer::World;
	Request.Quantity = Quantity;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestDropQuickSlotItem(int32 QuickSlotIndex, int32 Quantity)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Drop;
	Request.Source.Container = EPNInventoryActionContainer::QuickSlot;
	Request.Source.QuickSlotIndex = QuickSlotIndex;
	Request.Destination.Container = EPNInventoryActionContainer::World;
	Request.Quantity = Quantity;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestRotateInventoryItem(FPNInventoryGridPosition InventoryPosition)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Rotate;
	Request.Source.Container = EPNInventoryActionContainer::Inventory;
	Request.Source.InventoryPosition = InventoryPosition;

	RequestInventoryAction(Request);
}

FPNInventoryActionResponse UPNInventoryActionComponent::InspectTarget(const FPNInventoryActionTarget& Target) const
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Inspect;
	Request.Source = Target;

	return HandleInspectAction(Request);
}

FPNInventoryActionResponse UPNInventoryActionComponent::HandleUseAction(const FPNInventoryActionRequest& Request)
{
	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidInventory);
	}

	if (Request.Source.Container == EPNInventoryActionContainer::Inventory)
	{
		const FPNInventoryUseItemResponse UseResponse = InventoryComponent->UseItemFromInventoryPosition(Request.Source.InventoryPosition);
		return MakeResponse(
			Request,
			UseResponse.bSuccess ? EPNInventoryActionResult::Success : EPNInventoryActionResult::UseFailed,
			UseResponse.bSuccess,
			UseResponse.ItemData,
			UseResponse.UsedQuantity
		);
	}

	if (Request.Source.Container == EPNInventoryActionContainer::QuickSlot)
	{
		const FPNInventoryUseItemResponse UseResponse = InventoryComponent->UseItemFromQuickSlot(Request.Source.QuickSlotIndex);
		return MakeResponse(
			Request,
			UseResponse.bSuccess ? EPNInventoryActionResult::Success : EPNInventoryActionResult::UseFailed,
			UseResponse.bSuccess,
			UseResponse.ItemData,
			UseResponse.UsedQuantity
		);
	}

	return MakeResponse(Request, EPNInventoryActionResult::InvalidSource);
}

FPNInventoryActionResponse UPNInventoryActionComponent::HandleEquipAction(const FPNInventoryActionRequest& Request)
{
	UPNEquipmentComponent* EquipmentComponent = GetOwnerEquipmentComponent();
	if (!EquipmentComponent)
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidEquipment);
	}

	if (!Request.Source.IsInventory())
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidSource);
	}

	if (Request.Destination.IsEquipmentSlot())
	{
		const FPNEquipmentOperationResponse EquipResponse = EquipmentComponent->EquipFromInventoryToSlot(
			Request.Source.InventoryPosition,
			Request.Destination.EquipmentSlot
		);

		return MakeResponse(
			Request,
			EquipResponse.bSuccess ? EPNInventoryActionResult::Success : EPNInventoryActionResult::EquipFailed,
			EquipResponse.bSuccess,
			EquipResponse.ItemData,
			EquipResponse.bSuccess ? 1 : 0
		);
	}

	if (Request.Destination.IsEquipmentInternal())
	{
		const FPNEquipmentOperationResponse InsertResponse = EquipmentComponent->InsertFromInventoryToInternalSlot(
			Request.Source.InventoryPosition,
			Request.Destination.InternalContainer,
			Request.Destination.InternalSlotIndex
		);

		return MakeResponse(
			Request,
			InsertResponse.bSuccess ? EPNInventoryActionResult::Success : EPNInventoryActionResult::EquipFailed,
			InsertResponse.bSuccess,
			InsertResponse.ItemData,
			InsertResponse.bSuccess ? 1 : 0
		);
	}

	return MakeResponse(Request, EPNInventoryActionResult::InvalidDestination);
}

FPNInventoryActionResponse UPNInventoryActionComponent::HandleUnequipAction(const FPNInventoryActionRequest& Request)
{
	UPNEquipmentComponent* EquipmentComponent = GetOwnerEquipmentComponent();
	if (!EquipmentComponent)
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidEquipment);
	}

	if (Request.Source.IsEquipmentSlot())
	{
		const FPNEquipmentOperationResponse UnequipResponse = EquipmentComponent->UnequipSlotToInventory(Request.Source.EquipmentSlot);

		return MakeResponse(
			Request,
			UnequipResponse.bSuccess ? EPNInventoryActionResult::Success : EPNInventoryActionResult::UnequipFailed,
			UnequipResponse.bSuccess,
			UnequipResponse.ItemData,
			UnequipResponse.bSuccess ? 1 : 0
		);
	}

	if (Request.Source.IsEquipmentInternal())
	{
		const FPNEquipmentOperationResponse RemoveResponse = EquipmentComponent->RemoveInternalSlotToInventory(
			Request.Source.InternalContainer,
			Request.Source.InternalSlotIndex
		);

		return MakeResponse(
			Request,
			RemoveResponse.bSuccess ? EPNInventoryActionResult::Success : EPNInventoryActionResult::UnequipFailed,
			RemoveResponse.bSuccess,
			RemoveResponse.ItemData,
			RemoveResponse.bSuccess ? 1 : 0
		);
	}

	return MakeResponse(Request, EPNInventoryActionResult::InvalidSource);
}

FPNInventoryActionResponse UPNInventoryActionComponent::HandleMoveAction(const FPNInventoryActionRequest& Request)
{
	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidInventory);
	}

	if (Request.Source.IsInventory() && Request.Destination.IsInventory())
	{
		UPNItemInstance* SourceItem = InventoryComponent->GetItemAtPosition(Request.Source.InventoryPosition);
		if (!SourceItem)
		{
			return MakeResponse(Request, EPNInventoryActionResult::SourceEmpty);
		}

		const int32 Quantity = ResolveInventoryQuantity(SourceItem, Request);

		if (Quantity <= 0 || Quantity >= SourceItem->Quantity)
		{
			const FPNInventoryMoveItemResult MoveResult = InventoryComponent->MoveItemFromPosition(
				Request.Source.InventoryPosition,
				Request.Destination.InventoryPosition,
				Request.Destination.bRotated
			);

			return MakeResponse(
				Request,
				MoveResult.bSuccess ? EPNInventoryActionResult::Success : EPNInventoryActionResult::MoveFailed,
				MoveResult.bSuccess,
				SourceItem->GetItemData(),
				SourceItem->Quantity
			);
		}

		FPNInventoryActionRequest SplitRequest = Request;
		SplitRequest.ActionType = EPNInventoryActionType::SplitStack;
		SplitRequest.Quantity = Quantity;

		return HandleSplitStackAction(SplitRequest);
	}

	if (Request.Source.IsInventory() && Request.Destination.IsQuickSlot())
	{
		const FPNInventoryAddItemResult MoveResult = InventoryComponent->MoveItemFromInventoryToQuickSlot(
			Request.Source.InventoryPosition,
			Request.Destination.QuickSlotIndex
		);

		return MakeResponse(
			Request,
			MoveResult.bSuccess ? EPNInventoryActionResult::Success : EPNInventoryActionResult::MoveFailed,
			MoveResult.bSuccess,
			MoveResult.TargetItemInstance ? MoveResult.TargetItemInstance->GetItemData() : nullptr,
			MoveResult.AddedQuantity
		);
	}

	if (Request.Source.IsQuickSlot() && Request.Destination.IsInventory())
	{
		const FPNInventoryQuickSlotEntry SlotEntry = InventoryComponent->GetQuickSlotEntry(Request.Source.QuickSlotIndex);
		if (SlotEntry.IsEmpty())
		{
			return MakeResponse(Request, EPNInventoryActionResult::SourceEmpty);
		}

		const int32 Quantity = ResolveQuickSlotQuantity(SlotEntry, Request);
		if (Quantity <= 0)
		{
			return MakeResponse(Request, EPNInventoryActionResult::InvalidQuantity);
		}

		FPNInventoryRemoveItemResult RemoveResult = InventoryComponent->RemoveItemFromQuickSlot(Request.Source.QuickSlotIndex, Quantity);
		if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance)
		{
			return MakeResponse(Request, EPNInventoryActionResult::MoveFailed);
		}

		FPNInventoryAddItemResult AddResult = InventoryComponent->AddItemAtPosition(
			RemoveResult.RemovedItemInstance,
			Request.Destination.InventoryPosition,
			Request.Destination.bRotated,
			true
		);

		if (!AddResult.bSuccess)
		{
			InventoryComponent->AddItemToQuickSlot(RemoveResult.RemovedItemInstance, Request.Source.QuickSlotIndex);
			return MakeResponse(Request, EPNInventoryActionResult::MoveFailed);
		}

		return MakeResponse(
			Request,
			EPNInventoryActionResult::Success,
			true,
			RemoveResult.RemovedItemInstance->GetItemData(),
			AddResult.AddedQuantity
		);
	}

	if (Request.Source.IsInventory() && (Request.Destination.IsEquipmentSlot() || Request.Destination.IsEquipmentInternal()))
	{
		return HandleEquipAction(Request);
	}

	if (Request.Source.IsEquipmentSlot() || Request.Source.IsEquipmentInternal())
	{
		return HandleUnequipAction(Request);
	}

	return MakeResponse(Request, EPNInventoryActionResult::InvalidAction);
}

FPNInventoryActionResponse UPNInventoryActionComponent::HandleMergeStackAction(const FPNInventoryActionRequest& Request)
{
	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidInventory);
	}

	if (!Request.Source.IsInventory() || !Request.Destination.IsInventory())
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidSource);
	}

	UPNItemInstance* SourceItem = InventoryComponent->GetItemAtPosition(Request.Source.InventoryPosition);
	if (!SourceItem || !SourceItem->IsValidItem())
	{
		return MakeResponse(Request, EPNInventoryActionResult::SourceEmpty);
	}

	UPNItemInstance* TargetItem = InventoryComponent->GetItemAtPosition(Request.Destination.InventoryPosition);
	if (!TargetItem || !TargetItem->IsValidItem())
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidDestination);
	}

	if (!TargetItem->CanStackWith(SourceItem))
	{
		return MakeResponse(Request, EPNInventoryActionResult::MergeFailed);
	}

	const int32 Quantity = ResolveInventoryQuantity(SourceItem, Request);
	if (Quantity <= 0)
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidQuantity);
	}

	FPNInventoryRemoveItemResult RemoveResult = InventoryComponent->RemoveItemAtPosition(Request.Source.InventoryPosition, Quantity);
	if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance)
	{
		return MakeResponse(Request, EPNInventoryActionResult::MergeFailed);
	}

	FPNInventoryAddItemResult AddResult = InventoryComponent->AddItemAtPosition(
		RemoveResult.RemovedItemInstance,
		Request.Destination.InventoryPosition,
		false,
		true
	);

	if (!AddResult.bSuccess && AddResult.AddedQuantity <= 0)
	{
		InventoryComponent->AddItemAtPosition(RemoveResult.RemovedItemInstance, Request.Source.InventoryPosition, false, true);
		return MakeResponse(Request, EPNInventoryActionResult::MergeFailed);
	}

	if (AddResult.RemainingQuantity > 0)
	{
		RemoveResult.RemovedItemInstance->SetQuantityClamped(AddResult.RemainingQuantity);
		InventoryComponent->AddItemAtPosition(RemoveResult.RemovedItemInstance, Request.Source.InventoryPosition, false, true);
	}

	return MakeResponse(
		Request,
		EPNInventoryActionResult::Success,
		true,
		RemoveResult.RemovedItemInstance->GetItemData(),
		AddResult.AddedQuantity
	);
}

FPNInventoryActionResponse UPNInventoryActionComponent::HandleSplitStackAction(const FPNInventoryActionRequest& Request)
{
	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidInventory);
	}

	if (!Request.Source.IsInventory() || !Request.Destination.IsInventory())
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidSource);
	}

	UPNItemInstance* SourceItem = InventoryComponent->GetItemAtPosition(Request.Source.InventoryPosition);
	if (!SourceItem || !SourceItem->IsValidItem())
	{
		return MakeResponse(Request, EPNInventoryActionResult::SourceEmpty);
	}

	const int32 Quantity = ResolveInventoryQuantity(SourceItem, Request);
	if (Quantity <= 0 || Quantity >= SourceItem->Quantity)
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidQuantity);
	}

	if (InventoryComponent->GetItemAtPosition(Request.Destination.InventoryPosition))
	{
		return MakeResponse(Request, EPNInventoryActionResult::DestinationOccupied);
	}

	FPNInventoryRemoveItemResult RemoveResult = InventoryComponent->RemoveItemAtPosition(Request.Source.InventoryPosition, Quantity);
	if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance)
	{
		return MakeResponse(Request, EPNInventoryActionResult::SplitFailed);
	}

	FPNInventoryAddItemResult AddResult = InventoryComponent->AddItemAtPosition(
		RemoveResult.RemovedItemInstance,
		Request.Destination.InventoryPosition,
		Request.Destination.bRotated,
		false
	);

	if (!AddResult.bSuccess)
	{
		InventoryComponent->AddItemAtPosition(RemoveResult.RemovedItemInstance, Request.Source.InventoryPosition, false, true);
		return MakeResponse(Request, EPNInventoryActionResult::SplitFailed);
	}

	return MakeResponse(
		Request,
		EPNInventoryActionResult::Success,
		true,
		RemoveResult.RemovedItemInstance->GetItemData(),
		AddResult.AddedQuantity
	);
}

FPNInventoryActionResponse UPNInventoryActionComponent::HandleDropAction(const FPNInventoryActionRequest& Request)
{
	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidInventory);
	}

	FPNInventoryRemoveItemResult RemoveResult;
	UPNItemDataAsset* ItemData = nullptr;

	if (Request.Source.IsInventory())
	{
		UPNItemInstance* SourceItem = InventoryComponent->GetItemAtPosition(Request.Source.InventoryPosition);
		if (!SourceItem || !SourceItem->IsValidItem())
		{
			return MakeResponse(Request, EPNInventoryActionResult::SourceEmpty);
		}

		const int32 Quantity = ResolveInventoryQuantity(SourceItem, Request);
		if (Quantity <= 0)
		{
			return MakeResponse(Request, EPNInventoryActionResult::InvalidQuantity);
		}

		ItemData = SourceItem->GetItemData();
		RemoveResult = InventoryComponent->RemoveItemAtPosition(Request.Source.InventoryPosition, Quantity);
	}
	else if (Request.Source.IsQuickSlot())
	{
		const FPNInventoryQuickSlotEntry SlotEntry = InventoryComponent->GetQuickSlotEntry(Request.Source.QuickSlotIndex);
		if (SlotEntry.IsEmpty())
		{
			return MakeResponse(Request, EPNInventoryActionResult::SourceEmpty);
		}

		const int32 Quantity = ResolveQuickSlotQuantity(SlotEntry, Request);
		if (Quantity <= 0)
		{
			return MakeResponse(Request, EPNInventoryActionResult::InvalidQuantity);
		}

		ItemData = SlotEntry.InstanceData.ItemData;
		RemoveResult = InventoryComponent->RemoveItemFromQuickSlot(Request.Source.QuickSlotIndex, Quantity);
	}
	else
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidSource);
	}

	if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance)
	{
		return MakeResponse(Request, EPNInventoryActionResult::DropFailed);
	}

	if (!SpawnDroppedWorldItem(RemoveResult.RemovedItemInstance))
	{
		if (Request.Source.IsInventory())
		{
			InventoryComponent->AddItemAtPosition(RemoveResult.RemovedItemInstance, Request.Source.InventoryPosition, false, true);
		}
		else if (Request.Source.IsQuickSlot())
		{
			InventoryComponent->AddItemToQuickSlot(RemoveResult.RemovedItemInstance, Request.Source.QuickSlotIndex);
		}

		return MakeResponse(Request, EPNInventoryActionResult::DropFailed);
	}

	return MakeResponse(
		Request,
		EPNInventoryActionResult::Success,
		true,
		ItemData,
		RemoveResult.RemovedQuantity
	);
}

FPNInventoryActionResponse UPNInventoryActionComponent::HandleRotateAction(const FPNInventoryActionRequest& Request)
{
	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidInventory);
	}

	if (!Request.Source.IsInventory())
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidSource);
	}

	UPNItemInstance* SourceItem = InventoryComponent->GetItemAtPosition(Request.Source.InventoryPosition);
	if (!SourceItem || !SourceItem->IsValidItem())
	{
		return MakeResponse(Request, EPNInventoryActionResult::SourceEmpty);
	}

	for (const FPNInventoryItemEntry& Entry : InventoryComponent->GetItems())
	{
		if (Entry.ItemInstance == SourceItem)
		{
			const bool bNewRotated = !Entry.bRotated;

			FPNInventoryMoveItemResult MoveResult = InventoryComponent->MoveItemFromPosition(
				Entry.Position,
				Entry.Position,
				bNewRotated
			);

			return MakeResponse(
				Request,
				MoveResult.bSuccess ? EPNInventoryActionResult::Success : EPNInventoryActionResult::RotateFailed,
				MoveResult.bSuccess,
				SourceItem->GetItemData(),
				SourceItem->Quantity
			);
		}
	}

	return MakeResponse(Request, EPNInventoryActionResult::RotateFailed);
}

FPNInventoryActionResponse UPNInventoryActionComponent::HandleInspectAction(const FPNInventoryActionRequest& Request) const
{
	UPNItemDataAsset* ItemData = GetItemDataFromTarget(Request.Source);
	if (!ItemData)
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidItem);
	}

	FPNInventoryActionResponse Response = MakeResponse(Request, EPNInventoryActionResult::Success, true, ItemData, 0);
	Response.ItemDescription = ItemData->GetItemDescription();

	return Response;
}

UPNInventoryComponent* UPNInventoryActionComponent::GetOwnerInventoryComponent() const
{
	const APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		return OwnerCharacter->GetInventoryComponent();
	}

	const AActor* OwnerActor = GetOwner();
	return OwnerActor ? OwnerActor->FindComponentByClass<UPNInventoryComponent>() : nullptr;
}

UPNEquipmentComponent* UPNInventoryActionComponent::GetOwnerEquipmentComponent() const
{
	const APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		return OwnerCharacter->GetEquipmentComponent();
	}

	const AActor* OwnerActor = GetOwner();
	return OwnerActor ? OwnerActor->FindComponentByClass<UPNEquipmentComponent>() : nullptr;
}

bool UPNInventoryActionComponent::HasActionAuthority() const
{
	const AActor* OwnerActor = GetOwner();
	return !OwnerActor || OwnerActor->HasAuthority();
}

UPNItemInstance* UPNInventoryActionComponent::GetInventoryItemAtTarget(const FPNInventoryActionTarget& Target) const
{
	const UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		return nullptr;
	}

	if (!Target.IsInventory())
	{
		return nullptr;
	}

	return InventoryComponent->GetItemAtPosition(Target.InventoryPosition);
}

UPNItemDataAsset* UPNInventoryActionComponent::GetItemDataFromTarget(const FPNInventoryActionTarget& Target) const
{
	const UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	const UPNEquipmentComponent* EquipmentComponent = GetOwnerEquipmentComponent();

	if (Target.IsInventory() && InventoryComponent)
	{
		const UPNItemInstance* ItemInstance = InventoryComponent->GetItemAtPosition(Target.InventoryPosition);
		return ItemInstance ? ItemInstance->GetItemData() : nullptr;
	}

	if (Target.IsQuickSlot() && InventoryComponent)
	{
		return InventoryComponent->GetQuickSlotItemData(Target.QuickSlotIndex);
	}

	if (Target.IsEquipmentSlot() && EquipmentComponent)
	{
		return EquipmentComponent->GetEquippedItemData(Target.EquipmentSlot);
	}

	if (Target.IsEquipmentInternal() && EquipmentComponent)
	{
		return EquipmentComponent->GetInternalSlotItemData(Target.InternalContainer, Target.InternalSlotIndex);
	}

	return nullptr;
}

int32 UPNInventoryActionComponent::ResolveInventoryQuantity(UPNItemInstance* SourceItem, const FPNInventoryActionRequest& Request) const
{
	if (!SourceItem || !SourceItem->IsValidItem())
	{
		return 0;
	}

	if (Request.bHalfStack)
	{
		return FMath::Max(1, SourceItem->Quantity / 2);
	}

	if (Request.Quantity <= 0)
	{
		return SourceItem->Quantity;
	}

	return FMath::Clamp(Request.Quantity, 1, SourceItem->Quantity);
}

int32 UPNInventoryActionComponent::ResolveQuickSlotQuantity(const FPNInventoryQuickSlotEntry& SlotEntry, const FPNInventoryActionRequest& Request) const
{
	if (SlotEntry.IsEmpty())
	{
		return 0;
	}

	if (Request.bHalfStack)
	{
		return FMath::Max(1, SlotEntry.InstanceData.Quantity / 2);
	}

	if (Request.Quantity <= 0)
	{
		return SlotEntry.InstanceData.Quantity;
	}

	return FMath::Clamp(Request.Quantity, 1, SlotEntry.InstanceData.Quantity);
}

bool UPNInventoryActionComponent::SpawnDroppedWorldItem(UPNItemInstance* DroppedInstance)
{
	if (!DroppedInstance || !DroppedInstance->IsValidItem())
	{
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	UWorld* World = OwnerActor->GetWorld();
	if (!World)
	{
		return false;
	}

	TSubclassOf<APNWorldItemActor> SpawnClass = WorldItemActorClass;
	if (!SpawnClass)
	{
		SpawnClass = APNWorldItemActor::StaticClass();
	}

	const FVector SpawnLocation =
		OwnerActor->GetActorLocation()
		+ OwnerActor->GetActorForwardVector() * DropForwardDistance
		+ FVector(0.0f, 0.0f, DropUpOffset);

	const FRotator SpawnRotation = OwnerActor->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;
	SpawnParams.Instigator = Cast<APawn>(OwnerActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APNWorldItemActor* WorldItem = World->SpawnActor<APNWorldItemActor>(
		SpawnClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!WorldItem)
	{
		return false;
	}

	WorldItem->InitializeFromInstance(DroppedInstance);
	WorldItem->SetWorldItemPhysicsEnabled(true);

	return true;
}

FPNInventoryActionResponse UPNInventoryActionComponent::MakeResponse(
	const FPNInventoryActionRequest& Request,
	EPNInventoryActionResult Result,
	bool bSuccess,
	UPNItemDataAsset* ItemData,
	int32 Quantity
) const
{
	FPNInventoryActionResponse Response;
	Response.bSuccess = bSuccess;
	Response.Result = Result;
	Response.ActionType = Request.ActionType;
	Response.Source = Request.Source;
	Response.Destination = Request.Destination;
	Response.ItemData = ItemData;
	Response.Quantity = Quantity;

	return Response;
}

void UPNInventoryActionComponent::BroadcastActionCompleted(const FPNInventoryActionResponse& Response)
{
	OnInventoryActionCompleted.Broadcast(Response);

	if (!bDebugInventoryActions || !GEngine)
	{
		return;
	}

	const AActor* OwnerActor = GetOwner();

	const FString NetSide = OwnerActor && OwnerActor->HasAuthority()
		? TEXT("SERVER")
		: TEXT("CLIENT");

	const FString ItemName = Response.ItemData
		? (Response.ItemData->GetItemName().IsEmpty() ? Response.ItemData->GetItemId().ToString() : Response.ItemData->GetItemName().ToString())
		: TEXT("None");

	GEngine->AddOnScreenDebugMessage(
		OwnerActor ? static_cast<int32>(OwnerActor->GetUniqueID()) + 41000 : INDEX_NONE,
		3.0f,
		Response.bSuccess ? FColor::Green : FColor::Red,
		FString::Printf(
			TEXT("[%s] Action: %s | Result: %s | Item: %s | Qty: %d"),
			*NetSide,
			*UEnum::GetValueAsString(Response.ActionType),
			*UEnum::GetValueAsString(Response.Result),
			*ItemName,
			Response.Quantity
		)
	);
}