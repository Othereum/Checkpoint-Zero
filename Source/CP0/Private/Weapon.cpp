// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "Weapon.h"
#include "CP0Character.h"
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

bool AWeapon::IsFiring() const
{
    const auto Comp = GetWeaponComp();
    return Comp && Comp->bFiring;
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();
    FireLag = 60.0f / RPM;
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    const auto Comp = GetWeaponComp();

    const auto FireDelay = 60.0f / RPM;
    FireLag += DeltaTime;

    if (Comp && Comp->bFiring)
    {
        while (FireLag >= FireDelay)
        {
            FireLag -= FireDelay;
            Fire();

            if (!bAutomatic)
            {
                Comp->bFiring = false;
                break;
            }
        }
    }
    else
    {
        FireLag = FMath::Min(FireLag, FireDelay);
    }
}

void AWeapon::Fire()
{
    OnFire();
}
