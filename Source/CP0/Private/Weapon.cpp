// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "Weapon.h"
#include "CP0Character.h"
#include "CP0CharacterMovement.h"
#include "GameFramework/PlayerState.h"
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
		SetState(EWeaponState::Firing);
	}
}

void AWeapon::StopFiring()
{
	if (State == EWeaponState::Firing && FireMode != EWeaponFireMode::Burst)
	{
		SetState(EWeaponState::Idle);
	}
}

void AWeapon::SetAiming(bool bNewAiming)
{
	bAiming = bNewAiming;
	bAiming_LastModified = GetWorld()->GetRealTimeSeconds();
}

void AWeapon::Reload()
{
	if (State == EWeaponState::Idle && CanDoCommonAction())
	{
		SetState(EWeaponState::Reloading);
	}
}

void AWeapon::SwitchFiremode()
{
	if (FireModes && State == EWeaponState::Idle && CanDoCommonAction())
	{
		auto NewFM = static_cast<uint8>(FireMode);
		do
		{
			NewFM = (NewFM + 1) % 3;
		}
		while (!(FireModes & (1 << NewFM)));

		const auto OldFM = FireMode;
		SetFireMode(static_cast<EWeaponFireMode>(NewFM));

		if (OldFM != FireMode)
			OnFiremodeSwitched();
	}
}

bool AWeapon::CanFire() const
{
	return CanDoCommonAction();
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetOwner())
		return;

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

	CorrectClientState();
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AWeapon, Clip, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWeapon, FireMode, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWeapon, bAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWeapon, State, COND_SkipOwner);
}

void AWeapon::Tick_Idle(float DeltaTime)
{
	FireLag = FMath::Min(FireLag + DeltaTime, GetFireDelay());
}

void AWeapon::Tick_Firing(float DeltaTime)
{
	if (!CanFire())
	{
		SetState(EWeaponState::Idle);
		return;
	}

	FireLag += DeltaTime;

	const auto FireDelay = GetFireDelay();
	while (FireLag >= FireDelay)
	{
		FireLag -= FireDelay;
		if (!Fire())
		{
			SetState(EWeaponState::Idle);
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
		CurrentReloadTime = 0.0f;
		SetClip(ClipSize + bTactical);
		SetState(EWeaponState::Idle);
	}
}

void AWeapon::Tick_Deploying(float DeltaTime)
{
}

void AWeapon::Tick_Holstering(float DeltaTime)
{
}

void AWeapon::Enter_Idle()
{
}

void AWeapon::Enter_Firing()
{
	const auto FireDelay = GetFireDelay();
	if (FireLag >= FireDelay)
	{
		FireLag -= FireDelay;

		if (!Fire())
			SetState(EWeaponState::Idle);
	}
}

void AWeapon::Enter_Reloading()
{
	OnReloadStart(Clip <= 0);
}

void AWeapon::Enter_Deploying()
{
}

void AWeapon::Enter_Holstering()
{
}

void AWeapon::Exit_Idle()
{
}

void AWeapon::Exit_Firing()
{
	CurBurstCount = 0;
}

void AWeapon::Exit_Reloading()
{
	if (CurrentReloadTime != 0.0f)
	{
		CurrentReloadTime = 0.0f;
		OnReloadCancelled();
	}
}

void AWeapon::Exit_Deploying()
{
}

void AWeapon::Exit_Holstering()
{
}

bool AWeapon::Fire()
{
	if (Clip == 0)
	{
		OnDryFire();
		return false;
	}

	SetClip(Clip - 1);

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

void AWeapon::SetClip(uint8 NewClip)
{
	Clip = NewClip;
	Clip_LastModified = GetWorld()->GetRealTimeSeconds();
}

void AWeapon::SetState(EWeaponState NewState)
{
	switch (State)
	{
	case EWeaponState::Idle:
		Exit_Idle();
		break;
	case EWeaponState::Firing:
		Exit_Firing();
		break;
	case EWeaponState::Reloading:
		Exit_Reloading();
		break;
	case EWeaponState::Deploying:
		Exit_Deploying();
		break;
	case EWeaponState::Holstering:
		Exit_Holstering();
		break;
	}

	State = NewState;
	State_LastModified = GetWorld()->GetRealTimeSeconds();

	switch (State)
	{
	case EWeaponState::Idle:
		Enter_Idle();
		break;
	case EWeaponState::Firing:
		Enter_Firing();
		break;
	case EWeaponState::Reloading:
		Enter_Reloading();
		break;
	case EWeaponState::Deploying:
		Enter_Deploying();
		break;
	case EWeaponState::Holstering:
		Enter_Holstering();
		break;
	}
}

void AWeapon::SetFireMode(EWeaponFireMode NewFM)
{
	FireMode = NewFM;
	FireMode_LastModified = GetWorld()->GetRealTimeSeconds();
}

void AWeapon::CorrectClientState()
{
	if (GetOwner()->GetRemoteRole() != ROLE_AutonomousProxy)
		return;

	const auto Now = GetWorld()->GetRealTimeSeconds();
	if (NextCorrection <= Now)
	{
		Client_CorrectState({Clip, FireMode, State, bAiming});
		NextCorrection = Now + 0.5f;
	}
}

void AWeapon::Client_CorrectState_Implementation(FWeaponCorrectionData Data)
{
	const auto Char = GetCharOwner();
	if (!Char)
		return;

	const auto PS = Char->GetPlayerState();
	if (!PS)
		return;

	const auto PingMs = PS->GetPing() * 4.0f;
	const auto Now = GetWorld()->GetRealTimeSeconds();

	auto IsExpired = [&](float LastModified) { return (Now - LastModified) * 1000.0f >= PingMs; };

	if (Clip != Data.Clip && IsExpired(Clip_LastModified))
		Clip = Data.Clip;

	if (FireMode != Data.FireMode && IsExpired(FireMode_LastModified))
		FireMode = Data.FireMode;

	if (State != Data.State && IsExpired(State_LastModified))
		if (!(State == EWeaponState::Firing && Data.State == EWeaponState::Idle) && Data.State != EWeaponState::Firing)
			SetState(Data.State);

	if (bAiming != Data.bAiming && IsExpired(bAiming_LastModified))
		bAiming = Data.bAiming;
}
