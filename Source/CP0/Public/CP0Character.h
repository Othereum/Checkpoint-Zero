// © 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CP0Character.generated.h"

UCLASS()
class CP0_API ACP0Character : public ACharacter
{
	GENERATED_BODY()

public:
	ACP0Character();

protected:
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);

	void SprintPressed();
	void SprintReleased();

	bool bWantsSprint;
};
