// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "WeaponAnimInstance.h"
#include "Weapon.h"

void UWeaponAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const auto Weapon = Cast<AWeapon>(GetOwningActor());
	if (!Weapon)
		return;

	FireMode = Weapon->GetFireMode();
}
