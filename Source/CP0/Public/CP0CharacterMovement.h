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
    bool CanSprint(bool bIgnorePosture = false) const;
    bool TryStartSprint();
    void StopSprint() { bIsSprinting = false; }

    bool TrySetPosture(EPosture New, bool bIgnoreDelay = false);
    EPosture GetPosture() const { return Posture; }
    bool IsPostureSwitching() const;
    bool IsProneSwitching() const;
    float GetPostureSwitchTime(EPosture Prev, EPosture New) const;
    float GetDefaultHalfHeight(EPosture P) const;

    bool IsWalkingSlow() const { return bIsWalkingSlow; }
    bool CanWalkSlow() const;
    bool TryStartWalkingSlow();
    void StopWalkingSlow() { bIsWalkingSlow = false; }

    float CalcFloorPitch() const;

  protected:
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) final;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const final;

  private:
    float CurTime() const;
    const UCP0CharacterMovement* GetDefaultSelf() const;

    void ProcessSprint();
    void ProcessSlowWalk();
    void ProcessTurn();
    void ProcessForceTurn();
    void ProcessPronePush();
    void ProcessPronePitch(float DeltaTime);
    void UpdateViewPitchLimit(float DeltaTime);

    UFUNCTION()
    void OnRep_Posture(EPosture Prev);

    UPROPERTY(BlueprintAssignable)
    FOnPostureChanged OnPostureChanged;

    FVector ForceInput{0.0f};
    float NextPostureSwitch = 0.0f;

    UPROPERTY(EditAnywhere)
    TEnumAsByte<ECollisionChannel> PushTraceChannel;

    UPROPERTY(ReplicatedUsing = OnRep_Posture, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    EPosture Posture = EPosture::Stand;
    EPosture PrevPosture = EPosture::Stand;

    UPROPERTY(Replicated, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsSprinting = false;
    bool bIsWalkingSlow = false;
};

struct CP0_API FInputAction_Sprint
{
    static void Enable(ACP0Character* Character);
    static void Disable(ACP0Character* Character);
    static void Toggle(ACP0Character* Character);
};

struct CP0_API FInputAction_Crouch
{
    static void Enable(ACP0Character* Character);
    static void Disable(ACP0Character* Character);
    static void Toggle(ACP0Character* Character);
};

struct CP0_API FInputAction_Prone
{
    static void Enable(ACP0Character* Character);
    static void Disable(ACP0Character* Character);
    static void Toggle(ACP0Character* Character);
};

struct CP0_API FInputAction_WalkSlow
{
    static void Enable(ACP0Character* Character);
    static void Disable(ACP0Character* Character);
    static void Toggle(ACP0Character* Character);
};
