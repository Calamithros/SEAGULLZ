// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectSprint.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// AfirstpCharacter

AfirstpCharacter::AfirstpCharacter()
{
	Health = 100.0f;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

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

	IsSprinting = false;

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	InitialStamina = 100.0f;
	CharacterStamina = InitialStamina;
	MultiplicationFactor = 2.5f;

	BaseSpeed = GetCharacterMovement()->MaxWalkSpeed;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AfirstpCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveForward", this, &AfirstpCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AfirstpCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AfirstpCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AfirstpCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AfirstpCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AfirstpCharacter::TouchStopped);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AfirstpCharacter::CharacterSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AfirstpCharacter::StopCharacterSprint);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AfirstpCharacter::OnResetVR);
}


void AfirstpCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AfirstpCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AfirstpCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AfirstpCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AfirstpCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
float AfirstpCharacter::GetCurrentHealth()
{
	return Health;
}
void AfirstpCharacter::SetHealth(float desiredValue)
{
	Health += desiredValue;
}

void AfirstpCharacter::MoveForward(float Value)
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

void AfirstpCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
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


void AfirstpCharacter::CharacterSprint() {

	//Change the speed so the character sprints
	GetCharacterMovement()->MaxWalkSpeed *= MultiplicationFactor;
	IsSprinting = true;

}

void AfirstpCharacter::StopCharacterSprint() {

	//Change the speed so the character STOPS sprinting
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	IsSprinting = false;

}

// Called every frame
void AfirstpCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//If the character is sprinting
	if (IsSprinting) {

		//Condition if the character is out of stamina
		if (GetCurrentStamina() <= 0) {

			//Force the character to stop sprinting
			StopCharacterSprint();

		}

		else {

			//Then decrease the stamina
			UpdateStamina(-0.1f);

		}
	}

	//If the character is not sprinting
	if (GetCurrentStamina() < 100 && !IsSprinting) {

		//then replenish the staminia over time
		UpdateStamina(0.25f);

	}
}




//Get Initial stamina of the character
float AfirstpCharacter::GetInitialStamina()
{


	return InitialStamina;

}

//Get the current stamina of the character
float AfirstpCharacter::GetCurrentStamina()
{

	return CharacterStamina;

}

void AfirstpCharacter::UpdateStamina(float StaminaUpdate)
{

	//Change the stamina level of the character
	CharacterStamina = GetCurrentStamina() + StaminaUpdate;

	//Making sure we do not go beyond the maximum stamina capacity
	if (GetCurrentStamina() > 100) {

		CharacterStamina = 100;

	}

	//Making sure we do not go below 0 on stamina
	if (GetCurrentStamina() < 0)
	{

		CharacterStamina = 0;

	}

}
