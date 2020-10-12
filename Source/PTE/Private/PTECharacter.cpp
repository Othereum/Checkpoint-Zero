// © 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "PTECharacter.h"

APTECharacter::APTECharacter()
{
	PrimaryActorTick.bCanEverTick = true;

}

void APTECharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void APTECharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APTECharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &APTECharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &APTECharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APTECharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APTECharacter::LookUp);
}

void APTECharacter::MoveForward(float AxisValue)
{
	AddMovementInput(GetActorForwardVector(), AxisValue);
}

void APTECharacter::MoveRight(float AxisValue)
{
	AddMovementInput(GetActorRightVector(), AxisValue);
}

void APTECharacter::Turn(float AxisValue)
{
	AddControllerYawInput(AxisValue);
}

void APTECharacter::LookUp(float AxisValue)
{
	AddControllerPitchInput(AxisValue);
}

