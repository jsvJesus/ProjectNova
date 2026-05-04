#pragma once

#include "CoreMinimal.h"
#include "PNBaseCharacter.h"
#include "Items/PNItemTypes.h"
#include "PNPlayerCharacter.generated.h"

class UAnimInstance;
class UCameraComponent;
class UPNInteractionComponent;
class USkeletalMesh;
class USkeletalMeshComponent;

UCLASS()
class PROJECTNOVA_API APNPlayerCharacter : public APNBaseCharacter
{
	GENERATED_BODY()

public:
	APNPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "First Person")
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "First Person")
	TObjectPtr<USkeletalMeshComponent> FirstPersonArmsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UPNInteractionComponent> InteractionComponent;

	UPROPERTY(ReplicatedUsing = OnRep_FirstPersonArmsMesh, EditAnywhere, BlueprintReadOnly, Category = "First Person")
	TObjectPtr<USkeletalMesh> FirstPersonArmsMeshAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "First Person")
	TSubclassOf<UAnimInstance> FirstPersonArmsAnimClass;

	UPROPERTY(ReplicatedUsing = OnRep_FirstPersonAnimType, EditAnywhere, BlueprintReadOnly, Category = "First Person")
	EPNAnimType FirstPersonAnimType = EPNAnimType::Unarmed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "First Person")
	FVector FirstPersonArmsRelativeLocation = FVector(0.0f, 0.0f, -150.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "First Person")
	FRotator FirstPersonArmsRelativeRotation = FRotator(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quick Slots", meta = (ClampMin = "0.05"))
	float QuickSlotDoubleClickTime = 0.25f;

	float LastQuickSlotPressTime = -1000.0f;

	int32 LastQuickSlotPressIndex = INDEX_NONE;

public:
	UFUNCTION(BlueprintPure, Category = "First Person")
	UCameraComponent* GetFirstPersonCameraComponent() const;

	UFUNCTION(BlueprintPure, Category = "First Person")
	USkeletalMeshComponent* GetFirstPersonArmsMeshComponent() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UPNInteractionComponent* GetInteractionComponent() const;

	UFUNCTION(BlueprintPure, Category = "First Person")
	EPNAnimType GetFirstPersonAnimType() const;

	UFUNCTION(BlueprintCallable, Category = "First Person")
	void SetFirstPersonArmsMesh(USkeletalMesh* NewArmsMesh);

	UFUNCTION(BlueprintCallable, Category = "First Person")
	void SetFirstPersonAnimType(EPNAnimType NewAnimType);

	UFUNCTION(BlueprintCallable, Category = "First Person")
	void ApplyFirstPersonArmsMesh();

	UFUNCTION(BlueprintCallable, Category = "First Person")
	void ApplyFirstPersonArmsAnimClass();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StartInteractInput();

	UFUNCTION(Server, Reliable)
	void Server_SetFirstPersonAnimType(EPNAnimType NewAnimType);

protected:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	void StartJump();
	void StopJump();

	void StartCrouchInput();
	void StopCrouchInput();

	void RefreshFirstPersonVisibility();

	UFUNCTION()
	void OnRep_FirstPersonArmsMesh();

	UFUNCTION()
	void OnRep_FirstPersonAnimType();

	void QuickSlotInput_5();
	void QuickSlotInput_6();
	void QuickSlotInput_7();
	void QuickSlotInput_8();
	void QuickSlotInput_9();
	void QuickSlotInput_0();

	void HandleQuickSlotInput(int32 SlotIndex);
};