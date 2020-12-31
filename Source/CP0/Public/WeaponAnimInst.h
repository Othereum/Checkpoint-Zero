// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "Animation/AnimInstance.h"
#include "WeaponAnimInst.generated.h"

/**
 * 
 */
UCLASS()
class CP0_API UWeaponAnimInst : public UAnimInstance
{
	GENERATED_BODY()
	
  private:
    void NativeUpdateAnimation(float DeltaSeconds) final;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true, UIMin = 0))
    float MoveSpeed;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsSprinting = false;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsOnGround = true;
};
