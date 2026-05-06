#include "Inventory/PNInventoryContainerComponent.h"

#include "Characters/PNBaseCharacter.h"
#include "Engine/Engine.h"
#include "Inventory/PNInventoryComponent.h"
#include "Items/PNItemDataAsset.h"
#include "Items/PNItemInstance.h"
#include "Net/UnrealNetwork.h"

UPNInventoryContainerComponent::UPNInventoryContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPNInventoryContainerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPNInventoryContainerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPNInventoryContainerComponent, OpenedContainerInventory);
}

FPNInventoryContainerOpenResponse UPNInventoryContainerComponent::OpenContainer(UPNInventoryComponent* ContainerInventory)
{
	if (!HasContainerAuthority())
	{
		FPNInventoryContainerOpenResponse Response = MakeOpenResponse(EPNInventoryContainerOperationResult::NotAuthority, false, ContainerInventory);
		BroadcastOpenResponse(Response);
		return Response;
	}

	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	if (!OwnerInventory)
	{
		FPNInventoryContainerOpenResponse Response = MakeOpenResponse(EPNInventoryContainerOperationResult::InvalidOwnerInventory, false, ContainerInventory);
		BroadcastOpenResponse(Response);
		return Response;
	}

	if (!ContainerInventory)
	{
		FPNInventoryContainerOpenResponse Response = MakeOpenResponse(EPNInventoryContainerOperationResult::InvalidContainerInventory, false, nullptr);
		BroadcastOpenResponse(Response);
		return Response;
	}

	if (ContainerInventory == OwnerInventory)
	{
		FPNInventoryContainerOpenResponse Response = MakeOpenResponse(EPNInventoryContainerOperationResult::SameInventory, false, ContainerInventory);
		BroadcastOpenResponse(Response);
		return Response;
	}

	OpenedContainerInventory = ContainerInventory;

	FPNInventoryContainerOpenResponse Response = MakeOpenResponse(EPNInventoryContainerOperationResult::Success, true, ContainerInventory);
	BroadcastOpenResponse(Response);

	return Response;
}

void UPNInventoryContainerComponent::RequestOpenContainer(UPNInventoryComponent* ContainerInventory)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		OpenContainer(ContainerInventory);
		return;
	}

	Server_OpenContainer(ContainerInventory);
}

void UPNInventoryContainerComponent::Server_OpenContainer_Implementation(UPNInventoryComponent* ContainerInventory)
{
	OpenContainer(ContainerInventory);
}

void UPNInventoryContainerComponent::CloseContainer()
{
	if (!HasContainerAuthority())
	{
		return;
	}

	OpenedContainerInventory = nullptr;
	OnContainerClosed.Broadcast();

	PrintContainerDebugMessage(TEXT("Container closed"), true);
}

void UPNInventoryContainerComponent::RequestCloseContainer()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		CloseContainer();
		return;
	}

	Server_CloseContainer();
}

void UPNInventoryContainerComponent::Server_CloseContainer_Implementation()
{
	CloseContainer();
}

bool UPNInventoryContainerComponent::IsContainerOpen() const
{
	return OpenedContainerInventory != nullptr;
}

UPNInventoryComponent* UPNInventoryContainerComponent::GetOpenedContainerInventory() const
{
	return OpenedContainerInventory;
}

UPNInventoryComponent* UPNInventoryContainerComponent::GetOwnerInventoryComponent() const
{
	const APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		return OwnerCharacter->GetInventoryComponent();
	}

	const AActor* OwnerActor = GetOwner();
	return OwnerActor ? OwnerActor->FindComponentByClass<UPNInventoryComponent>() : nullptr;
}

UPNInventoryComponent* UPNInventoryContainerComponent::GetOwnerBackpackInventoryComponent() const
{
	const APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner());
	return OwnerCharacter ? OwnerCharacter->GetBackpackInventoryComponent() : nullptr;
}

UPNInventoryComponent* UPNInventoryContainerComponent::GetOwnerVestInventoryComponent() const
{
	const APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner());
	return OwnerCharacter ? OwnerCharacter->GetVestInventoryComponent() : nullptr;
}

EPNInventoryType UPNInventoryContainerComponent::GetOpenedContainerType() const
{
	return OpenedContainerInventory ? OpenedContainerInventory->Settings.InventoryType : EPNInventoryType::None;
}

FPNInventoryContainerTransferResponse UPNInventoryContainerComponent::MoveItemBetweenInventories(const FPNInventoryContainerTransferRequest& Request)
{
	if (!HasContainerAuthority())
	{
		FPNInventoryContainerTransferResponse Response = MakeTransferResponse(Request, EPNInventoryContainerOperationResult::NotAuthority);
		BroadcastTransferResponse(Response);
		return Response;
	}

	UPNInventoryComponent* SourceInventory = Request.SourceInventory;
	UPNInventoryComponent* TargetInventory = Request.TargetInventory;

	if (!SourceInventory)
	{
		FPNInventoryContainerTransferResponse Response = MakeTransferResponse(Request, EPNInventoryContainerOperationResult::InvalidSourceInventory);
		BroadcastTransferResponse(Response);
		return Response;
	}

	if (!TargetInventory)
	{
		FPNInventoryContainerTransferResponse Response = MakeTransferResponse(Request, EPNInventoryContainerOperationResult::InvalidTargetInventory);
		BroadcastTransferResponse(Response);
		return Response;
	}

	if (SourceInventory == TargetInventory)
	{
		FPNInventoryContainerTransferResponse Response = MakeTransferResponse(Request, EPNInventoryContainerOperationResult::SameInventory);
		BroadcastTransferResponse(Response);
		return Response;
	}

	if (!SourceInventory->CanRemoveItems())
	{
		FPNInventoryContainerTransferResponse Response = MakeTransferResponse(Request, EPNInventoryContainerOperationResult::SourceRemoveFailed);
		BroadcastTransferResponse(Response);
		return Response;
	}

	if (!TargetInventory->CanReceiveItems())
	{
		FPNInventoryContainerTransferResponse Response = MakeTransferResponse(Request, EPNInventoryContainerOperationResult::TargetAddFailed);
		BroadcastTransferResponse(Response);
		return Response;
	}

	UPNItemInstance* SourceItem = SourceInventory->GetItemAtPosition(Request.SourcePosition);
	if (!SourceItem || !SourceItem->IsValidItem())
	{
		FPNInventoryContainerTransferResponse Response = MakeTransferResponse(Request, EPNInventoryContainerOperationResult::SourceEmpty);
		BroadcastTransferResponse(Response);
		return Response;
	}

	UPNItemDataAsset* ItemData = SourceItem->GetItemData();
	const int32 QuantityToMove = ResolveTransferQuantity(SourceItem, Request);

	if (QuantityToMove <= 0)
	{
		FPNInventoryContainerTransferResponse Response = MakeTransferResponse(
			Request,
			EPNInventoryContainerOperationResult::InvalidQuantity,
			false,
			false,
			QuantityToMove,
			0,
			0,
			ItemData
		);

		BroadcastTransferResponse(Response);
		return Response;
	}

	FPNInventoryRemoveItemResult RemoveResult = SourceInventory->RemoveItemAtPosition(Request.SourcePosition, QuantityToMove);
	if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance || !RemoveResult.RemovedItemInstance->IsValidItem())
	{
		FPNInventoryContainerTransferResponse Response = MakeTransferResponse(
			Request,
			EPNInventoryContainerOperationResult::SourceRemoveFailed,
			false,
			false,
			QuantityToMove,
			0,
			QuantityToMove,
			ItemData
		);

		BroadcastTransferResponse(Response);
		return Response;
	}

	FPNInventoryAddItemResult AddResult;

	if (Request.bUseTargetPosition)
	{
		AddResult = TargetInventory->AddItemAtPosition(
			RemoveResult.RemovedItemInstance,
			Request.TargetPosition,
			Request.bTargetRotated,
			Request.bAllowStack
		);
	}
	else
	{
		AddResult = TargetInventory->AddItem(
			RemoveResult.RemovedItemInstance,
			Request.bAllowStack,
			Request.bAutoRotate
		);
	}

	const bool bMovedAnything = AddResult.AddedQuantity > 0;
	const int32 RemainingQuantity = FMath::Max(0, AddResult.RemainingQuantity);

	if (!bMovedAnything)
	{
		RemoveResult.RemovedItemInstance->SetQuantityClamped(QuantityToMove);

		FPNInventoryAddItemResult RollbackResult = SourceInventory->AddItemAtPosition(
			RemoveResult.RemovedItemInstance,
			Request.SourcePosition,
			false,
			true
		);

		const EPNInventoryContainerOperationResult FinalResult = RollbackResult.bSuccess
			? EPNInventoryContainerOperationResult::TargetAddFailed
			: EPNInventoryContainerOperationResult::RollbackFailed;

		FPNInventoryContainerTransferResponse Response = MakeTransferResponse(
			Request,
			FinalResult,
			false,
			false,
			QuantityToMove,
			0,
			QuantityToMove,
			ItemData
		);

		BroadcastTransferResponse(Response);
		return Response;
	}

	if (RemainingQuantity > 0)
	{
		RemoveResult.RemovedItemInstance->SetQuantityClamped(RemainingQuantity);

		SourceInventory->AddItemAtPosition(
			RemoveResult.RemovedItemInstance,
			Request.SourcePosition,
			false,
			true
		);

		FPNInventoryContainerTransferResponse Response = MakeTransferResponse(
			Request,
			EPNInventoryContainerOperationResult::PartialTransfer,
			true,
			true,
			QuantityToMove,
			AddResult.AddedQuantity,
			RemainingQuantity,
			ItemData
		);

		BroadcastTransferResponse(Response);
		return Response;
	}

	FPNInventoryContainerTransferResponse Response = MakeTransferResponse(
		Request,
		EPNInventoryContainerOperationResult::Success,
		true,
		false,
		QuantityToMove,
		AddResult.AddedQuantity,
		0,
		ItemData
	);

	BroadcastTransferResponse(Response);
	return Response;
}

void UPNInventoryContainerComponent::RequestMoveItemBetweenInventories(const FPNInventoryContainerTransferRequest& Request)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		MoveItemBetweenInventories(Request);
		return;
	}

	Server_MoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::Server_MoveItemBetweenInventories_Implementation(FPNInventoryContainerTransferRequest Request)
{
	MoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::RequestMoveOwnerItemToOpenedContainer(FPNInventoryGridPosition SourcePosition, int32 Quantity, bool bHalfStack)
{
	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	UPNInventoryComponent* ContainerInventory = GetOpenedContainerInventory();

	if (!OwnerInventory || !ContainerInventory)
	{
		return;
	}

	FPNInventoryContainerTransferRequest Request;
	Request.SourceInventory = OwnerInventory;
	Request.TargetInventory = ContainerInventory;
	Request.SourcePosition = SourcePosition;
	Request.Quantity = Quantity;
	Request.bHalfStack = bHalfStack;
	Request.bUseTargetPosition = false;

	RequestMoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::RequestMoveOwnerItemToOpenedContainerAtPosition(FPNInventoryGridPosition SourcePosition, FPNInventoryGridPosition TargetPosition, int32 Quantity, bool bHalfStack, bool bTargetRotated)
{
	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	UPNInventoryComponent* ContainerInventory = GetOpenedContainerInventory();

	if (!OwnerInventory || !ContainerInventory)
	{
		return;
	}

	FPNInventoryContainerTransferRequest Request;
	Request.SourceInventory = OwnerInventory;
	Request.TargetInventory = ContainerInventory;
	Request.SourcePosition = SourcePosition;
	Request.TargetPosition = TargetPosition;
	Request.Quantity = Quantity;
	Request.bHalfStack = bHalfStack;
	Request.bUseTargetPosition = true;
	Request.bTargetRotated = bTargetRotated;

	RequestMoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::RequestMoveOpenedContainerItemToOwner(FPNInventoryGridPosition SourcePosition, int32 Quantity, bool bHalfStack)
{
	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	UPNInventoryComponent* ContainerInventory = GetOpenedContainerInventory();

	if (!OwnerInventory || !ContainerInventory)
	{
		return;
	}

	FPNInventoryContainerTransferRequest Request;
	Request.SourceInventory = ContainerInventory;
	Request.TargetInventory = OwnerInventory;
	Request.SourcePosition = SourcePosition;
	Request.Quantity = Quantity;
	Request.bHalfStack = bHalfStack;
	Request.bUseTargetPosition = false;

	RequestMoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::RequestMoveOpenedContainerItemToOwnerAtPosition(FPNInventoryGridPosition SourcePosition, FPNInventoryGridPosition TargetPosition, int32 Quantity, bool bHalfStack, bool bTargetRotated)
{
	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	UPNInventoryComponent* ContainerInventory = GetOpenedContainerInventory();

	if (!OwnerInventory || !ContainerInventory)
	{
		return;
	}

	FPNInventoryContainerTransferRequest Request;
	Request.SourceInventory = ContainerInventory;
	Request.TargetInventory = OwnerInventory;
	Request.SourcePosition = SourcePosition;
	Request.TargetPosition = TargetPosition;
	Request.Quantity = Quantity;
	Request.bHalfStack = bHalfStack;
	Request.bUseTargetPosition = true;
	Request.bTargetRotated = bTargetRotated;

	RequestMoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::OnRep_OpenedContainerInventory()
{
	if (OpenedContainerInventory)
	{
		FPNInventoryContainerOpenResponse Response = MakeOpenResponse(
			EPNInventoryContainerOperationResult::Success,
			true,
			OpenedContainerInventory
		);

		OnContainerOpened.Broadcast(Response);
		PrintContainerDebugMessage(TEXT("Container opened replicated"), true);
		return;
	}

	OnContainerClosed.Broadcast();
	PrintContainerDebugMessage(TEXT("Container closed replicated"), true);
}

void UPNInventoryContainerComponent::RequestMoveOwnerItemToBackpack(FPNInventoryGridPosition SourcePosition, int32 Quantity, bool bHalfStack)
{
	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	UPNInventoryComponent* BackpackInventory = GetOwnerBackpackInventoryComponent();

	if (!OwnerInventory || !BackpackInventory)
	{
		return;
	}

	FPNInventoryContainerTransferRequest Request;
	Request.SourceInventory = OwnerInventory;
	Request.TargetInventory = BackpackInventory;
	Request.SourcePosition = SourcePosition;
	Request.Quantity = Quantity;
	Request.bHalfStack = bHalfStack;
	Request.bUseTargetPosition = false;

	RequestMoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::RequestMoveOwnerItemToBackpackAtPosition(FPNInventoryGridPosition SourcePosition, FPNInventoryGridPosition TargetPosition, int32 Quantity, bool bHalfStack, bool bTargetRotated)
{
	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	UPNInventoryComponent* BackpackInventory = GetOwnerBackpackInventoryComponent();

	if (!OwnerInventory || !BackpackInventory)
	{
		return;
	}

	FPNInventoryContainerTransferRequest Request;
	Request.SourceInventory = OwnerInventory;
	Request.TargetInventory = BackpackInventory;
	Request.SourcePosition = SourcePosition;
	Request.TargetPosition = TargetPosition;
	Request.Quantity = Quantity;
	Request.bHalfStack = bHalfStack;
	Request.bUseTargetPosition = true;
	Request.bTargetRotated = bTargetRotated;

	RequestMoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::RequestMoveBackpackItemToOwner(FPNInventoryGridPosition SourcePosition, int32 Quantity, bool bHalfStack)
{
	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	UPNInventoryComponent* BackpackInventory = GetOwnerBackpackInventoryComponent();

	if (!OwnerInventory || !BackpackInventory)
	{
		return;
	}

	FPNInventoryContainerTransferRequest Request;
	Request.SourceInventory = BackpackInventory;
	Request.TargetInventory = OwnerInventory;
	Request.SourcePosition = SourcePosition;
	Request.Quantity = Quantity;
	Request.bHalfStack = bHalfStack;
	Request.bUseTargetPosition = false;

	RequestMoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::RequestMoveBackpackItemToOwnerAtPosition(FPNInventoryGridPosition SourcePosition, FPNInventoryGridPosition TargetPosition, int32 Quantity, bool bHalfStack, bool bTargetRotated)
{
	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	UPNInventoryComponent* BackpackInventory = GetOwnerBackpackInventoryComponent();

	if (!OwnerInventory || !BackpackInventory)
	{
		return;
	}

	FPNInventoryContainerTransferRequest Request;
	Request.SourceInventory = BackpackInventory;
	Request.TargetInventory = OwnerInventory;
	Request.SourcePosition = SourcePosition;
	Request.TargetPosition = TargetPosition;
	Request.Quantity = Quantity;
	Request.bHalfStack = bHalfStack;
	Request.bUseTargetPosition = true;
	Request.bTargetRotated = bTargetRotated;

	RequestMoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::RequestMoveOwnerItemToVest(FPNInventoryGridPosition SourcePosition, int32 Quantity, bool bHalfStack)
{
	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	UPNInventoryComponent* VestInventory = GetOwnerVestInventoryComponent();

	if (!OwnerInventory || !VestInventory)
	{
		return;
	}

	FPNInventoryContainerTransferRequest Request;
	Request.SourceInventory = OwnerInventory;
	Request.TargetInventory = VestInventory;
	Request.SourcePosition = SourcePosition;
	Request.Quantity = Quantity;
	Request.bHalfStack = bHalfStack;
	Request.bUseTargetPosition = false;

	RequestMoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::RequestMoveOwnerItemToVestAtPosition(FPNInventoryGridPosition SourcePosition, FPNInventoryGridPosition TargetPosition, int32 Quantity, bool bHalfStack, bool bTargetRotated)
{
	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	UPNInventoryComponent* VestInventory = GetOwnerVestInventoryComponent();

	if (!OwnerInventory || !VestInventory)
	{
		return;
	}

	FPNInventoryContainerTransferRequest Request;
	Request.SourceInventory = OwnerInventory;
	Request.TargetInventory = VestInventory;
	Request.SourcePosition = SourcePosition;
	Request.TargetPosition = TargetPosition;
	Request.Quantity = Quantity;
	Request.bHalfStack = bHalfStack;
	Request.bUseTargetPosition = true;
	Request.bTargetRotated = bTargetRotated;

	RequestMoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::RequestMoveVestItemToOwner(FPNInventoryGridPosition SourcePosition, int32 Quantity, bool bHalfStack)
{
	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	UPNInventoryComponent* VestInventory = GetOwnerVestInventoryComponent();

	if (!OwnerInventory || !VestInventory)
	{
		return;
	}

	FPNInventoryContainerTransferRequest Request;
	Request.SourceInventory = VestInventory;
	Request.TargetInventory = OwnerInventory;
	Request.SourcePosition = SourcePosition;
	Request.Quantity = Quantity;
	Request.bHalfStack = bHalfStack;
	Request.bUseTargetPosition = false;

	RequestMoveItemBetweenInventories(Request);
}

void UPNInventoryContainerComponent::RequestMoveVestItemToOwnerAtPosition(FPNInventoryGridPosition SourcePosition, FPNInventoryGridPosition TargetPosition, int32 Quantity, bool bHalfStack, bool bTargetRotated)
{
	UPNInventoryComponent* OwnerInventory = GetOwnerInventoryComponent();
	UPNInventoryComponent* VestInventory = GetOwnerVestInventoryComponent();

	if (!OwnerInventory || !VestInventory)
	{
		return;
	}

	FPNInventoryContainerTransferRequest Request;
	Request.SourceInventory = VestInventory;
	Request.TargetInventory = OwnerInventory;
	Request.SourcePosition = SourcePosition;
	Request.TargetPosition = TargetPosition;
	Request.Quantity = Quantity;
	Request.bHalfStack = bHalfStack;
	Request.bUseTargetPosition = true;
	Request.bTargetRotated = bTargetRotated;

	RequestMoveItemBetweenInventories(Request);
}

bool UPNInventoryContainerComponent::HasContainerAuthority() const
{
	const AActor* OwnerActor = GetOwner();
	return !OwnerActor || OwnerActor->HasAuthority();
}

int32 UPNInventoryContainerComponent::ResolveTransferQuantity(UPNItemInstance* SourceItem, const FPNInventoryContainerTransferRequest& Request) const
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

FPNInventoryContainerTransferResponse UPNInventoryContainerComponent::MakeTransferResponse(
	const FPNInventoryContainerTransferRequest& Request,
	EPNInventoryContainerOperationResult Result,
	bool bSuccess,
	bool bPartial,
	int32 RequestedQuantity,
	int32 MovedQuantity,
	int32 RemainingQuantity,
	UPNItemDataAsset* ItemData
) const
{
	FPNInventoryContainerTransferResponse Response;
	Response.bSuccess = bSuccess;
	Response.bPartial = bPartial;
	Response.Result = Result;
	Response.SourceInventory = Request.SourceInventory;
	Response.TargetInventory = Request.TargetInventory;
	Response.SourcePosition = Request.SourcePosition;
	Response.TargetPosition = Request.TargetPosition;
	Response.RequestedQuantity = RequestedQuantity;
	Response.MovedQuantity = MovedQuantity;
	Response.RemainingQuantity = RemainingQuantity;
	Response.ItemData = ItemData;

	return Response;
}

FPNInventoryContainerOpenResponse UPNInventoryContainerComponent::MakeOpenResponse(
	EPNInventoryContainerOperationResult Result,
	bool bSuccess,
	UPNInventoryComponent* ContainerInventory
) const
{
	FPNInventoryContainerOpenResponse Response;
	Response.bSuccess = bSuccess;
	Response.Result = Result;
	Response.OwnerInventory = GetOwnerInventoryComponent();
	Response.ContainerInventory = ContainerInventory;
	Response.ContainerType = ContainerInventory ? ContainerInventory->Settings.InventoryType : EPNInventoryType::None;

	return Response;
}

void UPNInventoryContainerComponent::BroadcastOpenResponse(const FPNInventoryContainerOpenResponse& Response)
{
	if (Response.bSuccess)
	{
		OnContainerOpened.Broadcast(Response);
		PrintContainerDebugMessage(TEXT("Container opened"), true);
		return;
	}

	PrintContainerDebugMessage(FString::Printf(TEXT("Open container failed: %s"), *UEnum::GetValueAsString(Response.Result)), false);
}

void UPNInventoryContainerComponent::BroadcastTransferResponse(const FPNInventoryContainerTransferResponse& Response)
{
	OnContainerTransferCompleted.Broadcast(Response);

	PrintContainerDebugMessage(
		FString::Printf(
			TEXT("Transfer result: %s | Moved: %d | Remaining: %d"),
			*UEnum::GetValueAsString(Response.Result),
			Response.MovedQuantity,
			Response.RemainingQuantity
		),
		Response.bSuccess
	);
}

void UPNInventoryContainerComponent::PrintContainerDebugMessage(const FString& Message, bool bSuccess) const
{
	if (!bDebugContainerActions || !GEngine)
	{
		return;
	}

	const AActor* OwnerActor = GetOwner();

	GEngine->AddOnScreenDebugMessage(
		OwnerActor ? static_cast<int32>(OwnerActor->GetUniqueID()) + 52000 : INDEX_NONE,
		3.0f,
		bSuccess ? FColor::Green : FColor::Red,
		Message
	);
}