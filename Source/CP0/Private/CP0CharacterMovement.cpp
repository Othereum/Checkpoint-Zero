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

    const auto bClientSimulation = GetOwnerRole() == ROLE_SimulatedProxy;

    if (!bClientSimulation && IsPostureSwitching())
        return false;

    const auto Owner = GetCP0Owner();
    const auto Capsule = Owner->GetCapsuleComponent();
    const auto SwitchTime = GetPostureSwitchTime(Posture, New);
    const auto NewHalfHeight = GetDefaultHalfHeight(New);
    const auto HalfHeightAdjust = NewHalfHeight - Capsule->GetUnscaledCapsuleHalfHeight();
    const auto NewPawnLocation = UpdatedComponent->GetComponentLocation() + FVector{0.0f, 0.0f, HalfHeightAdjust};

    if (!bClientSimulation && HalfHeightAdjust > 0.0f)
    {
        FCollisionQueryParams CapsuleParams{NAME_None, false, Owner};
        FCollisionResponseParams ResponseParam;
        InitCollisionParams(CapsuleParams, ResponseParam);

        const auto CapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -HalfHeightAdjust);
        const auto CollisionChannel = UpdatedComponent->GetCollisionObjectType();
        const auto bEncroached = GetWorld()->OverlapBlockingTestByChannel(
            NewPawnLocation, FQuat::Identity, CollisionChannel, CapsuleShape, CapsuleParams, ResponseParam);

        if (bEncroached)
            return false;
    }

    Owner->SetEyeHeightWithBlend(Owner->GetDefaultEyeHeight(New), SwitchTime);
    Owner->BaseTranslationOffset = {0.0f, 0.0f, -NewHalfHeight};
    Owner->GetMesh()->SetRelativeLocation(Owner->BaseTranslationOffset);
    Capsule->SetCapsuleHalfHeight(NewHalfHeight);

    if (bClientSimulation)
    {
        const auto ClientData = GetPredictionData_Client_Character();
        ClientData->MeshTranslationOffset.Z += HalfHeightAdjust;
        ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;

        bShrinkProxyCapsule = true;
        AdjustProxyCapsuleSize();
    }
    else
    {
        UpdatedComponent->SetWorldLocation(NewPawnLocation);
    }

    PrevPosture = Posture;
    Posture = New;

    NextPostureSwitch = CurTime() + SwitchTime;
    OnPostureChanged.Broadcast(PrevPosture, Posture);

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

float UCP0CharacterMovement::GetDefaultHalfHeight(EPosture P) const
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
    ProcessProne();
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

void UCP0CharacterMovement::ProcessProne()
{
    const auto Owner = GetCP0Owner();
    if (!Owner->IsLocallyControlled())
        return;

    if (GetPosture() != EPosture::Prone)
        return;

    const auto Capsule = Owner->GetCapsuleComponent();
    const auto Radius = Capsule->GetUnscaledCapsuleRadius();
    const auto Location = Capsule->GetComponentLocation();
    const auto Offset = Capsule->GetForwardVector() * (ProneLength - Radius);
    const auto Channel = Capsule->GetCollisionObjectType();
    const auto Shape = FCollisionShape::MakeSphere(Radius);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    thread_local TArray<FHitResult> Hits;
    for (const auto& Off : {Offset, -Offset})
    {
        if (GetWorld()->SweepMultiByChannel(Hits, Location, Location + Off, FQuat::Identity, Channel, Shape, Params))
        {
            for (const auto& Hit : Hits)
            {
                AddInputVector(Location - Hit.ImpactPoint, true);
            }
        }
    }
}

float UCP0CharacterMovement::CurTime() const
{
    return GetWorld()->GetTimeSeconds();
}

void UCP0CharacterMovement::OnRep_Posture(EPosture Prev)
{
    if (GetOwnerRole() == ROLE_SimulatedProxy)
    {
        const auto New = Posture;
        Posture = Prev;
        TrySetPosture(New);
    }
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
