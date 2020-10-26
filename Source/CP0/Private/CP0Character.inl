// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

template <class Action>
static void DispatchInputAction(ACP0Character* character, EInputAction type)
{
    switch (type)
    {
    case EInputAction::Enable:
        Action::Enable(character);
        break;
    case EInputAction::Disable:
        Action::Disable(character);
        break;
    case EInputAction::Toggle:
        Action::Toggle(character);
        break;
    default:
        ensure(!"Enclosing block should never be called");
    }
}

template <class Action>
void ACP0Character::BindInputAction(UInputComponent* input, FName name)
{
    InputActionMap.Add(name, &::DispatchInputAction<Action>);

    FInputActionBinding pressed{name, IE_Pressed};
    pressed.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(this, [=, lastTime = -1.0f]() mutable {
        const auto gi = CastChecked<UCP0GameInstance>(GetGameInstance());
        const auto settings = gi->GetInputSettings();

        switch (settings->pressTypes[name])
        {
        case EPressType::Press:
            DispatchInputAction(name, EInputAction::Toggle);
            break;

        case EPressType::Continuous:
            DispatchInputAction(name, EInputAction::Enable);
            break;

        case EPressType::DoubleClick: {
            const auto curTime = GetGameTimeSinceCreation();
            if (curTime - lastTime <= settings->doubleClickTimeout)
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
        const auto gi = CastChecked<UCP0GameInstance>(GetGameInstance());
        const auto settings = gi->GetInputSettings();

        switch (settings->pressTypes[name])
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
