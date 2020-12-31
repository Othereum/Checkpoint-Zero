// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "WeaponAnimInst.h"
#include "CP0Character.h"
#include "CP0CharacterMovement.h"

void UWeaponAnimInst::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    const auto* const Character = CastChecked<ACP0Character>(TryGetPawnOwner(), ECastCheckedType::NullAllowed);
    if (!Character)
        return;

    const auto Velocity = Character->GetVelocity();
    MoveSpeed = Velocity.Size2D();

    const auto* const Movement = Character->GetCP0Movement();
    bIsOnGround = Movement->IsMovingOnGround();
    bIsSprinting = bIsOnGround && Movement->IsSprinting() && MoveSpeed > Movement->MaxWalkSpeed;
}
