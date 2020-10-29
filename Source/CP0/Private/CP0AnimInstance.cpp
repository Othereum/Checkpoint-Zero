// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0AnimInstance.h"
#include "GameFramework/PawnMovementComponent.h"

void UCP0AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (const auto Pawn = TryGetPawnOwner())
    {
        const auto Velocity = Pawn->GetVelocity();
        MoveSpeed = Velocity.Size();
        if (MoveSpeed > 1.0f)
        {
            MoveDirection = CalculateDirection(Velocity, Pawn->GetActorRotation());
        }

        bIsOnGround = Pawn->GetMovementComponent()->IsMovingOnGround();
    }
}
