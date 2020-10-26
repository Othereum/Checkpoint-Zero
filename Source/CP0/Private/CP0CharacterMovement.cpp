// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0CharacterMovement.h"
#include "CP0Character.h"

void FSprintAction::Enable(ACP0Character* Character)
{
    Character->bIsSprinting = true;
}

void FSprintAction::Disable(ACP0Character* Character)
{
    Character->bIsSprinting = false;
}

void FSprintAction::Toggle(ACP0Character* Character)
{
    Character->bIsSprinting = !Character->bIsSprinting;
}
