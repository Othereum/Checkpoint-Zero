// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

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

UCLASS() class CP0_API ACP0Character final : public ACharacter
{
    GENERATED_BODY()

  public:
    ACP0Character(const FObjectInitializer& initializer);
    [[nodiscard]] UCP0CharacterMovement* GetCP0Movement() const;

  protected:
    void BeginPlay() override;
    void Tick(float deltaTime) override;
    void SetupPlayerInputComponent(UInputComponent* inputComp) override;

  private:
    void MoveForward(float axisValue);
    void MoveRight(float axisValue);
    void Turn(float axisValue);
    void LookUp(float axisValue);

    template <class Action>
    void BindInputAction(UInputComponent* input, FName name);
    void DispatchInputAction(FName name, EInputAction type);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerInputAction(FName name, EInputAction type);

    TMap<FName, void (*)(ACP0Character*, EInputAction)> inputActionMap_;
};
