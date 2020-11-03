// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0CharacterMovement.h"
#include "CP0.h"
#include "CP0Character.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

UCP0CharacterMovement::UCP0CharacterMovement()
{
    SetIsReplicatedByDefault(true);
    MaxAcceleration = 1024.0f;
    CrouchedHalfHeight = 60.0f;
    GroundFriction = 6.0f;
    MaxWalkSpeed = 400.0f;
    MaxWalkSpeedCrouched = 200.0f;
    BrakingDecelerationWalking = 0.0f;
}

ACP0Character* UCP0CharacterMovement::GetCP0Owner() const
{
    return CastChecked<ACP0Character>(GetCharacterOwner(), ECastCheckedType::NullAllowed);
}

float UCP0CharacterMovement::GetMaxSpeed() const
{
    switch (MovementMode)
    {
    case MOVE_Walking:
    case MOVE_NavWalking:
        if (IsProneSwitching())
            return 0.0f;

        switch (Posture)
        {
        default:
            ensureNoEntry();
        case EPosture::Stand:
            return IsSprinting() ? MaxSprintSpeed : MaxWalkSpeed;
        case EPosture::Crouch:
            return MaxWalkSpeedCrouched;
        case EPosture::Prone:
            return MaxWalkSpeedProne;
        }
    }
    return Super::GetMaxSpeed();
}

float UCP0CharacterMovement::GetMaxAcceleration() const
{
    switch (Posture)
    {
    default:
        ensureNoEntry();
    case EPosture::Stand:
        return MaxAcceleration;
    case EPosture::Crouch:
        return MaxAccelerationCrouched;
    case EPosture::Prone:
        return MaxAccelerationProne;
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

bool UCP0CharacterMovement::TrySetPosture(EPosture New)
{
    if (Posture == New)
        return true;

    if (IsPostureSwitching())
        return false;

    const auto Owner = GetCP0Owner();
    const auto Height = GetHalfHeight(New);
    Owner->GetCapsuleComponent()->SetCapsuleHalfHeight(Height);
    Owner->GetMesh()->SetRelativeLocation({0.0f, 0.0f, -Height});

    PrevPosture = Posture;
    Posture = New;
    OnPostureChanged.Broadcast(PrevPosture, New);

    Owner->RecalculateBaseEyeHeight();

    NextPostureSwitch = CurTime() + GetPostureSwitchTime(PrevPosture, New);
    return true;
}

bool UCP0CharacterMovement::IsPostureSwitching() const
{
    return NextPostureSwitch > CurTime();
}

bool UCP0CharacterMovement::IsProneSwitching() const
{
    return IsPostureSwitching() && (PrevPosture == EPosture::Prone || Posture == EPosture::Prone);
}

float UCP0CharacterMovement::GetPostureSwitchTime(EPosture Prev, EPosture New) const
{
    switch (Prev)
    {
    case EPosture::Stand:
        switch (New)
        {
        case EPosture::Crouch:
            return StandToCrouchTime;
        case EPosture::Prone:
            return StandToProneTime;
        }
        break;
    case EPosture::Crouch:
        switch (New)
        {
        case EPosture::Stand:
            return CrouchToStandTime;
        case EPosture::Prone:
            return CrouchToProneTime;
        }
        break;
    case EPosture::Prone:
        switch (New)
        {
        case EPosture::Stand:
            return ProneToStandTime;
        case EPosture::Crouch:
            return ProneToCrouchTime;
        }
        break;
    }
    ensureNoEntry();
    return 0.0f;
}

float UCP0CharacterMovement::GetHalfHeight(EPosture P) const
{
    switch (P)
    {
    default:
        ensureNoEntry();
    case EPosture::Stand:
        return GetCharacterOwner()->GetDefaultHalfHeight();
    case EPosture::Crouch:
        return CrouchedHalfHeight;
    case EPosture::Prone:
        return ProneHalfHeight;
    }
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

float UCP0CharacterMovement::CurTime() const
{
    return GetWorld()->GetTimeSeconds();
}

void UCP0CharacterMovement::OnRep_Posture(EPosture Prev)
{
    PrevPosture = Prev;
    OnPostureChanged.Broadcast(Prev, Posture);
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
    Movement->TrySetPosture(EPosture::Crouch);
}

void FCrouchAction::Disable(UCP0CharacterMovement* Movement)
{
    if (Movement->GetPosture() == EPosture::Crouch)
        Movement->TrySetPosture(EPosture::Stand);
}

void FCrouchAction::Toggle(UCP0CharacterMovement* Movement)
{
    Movement->TrySetPosture(Movement->GetPosture() == EPosture::Crouch ? EPosture::Stand : EPosture::Crouch);
}

void FProneAction::Enable(UCP0CharacterMovement* Movement)
{
    Movement->TrySetPosture(EPosture::Prone);
}

void FProneAction::Disable(UCP0CharacterMovement* Movement)
{
    if (Movement->GetPosture() == EPosture::Prone)
        Movement->TrySetPosture(EPosture::Stand);
}

void FProneAction::Toggle(UCP0CharacterMovement* Movement)
{
    Movement->TrySetPosture(Movement->GetPosture() == EPosture::Prone ? EPosture::Stand : EPosture::Prone);
}
