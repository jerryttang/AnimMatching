// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AnimTestCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "CharacterAnimationComponent.h"

//////////////////////////////////////////////////////////////////////////
// AAnimTestCharacter

AAnimTestCharacter::AAnimTestCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AAnimTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AAnimTestCharacter::ToggleCrouch);
	PlayerInputComponent->BindAction("Prone", IE_Pressed, this, &AAnimTestCharacter::ToggleProne);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AAnimTestCharacter::BeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AAnimTestCharacter::EndFire);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AAnimTestCharacter::BeginAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AAnimTestCharacter::EndAim);

	PlayerInputComponent->BindAxis("MoveForward", this, &AAnimTestCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AAnimTestCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AAnimTestCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AAnimTestCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AAnimTestCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AAnimTestCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AAnimTestCharacter::OnResetVR);
}


void AAnimTestCharacter::BeginPlay()
{
	Super::BeginPlay();
}


void AAnimTestCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	if (AnimComponentClass)
	{
		AnimComp = NewObject<UCharacterAnimationComponent>(this, AnimComponentClass);
		AnimComp->RegisterComponent();
		AddInstanceComponent(AnimComp);
	}
}

void AAnimTestCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AAnimTestCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AAnimTestCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}


void AAnimTestCharacter::ToggleProne()
{
	if (bIsProne)
	{
		bIsProne = false;
	}
	else
	{
		bIsProne = true;
	}
}

void AAnimTestCharacter::ToggleCrouch()
{
	if (bIsCrouch)
	{
		bIsCrouch = false;
	}
	else
	{
		bIsCrouch = true;
	}
}


void AAnimTestCharacter::BeginFire()
{
	bIsFiring = true;
}

void AAnimTestCharacter::EndFire()
{
	bIsFiring = false;
}

void AAnimTestCharacter::BeginAim()
{
	bIsAiming = true;
}

void AAnimTestCharacter::EndAim()
{
	bIsAiming = false;
}

void AAnimTestCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AAnimTestCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AAnimTestCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AAnimTestCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
