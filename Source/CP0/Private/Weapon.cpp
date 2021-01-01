// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "Weapon.h"
#include "CP0Character.h"
#include "CP0CharacterMovement.h"
#include "Net/UnrealNetwork.h"
#include "WeaponComponent.h"

AWeapon::AWeapon() : Mesh{CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"))}
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = Mesh;
}

ACP0Character* AWeapon::GetCharOwner() const
{
    return CastChecked<ACP0Character>(GetOwner(), ECastCheckedType::NullAllowed);
}

UWeaponComponent* AWeapon::GetWeaponComp() const
{
    const auto Char = GetCharOwner();
    return Char ? Char->GetWeaponComp() : nullptr;
}

bool AWeapon::TrySetFiring(bool bNewFiring)
{
    if (bFiring == bNewFiring)
        return true;

    if (bNewFiring && !CanFire())
        return false;

    bFiring = bNewFiring;
    return true;
}

bool AWeapon::CanFire() const
{
    if (State != EWeaponState::Ready)
        return false;

    const auto Char = GetCharOwner();
    if (!Char)
        return false;

    const auto WepComp = Char->GetWeaponComp();
    if (WepComp->GetWeapon() != this)
        return false;

    const auto Movement = Char->GetCP0Movement();
    if (Movement->IsSprinting())
        return false;

    return true;
}

bool AWeapon::TrySetAiming(bool bNewAiming)
{
    bAiming = bNewAiming;
    return true;
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();
    FireLag = 60.0f / RPM;
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    const auto FireDelay = 60.0f / RPM;
    FireLag += DeltaTime;

    if (bFiring)
    {
        if (CanFire())
        {
            while (FireLag >= FireDelay)
            {
                FireLag -= FireDelay;
                Fire();

                if (FireMode == EWeaponFireMode::SemiAuto || Clip <= 0)
                {
                    bFiring = false;
                    break;
                }
            }
        }
        else
        {
            bFiring = false;
        }
    }
    else
    {
        FireLag = FMath::Min(FireLag, FireDelay);
    }
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWeapon, Clip);
    DOREPLIFETIME(AWeapon, FireMode);
    DOREPLIFETIME(AWeapon, State);
}

void AWeapon::Fire()
{
    if (Clip > 0)
    {
        --Clip;
        OnFire();
    }
    else
    {
        OnDryFire();
    }
}
