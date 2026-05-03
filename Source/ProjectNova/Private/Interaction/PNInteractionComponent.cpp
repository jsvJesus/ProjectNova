#include "Interaction/PNInteractionComponent.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/PNInteractableInterface.h"

UPNInteractionComponent::UPNInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SetIsReplicatedByDefault(true);
}

void UPNInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPNInteractionComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	RefreshFocusedInteractable();
}

AActor* UPNInteractionComponent::GetCurrentInteractableActor() const
{
	return CurrentInteractableActor;
}

bool UPNInteractionComponent::HasFocusedInteractable() const
{
	return CurrentInteractableActor != nullptr;
}

FText UPNInteractionComponent::GetCurrentInteractionDisplayName() const
{
	APawn* OwnerPawn = GetOwnerPawn();

	if (!CurrentInteractableActor || !CurrentInteractableActor->GetClass()->ImplementsInterface(UPNInteractableInterface::StaticClass()))
	{
		return FText::GetEmpty();
	}

	return IPNInteractableInterface::Execute_GetInteractionDisplayName(CurrentInteractableActor, OwnerPawn);
}

FText UPNInteractionComponent::GetCurrentInteractionActionText() const
{
	APawn* OwnerPawn = GetOwnerPawn();

	if (!CurrentInteractableActor || !CurrentInteractableActor->GetClass()->ImplementsInterface(UPNInteractableInterface::StaticClass()))
	{
		return FText::GetEmpty();
	}

	return IPNInteractableInterface::Execute_GetInteractionActionText(CurrentInteractableActor, OwnerPawn);
}

void UPNInteractionComponent::RequestInteract()
{
	APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn)
	{
		return;
	}

	AActor* TargetActor = CurrentInteractableActor;

	if (!TargetActor)
	{
		TraceForInteractable(TargetActor);
	}

	if (!TargetActor)
	{
		return;
	}

	if (OwnerPawn->HasAuthority())
	{
		Server_Interact_Implementation(TargetActor);
		return;
	}

	Server_Interact(TargetActor);
}

void UPNInteractionComponent::RefreshFocusedInteractable()
{
	AActor* NewInteractableActor = nullptr;
	TraceForInteractable(NewInteractableActor);
	SetCurrentInteractableActor(NewInteractableActor);
}

void UPNInteractionComponent::Server_Interact_Implementation(AActor* TargetActor)
{
	APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn || !TargetActor)
	{
		return;
	}

	if (!IsValidInteractable(TargetActor))
	{
		return;
	}

	if (!IsTargetInServerRange(TargetActor))
	{
		return;
	}

	if (!IPNInteractableInterface::Execute_CanInteract(TargetActor, OwnerPawn))
	{
		return;
	}

	IPNInteractableInterface::Execute_Interact(TargetActor, OwnerPawn);
}

APawn* UPNInteractionComponent::GetOwnerPawn() const
{
	return Cast<APawn>(GetOwner());
}

APlayerController* UPNInteractionComponent::GetOwnerPlayerController() const
{
	const APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn)
	{
		return nullptr;
	}

	return Cast<APlayerController>(OwnerPawn->GetController());
}

bool UPNInteractionComponent::GetInteractionViewPoint(FVector& OutViewLocation, FRotator& OutViewRotation) const
{
	APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn)
	{
		return false;
	}

	APlayerController* PlayerController = GetOwnerPlayerController();
	if (PlayerController)
	{
		PlayerController->GetPlayerViewPoint(OutViewLocation, OutViewRotation);
		return true;
	}

	OutViewLocation = OwnerPawn->GetPawnViewLocation();
	OutViewRotation = OwnerPawn->GetControlRotation();
	return true;
}

bool UPNInteractionComponent::TraceForInteractable(AActor*& OutInteractableActor) const
{
	OutInteractableActor = nullptr;

	APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn)
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	if (!GetInteractionViewPoint(ViewLocation, ViewRotation))
	{
		return false;
	}

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * InteractionDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(OwnerPawn);

	FHitResult HitResult;
	UWorld* World = GetWorld();

	if (!World)
	{
		return false;
	}

	const bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		InteractionTraceChannel,
		QueryParams
	);

	if (!bHit || !HitResult.GetActor())
	{
		return false;
	}

	AActor* HitActor = HitResult.GetActor();

	if (!IsValidInteractable(HitActor))
	{
		return false;
	}

	if (!IPNInteractableInterface::Execute_CanInteract(HitActor, OwnerPawn))
	{
		return false;
	}

	OutInteractableActor = HitActor;
	return true;
}

bool UPNInteractionComponent::IsValidInteractable(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return false;
	}

	if (TargetActor->IsPendingKillPending())
	{
		return false;
	}

	return TargetActor->GetClass()->ImplementsInterface(UPNInteractableInterface::StaticClass());
}

bool UPNInteractionComponent::IsTargetInServerRange(AActor* TargetActor) const
{
	APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn || !TargetActor)
	{
		return false;
	}

	const float MaxDistance = InteractionDistance + ServerValidationExtraDistance;
	const float DistanceSquared = FVector::DistSquared(OwnerPawn->GetActorLocation(), TargetActor->GetActorLocation());

	return DistanceSquared <= FMath::Square(MaxDistance);
}

void UPNInteractionComponent::SetCurrentInteractableActor(AActor* NewInteractableActor)
{
	if (CurrentInteractableActor == NewInteractableActor)
	{
		return;
	}

	AActor* OldInteractableActor = CurrentInteractableActor;
	CurrentInteractableActor = NewInteractableActor;

	OnFocusedInteractableChanged.Broadcast(OldInteractableActor, CurrentInteractableActor);
}