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
        return IsSprinting() ? GetSprintSpeed() : MaxWalkSpeed;
    }
    return Super::GetMaxSpeed();
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

bool UCP0CharacterMovement::CanSprint() const
{
    if (Posture != EPosture::Stand)
        return false;

    if (Velocity.SizeSquared() < MinSprintSpeed * MinSprintSpeed)
        return false;

    const auto ViewDir = GetOwner()->GetActorForwardVector();
    const auto VelDir = Velocity.GetUnsafeNormal();
    if ((ViewDir | VelDir) < MaxSprintAngleCos)
        return false;

    return true;
}

bool UCP0CharacterMovement::TryStartSprint()
{
    if (!CanSprint())
        return false;

    bIsSprinting = true;
    return true;
}

void UCP0CharacterMovement::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (GetOwnerRole() == ROLE_SimulatedProxy)
        return;

    ProcessSprint();
}

void UCP0CharacterMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCP0CharacterMovement, Posture);
    DOREPLIFETIME(UCP0CharacterMovement, bIsSprinting);
}

void UCP0CharacterMovement::ProcessSprint()
{
    if (IsSprinting() && !CanSprint())
        StopSprint();
}

UCP0CharacterMovement* FMovementActionBase::GetObject(ACP0Character* Character)
{
    return Character->GetCP0Movement();
}

void FSprintAction::Enable(UCP0CharacterMovement* Movement)
{
    Movement->TryStartSprint();
}

void FSprintAction::Disable(UCP0CharacterMovement* Movement)
{
    Movement->StopSprint();
}

void FSprintAction::Toggle(UCP0CharacterMovement* Movement)
{
    Movement->IsSprinting() ? Movement->StopSprint() : Movement->TryStartSprint();
}

void FCrouchAction::Enable(UCP0CharacterMovement* Movement)
{
}

void FCrouchAction::Disable(UCP0CharacterMovement* Movement)
{
}

void FCrouchAction::Toggle(UCP0CharacterMovement* Movement)
{
}

void FProneAction::Enable(UCP0CharacterMovement* Movement)
{
}

void FProneAction::Disable(UCP0CharacterMovement* Movement)
{
}

void FProneAction::Toggle(UCP0CharacterMovement* Movement)
{
}
