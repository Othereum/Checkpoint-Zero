// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CP0.h"
#include "GameFramework/Character.h"
#include "CP0Character.generated.h"

class UCP0CharacterMovement;
class UWeaponComponent;

UENUM()
enum class EInputAction : uint8
{
    Enable,
    Disable,
    Toggle
};

UCLASS()
class CP0_API ACP0Character : public ACharacter
{
    GENERATED_BODY()

  public:
    ACP0Character(const FObjectInitializer& Initializer);
    UCP0CharacterMovement* GetCP0Movement() const;
    UWeaponComponent* GetWeaponComp() const { return WeaponComp; }

    void RecalculateBaseEyeHeight() override {}
    bool IsMoveInputIgnored() const override;
    FRotator GetBaseAimRotation() const override;
    FRotator GetViewRotation() const override;

    void SetRemoteViewYaw(float NewRemoteViewYaw);
    void SetEyeHeight(float NewEyeHeight);
    void SetEyeHeightWithBlend(float NewEyeHeight, float BlendTime);
    float GetDefaultEyeHeight(EPosture Posture) const;
    float GetEyeHeight() const;

	UFUNCTION(BlueprintImplementableEvent)
    void OnPostureChanged(EPosture PrevPosture, EPosture NewPosture);

  protected:
    void BeginPlay() override;
    void Tick(float DeltaTime) override;
    void SetupPlayerInputComponent(UInputComponent* InputComp) override;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

  private:
    friend UCP0CharacterMovement;

    void InterpEyeHeight(float DeltaTime);
    void UpdateLegsTransform();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerInputAction(FName Name, EInputAction Type);
    void DispatchInputAction(FName Name, EInputAction Type);
    void BindInputAction(UInputComponent* Input, FName Name);

    void MoveForward(float AxisValue);
    void MoveRight(float AxisValue);
    void Turn(float AxisValue);
    void LookUp(float AxisValue);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    UWeaponComponent* WeaponComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    USkeletalMeshComponent* LegsMesh;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float ProneEyeHeight = 35.0f;

    float TargetEyeHeight = 150.0f;
    float PrevEyeHeight = 150.0f;
    float EyeHeightAlpha = 1.0f;
    float EyeHeightBlendTime = 1.0f;

    UPROPERTY(Replicated, Transient)
    uint8 RemoteViewYaw;
};
