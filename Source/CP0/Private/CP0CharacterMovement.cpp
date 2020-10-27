// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0CharacterMovement.h"
#include "CP0Character.h"
#include "Net/UnrealNetwork.h"

UCP0CharacterMovement* FSprintAction::GetObject(const ACP0Character* Character)
{
    return Character->GetCP0Movement();
}

void FSprintAction::Enable(UCP0CharacterMovement* Movement)
{
}

void FSprintAction::Disable(UCP0CharacterMovement* Movement)
{
}

void FSprintAction::Toggle(UCP0CharacterMovement* Movement)
{
}

UCP0CharacterMovement::UCP0CharacterMovement()
{
    SetIsReplicatedByDefault(true);
}

void UCP0CharacterMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(UCP0CharacterMovement, bIsSprinting, COND_SkipOwner);
}
