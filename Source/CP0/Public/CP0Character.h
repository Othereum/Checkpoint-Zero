// © 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "GameFramework/Character.h"
#include "CP0Character.generated.h"

class UCP0CharacterMovement;

UCLASS()
class CP0_API ACP0Character final : public ACharacter
{
    GENERATED_BODY()

  public:
    ACP0Character(const FObjectInitializer& ObjectInitializer);

    [[nodiscard]] auto GetCP0CharacterMovement() const
    {
        return CastChecked<UCP0CharacterMovement>(GetCharacterMovement());
    }

  protected:
    void BeginPlay() override;
    void Tick(float DeltaTime) override;
    void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

  private:
    void MoveForward(float AxisValue);
    void MoveRight(float AxisValue);
    void Turn(float AxisValue);
    void LookUp(float AxisValue);
};
