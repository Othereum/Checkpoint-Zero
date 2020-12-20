// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0PCM.h"

ACP0PCM::ACP0PCM()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ACP0PCM::SetPitchLimitWithBlend(float Min, float Max, float BlendTime)
{
    TargetPitchLimit = {Min, Max};
    PrevPitchLimit = {ViewPitchMin, ViewPitchMax};
    PitchLimitBlendTime = BlendTime;

    if (BlendTime > KINDA_SMALL_NUMBER)
    {
        PitchLimitAlpha = 0.0f;
    }
    else
    {
        PitchLimitAlpha = 1.0f;
        ViewPitchMin = Min;
        ViewPitchMax = Max;
    }
}

void ACP0PCM::BeginPlay()
{
    Super::BeginPlay();
    PrevPitchLimit = TargetPitchLimit = {ViewPitchMin, ViewPitchMax};
}

void ACP0PCM::Tick(float DeltaTime)
{
    InterpPitchLimit(DeltaTime);
    Super::Tick(DeltaTime);
}

void ACP0PCM::InterpPitchLimit(float DeltaTime)
{
    if (PitchLimitAlpha >= 1.0f)
        return;

    PitchLimitAlpha = FMath::Clamp(PitchLimitAlpha + DeltaTime / PitchLimitBlendTime, 0.0f, 1.0f);
    ViewPitchMin = FMath::CubicInterp(PrevPitchLimit.X, 0.0f, TargetPitchLimit.X, 0.0f, PitchLimitAlpha);
    ViewPitchMax = FMath::CubicInterp(PrevPitchLimit.Y, 0.0f, TargetPitchLimit.Y, 0.0f, PitchLimitAlpha);
}
