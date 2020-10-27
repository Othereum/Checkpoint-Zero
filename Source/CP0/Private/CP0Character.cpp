// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0Character.h"
#include "CP0CharacterInput.inl"
#include "CP0CharacterMovement.h"
#include "Net/UnrealNetwork.h"

ACP0Character::ACP0Character(const FObjectInitializer& Initializer)
    : Super{Initializer.SetDefaultSubobjectClass<UCP0CharacterMovement>(ACharacter::CharacterMovementComponentName)}
{
    PrimaryActorTick.bCanEverTick = true;
    RegisterInputActions();
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

    BindInputAction(Input, TEXT("Sprint"));
}

void ACP0Character::RegisterInputActions()
{
    RegisterInputAction<FSprintAction>(TEXT("Sprint"));
}

void ACP0Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCP0CharacterMovement* ACP0Character::GetCP0Movement() const
{
    return CastChecked<UCP0CharacterMovement>(GetCharacterMovement());
}
