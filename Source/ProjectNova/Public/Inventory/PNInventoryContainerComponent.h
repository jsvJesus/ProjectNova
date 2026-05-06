#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/PNInventoryContainerTypes.h"
#include "PNInventoryContainerComponent.generated.h"

class UPNInventoryComponent;
class UPNItemInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPNInventoryContainerOpenedSignature, const FPNInventoryContainerOpenResponse&, Response);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPNInventoryContainerClosedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPNInventoryContainerTransferSignature, const FPNInventoryContainerTransferResponse&, Response);

UCLASS(ClassGroup = (ProjectNova), meta = (BlueprintSpawnableComponent))
class PROJECTNOVA_API UPNInventoryContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPNInventoryContainerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory Container")
	FPNInventoryContainerOpenedSignature OnContainerOpened;

	UPROPERTY(BlueprintAssignable, Category = "Inventory Container")
	FPNInventoryContainerClosedSignature OnContainerClosed;

	UPROPERTY(BlueprintAssignable, Category = "Inventory Container")
	FPNInventoryContainerTransferSignature OnContainerTransferCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container|Debug")
	bool bDebugContainerActions = true;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_OpenedContainerInventory, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Container")
	TObjectPtr<UPNInventoryComponent> OpenedContainerInventory = nullptr;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory Container")
	FPNInventoryContainerOpenResponse OpenContainer(UPNInventoryComponent* ContainerInventory);

	UFUNCTION(BlueprintCallable, Category = "Inventory Container")
	void RequestOpenContainer(UPNInventoryComponent* ContainerInventory);

	UFUNCTION(Server, Reliable)
	void Server_OpenContainer(UPNInventoryComponent* ContainerInventory);

	UFUNCTION(BlueprintCallable, Category = "Inventory Container")
	void CloseContainer();

	UFUNCTION(BlueprintCallable, Category = "Inventory Container")
	void RequestCloseContainer();

	UFUNCTION(Server, Reliable)
	void Server_CloseContainer();

	UFUNCTION(BlueprintPure, Category = "Inventory Container")
	bool IsContainerOpen() const;

	UFUNCTION(BlueprintPure, Category = "Inventory Container")
	UPNInventoryComponent* GetOpenedContainerInventory() const;

	UFUNCTION(BlueprintPure, Category = "Inventory Container")
	UPNInventoryComponent* GetOwnerInventoryComponent() const;

	UFUNCTION(BlueprintPure, Category = "Inventory Container")
	EPNInventoryType GetOpenedContainerType() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory Container|Transfer")
	FPNInventoryContainerTransferResponse MoveItemBetweenInventories(const FPNInventoryContainerTransferRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Inventory Container|Transfer")
	void RequestMoveItemBetweenInventories(const FPNInventoryContainerTransferRequest& Request);

	UFUNCTION(Server, Reliable)
	void Server_MoveItemBetweenInventories(FPNInventoryContainerTransferRequest Request);

	UFUNCTION(BlueprintCallable, Category = "Inventory Container|Transfer")
	void RequestMoveOwnerItemToOpenedContainer(FPNInventoryGridPosition SourcePosition, int32 Quantity = -1, bool bHalfStack = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory Container|Transfer")
	void RequestMoveOwnerItemToOpenedContainerAtPosition(FPNInventoryGridPosition SourcePosition, FPNInventoryGridPosition TargetPosition, int32 Quantity = -1, bool bHalfStack = false, bool bTargetRotated = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory Container|Transfer")
	void RequestMoveOpenedContainerItemToOwner(FPNInventoryGridPosition SourcePosition, int32 Quantity = -1, bool bHalfStack = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory Container|Transfer")
	void RequestMoveOpenedContainerItemToOwnerAtPosition(FPNInventoryGridPosition SourcePosition, FPNInventoryGridPosition TargetPosition, int32 Quantity = -1, bool bHalfStack = false, bool bTargetRotated = false);

protected:
	UFUNCTION()
	void OnRep_OpenedContainerInventory();

	bool HasContainerAuthority() const;

	int32 ResolveTransferQuantity(UPNItemInstance* SourceItem, const FPNInventoryContainerTransferRequest& Request) const;

	FPNInventoryContainerTransferResponse MakeTransferResponse(
		const FPNInventoryContainerTransferRequest& Request,
		EPNInventoryContainerOperationResult Result,
		bool bSuccess = false,
		bool bPartial = false,
		int32 RequestedQuantity = 0,
		int32 MovedQuantity = 0,
		int32 RemainingQuantity = 0,
		UPNItemDataAsset* ItemData = nullptr
	) const;

	FPNInventoryContainerOpenResponse MakeOpenResponse(
		EPNInventoryContainerOperationResult Result,
		bool bSuccess = false,
		UPNInventoryComponent* ContainerInventory = nullptr
	) const;

	void BroadcastOpenResponse(const FPNInventoryContainerOpenResponse& Response);
	void BroadcastTransferResponse(const FPNInventoryContainerTransferResponse& Response);
	void PrintContainerDebugMessage(const FString& Message, bool bSuccess) const;
};