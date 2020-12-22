// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "Animation/AnimInstance.h"
#include "CP0.h"
#include "CP0AnimInstance.generated.h"

/**
 *
 */
UCLASS()
class CP0_API UCP0AnimInstance final : public UAnimInstance
{
    GENERATED_BODY()

  private:
    void NativeUpdateAnimation(float DeltaSeconds) final;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    FRotator MeshRotOffset;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true, UIMin = 0))
    float MoveSpeed;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly,
              meta = (AllowPrivateAccess = true, UIMin = -180, UIMax = 180))
    float MoveDirection;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly,
              meta = (AllowPrivateAccess = true, UIMin = -90, UIMax = 90))
    float AimPitch;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly,
              meta = (AllowPrivateAccess = true, UIMin = -180, UIMax = 180))
    float AimYaw;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly,
              meta = (AllowPrivateAccess = true, UIMin = -180, UIMax = 180))
    float YawSpeed;
    float PrevYaw;
    float YawCalcLag;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    EPosture Posture = EPosture::Stand;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsSprinting = false;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsOnGround = true;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bShouldPlayPostureAnim = true;
};
