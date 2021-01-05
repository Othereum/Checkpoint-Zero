// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "WeaponComponent.h"
#include "CP0Character.h"
#include "Net/UnrealNetwork.h"
#include "Weapon.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

ACP0Character* UWeaponComponent::GetCharOwner() const
{
	return CastChecked<ACP0Character>(GetOwner(), ECastCheckedType::NullAllowed);
}

const UWeaponComponent* UWeaponComponent::GetDefaultSelf() const
{
	return GetDefault<ACP0Character>(GetOwner()->GetClass())->GetWeaponComp();
}

void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponComponent, Weapon);
}

void UWeaponComponent::OnRep_Weapon()
{
	if (Weapon)
	{
		Weapon->Deploy(GetCharOwner());
	}
}

void UWeaponComponent::EquipWeapon(AWeapon* NewWeapon)
{
	if (Weapon)
	{
		Weapon->Holster(NewWeapon);
	}
	else
	{
		Weapon = NewWeapon;
		Weapon->Deploy(GetCharOwner());
	}
}

void FInputAction_Fire::Enable(ACP0Character* Character)
{
	if (const auto Weapon = Character->GetWeaponComp()->GetWeapon())
		Weapon->StartFiring();
}

void FInputAction_Fire::Disable(ACP0Character* Character)
{
	if (const auto Weapon = Character->GetWeaponComp()->GetWeapon())
		Weapon->StopFiring();
}

void FInputAction_Aim::Enable(ACP0Character* Character)
{
	if (const auto Weapon = Character->GetWeaponComp()->GetWeapon())
		Weapon->SetAiming(true);
}

void FInputAction_Aim::Disable(ACP0Character* Character)
{
	if (const auto Weapon = Character->GetWeaponComp()->GetWeapon())
		Weapon->SetAiming(false);
}

void FInputAction_Aim::Toggle(ACP0Character* Character)
{
	if (const auto Weapon = Character->GetWeaponComp()->GetWeapon())
		Weapon->SetAiming(!Weapon->IsAiming());
}

void FInputAction_Reload::Enable(ACP0Character* Character)
{
	if (const auto Weapon = Character->GetWeaponComp()->GetWeapon())
		Weapon->Reload();
}

void FInputAction_SwitchFiremode::Enable(ACP0Character* Character)
{
	if (const auto Weapon = Character->GetWeaponComp()->GetWeapon())
		Weapon->SwitchFireMode();
}
