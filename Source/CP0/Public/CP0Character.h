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
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  private:
    friend struct FSprintAction;

    void MoveForward(float AxisValue);
    void MoveRight(float AxisValue);
    void Turn(float AxisValue);
    void LookUp(float AxisValue);

    template <class Action>
    void RegisterInputAction(FName Name);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerInputAction(FName Name, EInputAction Type);

    void RegisterInputActions();
    void DispatchInputAction(FName Name, EInputAction Type);
    void BindInputAction(UInputComponent* Input, FName Name);

    TMap<FName, void (*)(ACP0Character*, EInputAction)> InputActionMap;

    UPROPERTY(Replicated, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bIsSprinting;
};
