// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0GameInstance.h"
#include "CP0InputSettings.h"

template <class Action>
static void DispatchInputActionByType(ACP0Character* Character, EInputAction Type)
{
    switch (Type)
    {
    case EInputAction::Enable:
        Action::Enable(Character);
        break;
    case EInputAction::Disable:
        Action::Disable(Character);
        break;
    case EInputAction::Toggle:
        Action::Toggle(Character);
        break;
    default:
        ensure(!"Enclosing block should never be called");
    }
}

template <class Action>
void ACP0Character::RegisterInputAction(FName Name)
{
    InputActionMap.Add(Name, &DispatchInputActionByType<Action>);
}

void ACP0Character::ServerInputAction_Implementation(FName Name, EInputAction Type)
{
    DispatchInputAction(Name, Type);
}

bool ACP0Character::ServerInputAction_Validate(FName Name, EInputAction Type)
{
    return true;
}

void ACP0Character::DispatchInputAction(FName Name, EInputAction Type)
{
    const auto dispatcher = InputActionMap.Find(Name);
    if (ensure(dispatcher))
    {
        (*dispatcher)(this, Type);
    }

    if (!HasAuthority())
        ServerInputAction(Name, Type);
}

void ACP0Character::BindInputAction(UInputComponent* input, FName name)
{
    FInputActionBinding pressed{name, IE_Pressed};
    pressed.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(this, [=, lastTime = -1.0f]() mutable {
        const auto GI = CastChecked<UCP0GameInstance>(GetGameInstance());
        const auto Settings = GI->GetInputSettings();

        switch (Settings->PressTypes[name])
        {
        case EPressType::Press:
            DispatchInputAction(name, EInputAction::Toggle);
            break;

        case EPressType::Continuous:
            DispatchInputAction(name, EInputAction::Enable);
            break;

        case EPressType::DoubleClick: {
            const auto curTime = GetGameTimeSinceCreation();
            if (curTime - lastTime <= Settings->DoubleClickTimeout)
            {
                DispatchInputAction(name, EInputAction::Toggle);
                lastTime = -1.0f;
            }
            else
            {
                lastTime = curTime;
            }
            break;
        }
        }
    });
    input->AddActionBinding(MoveTemp(pressed));

    FInputActionBinding released{name, IE_Released};
    released.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(this, [=] {
        const auto GI = CastChecked<UCP0GameInstance>(GetGameInstance());
        const auto Settings = GI->GetInputSettings();

        switch (Settings->PressTypes[name])
        {
        case EPressType::Release:
            DispatchInputAction(name, EInputAction::Toggle);
            break;

        case EPressType::Continuous:
            DispatchInputAction(name, EInputAction::Disable);
            break;
        }
    });
    input->AddActionBinding(MoveTemp(released));
}
