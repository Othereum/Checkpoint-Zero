// © 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CP0AnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class CP0_API UCP0AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	float MoveSpeed;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	float MoveDirection;
};
