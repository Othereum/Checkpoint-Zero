// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "Animation/AnimInstance.h"
#include "CP0AnimInstance.generated.h"

/**
 *
 */
UCLASS()
class CP0_API UCP0AnimInstance : public UAnimInstance
{
    GENERATED_BODY()

  public:
    void NativeUpdateAnimation(float DeltaSeconds) override;

  private:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    float MoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    float MoveDirection;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsOnGround = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsSprinting = false;
};
