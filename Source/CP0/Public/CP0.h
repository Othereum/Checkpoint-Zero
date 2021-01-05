// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CoreMinimal.h"
#include "CP0.generated.h"

UENUM(BlueprintType)
enum class EPosture : uint8
{
	Stand,
	Crouch,
	Prone
};

UENUM(BlueprintType, meta = (Bitflags))
enum class EWeaponFireMode : uint8
{
	SemiAuto,
	Burst,
	FullAuto
};
