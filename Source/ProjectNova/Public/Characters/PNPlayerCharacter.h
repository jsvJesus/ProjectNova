#pragma once

#include "CoreMinimal.h"
#include "PNBaseCharacter.h"
#include "PNPlayerCharacter.generated.h"

class UCameraComponent;
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

	UPROPERTY(ReplicatedUsing = OnRep_FirstPersonArmsMesh, EditAnywhere, BlueprintReadOnly, Category = "First Person")
	TObjectPtr<USkeletalMesh> FirstPersonArmsMeshAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "First Person")
	FVector FirstPersonArmsRelativeLocation = FVector(0.0f, 0.0f, -150.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "First Person")
	FRotator FirstPersonArmsRelativeRotation = FRotator(0.0f, 0.0f, 0.0f);

public:
	UFUNCTION(BlueprintPure, Category = "First Person")
	UCameraComponent* GetFirstPersonCameraComponent() const;

	UFUNCTION(BlueprintPure, Category = "First Person")
	USkeletalMeshComponent* GetFirstPersonArmsMeshComponent() const;

	UFUNCTION(BlueprintCallable, Category = "First Person")
	void SetFirstPersonArmsMesh(USkeletalMesh* NewArmsMesh);

	UFUNCTION(BlueprintCallable, Category = "First Person")
	void ApplyFirstPersonArmsMesh();

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
};