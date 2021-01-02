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
    {
        State = EWeaponState::Firing;
    }
}

void AWeapon::StopFiring(bool bForce)
{
    if (State == EWeaponState::Firing && (bForce || FireMode != EWeaponFireMode::Burst))
    {
        State = EWeaponState::Idle;
        CurBurstCount = 0;
    }
}

void AWeapon::SetAiming(bool bNewAiming)
{
    bAiming = bNewAiming;
}

void AWeapon::Reload()
{
    if (State != EWeaponState::Idle || !CanDoCommonAction())
        return;

    State = EWeaponState::Reloading;
    CurrentReloadTime = 0.0f;
    OnReloadStart(Clip <= 0);
}

void AWeapon::SwitchFiremode()
{
    if (!FireModes || State != EWeaponState::Idle || !CanDoCommonAction())
        return;

    auto NewFM = static_cast<uint8>(FireMode);
    do
    {
        NewFM = (NewFM + 1) % 3;
    } while (!(FireModes & (1 << NewFM)));

    const auto OldFM = FireMode;
    FireMode = static_cast<EWeaponFireMode>(NewFM);

    if (OldFM != FireMode)
        OnFiremodeSwitched();
}

bool AWeapon::CanFire() const
{
    return CanDoCommonAction();
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
        StopFiring(true);
        return;
    }

    FireLag += DeltaTime;

    const auto FireDelay = GetFireDelay();
    while (FireLag >= FireDelay)
    {
        FireLag -= FireDelay;
        if (!Fire())
        {
            StopFiring(true);
            break;
        }
    }
}

void AWeapon::Tick_Reloading(float DeltaTime)
{
    const auto bTactical = Clip > 0;
    const auto ReloadTime = bTactical ? ReloadTime_Tactical : ReloadTime_Empty;
    CurrentReloadTime += DeltaTime;
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

bool AWeapon::Fire()
{
    if (Clip == 0)
    {
        OnDryFire();
        return false;
    }

    --Clip;

    const auto bBurst = FireMode == EWeaponFireMode::Burst;
    if (bBurst)
        ++CurBurstCount;

    OnFire();

    if (Clip == 0)
        return false;

    switch (FireMode)
    {
    case EWeaponFireMode::SemiAuto:
        return false;
    case EWeaponFireMode::Burst:
        return CurBurstCount < BurstCount;
    }

    return true;
}

bool AWeapon::CanDoCommonAction() const
{
    const auto Char = GetCharOwner();
    return Char && Char->GetWeaponComp()->GetWeapon() == this &&
           GetWorld()->GetTimeSeconds() - Char->GetCP0Movement()->GetLastActualSprintTime() >= 0.15f;
}

