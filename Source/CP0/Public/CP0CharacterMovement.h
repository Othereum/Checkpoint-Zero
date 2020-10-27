// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "CP0CharacterMovement.generated.h"

class ACP0Character;

/**
 *
 */
UCLASS()
class CP0_API UCP0CharacterMovement final : public UCharacterMovementComponent
{
    GENERATED_BODY()

  public:
    UCP0CharacterMovement();

  protected:
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  private:
    UPROPERTY(Replicated, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsSprinting;
};

struct CP0_API FSprintAction
{
    [[nodiscard]] static UCP0CharacterMovement* GetObject(const ACP0Character* Character);
    static void Enable(UCP0CharacterMovement* Movement);
    static void Disable(UCP0CharacterMovement* Movement);
    static void Toggle(UCP0CharacterMovement* Movement);
};
