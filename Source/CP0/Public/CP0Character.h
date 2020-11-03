// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CP0.h"
#include "GameFramework/Character.h"
#include "CP0Character.generated.h"

class UCP0CharacterMovement;

UENUM()
enum class EInputAction : uint8
{
    Enable,
    Disable,
    Toggle
};

UCLASS()
class CP0_API ACP0Character final : public ACharacter
{
    GENERATED_BODY()

  public:
    ACP0Character(const FObjectInitializer& Initializer);
    UCP0CharacterMovement* GetCP0Movement() const;

    void SetEyeHeightWithBlend(float NewEyeHeight, float BlendTime);
    float GetEyeHeight(EPosture Posture) const;
    void RecalculateBaseEyeHeight() override {}

  protected:
    void BeginPlay() override;
    void Tick(float DeltaTime) override;
    void SetupPlayerInputComponent(UInputComponent* InputComp) override;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  private:
    friend struct FSprintAction;

    void InterpEyeHeight(float DeltaTime);

    void RegisterInputActions();
    void DispatchInputAction(FName Name, EInputAction Type);
    void BindInputAction(UInputComponent* Input, FName Name);

    template <class Action>
    void RegisterInputAction(FName Name);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerInputAction(FName Name, EInputAction Type);

    void MoveForward(float AxisValue);
    void MoveRight(float AxisValue);
    void Turn(float AxisValue);
    void LookUp(float AxisValue);

    TMap<FName, void (*)(ACP0Character*, EInputAction)> InputActionMap;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float ProneEyeHeight = 0.0f;

    float TargetEyeHeight = BaseEyeHeight;
    float PrevEyeHeight = BaseEyeHeight;
    float EyeHeightAlpha = 1.0f;
    float EyeHeightBlendTime = 1.0f;
};
