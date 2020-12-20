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

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true, UIMin = 0))
    float MoveSpeed = 0.0f;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly,
              meta = (AllowPrivateAccess = true, UIMin = -180, UIMax = 180))
    float MoveDirection = 0.0f;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly,
              meta = (AllowPrivateAccess = true, UIMin = -90, UIMax = 90))
    float AimPitch = 0.0f;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly,
              meta = (AllowPrivateAccess = true, UIMin = -180, UIMax = 180))
    float AimYaw = 0.0f;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly,
              meta = (AllowPrivateAccess = true, UIMin = -180, UIMax = 180))
    float YawRotationSpeed = 0.0f;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly,
              meta = (AllowPrivateAccess = true, UIMin = -45, UIMax = 45))
    float MeshPitch = 0.0f;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    EPosture Posture = EPosture::Stand;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsSprinting = false;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsOnGround = true;

    UPROPERTY(EditInstanceOnly, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bShouldPlayPostureAnim = true;
};
