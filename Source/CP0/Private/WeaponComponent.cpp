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

void UWeaponComponent::UpdateTransform(float DeltaTime)
{
    const auto Owner = GetCharOwner();
    const auto AimRot = Owner->GetBaseAimRotation().GetNormalized();
    auto Diff = (PrevAimRot - AimRot).GetNormalized();
    Diff.Yaw *= 1.0f - FMath::Abs(AimRot.Pitch) / 90.0f;

    AimRotSpeed = FMath::RInterpTo(AimRotSpeed, Diff, DeltaTime, 10.0f);
    PrevAimRot = AimRot;
    LocalOffset.SetRotation(FMath::QInterpTo(LocalOffset.GetRotation(), AimRotSpeed.Quaternion(), DeltaTime, 10.0f));

    auto NewTF = GetDefaultSelf()->GetRelativeTransform();
    if (Weapon)
        NewTF *= Weapon->ArmsOffset;
    NewTF *= LocalOffset;
    NewTF *= {AimRot, Owner->GetPawnViewLocation()};
    SetWorldTransform(NewTF);
}
