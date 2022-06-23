// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StreamLineTestCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

// Declaration of the delegate that will be called when the Primary Action is triggered
// It is declared as dynamic so it can be accessed also in Blueprints
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUseItem);

UCLASS(config=Game)
class AStreamLineTestCharacter : public ACharacter
{
	GENERATED_BODY()
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* FP_Gun;
	
	UPROPERTY(EditAnywhere, Category= "Components")
	class USceneComponent* GrabbedObjectLocation;
	
	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;


	
public:
	AStreamLineTestCharacter();


	
protected:
	virtual void BeginPlay();

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float TurnRateGamepad;

	/** Delegate to whom anyone can subscribe to receive this event */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnUseItem OnUseItem;
protected:
	
	/** Fires a projectile. */
	void OnFire();

	void EndFire();

	void SetGrabbedObject(UPrimitiveComponent* ObjectToGrab);

	UPROPERTY()
	UPrimitiveComponent* GrabbedObject;
	
	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);


	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface



public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	class UCharacterMovementComponent* CharacterMovement = GetCharacterMovement();
	
	virtual void Tick(float DeltaTime) override;

	 FVector StartLocation;

private:
	void DashEnables();

	void Dash();
	
	void CanDash();
	
	bool IsMoving();

	void JetPack(float Value);

	bool IsDashed = false;

	bool CanBeDashed = true;
	
	FTimerHandle DashTimerHandle;

	float WalkSpeed;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Dash" , meta = (ClampMin = "0",ClampMax = "10000.0"))
	float DashSpeed = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Dash" , meta = (ClampMin = "0.0",ClampMax = "1000.0"))
	float DashDistance = 500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Dash" , meta = (ClampMin = "1.0",ClampMax = "100.0"))
	float DashCooldown = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "GravGun" , meta = (ClampMin = "1.0",ClampMax = "10000.0"))
	float FiringForce = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "GravGun" , meta = (ClampMin = "1.0",ClampMax = "10000.0"))
	float PickUpRadius = 4000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Jetpack" , meta = (ClampMin = "1.0",ClampMax = "10000.0"))
	float MaxJetSpeed = 500.0f;
}	;

