// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "CP0PCM.generated.h"

/**
 *
 */
UCLASS()
class CP0_API ACP0PCM final : public APlayerCameraManager
{
    GENERATED_BODY()

  public:
    ACP0PCM();
    void SetPitchLimitWithBlend(float Min, float Max, float BlendTime);

  protected:
    void BeginPlay() final;
    void Tick(float DeltaTime) final;

  private:
    void InterpPitchLimit(float DeltaTime);

    FVector2D TargetPitchLimit;
    FVector2D PrevPitchLimit;
    float PitchLimitAlpha = 1.0f;
    float PitchLimitBlendTime = 1.0f;
};

