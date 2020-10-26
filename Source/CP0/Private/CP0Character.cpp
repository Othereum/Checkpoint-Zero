// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0Character.h"
#include "CP0Character.inl"
#include "CP0CharacterMovement.h"
#include "CP0GameInstance.h"
#include "CP0InputSettings.h"

ACP0Character::ACP0Character(const FObjectInitializer& Initializer)
    : Super{Initializer.SetDefaultSubobjectClass<UCP0CharacterMovement>(ACharacter::CharacterMovementComponentName)}
{
    PrimaryActorTick.bCanEverTick = true;
}

UCP0CharacterMovement* ACP0Character::GetCP0Movement() const
{
    return CastChecked<UCP0CharacterMovement>(GetCharacterMovement());
}

void ACP0Character::BeginPlay()
{
    Super::BeginPlay();
}

void ACP0Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACP0Character::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);

    Input->BindAxis(TEXT("MoveForward"), this, &ACP0Character::MoveForward);
    Input->BindAxis(TEXT("MoveRight"), this, &ACP0Character::MoveRight);
    Input->BindAxis(TEXT("Turn"), this, &ACP0Character::Turn);
    Input->BindAxis(TEXT("LookUp"), this, &ACP0Character::LookUp);

    BindInputAction<FSprintAction>(Input, TEXT("Sprint"));
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

void ACP0Character::DispatchInputAction(FName Name, EInputAction Type)
{
    const auto dispatcher = InputActionMap.Find(Name);
    if (ensure(dispatcher))
    {
        (*dispatcher)(this, Type);
    }

    if (IsLocallyControlled())
        ServerInputAction(Name, Type);
}

void ACP0Character::ServerInputAction_Implementation(FName Name, EInputAction Type)
{
    DispatchInputAction(Name, Type);
}

bool ACP0Character::ServerInputAction_Validate(FName Name, EInputAction Type)
{
    return true;
}
