// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0AnimInstance.h"
#include "CP0Character.h"
#include "CP0CharacterMovement.h"

void UCP0AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    const auto* const Character = CastChecked<ACP0Character>(TryGetPawnOwner(), ECastCheckedType::NullAllowed);
    if (!Character)
        return;

    const auto Velocity = Character->GetVelocity();
    MoveSpeed = Velocity.Size2D();
    MoveDirection = CalculateDirection(Velocity, Character->GetActorRotation());

    const auto AimRot = Character->GetBaseAimRotation() - Character->GetActorRotation();
    AimPitch = FRotator::NormalizeAxis(AimRot.Pitch);
    AimYaw = FRotator::NormalizeAxis(AimRot.Yaw);

    const auto* const Movement = Character->GetCP0Movement();
    Posture = Movement->GetPosture();
    bIsOnGround = Movement->IsMovingOnGround();
    bIsSprinting = Movement->IsActuallySprinting();
    bShouldPlayPostureAnim = MoveSpeed < 50.0f || Movement->IsProneSwitching();
    MeshRotOffset.Roll = FMath::FInterpTo(MeshRotOffset.Roll, Movement->GetMeshPitchOffset(), DeltaSeconds,
                                          Movement->IsProneSwitching() ? 1.0f : 10.0f);

    constexpr auto YawCalcDelay = 1.0f / 10.0f;
    YawCalcLag += DeltaSeconds;
    if (YawCalcLag >= YawCalcDelay)
    {
        const auto SkippedFrames = FMath::TruncToFloat(YawCalcLag / YawCalcDelay);
        const auto SkippedTime = YawCalcDelay * SkippedFrames;
        YawCalcLag -= SkippedTime;

        const auto Yaw = Character->GetActorRotation().Yaw;
        YawSpeed = MoveSpeed < 10.0f ? FMath::FindDeltaAngleDegrees(PrevYaw, Yaw) / SkippedTime : 0.0f;
        PrevYaw = Yaw;
    }
}
