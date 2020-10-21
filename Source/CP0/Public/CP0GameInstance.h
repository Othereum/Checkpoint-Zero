// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "Engine/GameInstance.h"
#include "CP0GameInstance.generated.h"

class UCP0InputSettings;

/**
 *
 */
UCLASS()
class CP0_API UCP0GameInstance final : public UGameInstance
{
    GENERATED_BODY()

  public:
    UCP0GameInstance();

    UCP0InputSettings* GetInputSettings() const
    {
        return InputSettings;
    }

  private:
    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    UCP0InputSettings* InputSettings;
};
