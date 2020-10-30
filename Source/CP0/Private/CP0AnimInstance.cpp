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
    MoveSpeed = Velocity.Size();
    MoveDirection = MoveSpeed > 1.0f ? CalculateDirection(Velocity, Character->GetActorRotation()) : 0.0f;

    const auto Movement = Character->GetCP0Movement();
    bIsOnGround = Movement->IsMovingOnGround();
    bIsSprinting = Movement->IsSprinting() && MoveSpeed > MinSprintSpeed;
}
