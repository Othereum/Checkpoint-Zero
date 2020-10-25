// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0Character.h"
#include "CP0CharacterMovement.h"
#include "CP0GameInstance.h"
#include "CP0InputSettings.h"

template <class UserClass, class ActionType>
void BindInputAction(UInputComponent* Input, FName ActionName, UserClass* Object, ActionType&& Action)
{
    FInputActionBinding Pressed{ActionName, IE_Pressed};
    Pressed.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(Object, [=, LastTime = -1.0f]() mutable {
        const auto GI = CastChecked<UCP0GameInstance>(Object->GetGameInstance());
        const auto Settings = GI->GetInputSettings();

        switch (Settings->PressTypes[ActionName])
        {
        case EPressType::Press:
            Action.Toggle(Object);
            break;

        case EPressType::Continuous:
            Action.Enable(Object);
            break;

        case EPressType::DoubleClick: {
            const auto CurTime = Object->GetGameTimeSinceCreation();
            if (CurTime - LastTime <= Settings->DoubleClickTimeout)
            {
                Action.Toggle(Object);
                LastTime = -1.0f;
            }
            else
            {
                LastTime = CurTime;
            }
            break;
        }
        }
    });
    Input->AddActionBinding(MoveTemp(Pressed));

    FInputActionBinding Released{ActionName, IE_Released};
    Released.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(Object, [=] {
        const auto GI = CastChecked<UCP0GameInstance>(Object->GetGameInstance());
        const auto Settings = GI->GetInputSettings();

        switch (Settings->PressTypes[ActionName])
        {
        case EPressType::Release:
            Action.Toggle(Object);
            break;

        case EPressType::Continuous:
            Action.Disable(Object);
            break;
        }
    });
    Input->AddActionBinding(MoveTemp(Released));
}

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

void ACP0Character::SprintEnable_Implementation()
{
    GetCP0CharacterMovement()->EnableSprint();
}

void ACP0Character::SprintDisable_Implementation()
{
    GetCP0CharacterMovement()->DisableSprint();
}

void ACP0Character::SprintToggle_Implementation()
{
    GetCP0CharacterMovement()->ToggleSprint();
}

bool ACP0Character::SprintEnable_Validation()
{
    return true;
}

bool ACP0Character::SprintDisable_Validation()
{
    return true;
}

bool ACP0Character::SprintToggle_Validation()
{
    return true;
}
