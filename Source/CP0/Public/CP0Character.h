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

UCLASS()
class CP0_API ACP0Character final : public ACharacter
{
    GENERATED_BODY()

  public:
    ACP0Character(const FObjectInitializer& Initializer);
    [[nodiscard]] UCP0CharacterMovement* GetCP0Movement() const;

  protected:
    void BeginPlay() override;
    void Tick(float DeltaTime) override;
    void SetupPlayerInputComponent(UInputComponent* InputComp) override;

  private:
    void MoveForward(float AxisValue);
    void MoveRight(float AxisValue);
    void Turn(float AxisValue);
    void LookUp(float AxisValue);

    template <class Action>
    void BindInputAction(UInputComponent* Input, FName Name);
    void DispatchInputAction(FName Name, EInputAction Type);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerInputAction(FName Name, EInputAction Type);

    TMap<FName, void (*)(ACP0Character*, EInputAction)> InputActionMap;
};
