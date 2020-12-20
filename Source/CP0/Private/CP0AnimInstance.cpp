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
    bIsSprinting = Movement->IsSprinting() && MoveSpeed > Movement->MaxWalkSpeed;
    bShouldPlayPostureAnim = MoveSpeed < 50.0f || Movement->IsProneSwitching();
    YawRotationSpeed = MoveSpeed < 10.0f ? Movement->GetYawRotationSpeed() : 0.0f;

    const auto* const Mesh = Character->GetMesh();
    MeshPitch = Mesh->GetRelativeRotation().Roll;
}
