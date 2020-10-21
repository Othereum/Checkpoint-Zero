// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

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
