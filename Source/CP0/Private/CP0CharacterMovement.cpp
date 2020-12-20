// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0CharacterMovement.h"
#include "CP0.h"
#include "CP0Character.h"
#include "CP0PCM.h"
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
    return CastChecked<ACP0Character>(CharacterOwner, ECastCheckedType::NullAllowed);
}

float UCP0CharacterMovement::GetMaxSpeed() const
{
    switch (MovementMode)
    {
    case MOVE_Walking:
    case MOVE_NavWalking:
        switch (Posture)
        {
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
    switch (Posture)
    {
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

    switch (Posture)
    {
    case EPosture::Prone:
        return false;
    case EPosture::Crouch:
        if (!TrySetPosture(EPosture::Stand))
            return false;
    }

    bIsSprinting = true;
    StopWalkingSlow();
    return true;
}

bool UCP0CharacterMovement::TrySetPosture(EPosture New, bool bIgnoreDelay)
{
    if (Posture == New)
        return true;

    const auto bClientSimulation = GetOwnerRole() == ROLE_SimulatedProxy;
    if (!bClientSimulation)
    {
        if (!bIgnoreDelay && IsPostureSwitching())
            return false;

        if (New != EPosture::Stand && !IsMovingOnGround())
            return false;
    }

    const auto Owner = GetCP0Owner();
    const auto Capsule = Owner->GetCapsuleComponent();
    const auto SwitchTime = GetPostureSwitchTime(Posture, New);
    const auto NewHalfHeight = GetDefaultHalfHeight(New);
    const auto OldHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
    const auto HalfHeightAdjust = NewHalfHeight - OldHalfHeight;
    const auto NewPawnLocation = Capsule->GetComponentLocation() + FVector{0.0f, 0.0f, HalfHeightAdjust};

    if (!bClientSimulation && HalfHeightAdjust > 0.0f)
    {
        FCollisionQueryParams CapsuleParams{NAME_None, false, Owner};
        FCollisionResponseParams ResponseParam;
        InitCollisionParams(CapsuleParams, ResponseParam);

        const auto CapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -HalfHeightAdjust);
        const auto CollisionChannel = Capsule->GetCollisionObjectType();
        const auto bEncroached = GetWorld()->OverlapBlockingTestByChannel(
            NewPawnLocation, FQuat::Identity, CollisionChannel, CapsuleShape, CapsuleParams, ResponseParam);

        if (bEncroached)
            return false;
    }

    const auto OldWalkableFloorAngle = GetWalkableFloorAngle();
    SetWalkableFloorAngle(New == EPosture::Prone ? 30.0f : GetDefaultSelf()->GetWalkableFloorAngle());
    Capsule->SetCapsuleHalfHeight(NewHalfHeight);

    if (!bClientSimulation)
    {
        FFindFloorResult Result;
        FindFloor(NewPawnLocation, Result, false);

        if (!Result.bWalkableFloor)
        {
            Capsule->SetCapsuleHalfHeight(OldHalfHeight);
            SetWalkableFloorAngle(OldWalkableFloorAngle);
            return false;
        }
    }

    Owner->BaseTranslationOffset = {0.0f, 0.0f, -NewHalfHeight};
    Owner->GetMesh()->SetRelativeLocation(Owner->BaseTranslationOffset);
    Owner->SetEyeHeightWithBlend(Owner->GetDefaultEyeHeight(New), SwitchTime);

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
        Capsule->SetWorldLocation(NewPawnLocation);
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
    switch (Prev)
    {
    case EPosture::Stand:
        switch (New)
        {
        case EPosture::Crouch:
            return 0.5f;
        case EPosture::Prone:
            return 1.5f;
        }
        break;
    case EPosture::Crouch:
        switch (New)
        {
        case EPosture::Stand:
            return 0.5f;
        case EPosture::Prone:
            return 1.0f;
        }
        break;
    case EPosture::Prone:
        switch (New)
        {
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
    switch (P)
    {
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

float UCP0CharacterMovement::CalcFloorPitch() const
{
    if (!CharacterOwner)
        return 0.0f;

    const auto World = GetWorld();
    const auto Capsule = CharacterOwner->GetCapsuleComponent();
    const auto BaseLoc = Capsule->GetComponentLocation();
    const auto ForwardOffset = Capsule->GetForwardVector() * Capsule->GetScaledCapsuleRadius();
    const FVector EndOffset{0.0f, 0.0f, -3.0f * Capsule->GetScaledCapsuleHalfHeight()};

    FHitResult Front, Rear;

    const auto FrontLoc = BaseLoc + ForwardOffset;
    if (!World->LineTraceSingleByChannel(Front, FrontLoc, FrontLoc + EndOffset, PushTraceChannel))
        return 0.0f;

    const auto RearLoc = BaseLoc - ForwardOffset;
    if (!World->LineTraceSingleByChannel(Rear, RearLoc, RearLoc + EndOffset, PushTraceChannel))
        return 0.0f;

    const auto HeightDiff = Rear.Location.Z - Front.Location.Z;
    const auto NormalizedHeight = FMath::Abs(HeightDiff) / FVector::Dist(Front.Location, Rear.Location);
    const auto AbsRadians = FMath::FastAsin(NormalizedHeight);
    return FMath::RadiansToDegrees(AbsRadians) * FMath::Sign(HeightDiff);
}

void UCP0CharacterMovement::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
    if (!IsMovingOnGround())
        TrySetPosture(EPosture::Stand, true);

    FVector ForceInput{0.0f};
    ProcessSprint();
    ProcessPronePush(ForceInput);
    ProcessSlowWalk();
    ProcessTurn();
    ProcessPronePitch(DeltaTime, ForceInput);
    UpdateViewPitchLimit(DeltaTime);
    AddInputVector(2.0f * ForceInput + ConsumeInputVector().GetClampedToMaxSize(1.0f), true);

    const auto PrevYaw = UpdatedComponent->GetComponentRotation().Yaw;
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    YawRotationSpeed = FMath::FindDeltaAngleDegrees(PrevYaw, UpdatedComponent->GetComponentRotation().Yaw) / DeltaTime;
}

void UCP0CharacterMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCP0CharacterMovement, Posture);
    DOREPLIFETIME(UCP0CharacterMovement, bIsSprinting);
}

void UCP0CharacterMovement::ProcessSprint()
{
    switch (GetOwnerRole())
    {
    case ROLE_Authority:
    case ROLE_AutonomousProxy:
        if (IsSprinting() && !CanSprint())
            StopSprint();
    }
}

void UCP0CharacterMovement::ProcessPronePush(FVector& ForceInput)
{
    if (Posture != EPosture::Prone)
        return;

    const auto* const Owner = GetCP0Owner();
    if (!Owner->IsLocallyControlled())
        return;

    const auto* const Mesh = Owner->GetMesh();
    const auto* const Capsule = Owner->GetCapsuleComponent();
    const auto* const DefaultCapsule = GetDefault<ACP0Character>(Owner->GetClass())->GetCapsuleComponent();
    const auto Radius = Capsule->GetScaledCapsuleRadius();
    const auto HalfLength = DefaultCapsule->GetScaledCapsuleHalfHeight();
    const auto Location = Capsule->GetComponentLocation();
    const auto Forward = Mesh->GetRightVector();
    const auto Shape = FCollisionShape::MakeBox({0.0f, Radius, 0.0f});
    constexpr auto OffsetX = -28.0f;

    FVector Input{0.0f};
    for (const auto Diff : {HalfLength, -HalfLength})
    {
        const auto Offset = Forward * (Diff + OffsetX);
        FHitResult Hit;
        if (GetWorld()->SweepSingleByChannel(Hit, Location, Location + Offset, Capsule->GetComponentQuat(),
                                             PushTraceChannel, Shape))
        {
            Input += (Location - Hit.ImpactPoint) * Hit.Distance;
        }
    }
    ForceInput += Input.GetClampedToMaxSize(1.0f);
}

void UCP0CharacterMovement::ProcessPronePitch(float DeltaTime, FVector& ForceInput)
{
    const auto bIsProne = Posture == EPosture::Prone;
    const auto Pitch = bIsProne ? CalcFloorPitch() : 0.0f;

    const auto Mesh = GetCP0Owner()->GetMesh();
    auto Rotation = Mesh->GetRelativeRotation();
    Rotation.Roll = FMath::FInterpTo(Rotation.Roll, Pitch, DeltaTime, bIsProne ? 10.0f : 1.0f);
    Mesh->SetRelativeRotation(Rotation);

    if (PawnOwner->IsLocallyControlled())
    {
        const auto Threshold = GetWalkableFloorAngle();
        if (Pitch < -Threshold)
        {
            ForceInput -= PawnOwner->GetActorForwardVector();
        }
        else if (Pitch > Threshold)
        {
            ForceInput += PawnOwner->GetActorForwardVector();
        }
    }
}

void UCP0CharacterMovement::UpdateViewPitchLimit(float DeltaTime)
{
    const auto* const PC = PawnOwner->GetController<APlayerController>();
    if (!PC)
        return;

    const auto PCM = PC->PlayerCameraManager;
    const auto Default = GetDefault<APlayerCameraManager>(PCM->GetClass());
    FVector2D Limit{Default->ViewPitchMin, Default->ViewPitchMax};

    if (Posture == EPosture::Prone)
        Limit /= 2.0f;

    const auto* const Mesh = CharacterOwner->GetMesh();
    Limit -= FVector2D{Mesh->GetRelativeRotation().Roll};

    constexpr auto Speed = 2.0f;
    PCM->ViewPitchMin = FMath::FInterpTo(PCM->ViewPitchMin, Limit.X, DeltaTime, Speed);
    PCM->ViewPitchMax = FMath::FInterpTo(PCM->ViewPitchMax, Limit.Y, DeltaTime, Speed);
}

void UCP0CharacterMovement::ProcessSlowWalk()
{
    if (IsWalkingSlow())
    {
        AddInputVector(ConsumeInputVector().GetClampedToMaxSize(0.5f));
    }
}

void UCP0CharacterMovement::ProcessTurn()
{
    RotationRate.Yaw = [this]() {
        switch (Posture)
        {
        default:
            ensureNoEntry();
        case EPosture::Stand:
            return GetDefaultSelf()->RotationRate.Yaw;
        case EPosture::Crouch:
            return 90.0f;
        case EPosture::Prone:
            return 45.0f;
        }
    }();

    constexpr auto MaxSpeed = 10.0f;
    if (Velocity.SizeSquared() > MaxSpeed * MaxSpeed)
    {
        RotationRate.Yaw *= 2.0f;
    }
}

float UCP0CharacterMovement::CurTime() const
{
    return GetWorld()->GetTimeSeconds();
}

const UCP0CharacterMovement* UCP0CharacterMovement::GetDefaultSelf() const
{
    return GetDefault<ACP0Character>(GetCP0Owner()->GetClass())->GetCP0Movement();
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
