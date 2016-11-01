// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UDPServer.h"
#include "UDPServerCharacter.h"
#include "UDPServerProjectile.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AUDPServerCharacter

AUDPServerCharacter::AUDPServerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 30.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P are set in the
	// derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	this->SetActorTickEnabled(true);
	servidorsito = CreateDefaultSubobject<AUDPReceiver>(TEXT("servidor"));;
	prevX = 0;
	prevY = 0;
	prevYaw = 0;
	prevPitch = 0;
}

void AUDPServerCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "BEGIN PLAY CHARACTER");
	servidorsito->StartServer();
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint")); //Attach gun mesh component to Skeleton, doing it here because the skelton is not yet created in the constructor
}
void AUDPServerCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,"Tick");
	servidorsito->SendData();
	servidorsito->ReceiveData();
	
}
//////////////////////////////////////////////////////////////////////////
// Input

void AUDPServerCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AUDPServerCharacter::TouchStarted);
	if (EnableTouchscreenMovement(InputComponent) == false)
	{
		InputComponent->BindAction("Fire", IE_Pressed, this, &AUDPServerCharacter::OnFire);
	}

	InputComponent->BindAxis("MoveForward", this, &AUDPServerCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AUDPServerCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AUDPServerCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AUDPServerCharacter::LookUpAtRate);
}

void AUDPServerCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		const FRotator SpawnRotation = GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile at the muzzle
			World->SpawnActor<AUDPServerProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

}

void AUDPServerCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AUDPServerCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = false;
}

void AUDPServerCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
	{
		if (TouchItem.bIsPressed)
		{
			if (GetWorld() != nullptr)
			{
				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
				if (ViewportClient != nullptr)
				{
					FVector MoveDelta = Location - TouchItem.Location;
					FVector2D ScreenSize;
					ViewportClient->GetViewportSize(ScreenSize);
					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.X * BaseTurnRate;
						AddControllerYawInput(Value);
					}
					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.Y * BaseTurnRate;
						AddControllerPitchInput(Value);
					}
					TouchItem.Location = Location;
				}
				TouchItem.Location = Location;
			}
		}
	}
}

void AUDPServerCharacter::MoveForward(float Value)
{
	float delta =fabsf(servidorsito->x - prevX);
	if (delta >1 && servidorsito->x > 0 ) {
		AddMovementInput(GetActorForwardVector(), 1);
	}
	if (delta > 1 && servidorsito->x < 0) {
		AddMovementInput(GetActorForwardVector(), -1);
	}
	prevX = servidorsito->x;
	
	
}

void AUDPServerCharacter::MoveRight(float Value)
{
	float delta = fabsf(servidorsito->y - prevY);
	if (delta > 1 && servidorsito->y > 0) {
		AddMovementInput(GetActorRightVector(), 1);
	}
	if (delta > 1 && servidorsito->y < 0) {
		AddMovementInput(GetActorRightVector(), -1);
	}
	prevY = servidorsito->y;
}

void AUDPServerCharacter::TurnAtRate(float Rate)
{
	/*Rads to Degrees*/
	float degrees = (servidorsito->yawRate * 180) / PI;
	degrees /= 180;
	float delta = fabsf(degrees - prevYaw);
	if (delta > 1 && degrees > 0) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,"ENTREE");
		AddControllerYawInput(servidorsito->yawRate* BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
	if (delta > 1 && degrees < 0) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "EMTRE");
		AddControllerYawInput(servidorsito->yawRate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::SanitizeFloat(degrees));
	//float delta = Rate - yawActual;
	prevYaw = GetActorRotation().Yaw;
	
	
}

void AUDPServerCharacter::LookUpAtRate(float Rate)
{
	float degrees = (servidorsito->pitchRate * 180) / PI;
	degrees /= 180;
	float delta = fabsf(degrees - prevPitch);
	if (delta > 1 && degrees > 0) {
		AddControllerPitchInput(servidorsito->pitchRate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
	if (delta > 1 && degrees < 0) {
		AddControllerPitchInput(servidorsito->pitchRate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
	// calculate delta for this frame from the rate information
	prevPitch = GetActorRotation().Pitch;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::SanitizeFloat(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()));
}

bool AUDPServerCharacter::EnableTouchscreenMovement(class UInputComponent* InputComponent)
{
	bool bResult = false;
	if (FPlatformMisc::GetUseVirtualJoysticks() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		bResult = true;
		InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AUDPServerCharacter::BeginTouch);
		InputComponent->BindTouch(EInputEvent::IE_Released, this, &AUDPServerCharacter::EndTouch);
		InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AUDPServerCharacter::TouchUpdate);
	}
	return bResult;
}
