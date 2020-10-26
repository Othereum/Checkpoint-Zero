// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0Character.h"
#include "CP0Character.inl"
#include "CP0CharacterMovement.h"
#include "CP0GameInstance.h"
#include "CP0InputSettings.h"

ACP0Character::ACP0Character(const FObjectInitializer& initializer)
    : Super{initializer.SetDefaultSubobjectClass<UCP0CharacterMovement>(ACharacter::CharacterMovementComponentName)}
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

void ACP0Character::Tick(float deltaTime)
{
    Super::Tick(deltaTime);
}

void ACP0Character::SetupPlayerInputComponent(UInputComponent* input)
{
    Super::SetupPlayerInputComponent(input);

    input->BindAxis(TEXT("MoveForward"), this, &ACP0Character::MoveForward);
    input->BindAxis(TEXT("MoveRight"), this, &ACP0Character::MoveRight);
    input->BindAxis(TEXT("Turn"), this, &ACP0Character::Turn);
    input->BindAxis(TEXT("LookUp"), this, &ACP0Character::LookUp);

    BindInputAction<FSprintAction>(input, TEXT("Sprint"));
}

void ACP0Character::MoveForward(float axisValue)
{
    AddMovementInput(GetActorForwardVector(), axisValue);
}

void ACP0Character::MoveRight(float axisValue)
{
    AddMovementInput(GetActorRightVector(), axisValue);
}

void ACP0Character::Turn(float axisValue)
{
    AddControllerYawInput(axisValue);
}

void ACP0Character::LookUp(float axisValue)
{
    AddControllerPitchInput(axisValue);
}

void ACP0Character::DispatchInputAction(FName name, EInputAction type)
{
    const auto dispatcher = inputActionMap_.Find(name);
    if (ensure(dispatcher))
    {
        (*dispatcher)(this, type);
    }

    if (IsLocallyControlled())
        ServerInputAction(name, type);
}

void ACP0Character::ServerInputAction_Implementation(FName name, EInputAction type)
{
    DispatchInputAction(name, type);
}

bool ACP0Character::ServerInputAction_Validate(FName Name, EInputAction Type)
{
    return true;
}
