// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0AnimInstance.h"
#include "CP0Character.h"
#include "CP0CharacterMovement.h"

void UCP0AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    const auto Character = CastChecked<ACP0Character>(TryGetPawnOwner(), ECastCheckedType::NullAllowed);
    if (!Character)
        return;

    const auto Velocity = Character->GetVelocity();
    MoveSpeed = Velocity.Size2D();
    MoveDirection = MoveSpeed > 1.0f ? CalculateDirection(Velocity, Character->GetActorRotation()) : 0.0f;
    bShouldPlayPostureAnim = MoveSpeed < MaxPostureAnimWalkSpeed;

    const auto Movement = Character->GetCP0Movement();
    Posture = Movement->GetPosture();
    bIsOnGround = Movement->IsMovingOnGround();
    bIsSprinting = Movement->IsSprinting() && MoveSpeed > MinSprintSpeed;
}
