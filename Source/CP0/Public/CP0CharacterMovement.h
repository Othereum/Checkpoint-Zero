// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CP0.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CP0CharacterMovement.generated.h"

class ACP0Character;

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
    ACP0Character* GetCP0Owner() const;

    float GetMaxSpeed() const final;
    float GetMaxAcceleration() const final;

    bool IsSprinting() const { return bIsSprinting; }
    bool CanSprint() const;
    bool TryStartSprint();
    void StopSprint() { bIsSprinting = false; }

    bool TrySetPosture(EPosture New);
    EPosture GetPosture() const { return Posture; }
    bool IsPostureSwitching() const;
    bool IsProneSwitching() const;
    float GetPostureSwitchTime(EPosture Prev, EPosture New) const;
    float GetDefaultHalfHeight(EPosture P) const;

  protected:
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  private:
    float CurTime() const;

    void ProcessSprint();
    void ProcessProne();

    UFUNCTION()
    void OnRep_Posture(EPosture Prev);

    UPROPERTY(BlueprintAssignable)
    FOnPostureChanged OnPostureChanged;

    float NextPostureSwitch = 0.0f;

    UPROPERTY(EditAnywhere)
    TEnumAsByte<ECollisionChannel> ProneTraceChannel;

    UPROPERTY(ReplicatedUsing = OnRep_Posture, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    EPosture Posture = EPosture::Stand;
    EPosture PrevPosture = EPosture::Stand;

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
