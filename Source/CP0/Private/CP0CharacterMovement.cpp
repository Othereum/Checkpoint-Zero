// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0CharacterMovement.h"
#include "CP0.h"
#include "CP0Character.h"
#include "Net/UnrealNetwork.h"

UCP0CharacterMovement::UCP0CharacterMovement()
{
    SetIsReplicatedByDefault(true);
}

float UCP0CharacterMovement::GetMaxSpeed() const
{
    switch (MovementMode)
    {
    case MOVE_Walking:
    case MOVE_NavWalking:
        return bIsSprinting ? GetSprintSpeed() : MaxWalkSpeed;
    default:
        return Super::GetMaxSpeed();
    }
}

float UCP0CharacterMovement::GetSprintSpeed() const
{
    switch (SprintSpeedType)
    {
    case ESprintSpeed::Absolute:
        return SprintSpeed;
    case ESprintSpeed::Relative:
        return MaxWalkSpeed + SprintSpeed;
    case ESprintSpeed::Multiply:
        return MaxWalkSpeed * SprintSpeed;
    default:
        ensureNoEntry();
        return SprintSpeed;
    }
}

void UCP0CharacterMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCP0CharacterMovement, bIsSprinting);
}

UCP0CharacterMovement* FSprintAction::GetObject(ACP0Character* Character)
{
    return Character->GetCP0Movement();
}

void FSprintAction::Enable(UCP0CharacterMovement* Movement)
{
    Movement->bIsSprinting = true;
}

void FSprintAction::Disable(UCP0CharacterMovement* Movement)
{
    Movement->bIsSprinting = false;
}

void FSprintAction::Toggle(UCP0CharacterMovement* Movement)
{
    Movement->bIsSprinting = !Movement->bIsSprinting;
}
