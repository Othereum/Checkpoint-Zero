// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "CP0CharacterMovement.generated.h"

UENUM(BlueprintType)
enum class ESprintSpeed : uint8
{
    Absolute,
    Relative,
    Multiply
};

UENUM(BlueprintType)
enum class EPosture : uint8
{
    Stand,
    Crouch,
    Prone
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

    float GetMaxSpeed() const override;
    float GetSprintSpeed() const;

    EPosture GetPosture() const { return Posture; }

    bool IsSprinting() const { return bIsSprinting; }
    bool CanSprint() const;
    bool TryStartSprint();
    void StopSprint() { bIsSprinting = false; }

  protected:
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  private:
    void ProcessSprint();

    UPROPERTY(EditAnywhere, meta = (UIMin = 0))
    float SprintSpeed = 600.0f;

    UPROPERTY(EditAnywhere, meta = (UIMin = -1, UIMax = 1))
    float MaxSprintAngleCos = 0.1f;

    UPROPERTY(EditAnywhere, meta = (UIMin = 0))
    float MinSprintSpeed = 10.0f;

    UPROPERTY(EditAnywhere)
    ESprintSpeed SprintSpeedType = ESprintSpeed::Absolute;

    UPROPERTY(Replicated, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    EPosture Posture = EPosture::Stand;

    UPROPERTY(Replicated, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsSprinting = false;
};

struct CP0_API FMovementActionBase
{
    [[nodiscard]] static UCP0CharacterMovement* GetObject(class ACP0Character* Character);
};

struct CP0_API FSprintAction : FMovementActionBase
{
    static void Enable(UCP0CharacterMovement* Movement);
    static void Disable(UCP0CharacterMovement* Movement);
    static void Toggle(UCP0CharacterMovement* Movement);
};

struct CP0_API FCrouchAction : FMovementActionBase
{
    static void Enable(UCP0CharacterMovement* Movement);
    static void Disable(UCP0CharacterMovement* Movement);
    static void Toggle(UCP0CharacterMovement* Movement);
};

struct CP0_API FProneAction : FMovementActionBase
{
    static void Enable(UCP0CharacterMovement* Movement);
    static void Disable(UCP0CharacterMovement* Movement);
    static void Toggle(UCP0CharacterMovement* Movement);
};
