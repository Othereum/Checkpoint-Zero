// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0Character.h"
#include "CP0CharacterMovement.h"
#include "CP0GameInstance.h"
#include "CP0InputSettings.h"

ACP0Character::ACP0Character(const FObjectInitializer& ObjectInitializer)
    : Super(
          ObjectInitializer.SetDefaultSubobjectClass<UCP0CharacterMovement>(ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;
}

void ACP0Character::BeginPlay()
{
    Super::BeginPlay();
}

void ACP0Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACP0Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACP0Character::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACP0Character::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ACP0Character::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ACP0Character::LookUp);

    GetCP0CharacterMovement()->SetupPlayerInputComponent(PlayerInputComponent);
}

void ACP0Character::MoveForward(float AxisValue)
{
    AddMovementInput(GetActorForwardVector(), AxisValue);
}

void ACP0Character::MoveRight(float AxisValue)
{
    AddMovementInput(GetActorRightVector(), AxisValue);
}

void ACP0Character::Turn(float AxisValue)
{
    AddControllerYawInput(AxisValue);
}

void ACP0Character::LookUp(float AxisValue)
{
    AddControllerPitchInput(AxisValue);
}
