// © 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PTECharacter.generated.h"

UCLASS()
class PTE_API APTECharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APTECharacter();

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
