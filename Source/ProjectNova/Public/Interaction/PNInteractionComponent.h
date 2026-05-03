#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PNInteractionComponent.generated.h"

class APawn;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPNFocusedInteractableChangedSignature,
	AActor*, OldInteractable,
	AActor*, NewInteractable
);

UCLASS(ClassGroup = (ProjectNova), meta = (BlueprintSpawnableComponent))
class PROJECTNOVA_API UPNInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPNInteractionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "50.0"))
	float InteractionDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0"))
	float ServerValidationExtraDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_Visibility;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<AActor> CurrentInteractableActor = nullptr;

public:
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FPNFocusedInteractableChangedSignature OnFocusedInteractableChanged;

public:
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentInteractableActor() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasFocusedInteractable() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetCurrentInteractionDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetCurrentInteractionActionText() const;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RequestInteract();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RefreshFocusedInteractable();

	UFUNCTION(Server, Reliable)
	void Server_Interact(AActor* TargetActor);

protected:
	APawn* GetOwnerPawn() const;
	APlayerController* GetOwnerPlayerController() const;

	bool GetInteractionViewPoint(FVector& OutViewLocation, FRotator& OutViewRotation) const;
	bool TraceForInteractable(AActor*& OutInteractableActor) const;
	bool IsValidInteractable(AActor* TargetActor) const;
	bool IsTargetInServerRange(AActor* TargetActor) const;

	void SetCurrentInteractableActor(AActor* NewInteractableActor);
};