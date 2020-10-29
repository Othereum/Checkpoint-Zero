// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "CP0CharacterMovement.generated.h"

class ACP0Character;

UENUM(BlueprintType)
enum class ESprintSpeed : uint8
{
    Absolute,
    Relative,
    Multiply
};

/**
 *
 */
UCLASS()
class CP0_API UCP0CharacterMovement final : public UCharacterMovementComponent
{
    GENERATED_BODY()

  public:
    UCP0CharacterMovement();

    [[nodiscard]] float GetMaxSpeed() const override;
    [[nodiscard]] float GetSprintSpeed() const;

    [[nodiscard]] bool IsSprinting() const
    {
        return bIsSprinting;
    }

  protected:
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  private:
    friend struct FSprintAction;

    UPROPERTY(EditAnywhere, meta = (UIMin = 0))
    float SprintSpeed = 600.0f;

    UPROPERTY(EditAnywhere, meta = (UIMin = -1, UIMax = 1))
    float MaxSprintAngleCos = 0.5f;

    UPROPERTY(EditAnywhere, meta = (UIMin = 0))
    float MinSprintSpeed = 100.0f;

    UPROPERTY(EditAnywhere)
    ESprintSpeed SprintSpeedType = ESprintSpeed::Absolute;

    UPROPERTY(Replicated, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsSprinting = false;
};

struct CP0_API FSprintAction
{
    [[nodiscard]] static UCP0CharacterMovement* GetObject(ACP0Character* Character);
    static void Enable(UCP0CharacterMovement* Movement);
    static void Disable(UCP0CharacterMovement* Movement);
    static void Toggle(UCP0CharacterMovement* Movement);
};
