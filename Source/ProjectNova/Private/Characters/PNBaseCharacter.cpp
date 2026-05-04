#include "Characters/PNBaseCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/PNInventoryComponent.h"
#include "Equipment/PNEquipmentComponent.h"
#include "Net/UnrealNetwork.h"

APNBaseCharacter::APNBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	InventoryComponent = CreateDefaultSubobject<UPNInventoryComponent>(TEXT("InventoryComponent"));
	if (InventoryComponent)
	{
		InventoryComponent->SetIsReplicated(true);
	}

	EquipmentComponent = CreateDefaultSubobject<UPNEquipmentComponent>(TEXT("EquipmentComponent"));
	if (EquipmentComponent)
	{
		EquipmentComponent->SetIsReplicated(true);
	}

	GetMesh()->SetIsReplicated(true);
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetOnlyOwnerSee(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ThirdPersonHeadMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonHeadMeshComponent"));
	ThirdPersonHeadMeshComponent->SetupAttachment(GetMesh());
	ThirdPersonHeadMeshComponent->SetIsReplicated(true);
	ThirdPersonHeadMeshComponent->SetOwnerNoSee(true);
	ThirdPersonHeadMeshComponent->SetOnlyOwnerSee(false);
	ThirdPersonHeadMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ThirdPersonLegsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonLegsMeshComponent"));
	ThirdPersonLegsMeshComponent->SetupAttachment(GetMesh());
	ThirdPersonLegsMeshComponent->SetIsReplicated(true);
	ThirdPersonLegsMeshComponent->SetOwnerNoSee(true);
	ThirdPersonLegsMeshComponent->SetOnlyOwnerSee(false);
	ThirdPersonLegsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ThirdPersonHandsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonHandsMeshComponent"));
	ThirdPersonHandsMeshComponent->SetupAttachment(GetMesh());
	ThirdPersonHandsMeshComponent->SetIsReplicated(true);
	ThirdPersonHandsMeshComponent->SetOwnerNoSee(true);
	ThirdPersonHandsMeshComponent->SetOnlyOwnerSee(false);
	ThirdPersonHandsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

void APNBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetupThirdPersonMeshVisibility();
	ApplyModularCharacterMeshes();
	ApplyMovementSpeed();
}

void APNBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APNBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APNBaseCharacter, ThirdPersonMasterBodyMeshAsset);
	DOREPLIFETIME(APNBaseCharacter, ThirdPersonHeadMeshAsset);
	DOREPLIFETIME(APNBaseCharacter, ThirdPersonLegsMeshAsset);
	DOREPLIFETIME(APNBaseCharacter, ThirdPersonHandsMeshAsset);
	DOREPLIFETIME(APNBaseCharacter, bIsSprinting);
	DOREPLIFETIME(APNBaseCharacter, bIsDead);
}

float APNBaseCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!HasAuthority())
	{
		return AppliedDamage;
	}

	if (bIsDead)
	{
		return AppliedDamage;
	}

	if (AppliedDamage <= 0.0f)
	{
		return AppliedDamage;
	}

	// Временно. Нормальное здоровье будет позже в UPNHealthComponent.
	if (AppliedDamage >= 99999.0f)
	{
		Die(EventInstigator);
	}

	return AppliedDamage;
}

UPNInventoryComponent* APNBaseCharacter::GetInventoryComponent() const
{
	return InventoryComponent;
}

UPNEquipmentComponent* APNBaseCharacter::GetEquipmentComponent() const
{
	return EquipmentComponent;
}

USkeletalMeshComponent* APNBaseCharacter::GetThirdPersonHeadMeshComponent() const
{
	return ThirdPersonHeadMeshComponent;
}

USkeletalMeshComponent* APNBaseCharacter::GetThirdPersonLegsMeshComponent() const
{
	return ThirdPersonLegsMeshComponent;
}

USkeletalMeshComponent* APNBaseCharacter::GetThirdPersonHandsMeshComponent() const
{
	return ThirdPersonHandsMeshComponent;
}

void APNBaseCharacter::SetModularCharacterMeshes(
	USkeletalMesh* NewMasterBodyMesh,
	USkeletalMesh* NewHeadMesh,
	USkeletalMesh* NewLegsMesh,
	USkeletalMesh* NewHandsMesh
)
{
	if (!HasAuthority())
	{
		return;
	}

	ThirdPersonMasterBodyMeshAsset = NewMasterBodyMesh;
	ThirdPersonHeadMeshAsset = NewHeadMesh;
	ThirdPersonLegsMeshAsset = NewLegsMesh;
	ThirdPersonHandsMeshAsset = NewHandsMesh;

	ApplyModularCharacterMeshes();
}

void APNBaseCharacter::ApplyModularCharacterMeshes()
{
	if (GetMesh())
	{
		GetMesh()->SetSkeletalMesh(ThirdPersonMasterBodyMeshAsset);
	}

	if (ThirdPersonHeadMeshComponent)
	{
		ThirdPersonHeadMeshComponent->SetSkeletalMesh(ThirdPersonHeadMeshAsset);
		ThirdPersonHeadMeshComponent->SetLeaderPoseComponent(GetMesh());
	}

	if (ThirdPersonLegsMeshComponent)
	{
		ThirdPersonLegsMeshComponent->SetSkeletalMesh(ThirdPersonLegsMeshAsset);
		ThirdPersonLegsMeshComponent->SetLeaderPoseComponent(GetMesh());
	}

	if (ThirdPersonHandsMeshComponent)
	{
		ThirdPersonHandsMeshComponent->SetSkeletalMesh(ThirdPersonHandsMeshAsset);
		ThirdPersonHandsMeshComponent->SetLeaderPoseComponent(GetMesh());
	}

	SetupThirdPersonMeshVisibility();
}

bool APNBaseCharacter::IsSprinting() const
{
	return bIsSprinting;
}

bool APNBaseCharacter::IsDead() const
{
	return bIsDead;
}

void APNBaseCharacter::StartSprint()
{
	if (bIsDead || bIsCrouched)
	{
		return;
	}

	if (HasAuthority())
	{
		SetSprintingInternal(true);
		return;
	}

	Server_SetSprinting(true);
}

void APNBaseCharacter::StopSprint()
{
	if (HasAuthority())
	{
		SetSprintingInternal(false);
		return;
	}

	Server_SetSprinting(false);
}

void APNBaseCharacter::Die(AController* KillerController)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	bIsSprinting = false;

	ApplyMovementSpeed();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	OnRep_IsDead();
}

void APNBaseCharacter::Revive()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsDead = false;
	bIsSprinting = false;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	ApplyMovementSpeed();

	OnRep_IsDead();
}

void APNBaseCharacter::Server_SetSprinting_Implementation(bool bNewSprinting)
{
	SetSprintingInternal(bNewSprinting);
}

void APNBaseCharacter::SetSprintingInternal(bool bNewSprinting)
{
	if (bIsDead || bIsCrouched)
	{
		bNewSprinting = false;
	}

	if (bIsSprinting == bNewSprinting)
	{
		return;
	}

	bIsSprinting = bNewSprinting;
	ApplyMovementSpeed();

	OnRep_IsSprinting();
}

void APNBaseCharacter::ApplyMovementSpeed()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	if (bIsDead)
	{
		GetCharacterMovement()->MaxWalkSpeed = 0.0f;
		GetCharacterMovement()->MaxWalkSpeedCrouched = 0.0f;
		return;
	}

	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

	if (bIsSprinting && !bIsCrouched)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APNBaseCharacter::SetupThirdPersonMeshVisibility()
{
	if (GetMesh())
	{
		GetMesh()->SetOwnerNoSee(true);
		GetMesh()->SetOnlyOwnerSee(false);
	}

	if (ThirdPersonHeadMeshComponent)
	{
		ThirdPersonHeadMeshComponent->SetOwnerNoSee(true);
		ThirdPersonHeadMeshComponent->SetOnlyOwnerSee(false);
	}

	if (ThirdPersonLegsMeshComponent)
	{
		ThirdPersonLegsMeshComponent->SetOwnerNoSee(true);
		ThirdPersonLegsMeshComponent->SetOnlyOwnerSee(false);
	}

	if (ThirdPersonHandsMeshComponent)
	{
		ThirdPersonHandsMeshComponent->SetOwnerNoSee(true);
		ThirdPersonHandsMeshComponent->SetOnlyOwnerSee(false);
	}
}

void APNBaseCharacter::OnRep_ModularMeshes()
{
	ApplyModularCharacterMeshes();
}

void APNBaseCharacter::OnRep_IsSprinting()
{
	ApplyMovementSpeed();
}

void APNBaseCharacter::OnRep_IsDead()
{
	ApplyMovementSpeed();
}