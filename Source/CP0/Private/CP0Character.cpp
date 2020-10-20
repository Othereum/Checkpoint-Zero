// © 2020 Seokjin Lee <seokjin.dev@gmail.com>

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
            float CurTime;

        case EPressType::Press:
            Action.Toggle(Object);
            break;

        case EPressType::Continuous:
            Action.Enable(Object);
            break;

        case EPressType::DoubleClick:
            CurTime = Object->GetGameTimeSinceCreation();
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

struct FSprintAction
{
    void Enable(ACP0Character* Character) const
    {
    }

    void Disable(ACP0Character* Character) const
    {
    }

    void Toggle(ACP0Character* Character) const
    {
    }
};

void ACP0Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACP0Character::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACP0Character::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ACP0Character::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ACP0Character::LookUp);

    BindInputAction(PlayerInputComponent, TEXT("Sprint"), this, FSprintAction{});
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
