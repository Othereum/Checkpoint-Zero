// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0GameInstance.h"
#include "CP0InputSettings.h"

template <class Action>
[[nodiscard]] static decltype(Action::GetObject(DeclVal<ACP0Character*>())) GetActionObject(ACP0Character* Character)
{
    return Action::GetObject(Character);
}

template <class Action>
[[nodiscard]] static TEnableIf<TIsPointer<decltype(Action::Enable(DeclVal<ACP0Character*>()))*>::Value, ACP0Character*>
GetActionObject(ACP0Character* Character)
{
    return Character;
}

template <class Action>
static void DispatchInputActionByType(ACP0Character* Character, EInputAction Type)
{
    switch (Type)
    {
    case EInputAction::Enable:
        Action::Enable(GetActionObject<Action>(Character));
        break;
    case EInputAction::Disable:
        Action::Disable(GetActionObject<Action>(Character));
        break;
    case EInputAction::Toggle:
        Action::Toggle(GetActionObject<Action>(Character));
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
    const auto aa = sizeof(void&&);
    DispatchInputAction(Name, Type);
}

bool ACP0Character::ServerInputAction_Validate(FName Name, EInputAction Type)
{
    return true;
}

void ACP0Character::DispatchInputAction(FName Name, EInputAction Type)
{
    const auto Dispatcher = InputActionMap.Find(Name);
    if (ensure(Dispatcher))
    {
        (*Dispatcher)(this, Type);
    }

    if (!HasAuthority())
        ServerInputAction(Name, Type);
}

void ACP0Character::BindInputAction(UInputComponent* Input, FName Name)
{
    FInputActionBinding Pressed{Name, IE_Pressed};
    Pressed.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(this, [=, LastTime = -1.0f]() mutable {
        const auto GI = CastChecked<UCP0GameInstance>(GetGameInstance());
        const auto Settings = GI->GetInputSettings();

        switch (Settings->PressTypes[Name])
        {
        case EPressType::Press:
            DispatchInputAction(Name, EInputAction::Toggle);
            break;

        case EPressType::Continuous:
            DispatchInputAction(Name, EInputAction::Enable);
            break;

        case EPressType::DoubleClick: {
            const auto CurTime = GetGameTimeSinceCreation();
            if (CurTime - LastTime <= Settings->DoubleClickTimeout)
            {
                DispatchInputAction(Name, EInputAction::Toggle);
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

    FInputActionBinding Released{Name, IE_Released};
    Released.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(this, [=] {
        const auto GI = CastChecked<UCP0GameInstance>(GetGameInstance());
        const auto Settings = GI->GetInputSettings();

        switch (Settings->PressTypes[Name])
        {
        case EPressType::Release:
            DispatchInputAction(Name, EInputAction::Toggle);
            break;

        case EPressType::Continuous:
            DispatchInputAction(Name, EInputAction::Disable);
            break;
        }
    });
    Input->AddActionBinding(MoveTemp(Released));
}
