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

    UCP0CharacterMovement* GetCP0Movement() const;
    bool IsUpperBodyHidden() const { return bIsUpperBodyHidden; }

  protected:
    void BeginPlay() override;
    void Tick(float DeltaTime) override;
    void SetupPlayerInputComponent(UInputComponent* InputComp) override;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void PossessedBy(AController* NewController) override;
    void UnPossessed() override;

  private:
    friend struct FSprintAction;

    void UpdateLegs();

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

    UPROPERTY(EditAnywhere)
    FVector LegsOffset{-30.0f, 0.0f, 10.0f};
    TOptional<FVector> InitialMeshLocation;

    UPROPERTY(EditAnywhere)
    bool bHideUpperBodyForOwner = true;
    bool bIsUpperBodyHidden = false;
};
