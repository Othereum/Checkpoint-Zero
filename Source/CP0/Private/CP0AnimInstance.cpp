// © 2020 Seokjin Lee <seokjin.dev@gmail.com>


#include "CP0AnimInstance.h"

void UCP0AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (const auto Character = TryGetPawnOwner())
	{
		const auto Velocity = Character->GetVelocity();
		MoveSpeed = Velocity.Size();
		if (MoveSpeed > 1)
		{
			MoveDirection = CalculateDirection(Velocity, Character->GetActorRotation());
		}
	}
}
