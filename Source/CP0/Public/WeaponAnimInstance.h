// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "Animation/AnimInstance.h"
#include "CP0.h"
#include "WeaponAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class CP0_API UWeaponAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

  private:
    void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    EWeaponFireMode FireMode;
};
