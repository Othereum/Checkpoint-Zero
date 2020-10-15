// © 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0Character.h"
#include "CP0CharacterMovement.h"
#include "CP0GameInstance.h"
#include "CP0InputSettings.h"

template <class UserClass, class FunctorType>
void BindInputAction(UInputComponent* Input, FName Action, UserClass* Object, FunctorType&& Functor)
{
    FInputActionBinding Pressed{Action, IE_Pressed};
    Pressed.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(Object, [=, LastTime = -1.0f] mutable {
        const auto GI = CastChecked<UCP0GameInstance>(Object->GetGameInstance());
        const auto Settings = GI->GetInputSettings();

        switch (Settings->PressTypes[Action])
        {
        case EPressType::Press:
            Functor.Toggle(Object);
            break;

        case EPressType::Continuous:
            Functor.Enable(Object);
            break;

        case EPressType::DoubleClick:
            if (const auto CurTime = GetGameTimeSinceCreation(); CurTime - LastTime <= Settings->DoubleClickTimeout)
            {
                Functor.Toggle(Object);
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

    FInputActionBinding Released{Action, IE_Released};
    Released.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(Object, [=] {
        const auto GI = CastChecked<UCP0GameInstance>(Object->GetGameInstance());
        const auto Settings = GI->GetInputSettings();

        switch (Settings->PressTypes[Action])
        {
        case EPressType::Release:
            Functor.Toggle(Object);
            break;

        case EPressType::Continuous:
            Functor.Disable(Object);
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
