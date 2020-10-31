// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CoreMinimal.h"
#include "CP0.generated.h"

#define ensureNoEntry() ensure(!"Enclosing block should never be called")

UENUM(BlueprintType)
enum class EPosture : uint8
{
    Stand,
    Crouch,
    Prone
};
