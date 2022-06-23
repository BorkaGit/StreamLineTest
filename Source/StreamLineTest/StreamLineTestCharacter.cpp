// Copyright Epic Games, Inc. All Rights Reserved.

#include "StreamLineTestCharacter.h"
#include "StreamLineTestProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/InputSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogDash, All, All)

AStreamLineTestCharacter::AStreamLineTestCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	
	PrimaryActorTick.bCanEverTick = true;
	
	// set our turn rates for input
	TurnRateGamepad = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(RootComponent);

	GrabbedObjectLocation = CreateDefaultSubobject<USceneComponent>(TEXT("GrabbedObjectLocation"));
	GrabbedObjectLocation->SetupAttachment(FP_Gun);
}

void AStreamLineTestCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	FP_Gun->AttachToComponent(Mesh1P,FAttachmentTransformRules(EAttachmentRule::SnapToTarget,true),TEXT("GripPoint"));
}

//////////////////////////////////////////////////////////////////////////// Input

void AStreamLineTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind dash
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AStreamLineTestCharacter::DashEnables);
	
	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AStreamLineTestCharacter::OnFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AStreamLineTestCharacter::EndFire);
	
	//Bind JetPack
	PlayerInputComponent->BindAxis("JetPack",this,&AStreamLineTestCharacter::JetPack);
	
	// Bind movement events
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AStreamLineTestCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AStreamLineTestCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "Mouse" versions handle devices that provide an absolute delta, such as a mouse.
	// "Gamepad" versions are for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AStreamLineTestCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AStreamLineTestCharacter::LookUpAtRate);
}

void AStreamLineTestCharacter::OnFire()
{
		const FCollisionQueryParams QueryParams("GravityGunTrace",false,this);
		const FVector StartTrace = GetFirstPersonCameraComponent()->GetComponentLocation();
		const FVector EndTrace = (GetFirstPersonCameraComponent()->GetForwardVector() * PickUpRadius) +	StartTrace;
		FHitResult HitResult;

		if(GetWorld()->LineTraceSingleByChannel(HitResult,StartTrace,EndTrace,ECC_Visibility,QueryParams))
		{
			if(UPrimitiveComponent* Primitive = HitResult.GetComponent())
			{
				SetGrabbedObject(Primitive);
			}
		}
}

void AStreamLineTestCharacter::EndFire()
{
	if( GrabbedObject)
	{
		const FVector ShootVelocity = GetFirstPersonCameraComponent()->GetForwardVector() * FiringForce;

		GrabbedObject->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GrabbedObject->SetSimulatePhysics(true);
		GrabbedObject->AddImpulse(ShootVelocity,NAME_None,true);

		SetGrabbedObject(nullptr);
	}
}

void AStreamLineTestCharacter::SetGrabbedObject(UPrimitiveComponent* ObjectToGrab)
{
	GrabbedObject = ObjectToGrab;

	if(GrabbedObject)
	{
		GrabbedObject->SetSimulatePhysics(false);
		GrabbedObject->AttachToComponent(GrabbedObjectLocation, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}


void AStreamLineTestCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AStreamLineTestCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AStreamLineTestCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AStreamLineTestCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AStreamLineTestCharacter::DashEnables()
{
	
	if(!IsMoving() || !CanBeDashed) return;
	IsDashed = true;
	CanBeDashed = false;
	
	GetWorld()->GetTimerManager().SetTimer(DashTimerHandle, this, &AStreamLineTestCharacter::CanDash,DashCooldown,false);
	StartLocation = GetActorLocation();
}

bool AStreamLineTestCharacter::IsMoving()
{
	return !GetVelocity().IsZero();
}

void AStreamLineTestCharacter::JetPack(float Value)
{
	if (Value != 0.0f)
	{
		FVector JetUpVector = FVector::ZAxisVector * MaxJetSpeed;
		LaunchCharacter(JetUpVector * Value,false, true);
	}
}

void AStreamLineTestCharacter::Dash()
{
	if(!IsDashed) return;
	
	FVector	CurrentLocation = GetActorLocation();
	auto const CurrentDistance = FVector::Distance(CurrentLocation,StartLocation);
	UE_LOG(LogDash,Warning,TEXT("Current Distance: %.0f "),CurrentDistance);
	
	if(CurrentDistance < DashDistance)
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * DashSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		IsDashed = false;
	}
}

void AStreamLineTestCharacter::CanDash()
{
	CanBeDashed =  true;
}

void AStreamLineTestCharacter::Tick(float DeltaTime)
{
	Super::Tick (DeltaTime);
	Dash();
}

