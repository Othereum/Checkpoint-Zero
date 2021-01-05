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
	void UpdateCamera(float DeltaTime) override;
};
