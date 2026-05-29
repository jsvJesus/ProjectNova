#include "Characters/PNPlayerCharacter.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interaction/PNInteractionComponent.h"
#include "Items/PNQuickSlotComponent.h"
#include "Equipment/PNEquipmentComponent.h"
#include "Items/PNItemDataAsset.h"
#include "Net/UnrealNetwork.h"

APNPlayerCharacter::APNPlayerCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCameraComponent"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	FirstPersonArmsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArmsMeshComponent"));
	FirstPersonArmsMeshComponent->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonArmsMeshComponent->SetRelativeLocation(FirstPersonArmsRelativeLocation);
	FirstPersonArmsMeshComponent->SetRelativeRotation(FirstPersonArmsRelativeRotation);
	FirstPersonArmsMeshComponent->SetOnlyOwnerSee(true);
	FirstPersonArmsMeshComponent->SetOwnerNoSee(false);
	FirstPersonArmsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonArmsMeshComponent->SetVisibility(true, true);
	FirstPersonArmsMeshComponent->SetHiddenInGame(false, true);
	FirstPersonArmsMeshComponent->CastShadow = false;
	FirstPersonArmsMeshComponent->bCastDynamicShadow = false;

	FirstPersonEquippedStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstPersonEquippedStaticMeshComponent"));
	FirstPersonEquippedStaticMeshComponent->SetupAttachment(FirstPersonArmsMeshComponent);
	FirstPersonEquippedStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonEquippedStaticMeshComponent->SetOnlyOwnerSee(true);
	FirstPersonEquippedStaticMeshComponent->SetOwnerNoSee(false);
	FirstPersonEquippedStaticMeshComponent->SetVisibility(false, true);
	FirstPersonEquippedStaticMeshComponent->SetHiddenInGame(true, true);
	FirstPersonEquippedStaticMeshComponent->CastShadow = false;
	FirstPersonEquippedStaticMeshComponent->bCastDynamicShadow = false;

	FirstPersonEquippedSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonEquippedSkeletalMeshComponent"));
	FirstPersonEquippedSkeletalMeshComponent->SetupAttachment(FirstPersonArmsMeshComponent);
	FirstPersonEquippedSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonEquippedSkeletalMeshComponent->SetOnlyOwnerSee(true);
	FirstPersonEquippedSkeletalMeshComponent->SetOwnerNoSee(false);
	FirstPersonEquippedSkeletalMeshComponent->SetVisibility(false, true);
	FirstPersonEquippedSkeletalMeshComponent->SetHiddenInGame(true, true);
	FirstPersonEquippedSkeletalMeshComponent->CastShadow = false;
	FirstPersonEquippedSkeletalMeshComponent->bCastDynamicShadow = false;

	InteractionComponent = CreateDefaultSubobject<UPNInteractionComponent>(TEXT("InteractionComponent"));
}

void APNPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	RefreshFirstPersonVisibility();

	if (FirstPersonArmsMeshComponent)
	{
		FirstPersonArmsMeshComponent->SetVisibility(true, true);
		FirstPersonArmsMeshComponent->SetHiddenInGame(false, true);
		FirstPersonArmsMeshComponent->SetOnlyOwnerSee(true);
		FirstPersonArmsMeshComponent->SetOwnerNoSee(false);
		FirstPersonArmsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FirstPersonArmsMeshComponent->CastShadow = false;
		FirstPersonArmsMeshComponent->bCastDynamicShadow = false;
	}

	ApplyFirstPersonArmsMesh();
	ApplyFirstPersonArmsAnimClass();
	RefreshFirstPersonEquippedItemVisual();

	if (UPNEquipmentComponent* PNEquipmentComponent = GetEquipmentComponent())
	{
		PNEquipmentComponent->OnEquipmentChanged.AddUniqueDynamic(
			this,
			&APNPlayerCharacter::HandleFirstPersonEquipmentChanged
		);
	}

	if (HasAuthority())
	{
		RefreshFirstPersonAnimTypeFromEquipment();
	}
}

void APNPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UPNEquipmentComponent* PNEquipmentComponent = GetEquipmentComponent())
	{
		PNEquipmentComponent->OnEquipmentChanged.RemoveDynamic(
			this,
			&APNPlayerCharacter::HandleFirstPersonEquipmentChanged
		);
	}

	Super::EndPlay(EndPlayReason);
}

void APNPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent)
	{
		return;
	}

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &APNPlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &APNPlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APNPlayerCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APNPlayerCharacter::LookUp);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &APNPlayerCharacter::StartJump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &APNPlayerCharacter::StopJump);

	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &APNPlayerCharacter::StartSprint);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &APNPlayerCharacter::StopSprint);

	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &APNPlayerCharacter::StartCrouchInput);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Released, this, &APNPlayerCharacter::StopCrouchInput);

	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &APNPlayerCharacter::StartInteractInput);

	PlayerInputComponent->BindAction(TEXT("QuickSlot_5"), IE_Pressed, this, &APNPlayerCharacter::QuickSlotInput_5);
	PlayerInputComponent->BindAction(TEXT("QuickSlot_6"), IE_Pressed, this, &APNPlayerCharacter::QuickSlotInput_6);
	PlayerInputComponent->BindAction(TEXT("QuickSlot_7"), IE_Pressed, this, &APNPlayerCharacter::QuickSlotInput_7);
	PlayerInputComponent->BindAction(TEXT("QuickSlot_8"), IE_Pressed, this, &APNPlayerCharacter::QuickSlotInput_8);
	PlayerInputComponent->BindAction(TEXT("QuickSlot_9"), IE_Pressed, this, &APNPlayerCharacter::QuickSlotInput_9);
	PlayerInputComponent->BindAction(TEXT("QuickSlot_0"), IE_Pressed, this, &APNPlayerCharacter::QuickSlotInput_0);
}

void APNPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APNPlayerCharacter, FirstPersonArmsMeshAsset);
	DOREPLIFETIME(APNPlayerCharacter, FirstPersonAnimType);
}

UCameraComponent* APNPlayerCharacter::GetFirstPersonCameraComponent() const
{
	return FirstPersonCameraComponent;
}

USkeletalMeshComponent* APNPlayerCharacter::GetFirstPersonArmsMeshComponent() const
{
	return FirstPersonArmsMeshComponent;
}

UStaticMeshComponent* APNPlayerCharacter::GetFirstPersonEquippedStaticMeshComponent() const
{
	return FirstPersonEquippedStaticMeshComponent;
}

USkeletalMeshComponent* APNPlayerCharacter::GetFirstPersonEquippedSkeletalMeshComponent() const
{
	return FirstPersonEquippedSkeletalMeshComponent;
}

UPNInteractionComponent* APNPlayerCharacter::GetInteractionComponent() const
{
	return InteractionComponent;
}

EPNAnimType APNPlayerCharacter::GetFirstPersonAnimType() const
{
	return FirstPersonAnimType;
}

void APNPlayerCharacter::SetFirstPersonArmsMesh(USkeletalMesh* NewArmsMesh)
{
	if (!HasAuthority())
	{
		return;
	}

	FirstPersonArmsMeshAsset = NewArmsMesh;
	ApplyFirstPersonArmsMesh();
}

void APNPlayerCharacter::SetFirstPersonAnimType(EPNAnimType NewAnimType)
{
	if (HasAuthority())
	{
		FirstPersonAnimType = NewAnimType;
		OnRep_FirstPersonAnimType();
		return;
	}

	Server_SetFirstPersonAnimType(NewAnimType);
}

void APNPlayerCharacter::ApplyFirstPersonArmsMesh()
{
	if (!FirstPersonArmsMeshComponent)
	{
		return;
	}

	if (FirstPersonArmsMeshAsset)
	{
		FirstPersonArmsMeshComponent->SetSkeletalMesh(FirstPersonArmsMeshAsset);
	}

	FirstPersonArmsMeshComponent->SetOnlyOwnerSee(true);
	FirstPersonArmsMeshComponent->SetOwnerNoSee(false);
	FirstPersonArmsMeshComponent->SetVisibility(true, true);
	FirstPersonArmsMeshComponent->SetHiddenInGame(false, true);
	FirstPersonArmsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonArmsMeshComponent->CastShadow = false;
	FirstPersonArmsMeshComponent->bCastDynamicShadow = false;

	FirstPersonArmsMeshComponent->SetRelativeLocation(FirstPersonArmsRelativeLocation);
	FirstPersonArmsMeshComponent->SetRelativeRotation(FirstPersonArmsRelativeRotation);

	ApplyFirstPersonArmsAnimClass();
}

void APNPlayerCharacter::ApplyFirstPersonArmsAnimClass()
{
	if (!FirstPersonArmsMeshComponent)
	{
		return;
	}

	if (FirstPersonArmsAnimClass)
	{
		FirstPersonArmsMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		FirstPersonArmsMeshComponent->SetAnimInstanceClass(FirstPersonArmsAnimClass);
		return;
	}

	if (FirstPersonArmsMeshComponent->GetAnimClass())
	{
		FirstPersonArmsMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}
}

void APNPlayerCharacter::StartInteractInput()
{
	if (IsDead())
	{
		return;
	}

	if (!InteractionComponent)
	{
		return;
	}

	InteractionComponent->RequestInteract();
}

void APNPlayerCharacter::QuickSlotInput_5()
{
	HandleQuickSlotInput(0);
}

void APNPlayerCharacter::QuickSlotInput_6()
{
	HandleQuickSlotInput(1);
}

void APNPlayerCharacter::QuickSlotInput_7()
{
	HandleQuickSlotInput(2);
}

void APNPlayerCharacter::QuickSlotInput_8()
{
	HandleQuickSlotInput(3);
}

void APNPlayerCharacter::QuickSlotInput_9()
{
	HandleQuickSlotInput(4);
}

void APNPlayerCharacter::QuickSlotInput_0()
{
	HandleQuickSlotInput(5);
}

void APNPlayerCharacter::HandleQuickSlotInput(int32 SlotIndex)
{
	if (IsDead())
	{
		return;
	}

	UPNQuickSlotComponent* QuickSlots = GetQuickSlotComponent();
	if (!QuickSlots)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;

	const bool bDoubleClick =
		LastQuickSlotPressIndex == SlotIndex &&
		CurrentTime - LastQuickSlotPressTime <= QuickSlotDoubleClickTime;

	LastQuickSlotPressIndex = SlotIndex;
	LastQuickSlotPressTime = CurrentTime;

	QuickSlots->RequestActivateQuickSlot(SlotIndex, bDoubleClick);
}

void APNPlayerCharacter::Server_SetFirstPersonAnimType_Implementation(EPNAnimType NewAnimType)
{
	SetFirstPersonAnimType(NewAnimType);
}

void APNPlayerCharacter::MoveForward(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	if (IsDead() || !Controller)
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	AddMovementInput(Direction, Value);
}

void APNPlayerCharacter::MoveRight(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	if (IsDead() || !Controller)
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Direction, Value);
}

void APNPlayerCharacter::Turn(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	AddControllerYawInput(Value);
}

void APNPlayerCharacter::LookUp(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	AddControllerPitchInput(Value);
}

void APNPlayerCharacter::StartJump()
{
	if (IsDead())
	{
		return;
	}

	Jump();
}

void APNPlayerCharacter::StopJump()
{
	StopJumping();
}

void APNPlayerCharacter::StartCrouchInput()
{
	if (IsDead())
	{
		return;
	}

	StopSprint();
	Crouch();
}

void APNPlayerCharacter::StopCrouchInput()
{
	UnCrouch();
}

void APNPlayerCharacter::RefreshFirstPersonVisibility()
{
	if (GetMesh())
	{
		GetMesh()->SetOwnerNoSee(true);
		GetMesh()->SetOnlyOwnerSee(false);
	}

	if (GetThirdPersonHeadMeshComponent())
	{
		GetThirdPersonHeadMeshComponent()->SetOwnerNoSee(true);
		GetThirdPersonHeadMeshComponent()->SetOnlyOwnerSee(false);
	}

	if (GetThirdPersonLegsMeshComponent())
	{
		GetThirdPersonLegsMeshComponent()->SetOwnerNoSee(true);
		GetThirdPersonLegsMeshComponent()->SetOnlyOwnerSee(false);
	}

	if (GetThirdPersonHandsMeshComponent())
	{
		GetThirdPersonHandsMeshComponent()->SetOwnerNoSee(true);
		GetThirdPersonHandsMeshComponent()->SetOnlyOwnerSee(false);
	}

	if (FirstPersonArmsMeshComponent)
	{
		FirstPersonArmsMeshComponent->SetOnlyOwnerSee(true);
		FirstPersonArmsMeshComponent->SetOwnerNoSee(false);
		FirstPersonArmsMeshComponent->SetVisibility(true, true);
		FirstPersonArmsMeshComponent->SetHiddenInGame(false, true);
	}
}

void APNPlayerCharacter::OnRep_FirstPersonArmsMesh()
{
	ApplyFirstPersonArmsMesh();
}

void APNPlayerCharacter::OnRep_FirstPersonAnimType()
{
	RefreshFirstPersonEquippedItemVisual();
}

void APNPlayerCharacter::HandleFirstPersonEquipmentChanged()
{
	if (HasAuthority())
	{
		RefreshFirstPersonAnimTypeFromEquipment();
	}

	RefreshFirstPersonEquippedItemVisual();
}

void APNPlayerCharacter::RefreshFirstPersonAnimTypeFromEquipment()
{
	if (!HasAuthority())
	{
		return;
	}

	const EPNAnimType NewAnimType = ResolveFirstPersonAnimTypeFromEquipment();

	if (FirstPersonAnimType == NewAnimType)
	{
		return;
	}

	FirstPersonAnimType = NewAnimType;
	OnRep_FirstPersonAnimType();
}

EPNAnimType APNPlayerCharacter::ResolveFirstPersonAnimTypeFromEquipment() const
{
	const UPNEquipmentComponent* PNEquipmentComponent = GetEquipmentComponent();
	if (!PNEquipmentComponent)
	{
		return EPNAnimType::Unarmed;
	}

	const TArray<EPNEquipmentSlot> WeaponSlots =
	{
		EPNEquipmentSlot::PrimaryWeapon1,
		EPNEquipmentSlot::PrimaryWeapon2,
		EPNEquipmentSlot::Sidearm,
		EPNEquipmentSlot::Knife
	};

	for (const EPNEquipmentSlot WeaponSlot : WeaponSlots)
	{
		UPNItemDataAsset* WeaponData = PNEquipmentComponent->GetEquippedItemData(WeaponSlot);
		if (!WeaponData || WeaponData->ItemType != EPNItemType::IT_Weapon)
		{
			continue;
		}

		if (WeaponData->WeaponStats.AnimType != EPNAnimType::None)
		{
			return WeaponData->WeaponStats.AnimType;
		}
	}

	return EPNAnimType::Unarmed;
}

void APNPlayerCharacter::RefreshFirstPersonEquippedItemVisual()
{
	if (!FirstPersonArmsMeshComponent)
	{
		ClearFirstPersonEquippedItemVisual();
		return;
	}

	UPNItemDataAsset* WeaponData = GetFirstPersonEquippedWeaponData();
	if (!WeaponData)
	{
		ClearFirstPersonEquippedItemVisual();
		return;
	}

	const FName AttachSocketName = ResolveFirstPersonWeaponAttachSocketName(WeaponData);

	if (FirstPersonEquippedStaticMeshComponent)
	{
		FirstPersonEquippedStaticMeshComponent->SetStaticMesh(nullptr);
		FirstPersonEquippedStaticMeshComponent->SetVisibility(false, true);
		FirstPersonEquippedStaticMeshComponent->SetHiddenInGame(true, true);
	}

	if (FirstPersonEquippedSkeletalMeshComponent)
	{
		FirstPersonEquippedSkeletalMeshComponent->SetSkeletalMesh(nullptr);
		FirstPersonEquippedSkeletalMeshComponent->SetVisibility(false, true);
		FirstPersonEquippedSkeletalMeshComponent->SetHiddenInGame(true, true);
	}

	if (!WeaponData->Visual.SkeletalMesh.IsNull())
	{
		USkeletalMesh* WeaponSkeletalMesh = WeaponData->Visual.SkeletalMesh.LoadSynchronous();
		if (WeaponSkeletalMesh && FirstPersonEquippedSkeletalMeshComponent)
		{
			FirstPersonEquippedSkeletalMeshComponent->AttachToComponent(
				FirstPersonArmsMeshComponent,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				AttachSocketName
			);

			FirstPersonEquippedSkeletalMeshComponent->SetSkeletalMesh(WeaponSkeletalMesh);
			FirstPersonEquippedSkeletalMeshComponent->SetRelativeLocation(FirstPersonWeaponRelativeLocation);
			FirstPersonEquippedSkeletalMeshComponent->SetRelativeRotation(FirstPersonWeaponRelativeRotation);
			FirstPersonEquippedSkeletalMeshComponent->SetRelativeScale3D(FirstPersonWeaponRelativeScale);
			FirstPersonEquippedSkeletalMeshComponent->SetVisibility(true, true);
			FirstPersonEquippedSkeletalMeshComponent->SetHiddenInGame(false, true);
		}

		return;
	}

	if (!WeaponData->Visual.StaticMesh.IsNull())
	{
		UStaticMesh* WeaponStaticMesh = WeaponData->Visual.StaticMesh.LoadSynchronous();
		if (WeaponStaticMesh && FirstPersonEquippedStaticMeshComponent)
		{
			FirstPersonEquippedStaticMeshComponent->AttachToComponent(
				FirstPersonArmsMeshComponent,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				AttachSocketName
			);

			FirstPersonEquippedStaticMeshComponent->SetStaticMesh(WeaponStaticMesh);
			FirstPersonEquippedStaticMeshComponent->SetRelativeLocation(FirstPersonWeaponRelativeLocation);
			FirstPersonEquippedStaticMeshComponent->SetRelativeRotation(FirstPersonWeaponRelativeRotation);
			FirstPersonEquippedStaticMeshComponent->SetRelativeScale3D(FirstPersonWeaponRelativeScale);
			FirstPersonEquippedStaticMeshComponent->SetVisibility(true, true);
			FirstPersonEquippedStaticMeshComponent->SetHiddenInGame(false, true);
		}

		return;
	}

	ClearFirstPersonEquippedItemVisual();
}

void APNPlayerCharacter::ClearFirstPersonEquippedItemVisual()
{
	if (FirstPersonEquippedStaticMeshComponent)
	{
		FirstPersonEquippedStaticMeshComponent->SetStaticMesh(nullptr);
		FirstPersonEquippedStaticMeshComponent->SetVisibility(false, true);
		FirstPersonEquippedStaticMeshComponent->SetHiddenInGame(true, true);
	}

	if (FirstPersonEquippedSkeletalMeshComponent)
	{
		FirstPersonEquippedSkeletalMeshComponent->SetSkeletalMesh(nullptr);
		FirstPersonEquippedSkeletalMeshComponent->SetVisibility(false, true);
		FirstPersonEquippedSkeletalMeshComponent->SetHiddenInGame(true, true);
	}
}

UPNItemDataAsset* APNPlayerCharacter::GetFirstPersonEquippedWeaponData() const
{
	const UPNEquipmentComponent* PNEquipmentComponent = GetEquipmentComponent();
	if (!PNEquipmentComponent)
	{
		return nullptr;
	}

	const TArray<EPNEquipmentSlot> WeaponSlots =
	{
		EPNEquipmentSlot::PrimaryWeapon1,
		EPNEquipmentSlot::PrimaryWeapon2,
		EPNEquipmentSlot::Sidearm,
		EPNEquipmentSlot::Knife
	};

	for (const EPNEquipmentSlot WeaponSlot : WeaponSlots)
	{
		UPNItemDataAsset* WeaponData = PNEquipmentComponent->GetEquippedItemData(WeaponSlot);
		if (!WeaponData || WeaponData->ItemType != EPNItemType::IT_Weapon)
		{
			continue;
		}

		if (WeaponData->WeaponStats.AnimType != EPNAnimType::None)
		{
			return WeaponData;
		}
	}

	return nullptr;
}

FName APNPlayerCharacter::ResolveFirstPersonWeaponAttachSocketName(UPNItemDataAsset* WeaponData) const
{
	if (WeaponData && !WeaponData->WeaponStats.HandSocketName.IsNone())
	{
		return WeaponData->WeaponStats.HandSocketName;
	}

	return DefaultFirstPersonWeaponSocketName;
}