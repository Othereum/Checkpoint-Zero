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

void AWeapon::StartFiring()
{
    if (State == EWeaponState::Idle && CanFire())
        State = EWeaponState::Firing;
}

void AWeapon::StopFiring()
{
    if (State == EWeaponState::Firing)
        State = EWeaponState::Idle;
}

void AWeapon::SetAiming(bool bNewAiming)
{
    bAiming = bNewAiming;
}

void AWeapon::Reload()
{
    if (State == EWeaponState::Idle)
    {
        State = EWeaponState::Reloading;
        CurrentReloadTime = 0.0f;
        OnReloadStart(Clip <= 0);
    }
}

bool AWeapon::CanFire() const
{
    const auto Char = GetCharOwner();
    if (!Char)
        return false;

    if (Char->GetWeaponComp()->GetWeapon() != this)
        return false;

    if (Char->GetCP0Movement()->IsSprinting())
        return false;

    return true;
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();
    FireLag = GetFireDelay();
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    switch (State)
    {
    case EWeaponState::Idle:
        Tick_Idle(DeltaTime);
        break;
    case EWeaponState::Firing:
        Tick_Firing(DeltaTime);
        break;
    case EWeaponState::Reloading:
        Tick_Reloading(DeltaTime);
        break;
    case EWeaponState::Deploying:
        Tick_Deploying(DeltaTime);
        break;
    case EWeaponState::Holstering:
        Tick_Holstering(DeltaTime);
        break;
    default:
        break;
    }
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWeapon, Clip);
    DOREPLIFETIME(AWeapon, FireMode);
    DOREPLIFETIME(AWeapon, State);
    DOREPLIFETIME(AWeapon, bAiming);
}

void AWeapon::Tick_Idle(float DeltaTime)
{
    FireLag = FMath::Min(FireLag + DeltaTime, GetFireDelay());
}

void AWeapon::Tick_Firing(float DeltaTime)
{
    if (!CanFire())
    {
        State = EWeaponState::Idle;
        return;
    }

    FireLag += DeltaTime;

    const auto FireDelay = GetFireDelay();
    while (FireLag >= FireDelay)
    {
        FireLag -= FireDelay;
        Fire();

        if (FireMode == EWeaponFireMode::SemiAuto || Clip <= 0)
        {
            State = EWeaponState::Idle;
            break;
        }
    }
}

void AWeapon::Tick_Reloading(float DeltaTime)
{
    CurrentReloadTime += DeltaTime;
    const auto bTactical = Clip > 0;
    const auto ReloadTime = bTactical ? ReloadTime_Tactical : ReloadTime_Empty;
    if (CurrentReloadTime >= ReloadTime)
    {
        Clip = ClipSize + bTactical;
        CurrentReloadTime = 0.0f;
        State = EWeaponState::Idle;
    }
}

void AWeapon::Tick_Deploying(float DeltaTime)
{
}

void AWeapon::Tick_Holstering(float DeltaTime)
{
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
