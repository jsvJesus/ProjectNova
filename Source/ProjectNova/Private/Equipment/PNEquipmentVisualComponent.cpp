#include "Equipment/PNEquipmentVisualComponent.h"

#include "Characters/PNBaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Equipment/PNEquipmentComponent.h"
#include "Items/PNItemDataAsset.h"
#include "Items/PNItemTypes.h"

UPNEquipmentVisualComponent::UPNEquipmentVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UPNEquipmentVisualComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	CreateVisualComponents();

	if (UPNEquipmentComponent* EquipmentComponent = GetOwnerEquipmentComponent())
	{
		EquipmentComponent->OnEquipmentChanged.AddDynamic(this, &UPNEquipmentVisualComponent::HandleEquipmentChanged);
	}

	RefreshEquipmentVisuals();
}

void UPNEquipmentVisualComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UPNEquipmentComponent* EquipmentComponent = GetOwnerEquipmentComponent())
	{
		EquipmentComponent->OnEquipmentChanged.RemoveDynamic(this, &UPNEquipmentVisualComponent::HandleEquipmentChanged);
	}

	DestroyVisualComponents();

	Super::EndPlay(EndPlayReason);
}

void UPNEquipmentVisualComponent::RefreshEquipmentVisuals()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	UPNEquipmentComponent* EquipmentComponent = GetOwnerEquipmentComponent();
	if (!EquipmentComponent)
	{
		ClearAllEquipmentVisuals();
		return;
	}

	RefreshSlotVisual(EPNEquipmentSlot::Helmet, EquipmentComponent->GetEquippedItemData(EPNEquipmentSlot::Helmet));
	RefreshSlotVisual(EPNEquipmentSlot::Armor, EquipmentComponent->GetEquippedItemData(EPNEquipmentSlot::Armor));
	RefreshSlotVisual(EPNEquipmentSlot::Backpack, EquipmentComponent->GetEquippedItemData(EPNEquipmentSlot::Backpack));
	RefreshSlotVisual(EPNEquipmentSlot::Gloves, EquipmentComponent->GetEquippedItemData(EPNEquipmentSlot::Gloves));

	RefreshSlotVisual(EPNEquipmentSlot::PrimaryWeapon1, EquipmentComponent->GetEquippedItemData(EPNEquipmentSlot::PrimaryWeapon1));
	RefreshSlotVisual(EPNEquipmentSlot::PrimaryWeapon2, EquipmentComponent->GetEquippedItemData(EPNEquipmentSlot::PrimaryWeapon2));
	RefreshSlotVisual(EPNEquipmentSlot::Sidearm, EquipmentComponent->GetEquippedItemData(EPNEquipmentSlot::Sidearm));
	RefreshSlotVisual(EPNEquipmentSlot::Knife, EquipmentComponent->GetEquippedItemData(EPNEquipmentSlot::Knife));

	BroadcastVisualsChanged();
}

void UPNEquipmentVisualComponent::ClearAllEquipmentVisuals()
{
	ClearSlotVisual(EPNEquipmentSlot::Helmet);
	ClearSlotVisual(EPNEquipmentSlot::Armor);
	ClearSlotVisual(EPNEquipmentSlot::Backpack);
	ClearSlotVisual(EPNEquipmentSlot::Gloves);

	ClearSlotVisual(EPNEquipmentSlot::PrimaryWeapon1);
	ClearSlotVisual(EPNEquipmentSlot::PrimaryWeapon2);
	ClearSlotVisual(EPNEquipmentSlot::Sidearm);
	ClearSlotVisual(EPNEquipmentSlot::Knife);

	BroadcastVisualsChanged();
}

USkeletalMeshComponent* UPNEquipmentVisualComponent::GetSkeletalVisualComponent(EPNEquipmentSlot Slot) const
{
	switch (Slot)
	{
	case EPNEquipmentSlot::Helmet:
		return HelmetSkeletalVisualComponent;

	case EPNEquipmentSlot::Armor:
		return ArmorSkeletalVisualComponent;

	case EPNEquipmentSlot::Backpack:
		return BackpackSkeletalVisualComponent;

	case EPNEquipmentSlot::Gloves:
		return GlovesSkeletalVisualComponent;

	case EPNEquipmentSlot::PrimaryWeapon1:
		return PrimaryWeapon1SkeletalVisualComponent;

	case EPNEquipmentSlot::PrimaryWeapon2:
		return PrimaryWeapon2SkeletalVisualComponent;

	case EPNEquipmentSlot::Sidearm:
		return SidearmSkeletalVisualComponent;

	case EPNEquipmentSlot::Knife:
		return KnifeSkeletalVisualComponent;

	default:
		return nullptr;
	}
}

UStaticMeshComponent* UPNEquipmentVisualComponent::GetStaticVisualComponent(EPNEquipmentSlot Slot) const
{
	switch (Slot)
	{
	case EPNEquipmentSlot::Helmet:
		return HelmetStaticVisualComponent;

	case EPNEquipmentSlot::Armor:
		return ArmorStaticVisualComponent;

	case EPNEquipmentSlot::Backpack:
		return BackpackStaticVisualComponent;

	case EPNEquipmentSlot::Gloves:
		return GlovesStaticVisualComponent;

	case EPNEquipmentSlot::PrimaryWeapon1:
		return PrimaryWeapon1StaticVisualComponent;

	case EPNEquipmentSlot::PrimaryWeapon2:
		return PrimaryWeapon2StaticVisualComponent;

	case EPNEquipmentSlot::Sidearm:
		return SidearmStaticVisualComponent;

	case EPNEquipmentSlot::Knife:
		return KnifeStaticVisualComponent;

	default:
		return nullptr;
	}
}

FString UPNEquipmentVisualComponent::GetEquipmentVisualDebugString() const
{
	const AActor* OwnerActor = GetOwner();

	FString Result = FString::Printf(
		TEXT("[EquipmentVisuals] Owner: %s"),
		OwnerActor ? *OwnerActor->GetName() : TEXT("NoOwner")
	);

	const UPNEquipmentComponent* EquipmentComponent = GetOwnerEquipmentComponent();
	if (!EquipmentComponent)
	{
		Result += TEXT("\nNo EquipmentComponent");
		return Result;
	}

	const TArray<EPNEquipmentSlot> Slots =
	{
		EPNEquipmentSlot::Helmet,
		EPNEquipmentSlot::Armor,
		EPNEquipmentSlot::Backpack,
		EPNEquipmentSlot::Gloves,
		EPNEquipmentSlot::PrimaryWeapon1,
		EPNEquipmentSlot::PrimaryWeapon2,
		EPNEquipmentSlot::Sidearm,
		EPNEquipmentSlot::Knife
	};

	for (EPNEquipmentSlot Slot : Slots)
	{
		const UPNItemDataAsset* ItemData = EquipmentComponent->GetEquippedItemData(Slot);
		const FString ItemName = ItemData
			? (ItemData->GetItemName().IsEmpty() ? ItemData->GetItemId().ToString() : ItemData->GetItemName().ToString())
			: TEXT("Empty");

		const USkeletalMeshComponent* SkeletalComp = GetSkeletalVisualComponent(Slot);
		const UStaticMeshComponent* StaticComp = GetStaticVisualComponent(Slot);

		Result += FString::Printf(
			TEXT("\n%s -> %s | SK:%s | SM:%s"),
			*UEnum::GetValueAsString(Slot),
			*ItemName,
			SkeletalComp && SkeletalComp->GetSkeletalMeshAsset() ? TEXT("Yes") : TEXT("No"),
			StaticComp && StaticComp->GetStaticMesh() ? TEXT("Yes") : TEXT("No")
		);
	}

	return Result;
}

void UPNEquipmentVisualComponent::PrintEquipmentVisualDebug() const
{
	const FString DebugText = GetEquipmentVisualDebugString();

	UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugText);

	if (GEngine)
	{
		const int32 DebugKey = GetOwner()
			? static_cast<int32>(GetOwner()->GetUniqueID()) + 40000
			: INDEX_NONE;

		GEngine->AddOnScreenDebugMessage(
			DebugKey,
			5.0f,
			FColor::Purple,
			DebugText
		);
	}
}

void UPNEquipmentVisualComponent::HandleEquipmentChanged()
{
	RefreshEquipmentVisuals();
}

APNBaseCharacter* UPNEquipmentVisualComponent::GetOwnerCharacter() const
{
	return Cast<APNBaseCharacter>(GetOwner());
}

UPNEquipmentComponent* UPNEquipmentVisualComponent::GetOwnerEquipmentComponent() const
{
	const APNBaseCharacter* OwnerCharacter = GetOwnerCharacter();
	return OwnerCharacter ? OwnerCharacter->GetEquipmentComponent() : nullptr;
}

void UPNEquipmentVisualComponent::CreateVisualComponents()
{
	if (HelmetSkeletalVisualComponent)
	{
		return;
	}

	HelmetSkeletalVisualComponent = CreateSkeletalVisualComponent(TEXT("HelmetSkeletalVisualComponent"));
	HelmetStaticVisualComponent = CreateStaticVisualComponent(TEXT("HelmetStaticVisualComponent"));

	ArmorSkeletalVisualComponent = CreateSkeletalVisualComponent(TEXT("ArmorSkeletalVisualComponent"));
	ArmorStaticVisualComponent = CreateStaticVisualComponent(TEXT("ArmorStaticVisualComponent"));

	BackpackSkeletalVisualComponent = CreateSkeletalVisualComponent(TEXT("BackpackSkeletalVisualComponent"));
	BackpackStaticVisualComponent = CreateStaticVisualComponent(TEXT("BackpackStaticVisualComponent"));

	GlovesSkeletalVisualComponent = CreateSkeletalVisualComponent(TEXT("GlovesSkeletalVisualComponent"));
	GlovesStaticVisualComponent = CreateStaticVisualComponent(TEXT("GlovesStaticVisualComponent"));

	PrimaryWeapon1SkeletalVisualComponent = CreateSkeletalVisualComponent(TEXT("PrimaryWeapon1SkeletalVisualComponent"));
	PrimaryWeapon1StaticVisualComponent = CreateStaticVisualComponent(TEXT("PrimaryWeapon1StaticVisualComponent"));

	PrimaryWeapon2SkeletalVisualComponent = CreateSkeletalVisualComponent(TEXT("PrimaryWeapon2SkeletalVisualComponent"));
	PrimaryWeapon2StaticVisualComponent = CreateStaticVisualComponent(TEXT("PrimaryWeapon2StaticVisualComponent"));

	SidearmSkeletalVisualComponent = CreateSkeletalVisualComponent(TEXT("SidearmSkeletalVisualComponent"));
	SidearmStaticVisualComponent = CreateStaticVisualComponent(TEXT("SidearmStaticVisualComponent"));

	KnifeSkeletalVisualComponent = CreateSkeletalVisualComponent(TEXT("KnifeSkeletalVisualComponent"));
	KnifeStaticVisualComponent = CreateStaticVisualComponent(TEXT("KnifeStaticVisualComponent"));

	ClearAllEquipmentVisuals();
}

void UPNEquipmentVisualComponent::DestroyVisualComponents()
{
	TArray<UActorComponent*> ComponentsToDestroy =
	{
		HelmetSkeletalVisualComponent,
		HelmetStaticVisualComponent,
		ArmorSkeletalVisualComponent,
		ArmorStaticVisualComponent,
		BackpackSkeletalVisualComponent,
		BackpackStaticVisualComponent,
		GlovesSkeletalVisualComponent,
		GlovesStaticVisualComponent,
		PrimaryWeapon1SkeletalVisualComponent,
		PrimaryWeapon1StaticVisualComponent,
		PrimaryWeapon2SkeletalVisualComponent,
		PrimaryWeapon2StaticVisualComponent,
		SidearmSkeletalVisualComponent,
		SidearmStaticVisualComponent,
		KnifeSkeletalVisualComponent,
		KnifeStaticVisualComponent
	};

	for (UActorComponent* Component : ComponentsToDestroy)
	{
		if (Component)
		{
			Component->DestroyComponent();
		}
	}

	HelmetSkeletalVisualComponent = nullptr;
	HelmetStaticVisualComponent = nullptr;
	ArmorSkeletalVisualComponent = nullptr;
	ArmorStaticVisualComponent = nullptr;
	BackpackSkeletalVisualComponent = nullptr;
	BackpackStaticVisualComponent = nullptr;
	GlovesSkeletalVisualComponent = nullptr;
	GlovesStaticVisualComponent = nullptr;
	PrimaryWeapon1SkeletalVisualComponent = nullptr;
	PrimaryWeapon1StaticVisualComponent = nullptr;
	PrimaryWeapon2SkeletalVisualComponent = nullptr;
	PrimaryWeapon2StaticVisualComponent = nullptr;
	SidearmSkeletalVisualComponent = nullptr;
	SidearmStaticVisualComponent = nullptr;
	KnifeSkeletalVisualComponent = nullptr;
	KnifeStaticVisualComponent = nullptr;
}

USkeletalMeshComponent* UPNEquipmentVisualComponent::CreateSkeletalVisualComponent(FName ComponentName)
{
	APNBaseCharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
	{
		return nullptr;
	}

	USkeletalMeshComponent* NewComponent = NewObject<USkeletalMeshComponent>(OwnerCharacter, ComponentName);
	if (!NewComponent)
	{
		return nullptr;
	}

	NewComponent->SetIsReplicated(false);
	NewComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NewComponent->SetGenerateOverlapEvents(false);
	NewComponent->CastShadow = true;
	NewComponent->bCastDynamicShadow = true;
	NewComponent->SetVisibility(false, true);
	NewComponent->SetHiddenInGame(true, true);

	OwnerCharacter->AddInstanceComponent(NewComponent);
	NewComponent->RegisterComponent();

	NewComponent->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	return NewComponent;
}

UStaticMeshComponent* UPNEquipmentVisualComponent::CreateStaticVisualComponent(FName ComponentName)
{
	APNBaseCharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
	{
		return nullptr;
	}

	UStaticMeshComponent* NewComponent = NewObject<UStaticMeshComponent>(OwnerCharacter, ComponentName);
	if (!NewComponent)
	{
		return nullptr;
	}

	NewComponent->SetIsReplicated(false);
	NewComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NewComponent->SetGenerateOverlapEvents(false);
	NewComponent->CastShadow = true;
	NewComponent->bCastDynamicShadow = true;
	NewComponent->SetVisibility(false, true);
	NewComponent->SetHiddenInGame(true, true);

	OwnerCharacter->AddInstanceComponent(NewComponent);
	NewComponent->RegisterComponent();

	NewComponent->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	return NewComponent;
}

void UPNEquipmentVisualComponent::RefreshSlotVisual(EPNEquipmentSlot Slot, UPNItemDataAsset* ItemData)
{
	if (!ItemData)
	{
		ClearSlotVisual(Slot);
		return;
	}

	const USkeletalMesh* SkeletalMesh = ItemData->Visual.SkeletalMesh.LoadSynchronous();
	const UStaticMesh* StaticMesh = ItemData->Visual.StaticMesh.LoadSynchronous();

	if (SkeletalMesh)
	{
		ApplySkeletalVisual(Slot, ItemData);
		return;
	}

	if (StaticMesh)
	{
		ApplyStaticVisual(Slot, ItemData);
		return;
	}

	ClearSlotVisual(Slot);
}

void UPNEquipmentVisualComponent::ClearSlotVisual(EPNEquipmentSlot Slot)
{
	ClearSkeletalVisual(GetSkeletalVisualComponent(Slot));
	ClearStaticVisual(GetStaticVisualComponent(Slot));
}

void UPNEquipmentVisualComponent::ApplySkeletalVisual(EPNEquipmentSlot Slot, UPNItemDataAsset* ItemData)
{
	if (!ItemData)
	{
		ClearSlotVisual(Slot);
		return;
	}

	APNBaseCharacter* OwnerCharacter = GetOwnerCharacter();
	USkeletalMeshComponent* SkeletalComponent = GetSkeletalVisualComponent(Slot);
	UStaticMeshComponent* StaticComponent = GetStaticVisualComponent(Slot);

	if (!OwnerCharacter || !OwnerCharacter->GetMesh() || !SkeletalComponent)
	{
		return;
	}

	USkeletalMesh* SkeletalMesh = ItemData->Visual.SkeletalMesh.LoadSynchronous();
	if (!SkeletalMesh)
	{
		ClearSlotVisual(Slot);
		return;
	}

	ClearStaticVisual(StaticComponent);

	SkeletalComponent->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		GetAttachSocketForSlot(Slot, ItemData)
	);

	SkeletalComponent->SetSkeletalMesh(SkeletalMesh);

	if (ShouldUseLeaderPose(Slot))
	{
		SkeletalComponent->SetLeaderPoseComponent(OwnerCharacter->GetMesh());
	}
	else
	{
		SkeletalComponent->SetLeaderPoseComponent(nullptr);
	}

	SkeletalComponent->SetVisibility(true, true);
	SkeletalComponent->SetHiddenInGame(false, true);
}

void UPNEquipmentVisualComponent::ApplyStaticVisual(EPNEquipmentSlot Slot, UPNItemDataAsset* ItemData)
{
	if (!ItemData)
	{
		ClearSlotVisual(Slot);
		return;
	}

	APNBaseCharacter* OwnerCharacter = GetOwnerCharacter();
	UStaticMeshComponent* StaticComponent = GetStaticVisualComponent(Slot);
	USkeletalMeshComponent* SkeletalComponent = GetSkeletalVisualComponent(Slot);

	if (!OwnerCharacter || !OwnerCharacter->GetMesh() || !StaticComponent)
	{
		return;
	}

	UStaticMesh* StaticMesh = ItemData->Visual.StaticMesh.LoadSynchronous();
	if (!StaticMesh)
	{
		ClearSlotVisual(Slot);
		return;
	}

	ClearSkeletalVisual(SkeletalComponent);

	StaticComponent->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		GetAttachSocketForSlot(Slot, ItemData)
	);

	StaticComponent->SetStaticMesh(StaticMesh);
	StaticComponent->SetVisibility(true, true);
	StaticComponent->SetHiddenInGame(false, true);
}

void UPNEquipmentVisualComponent::ClearSkeletalVisual(USkeletalMeshComponent* Component)
{
	if (!Component)
	{
		return;
	}

	Component->SetLeaderPoseComponent(nullptr);
	Component->SetSkeletalMesh(nullptr);
	Component->SetVisibility(false, true);
	Component->SetHiddenInGame(true, true);
}

void UPNEquipmentVisualComponent::ClearStaticVisual(UStaticMeshComponent* Component)
{
	if (!Component)
	{
		return;
	}

	Component->SetStaticMesh(nullptr);
	Component->SetVisibility(false, true);
	Component->SetHiddenInGame(true, true);
}

FName UPNEquipmentVisualComponent::GetAttachSocketForSlot(EPNEquipmentSlot Slot, UPNItemDataAsset* ItemData) const
{
	if (ItemData && ItemData->WeaponStats.HandSocketName != NAME_None)
	{
		if (Slot == EPNEquipmentSlot::PrimaryWeapon1
			|| Slot == EPNEquipmentSlot::PrimaryWeapon2
			|| Slot == EPNEquipmentSlot::Sidearm
			|| Slot == EPNEquipmentSlot::Knife)
		{
			return ItemData->WeaponStats.HandSocketName;
		}
	}

	switch (Slot)
	{
	case EPNEquipmentSlot::Helmet:
		return HelmetSocketName;

	case EPNEquipmentSlot::Armor:
		return ArmorSocketName;

	case EPNEquipmentSlot::Backpack:
		return BackpackSocketName;

	case EPNEquipmentSlot::Gloves:
		return GlovesSocketName;

	case EPNEquipmentSlot::PrimaryWeapon1:
		return PrimaryWeapon1SocketName;

	case EPNEquipmentSlot::PrimaryWeapon2:
		return PrimaryWeapon2SocketName;

	case EPNEquipmentSlot::Sidearm:
		return SidearmSocketName;

	case EPNEquipmentSlot::Knife:
		return KnifeSocketName;

	default:
		return NAME_None;
	}
}

bool UPNEquipmentVisualComponent::ShouldUseLeaderPose(EPNEquipmentSlot Slot) const
{
	return Slot == EPNEquipmentSlot::Helmet
		|| Slot == EPNEquipmentSlot::Armor
		|| Slot == EPNEquipmentSlot::Gloves;
}

void UPNEquipmentVisualComponent::BroadcastVisualsChanged()
{
	OnEquipmentVisualsChanged.Broadcast();

	if (bDebugEquipmentVisuals)
	{
		PrintEquipmentVisualDebug();
	}
}