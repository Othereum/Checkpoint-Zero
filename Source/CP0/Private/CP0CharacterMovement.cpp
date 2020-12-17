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
    GroundFriction = 6.0f;
    BrakingDecelerationWalking = 0.0f;

    MaxWalkSpeed = 300.0f;
    MaxWalkSpeedCrouched = 150.0f;
    CrouchedHalfHeight = 60.0f;

    bUseControllerDesiredRotation = true;
    RotationRate.Yaw = 300.0f;
}

ACP0Character* UCP0CharacterMovement::GetCP0Owner() const
{
    return CastChecked<ACP0Character>(GetCharacterOwner(), ECastCheckedType::NullAllowed);
}

float UCP0CharacterMovement::GetMaxSpeed() const
{
    switch (MovementMode) {
    case MOVE_Walking:
    case MOVE_NavWalking:
        switch (Posture) {
        default:
            ensureNoEntry();
        case EPosture::Stand:
            return IsSprinting() ? 500.0f : MaxWalkSpeed;
        case EPosture::Crouch:
            return MaxWalkSpeedCrouched;
        case EPosture::Prone:
            return 100.0f;
        }
    }
    return Super::GetMaxSpeed();
}

float UCP0CharacterMovement::GetMaxAcceleration() const
{
    switch (Posture) {
    default:
        ensureNoEntry();
    case EPosture::Stand:
        return MaxAcceleration;
    case EPosture::Crouch:
        return 512.0f;
    case EPosture::Prone:
        return 256.0f;
    }
}

bool UCP0CharacterMovement::CanSprint(bool bIgnorePosture) const
{
    constexpr auto MinSprintSpeed = 10.0f;
    constexpr auto MaxSprintAngleCos = 0.1f; // ~=84.26 deg

    if (!bIgnorePosture && Posture != EPosture::Stand)
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
    if (!CanSprint(true))
        return false;

    switch (Posture) {
    case EPosture::Crouch:
        if (!TrySetPosture(EPosture::Stand))
            return false;
        break;
    case EPosture::Prone:
        return false;
    }

    bIsSprinting = true;
    StopWalkingSlow();
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

    if (!bClientSimulation && HalfHeightAdjust > 0.0f) {
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

    if (New == EPosture::Prone) {
        RotationRate.Yaw = 100.0f;
    } else {
        const auto Default = GetDefault<UCP0CharacterMovement>(GetClass());
        RotationRate.Yaw = Default->RotationRate.Yaw;
    }

    if (bClientSimulation) {
        const auto ClientData = GetPredictionData_Client_Character();
        ClientData->MeshTranslationOffset.Z += HalfHeightAdjust;
        ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;

        bShrinkProxyCapsule = true;
        AdjustProxyCapsuleSize();
    } else {
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
    return NextPostureSwitch - 0.2f > CurTime() && (PrevPosture == EPosture::Prone || Posture == EPosture::Prone);
}

float UCP0CharacterMovement::GetPostureSwitchTime(EPosture Prev, EPosture New) const
{
    switch (Prev) {
    case EPosture::Stand:
        switch (New) {
        case EPosture::Crouch:
            return 0.5f;
        case EPosture::Prone:
            return 1.5f;
        }
        break;
    case EPosture::Crouch:
        switch (New) {
        case EPosture::Stand:
            return 0.5f;
        case EPosture::Prone:
            return 1.0f;
        }
        break;
    case EPosture::Prone:
        switch (New) {
        case EPosture::Stand:
            return 1.8f;
        case EPosture::Crouch:
            return 1.2f;
        }
        break;
    }
    ensureNoEntry();
    return 0.0f;
}

float UCP0CharacterMovement::GetDefaultHalfHeight(EPosture P) const
{
    switch (P) {
    default:
        ensureNoEntry();
    case EPosture::Stand:
        return GetCharacterOwner()->GetDefaultHalfHeight();
    case EPosture::Crouch:
        return CrouchedHalfHeight;
    case EPosture::Prone:
        return 34.0f;
    }
}

bool UCP0CharacterMovement::CanWalkSlow() const
{
    return !IsSprinting();
}

bool UCP0CharacterMovement::TryStartWalkingSlow()
{
    if (!CanWalkSlow())
        return false;

    bIsWalkingSlow = true;
    return true;
}

void UCP0CharacterMovement::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
    ProcessSprint();
    ProcessProne();
    ProcessSlowWalk();

    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCP0CharacterMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCP0CharacterMovement, Posture);
    DOREPLIFETIME(UCP0CharacterMovement, bIsSprinting);
}

void UCP0CharacterMovement::ProcessSprint()
{
    switch (GetOwnerRole()) {
    case ROLE_Authority:
    case ROLE_AutonomousProxy:
        if (IsSprinting() && !CanSprint())
            StopSprint();
    }
}

void UCP0CharacterMovement::ProcessProne()
{
    const auto Owner = GetCP0Owner();
    if (!Owner->IsLocallyControlled())
        return;

    if (GetPosture() != EPosture::Prone)
        return;

    constexpr auto Radius = 17.0f;
    constexpr auto HalfLength = 88.0f;
    constexpr auto OffsetX = -30.0f;
    constexpr auto HalfDistance = HalfLength - Radius;
    static_assert(34.0f - HalfLength < OffsetX && OffsetX < HalfLength - 34.0f, "invalid offset");

    const auto Capsule = Owner->GetCapsuleComponent();
    const auto Location = Capsule->GetComponentLocation();
    const auto Forward = Capsule->GetForwardVector();
    const auto Shape = FCollisionShape::MakeSphere(Radius);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    FVector Input{0.0f};
    for (const auto Diff : {HalfDistance, -HalfDistance}) {
        const auto Offset = Forward * (Diff + OffsetX);
        FHitResult Hit;
        if (GetWorld()->SweepSingleByChannel(Hit, Location, Location + Offset, FQuat::Identity, ProneTraceChannel,
                                             Shape, Params)) {
            Input += (Hit.Location - Hit.ImpactPoint) * Hit.Distance;
        }
    }

    Input = 2.0f * Input.GetClampedToMaxSize(1.0f);
    Input += ConsumeInputVector().GetClampedToMaxSize(1.0f);
    AddInputVector(Input, true);
}

void UCP0CharacterMovement::ProcessSlowWalk()
{
    if (IsWalkingSlow()) {
        AddInputVector(ConsumeInputVector().GetClampedToMaxSize(0.5f));
    }
}

float UCP0CharacterMovement::CurTime() const
{
    return GetWorld()->GetTimeSeconds();
}

void UCP0CharacterMovement::OnRep_Posture(EPosture Prev)
{
    if (GetOwnerRole() == ROLE_SimulatedProxy) {
        const auto New = Posture;
        Posture = Prev;
        TrySetPosture(New);
    }
}

void FInputAction_Sprint::Enable(ACP0Character* Character)
{
    Character->GetCP0Movement()->TryStartSprint();
}

void FInputAction_Sprint::Disable(ACP0Character* Character)
{
    Character->GetCP0Movement()->StopSprint();
}

void FInputAction_Sprint::Toggle(ACP0Character* Character)
{
    const auto Movement = Character->GetCP0Movement();
    Movement->IsSprinting() ? Movement->StopSprint() : Movement->TryStartSprint();
}

void FInputAction_Crouch::Enable(ACP0Character* Character)
{
    Character->GetCP0Movement()->TrySetPosture(EPosture::Crouch);
}

void FInputAction_Crouch::Disable(ACP0Character* Character)
{
    const auto Movement = Character->GetCP0Movement();
    if (Movement->GetPosture() == EPosture::Crouch)
        Movement->TrySetPosture(EPosture::Stand);
}

void FInputAction_Crouch::Toggle(ACP0Character* Character)
{
    const auto Movement = Character->GetCP0Movement();
    Movement->TrySetPosture(Movement->GetPosture() == EPosture::Crouch ? EPosture::Stand : EPosture::Crouch);
}

void FInputAction_Prone::Enable(ACP0Character* Character)
{
    Character->GetCP0Movement()->TrySetPosture(EPosture::Prone);
}

void FInputAction_Prone::Disable(ACP0Character* Character)
{
    const auto Movement = Character->GetCP0Movement();
    if (Movement->GetPosture() == EPosture::Prone)
        Movement->TrySetPosture(EPosture::Stand);
}

void FInputAction_Prone::Toggle(ACP0Character* Character)
{
    const auto Movement = Character->GetCP0Movement();
    Movement->TrySetPosture(Movement->GetPosture() == EPosture::Prone ? EPosture::Stand : EPosture::Prone);
}

void FInputAction_WalkSlow::Enable(ACP0Character* Character)
{
    Character->GetCP0Movement()->TryStartWalkingSlow();
}

void FInputAction_WalkSlow::Disable(ACP0Character* Character)
{
    Character->GetCP0Movement()->StopWalkingSlow();
}

void FInputAction_WalkSlow::Toggle(ACP0Character* Character)
{
    const auto Movement = Character->GetCP0Movement();
    Movement->IsWalkingSlow() ? Movement->StopWalkingSlow() : Movement->TryStartWalkingSlow();
}
