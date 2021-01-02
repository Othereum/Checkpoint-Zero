// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CP0.h"
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

enum ESetPostureCheckLevel
{
    SPCL_Correction,
    SPCL_ClientSimulation,
    SPCL_IgnoreDelay,
    SPCL_CheckAll,
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

    float GetMaxSpeed() const override;
    float GetMaxAcceleration() const override;
    bool CanAttemptJump() const override;

    bool IsInSprintMode() const { return bSprinting; }
    bool IsActuallySprinting() const;
    bool CanSprint(bool bIgnorePosture = false) const;
    bool TryStartSprint();
    void StopSprint() { bSprinting = false; }
    float GetLastActualSprintTime() const { return LastActualSprintTime; }

    bool TrySetPosture(EPosture New, ESetPostureCheckLevel CheckLevel = SPCL_CheckAll);
    EPosture GetPosture() const { return Posture; }
    bool IsPostureSwitching() const;
    bool IsProneSwitching() const;
    float GetPostureSwitchTime(EPosture Prev, EPosture New) const;
    float GetDefaultHalfHeight(EPosture P) const;

    bool IsWalkingSlow() const { return bWalkingSlow; }
    bool CanWalkSlow() const;
    bool TryStartWalkingSlow();
    void StopWalkingSlow() { bWalkingSlow = false; }

    float CalcFloorPitch() const;
    float GetMeshPitchOffset() const { return MeshPitchOffset; }

  protected:
    void BeginPlay() override;
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
    void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;
    bool DoJump(bool bReplayingMoves) override;

  private:
    float CurTime() const;
    const UCP0CharacterMovement* GetDefaultSelf() const;

    void ProcessSprint();
    void ProcessSlowWalk();
    void ProcessForceTurn();
    void ProcessPronePush();
    void ProcessPronePitch(float DeltaTime);
    void UpdateRotationRate();
    void UpdateViewPitchLimit(float DeltaTime);

    bool TrySetPosture_Impl(EPosture New, ESetPostureCheckLevel CheckLevel);
    void ShrinkPerchRadius();

    UFUNCTION(Client, Reliable)
    void ClientCorrectPosture(EPosture Prev, EPosture Cur);

    UFUNCTION()
    void OnRep_Posture(EPosture Prev);

    FVector ForceInput{0.0f};
    float NextPostureSwitch = 0.0f;
    float MeshPitchOffset = 0.0f;
    
    float LastActualSprintTime;

    UPROPERTY(EditAnywhere)
    TEnumAsByte<ECollisionChannel> PushTraceChannel;

    UPROPERTY(ReplicatedUsing = OnRep_Posture, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    EPosture Posture = EPosture::Stand;
    EPosture PrevPosture = EPosture::Stand;

    UPROPERTY(Replicated, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bSprinting = false;
    bool bWalkingSlow = false;
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
