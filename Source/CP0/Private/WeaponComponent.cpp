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
		SetAnimInstanceClass(Weapon->GetArmsAnimClass());
	}
}

void UWeaponComponent::UpdateTransform(float DeltaTime)
{
	const auto Owner = GetCharOwner();
	if (!Owner->IsLocallyControlled())
		return;

	const auto AimRot = Owner->GetBaseAimRotation().GetNormalized();
	auto Diff = (PrevAimRot - AimRot).GetNormalized();
	Diff.Yaw *= 1.0f - FMath::Abs(AimRot.Pitch) / 90.0f;

	AimRotSpeed = FMath::RInterpTo(AimRotSpeed, Diff, DeltaTime, 10.0f);
	PrevAimRot = AimRot;
	LocalOffset.SetRotation(FMath::QInterpTo(LocalOffset.GetRotation(), AimRotSpeed.Quaternion().Inverse(), DeltaTime,
	                                         10.0f));

	auto NewTF = GetDefaultSelf()->GetRelativeTransform();
	if (Weapon)
		NewTF *= Weapon->ArmsOffset;
	NewTF *= LocalOffset;
	NewTF *= {AimRot, Owner->GetPawnViewLocation()};
	SetWorldTransform(NewTF);
}

void UWeaponComponent::SetWeapon(AWeapon* NewWeapon)
{
	if (Weapon)
	{
		Weapon->SetOwner(nullptr);
		Weapon->SetInstigator(nullptr);
	}

	Weapon = NewWeapon;
	SetAnimInstanceClass(Weapon->GetArmsAnimClass());

	Weapon->AttachToComponent(this, {EAttachmentRule::KeepRelative, true}, TEXT("R_GunSocket"));
	Weapon->SetActorRelativeTransform(FTransform::Identity);

	const auto Owner = GetCharOwner();
	Weapon->SetOwner(Owner);
	Weapon->SetInstigator(Owner);
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
		Weapon->SwitchFiremode();
}
