// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CP0.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CP0CharacterMovement.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostureChanged, EPosture, Prev, EPosture, New);

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

    float GetMaxSpeed() const override;
    float GetSprintSpeed() const;

    bool IsSprinting() const { return bIsSprinting; }
    bool CanSprint() const;
    bool TryStartSprint();
    void StopSprint() { bIsSprinting = false; }

    EPosture GetPosture() const { return Posture; }
    bool TryStand();
    bool TryCrouch();
    bool TryProne();

  protected:
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  private:
    void ProcessSprint();

    UFUNCTION()
    void OnRep_Posture(EPosture Prev);

    UPROPERTY(BlueprintAssignable)
    FOnPostureChanged OnPostureChanged;

    UPROPERTY(EditAnywhere, meta = (UIMin = 0))
    float CrouchSpeed = 200.0f;

    UPROPERTY(EditAnywhere, meta = (UIMin = 0))
    float ProneSpeed = 100.0f;

    UPROPERTY(EditAnywhere, meta = (UIMin = 0))
    float SprintSpeed = 600.0f;

    UPROPERTY(EditAnywhere, meta = (UIMin = -1, UIMax = 1))
    float MaxSprintAngleCos = 0.1f;

    UPROPERTY(EditAnywhere, meta = (UIMin = 0))
    float MinSprintSpeed = 10.0f;

    UPROPERTY(EditAnywhere)
    ESprintSpeed SprintSpeedType = ESprintSpeed::Absolute;

    UPROPERTY(ReplicatedUsing = OnRep_Posture, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
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
