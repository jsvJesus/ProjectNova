#include "Inventory/PNInventoryActionComponent.h"

#include "Characters/PNBaseCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Equipment/PNEquipmentComponent.h"
#include "Inventory/PNInventoryComponent.h"
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

void UPNInventoryActionComponent::RequestEquipKnifeFromInventory(FPNInventoryGridPosition InventoryPosition)
{
	RequestEquipInventoryItem(InventoryPosition, EPNEquipmentSlot::Knife);
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

void UPNInventoryActionComponent::RequestDropEquipmentItem(EPNEquipmentSlot SourceSlot)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Drop;
	Request.Source.Container = EPNInventoryActionContainer::EquipmentSlot;
	Request.Source.EquipmentSlot = SourceSlot;
	Request.Destination.Container = EPNInventoryActionContainer::World;
	Request.Quantity = 1;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestDropEquipmentInternalItem(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Drop;
	Request.Source.Container = EPNInventoryActionContainer::EquipmentInternal;
	Request.Source.InternalContainer = Container;
	Request.Source.InternalSlotIndex = InternalSlotIndex;
	Request.Destination.Container = EPNInventoryActionContainer::World;
	Request.Quantity = 1;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestMoveEquipmentItemToQuickSlot(EPNEquipmentSlot SourceSlot, int32 QuickSlotIndex)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Move;
	Request.Source.Container = EPNInventoryActionContainer::EquipmentSlot;
	Request.Source.EquipmentSlot = SourceSlot;
	Request.Destination.Container = EPNInventoryActionContainer::QuickSlot;
	Request.Destination.QuickSlotIndex = QuickSlotIndex;
	Request.Quantity = 1;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestMoveEquipmentInternalItemToQuickSlot(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex, int32 QuickSlotIndex)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Move;
	Request.Source.Container = EPNInventoryActionContainer::EquipmentInternal;
	Request.Source.InternalContainer = Container;
	Request.Source.InternalSlotIndex = InternalSlotIndex;
	Request.Destination.Container = EPNInventoryActionContainer::QuickSlot;
	Request.Destination.QuickSlotIndex = QuickSlotIndex;
	Request.Quantity = 1;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestMoveQuickSlotItemToEquipment(int32 QuickSlotIndex, EPNEquipmentSlot TargetSlot)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Move;
	Request.Source.Container = EPNInventoryActionContainer::QuickSlot;
	Request.Source.QuickSlotIndex = QuickSlotIndex;
	Request.Destination.Container = EPNInventoryActionContainer::EquipmentSlot;
	Request.Destination.EquipmentSlot = TargetSlot;
	Request.Quantity = 1;

	RequestInventoryAction(Request);
}

void UPNInventoryActionComponent::RequestMoveQuickSlotItemToEquipmentInternal(int32 QuickSlotIndex, EPNEquipmentInternalContainer Container, int32 InternalSlotIndex)
{
	FPNInventoryActionRequest Request;
	Request.ActionType = EPNInventoryActionType::Move;
	Request.Source.Container = EPNInventoryActionContainer::QuickSlot;
	Request.Source.QuickSlotIndex = QuickSlotIndex;
	Request.Destination.Container = EPNInventoryActionContainer::EquipmentInternal;
	Request.Destination.InternalContainer = Container;
	Request.Destination.InternalSlotIndex = InternalSlotIndex;
	Request.Quantity = 1;

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

	UPNEquipmentComponent* EquipmentComponent = GetOwnerEquipmentComponent();

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

	if (Request.Source.IsEquipmentSlot() && Request.Destination.IsInventory())
	{
		return HandleUnequipAction(Request);
	}

	if (Request.Source.IsEquipmentInternal() && Request.Destination.IsInventory())
	{
		return HandleUnequipAction(Request);
	}

	if (Request.Source.IsQuickSlot() && Request.Destination.IsEquipmentSlot())
	{
		if (!EquipmentComponent)
		{
			return MakeResponse(Request, EPNInventoryActionResult::InvalidEquipment);
		}

		const FPNInventoryQuickSlotEntry SlotEntry = InventoryComponent->GetQuickSlotEntry(Request.Source.QuickSlotIndex);
		if (SlotEntry.IsEmpty())
		{
			return MakeResponse(Request, EPNInventoryActionResult::SourceEmpty);
		}

		FPNInventoryRemoveItemResult RemoveResult = InventoryComponent->RemoveItemFromQuickSlot(Request.Source.QuickSlotIndex, 1);
		if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance)
		{
			return MakeResponse(Request, EPNInventoryActionResult::MoveFailed);
		}

		FPNEquipmentOperationResponse EquipResponse = EquipmentComponent->EquipItemInstanceToSlot(
			RemoveResult.RemovedItemInstance,
			Request.Destination.EquipmentSlot
		);

		if (!EquipResponse.bSuccess)
		{
			InventoryComponent->AddItemToQuickSlot(RemoveResult.RemovedItemInstance, Request.Source.QuickSlotIndex);
			return MakeResponse(Request, EPNInventoryActionResult::MoveFailed, false, RemoveResult.RemovedItemInstance->GetItemData(), 0);
		}

		return MakeResponse(Request, EPNInventoryActionResult::Success, true, EquipResponse.ItemData, 1);
	}

	if (Request.Source.IsQuickSlot() && Request.Destination.IsEquipmentInternal())
	{
		if (!EquipmentComponent)
		{
			return MakeResponse(Request, EPNInventoryActionResult::InvalidEquipment);
		}

		const FPNInventoryQuickSlotEntry SlotEntry = InventoryComponent->GetQuickSlotEntry(Request.Source.QuickSlotIndex);
		if (SlotEntry.IsEmpty())
		{
			return MakeResponse(Request, EPNInventoryActionResult::SourceEmpty);
		}

		FPNInventoryRemoveItemResult RemoveResult = InventoryComponent->RemoveItemFromQuickSlot(Request.Source.QuickSlotIndex, 1);
		if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance)
		{
			return MakeResponse(Request, EPNInventoryActionResult::MoveFailed);
		}

		FPNEquipmentOperationResponse InsertResponse = EquipmentComponent->InsertItemInstanceToInternalSlot(
			RemoveResult.RemovedItemInstance,
			Request.Destination.InternalContainer,
			Request.Destination.InternalSlotIndex
		);

		if (!InsertResponse.bSuccess)
		{
			InventoryComponent->AddItemToQuickSlot(RemoveResult.RemovedItemInstance, Request.Source.QuickSlotIndex);
			return MakeResponse(Request, EPNInventoryActionResult::MoveFailed, false, RemoveResult.RemovedItemInstance->GetItemData(), 0);
		}

		return MakeResponse(Request, EPNInventoryActionResult::Success, true, InsertResponse.ItemData, 1);
	}

	if (Request.Source.IsEquipmentSlot() && Request.Destination.IsQuickSlot())
	{
		if (!EquipmentComponent)
		{
			return MakeResponse(Request, EPNInventoryActionResult::InvalidEquipment);
		}

		FPNEquipmentOperationResponse RemoveEquipResponse;
		UPNItemInstance* RemovedItem = EquipmentComponent->RemoveEquipmentSlotAsItemInstance(Request.Source.EquipmentSlot, RemoveEquipResponse);

		if (!RemoveEquipResponse.bSuccess || !RemovedItem)
		{
			return MakeResponse(Request, EPNInventoryActionResult::MoveFailed, false, RemoveEquipResponse.ItemData, 0);
		}

		FPNInventoryAddItemResult AddResult = InventoryComponent->AddItemToQuickSlot(RemovedItem, Request.Destination.QuickSlotIndex);
		if (!AddResult.bSuccess)
		{
			EquipmentComponent->EquipItemInstanceToSlot(RemovedItem, Request.Source.EquipmentSlot);
			return MakeResponse(Request, EPNInventoryActionResult::MoveFailed, false, RemovedItem->GetItemData(), 0);
		}

		return MakeResponse(Request, EPNInventoryActionResult::Success, true, RemovedItem->GetItemData(), 1);
	}

	if (Request.Source.IsEquipmentInternal() && Request.Destination.IsQuickSlot())
	{
		if (!EquipmentComponent)
		{
			return MakeResponse(Request, EPNInventoryActionResult::InvalidEquipment);
		}

		FPNEquipmentOperationResponse RemoveInternalResponse;
		UPNItemInstance* RemovedItem = EquipmentComponent->RemoveInternalSlotAsItemInstance(
			Request.Source.InternalContainer,
			Request.Source.InternalSlotIndex,
			RemoveInternalResponse
		);

		if (!RemoveInternalResponse.bSuccess || !RemovedItem)
		{
			return MakeResponse(Request, EPNInventoryActionResult::MoveFailed, false, RemoveInternalResponse.ItemData, 0);
		}

		FPNInventoryAddItemResult AddResult = InventoryComponent->AddItemToQuickSlot(RemovedItem, Request.Destination.QuickSlotIndex);
		if (!AddResult.bSuccess)
		{
			EquipmentComponent->InsertItemInstanceToInternalSlot(
				RemovedItem,
				Request.Source.InternalContainer,
				Request.Source.InternalSlotIndex
			);

			return MakeResponse(Request, EPNInventoryActionResult::MoveFailed, false, RemovedItem->GetItemData(), 0);
		}

		return MakeResponse(Request, EPNInventoryActionResult::Success, true, RemovedItem->GetItemData(), 1);
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

	UPNEquipmentComponent* EquipmentComponent = GetOwnerEquipmentComponent();

	FPNInventoryRemoveItemResult RemoveResult;
	UPNItemInstance* DroppedItem = nullptr;
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

		if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance)
		{
			return MakeResponse(Request, EPNInventoryActionResult::DropFailed);
		}

		DroppedItem = RemoveResult.RemovedItemInstance;
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

		if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance)
		{
			return MakeResponse(Request, EPNInventoryActionResult::DropFailed);
		}

		DroppedItem = RemoveResult.RemovedItemInstance;
	}
	else if (Request.Source.IsEquipmentSlot())
	{
		if (!EquipmentComponent)
		{
			return MakeResponse(Request, EPNInventoryActionResult::InvalidEquipment);
		}

		FPNEquipmentOperationResponse RemoveEquipResponse;
		DroppedItem = EquipmentComponent->RemoveEquipmentSlotAsItemInstance(Request.Source.EquipmentSlot, RemoveEquipResponse);

		if (!RemoveEquipResponse.bSuccess || !DroppedItem)
		{
			return MakeResponse(Request, EPNInventoryActionResult::DropFailed, false, RemoveEquipResponse.ItemData, 0);
		}

		ItemData = DroppedItem->GetItemData();
	}
	else if (Request.Source.IsEquipmentInternal())
	{
		if (!EquipmentComponent)
		{
			return MakeResponse(Request, EPNInventoryActionResult::InvalidEquipment);
		}

		FPNEquipmentOperationResponse RemoveInternalResponse;
		DroppedItem = EquipmentComponent->RemoveInternalSlotAsItemInstance(
			Request.Source.InternalContainer,
			Request.Source.InternalSlotIndex,
			RemoveInternalResponse
		);

		if (!RemoveInternalResponse.bSuccess || !DroppedItem)
		{
			return MakeResponse(Request, EPNInventoryActionResult::DropFailed, false, RemoveInternalResponse.ItemData, 0);
		}

		ItemData = DroppedItem->GetItemData();
	}
	else
	{
		return MakeResponse(Request, EPNInventoryActionResult::InvalidSource);
	}

	if (!DroppedItem || !DroppedItem->IsValidItem())
	{
		return MakeResponse(Request, EPNInventoryActionResult::DropFailed);
	}

	if (!SpawnDroppedWorldItem(DroppedItem))
	{
		if (Request.Source.IsInventory())
		{
			InventoryComponent->AddItemAtPosition(DroppedItem, Request.Source.InventoryPosition, false, true);
		}
		else if (Request.Source.IsQuickSlot())
		{
			InventoryComponent->AddItemToQuickSlot(DroppedItem, Request.Source.QuickSlotIndex);
		}
		else if (Request.Source.IsEquipmentSlot() && EquipmentComponent)
		{
			EquipmentComponent->EquipItemInstanceToSlot(DroppedItem, Request.Source.EquipmentSlot);
		}
		else if (Request.Source.IsEquipmentInternal() && EquipmentComponent)
		{
			EquipmentComponent->InsertItemInstanceToInternalSlot(
				DroppedItem,
				Request.Source.InternalContainer,
				Request.Source.InternalSlotIndex
			);
		}

		return MakeResponse(Request, EPNInventoryActionResult::DropFailed, false, ItemData, 0);
	}

	return MakeResponse(
		Request,
		EPNInventoryActionResult::Success,
		true,
		ItemData,
		DroppedItem->Quantity
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

TArray<FPNInventoryContextMenuEntry> UPNInventoryActionComponent::GetContextMenuActions(const FPNInventoryActionTarget& Target) const
{
	TArray<FPNInventoryContextMenuEntry> Entries;

	UPNItemDataAsset* ItemData = GetItemDataFromTarget(Target);
	if (!ItemData)
	{
		return Entries;
	}

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();

	Entries.Add(MakeContextMenuEntry(
		Target,
		EPNInventoryActionType::Inspect,
		FText::FromString(TEXT("Описание")),
		true,
		100
	));

	if (Target.IsInventory())
	{
		const int32 Quantity = GetQuantityFromTarget(Target);

		const bool bCanUse = InventoryComponent && InventoryComponent->CanUseItemData(ItemData);

		Entries.Add(MakeContextMenuEntry(
			Target,
			EPNInventoryActionType::Use,
			FText::FromString(TEXT("Использовать")),
			bCanUse,
			10,
			false,
			false,
			false,
			bCanUse ? EPNInventoryActionResult::Success : EPNInventoryActionResult::ItemNotUsable
		));

		EPNEquipmentSlot BestSlot = EPNEquipmentSlot::None;
		const bool bCanEquip = FindBestEquipmentSlotForItemData(ItemData, BestSlot);

		Entries.Add(MakeContextMenuEntry(
			Target,
			EPNInventoryActionType::Equip,
			FText::FromString(TEXT("Надеть")),
			bCanEquip,
			20,
			false,
			false,
			false,
			bCanEquip ? EPNInventoryActionResult::Success : EPNInventoryActionResult::EquipFailed
		));

		Entries.Add(MakeContextMenuEntry(
			Target,
			EPNInventoryActionType::Drop,
			FText::FromString(TEXT("Выкинуть")),
			true,
			80,
			false,
			true,
			true
		));

		Entries.Add(MakeContextMenuEntry(
			Target,
			EPNInventoryActionType::SplitStack,
			FText::FromString(TEXT("Разделить")),
			Quantity > 1,
			60,
			true,
			true,
			false,
			Quantity > 1 ? EPNInventoryActionResult::Success : EPNInventoryActionResult::InvalidQuantity
		));

		Entries.Add(MakeContextMenuEntry(
			Target,
			EPNInventoryActionType::Rotate,
			FText::FromString(TEXT("Повернуть")),
			true,
			70
		));
	}
	else if (Target.IsQuickSlot())
	{
		const bool bCanUse = InventoryComponent && InventoryComponent->CanUseItemData(ItemData);

		Entries.Add(MakeContextMenuEntry(
			Target,
			EPNInventoryActionType::Use,
			FText::FromString(TEXT("Использовать")),
			bCanUse,
			10,
			false,
			false,
			false,
			bCanUse ? EPNInventoryActionResult::Success : EPNInventoryActionResult::ItemNotUsable
		));

		EPNEquipmentSlot BestSlot = EPNEquipmentSlot::None;
		const bool bCanEquip = FindBestEquipmentSlotForItemData(ItemData, BestSlot);

		Entries.Add(MakeContextMenuEntry(
			Target,
			EPNInventoryActionType::Equip,
			FText::FromString(TEXT("Надеть")),
			bCanEquip,
			20,
			false,
			false,
			false,
			bCanEquip ? EPNInventoryActionResult::Success : EPNInventoryActionResult::EquipFailed
		));

		Entries.Add(MakeContextMenuEntry(
			Target,
			EPNInventoryActionType::Drop,
			FText::FromString(TEXT("Выкинуть")),
			true,
			80,
			false,
			true,
			true
		));
	}
	else if (Target.IsEquipmentSlot())
	{
		Entries.Add(MakeContextMenuEntry(
			Target,
			EPNInventoryActionType::Unequip,
			FText::FromString(TEXT("Снять")),
			true,
			20
		));

		Entries.Add(MakeContextMenuEntry(
			Target,
			EPNInventoryActionType::Drop,
			FText::FromString(TEXT("Выкинуть")),
			true,
			80,
			false,
			false,
			true
		));
	}
	else if (Target.IsEquipmentInternal())
	{
		Entries.Add(MakeContextMenuEntry(
			Target,
			EPNInventoryActionType::Unequip,
			FText::FromString(TEXT("Снять")),
			true,
			20
		));

		Entries.Add(MakeContextMenuEntry(
			Target,
			EPNInventoryActionType::Drop,
			FText::FromString(TEXT("Выкинуть")),
			true,
			80,
			false,
			false,
			true
		));
	}

	Entries.Sort([](const FPNInventoryContextMenuEntry& A, const FPNInventoryContextMenuEntry& B)
	{
		return A.SortPriority < B.SortPriority;
	});

	return Entries;
}

bool UPNInventoryActionComponent::BuildContextActionRequest(const FPNInventoryActionTarget& Target, EPNInventoryActionType ActionType, FPNInventoryActionRequest& OutRequest) const
{
	OutRequest = FPNInventoryActionRequest();
	OutRequest.ActionType = ActionType;
	OutRequest.Source = Target;

	if (!GetItemDataFromTarget(Target))
	{
		return false;
	}

	switch (ActionType)
	{
	case EPNInventoryActionType::Use:
		return Target.IsInventory() || Target.IsQuickSlot();

	case EPNInventoryActionType::Inspect:
		return true;

	case EPNInventoryActionType::Drop:
		OutRequest.Destination.Container = EPNInventoryActionContainer::World;
		OutRequest.Quantity = -1;
		return true;

	case EPNInventoryActionType::Unequip:
		if (Target.IsEquipmentSlot() || Target.IsEquipmentInternal())
		{
			OutRequest.Destination.Container = EPNInventoryActionContainer::Inventory;
			OutRequest.Quantity = 1;
			return true;
		}
		return false;

	case EPNInventoryActionType::Equip:
		if (Target.IsInventory())
		{
			UPNItemInstance* ItemInstance = GetInventoryItemAtTarget(Target);
			if (!ItemInstance || !ItemInstance->GetItemData())
			{
				return false;
			}

			EPNEquipmentSlot BestSlot = EPNEquipmentSlot::None;
			if (!FindBestEquipmentSlotForItemData(ItemInstance->GetItemData(), BestSlot))
			{
				return false;
			}

			OutRequest.Destination.Container = EPNInventoryActionContainer::EquipmentSlot;
			OutRequest.Destination.EquipmentSlot = BestSlot;
			OutRequest.Quantity = 1;
			return true;
		}

		if (Target.IsQuickSlot())
		{
			UPNItemDataAsset* ItemData = GetItemDataFromTarget(Target);

			EPNEquipmentSlot BestSlot = EPNEquipmentSlot::None;
			if (!FindBestEquipmentSlotForItemData(ItemData, BestSlot))
			{
				return false;
			}

			OutRequest.Destination.Container = EPNInventoryActionContainer::EquipmentSlot;
			OutRequest.Destination.EquipmentSlot = BestSlot;
			OutRequest.Quantity = 1;
			return true;
		}

		return false;

	case EPNInventoryActionType::Rotate:
		return Target.IsInventory();

	case EPNInventoryActionType::SplitStack:
		OutRequest.bHalfStack = true;
		OutRequest.Quantity = -1;
		return Target.IsInventory() && GetQuantityFromTarget(Target) > 1;

	default:
		return false;
	}
}

int32 UPNInventoryActionComponent::GetQuantityFromTarget(const FPNInventoryActionTarget& Target) const
{
	const UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	const UPNEquipmentComponent* EquipmentComponent = GetOwnerEquipmentComponent();

	if (Target.IsInventory() && InventoryComponent)
	{
		const UPNItemInstance* ItemInstance = InventoryComponent->GetItemAtPosition(Target.InventoryPosition);
		return ItemInstance ? ItemInstance->Quantity : 0;
	}

	if (Target.IsQuickSlot() && InventoryComponent)
	{
		const FPNInventoryQuickSlotEntry SlotEntry = InventoryComponent->GetQuickSlotEntry(Target.QuickSlotIndex);
		return SlotEntry.IsOccupied() ? SlotEntry.InstanceData.Quantity : 0;
	}

	if (Target.IsEquipmentSlot() && EquipmentComponent)
	{
		const FPNEquipmentSlotEntry SlotEntry = EquipmentComponent->GetEquipmentSlotEntry(Target.EquipmentSlot);
		return SlotEntry.IsOccupied() ? SlotEntry.InstanceData.Quantity : 0;
	}

	if (Target.IsEquipmentInternal() && EquipmentComponent)
	{
		const FPNEquipmentInternalSlotEntry SlotEntry = EquipmentComponent->GetInternalSlotEntry(Target.InternalContainer, Target.InternalSlotIndex);
		return SlotEntry.IsOccupied() ? SlotEntry.InstanceData.Quantity : 0;
	}

	return 0;
}

bool UPNInventoryActionComponent::FindBestEquipmentSlotForItemData(UPNItemDataAsset* ItemData, EPNEquipmentSlot& OutSlot) const
{
	OutSlot = EPNEquipmentSlot::None;

	if (!ItemData)
	{
		return false;
	}

	const UPNEquipmentComponent* EquipmentComponent = GetOwnerEquipmentComponent();

	auto IsSlotFree = [EquipmentComponent](EPNEquipmentSlot Slot)
	{
		return !EquipmentComponent || !EquipmentComponent->IsEquipmentSlotOccupied(Slot);
	};

	if (ItemData->ItemType == EPNItemType::IT_HArmor)
	{
		if (IsSlotFree(EPNEquipmentSlot::Helmet))
		{
			OutSlot = EPNEquipmentSlot::Helmet;
			return true;
		}

		return false;
	}

	if (ItemData->ItemType == EPNItemType::IT_Gloves || ItemData->ItemCategory == EPNItemCategory::Gloves)
	{
		if (IsSlotFree(EPNEquipmentSlot::Gloves))
		{
			OutSlot = EPNEquipmentSlot::Gloves;
			return true;
		}

		return false;
	}

	if (ItemData->ItemType == EPNItemType::IT_Armor)
	{
		if (IsSlotFree(EPNEquipmentSlot::Armor))
		{
			OutSlot = EPNEquipmentSlot::Armor;
			return true;
		}

		return false;
	}

	if (ItemData->ItemType == EPNItemType::IT_Backpack)
	{
		if (IsSlotFree(EPNEquipmentSlot::Backpack))
		{
			OutSlot = EPNEquipmentSlot::Backpack;
			return true;
		}

		return false;
	}

	if (ItemData->ItemType != EPNItemType::IT_Weapon)
	{
		return false;
	}

	if (ItemData->WeaponStats.AnimType == EPNAnimType::Knife
	|| ItemData->ItemCategory == EPNItemCategory::Melee)
	{
		if (IsSlotFree(EPNEquipmentSlot::Knife))
		{
			OutSlot = EPNEquipmentSlot::Knife;
			return true;
		}

		return false;
	}

	if (ItemData->ItemCategory == EPNItemCategory::HG_Single
	|| ItemData->ItemCategory == EPNItemCategory::HG_Knife
	|| ItemData->ItemCategory == EPNItemCategory::HG_Shield
	|| ItemData->ItemCategory == EPNItemCategory::HG_Items
	|| ItemData->WeaponStats.AnimType == EPNAnimType::Pistol_Single
	|| ItemData->WeaponStats.AnimType == EPNAnimType::Pistol_Knife
	|| ItemData->WeaponStats.AnimType == EPNAnimType::Pistol_Shield)
	{
		if (IsSlotFree(EPNEquipmentSlot::Sidearm))
		{
			OutSlot = EPNEquipmentSlot::Sidearm;
			return true;
		}

		return false;
	}

	if (IsSlotFree(EPNEquipmentSlot::PrimaryWeapon1))
	{
		OutSlot = EPNEquipmentSlot::PrimaryWeapon1;
		return true;
	}

	if (IsSlotFree(EPNEquipmentSlot::PrimaryWeapon2))
	{
		OutSlot = EPNEquipmentSlot::PrimaryWeapon2;
		return true;
	}

	return false;
}

FPNInventoryContextMenuEntry UPNInventoryActionComponent::MakeContextMenuEntry(
	const FPNInventoryActionTarget& Target,
	EPNInventoryActionType ActionType,
	const FText& DisplayName,
	bool bEnabled,
	int32 SortPriority,
	bool bRequiresDestination,
	bool bRequiresQuantity,
	bool bDestructive,
	EPNInventoryActionResult DisabledReason
) const
{
	FPNInventoryContextMenuEntry Entry;
	Entry.ActionType = ActionType;
	Entry.DisplayName = DisplayName;
	Entry.bEnabled = bEnabled;
	Entry.bRequiresDestination = bRequiresDestination;
	Entry.bRequiresQuantity = bRequiresQuantity;
	Entry.bDestructive = bDestructive;
	Entry.SortPriority = SortPriority;
	Entry.DisabledReason = bEnabled ? EPNInventoryActionResult::Success : DisabledReason;

	Entry.Request.ActionType = ActionType;
	Entry.Request.Source = Target;

	BuildContextActionRequest(Target, ActionType, Entry.Request);

	return Entry;
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