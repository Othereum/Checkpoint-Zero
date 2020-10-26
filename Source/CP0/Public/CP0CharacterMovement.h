// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "CP0CharacterMovement.generated.h"

class ACP0Character;

struct CP0_API FSprintAction
{
    static void Enable(ACP0Character* character);
    static void Disable(ACP0Character* character);
    static void Toggle(ACP0Character* character);
};

/**
 *
 */
UCLASS()
class CP0_API UCP0CharacterMovement final : public UCharacterMovementComponent
{
    GENERATED_BODY()

  public:
};
