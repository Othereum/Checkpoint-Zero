// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "Weapon.h"
#include "CP0Character.h"
#include "CP0CharacterMovement.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "WeaponComponent.h"

AWeapon::AWeapon()
	: Mesh1P{CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"))},
	  Mesh3P{CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh3P"))}
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = Mesh1P;
	Mesh3P->SetupAttachment(Mesh1P);
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

void AWeapon::Deploy(ACP0Character* Char)
{
	SetOwner(Char);
	SetInstigator(Char);

	const FName GunSock = TEXT("R_GunSocket");
	const FAttachmentTransformRules Rules{EAttachmentRule::KeepRelative, true};
	Mesh1P->AttachToComponent(Char->GetArms(), Rules, GunSock);
	Mesh1P->SetRelativeTransform(FTransform::Identity);
	Mesh3P->AttachToComponent(Char->GetMesh(), Rules, GunSock);
	Mesh3P->SetRelativeTransform(FTransform::Identity);
	
	SetState(EWeaponState::Deploying);
	Char->GetArms()->SetAnimInstanceClass(ArmsAnimClass);
	
	OnDeploy();
}

void AWeapon::Holster(AWeapon* SwitchTo)
{
	SwitchingTo = SwitchTo;
	SetState(EWeaponState::Holstering);
	OnHolster();
}

void AWeapon::StartFiring()
{
	check(GetCharOwner() && GetCharOwner()->IsLocallyControlled());
	
	if (!bFiring && State == EWeaponState::Ready && CanDoCommonAction())
	{
		const auto RandSeed = FMath::Rand();
		BeginFiring(RandSeed);

		if (!HasAuthority() && GetCharOwner()->IsLocallyControlled())
			Server_StartFiring(RandSeed);
	}
}

void AWeapon::StopFiring()
{
	check(GetCharOwner() && GetCharOwner()->IsLocallyControlled());
	
	if (bFiring && FireMode != EWeaponFireMode::Burst)
	{
		EndFiring();

		if (!HasAuthority() && GetCharOwner()->IsLocallyControlled())
			Server_StopFiring();
	}
}

void AWeapon::SetAiming(bool bNewAiming)
{
	bAiming = bNewAiming;
	Aiming_LastModified = GetWorld()->GetRealTimeSeconds();
}

void AWeapon::Reload()
{
	if (State == EWeaponState::Ready && CanDoCommonAction())
	{
		SetState(EWeaponState::Reloading);
	}
}

void AWeapon::SwitchFireMode()
{
	if (FireModes && State == EWeaponState::Ready && CanDoCommonAction())
	{
		auto NewFm = static_cast<uint8>(FireMode);
		do
		{
			NewFm = (NewFm + 1) % 3;
		}
		while (!(FireModes & 1 << NewFm));

		const auto OldFm = FireMode;
		SetFireMode(static_cast<EWeaponFireMode>(NewFm));

		if (OldFm != FireMode)
			OnFireModeSwitched();
	}
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
	case EWeaponState::Ready:
		Tick_Ready(DeltaTime);
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
	DOREPLIFETIME_CONDITION(AWeapon, FireMode, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWeapon, State, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWeapon, bAiming, COND_SkipOwner);
}

void AWeapon::PlayMontage(UAnimMontage* Montage) const
{
	if (Montage)
	{
		Mesh1P->GetAnimInstance()->Montage_Play(Montage);
		Mesh3P->GetAnimInstance()->Montage_Play(Montage);
	}
}

void AWeapon::StopMontage(float BlendOutTime, UAnimMontage* Montage) const
{
	Mesh1P->GetAnimInstance()->Montage_Stop(BlendOutTime, Montage);
	Mesh3P->GetAnimInstance()->Montage_Stop(BlendOutTime, Montage);
}

void AWeapon::Tick_Ready(float DeltaTime)
{
	if (bFiring)
	{
		if (State != EWeaponState::Ready || !CanDoCommonAction())
		{
			EndFiring();
		}
		else
		{
			FireLag += DeltaTime;

			const auto FireDelay = GetFireDelay();
			while (FireLag >= FireDelay)
			{
				FireLag -= FireDelay;
				if (!Fire())
				{
					EndFiring();
					break;
				}
			}
		}
	}
	else
	{
		FireLag = FMath::Min(FireLag + DeltaTime, GetFireDelay());
	}
}

void AWeapon::Tick_Reloading(float DeltaTime)
{
	const auto bTactical = Clip > 0;
	const auto ReloadTime = bTactical ? ReloadTime_Tactical : ReloadTime_Empty;
	LastStateElapsedTime += DeltaTime;
	if (LastStateElapsedTime >= ReloadTime)
	{
		SetClip(ClipSize + bTactical);
		SetState(EWeaponState::Ready);
	}
}

void AWeapon::Tick_Deploying(float DeltaTime)
{
	LastStateElapsedTime += DeltaTime;
	if (LastStateElapsedTime >= DeployTime)
	{
		SetState(EWeaponState::Ready);
	}
}

void AWeapon::Tick_Holstering(float DeltaTime)
{
	LastStateElapsedTime += DeltaTime;
	if (LastStateElapsedTime >= HolsterTime)
	{
		if (HasAuthority())
		{
			const auto WepComp = GetWeaponComp();
			WepComp->Weapon = SwitchingTo;
			if (WepComp->Weapon)
			{
				WepComp->Weapon->Deploy(GetCharOwner());
			}
			SwitchingTo = nullptr;
		}
	}
}

void AWeapon::Enter_Ready()
{
}

void AWeapon::Enter_Reloading()
{
	OnReloadStart(Clip <= 0);
}

void AWeapon::Enter_Deploying()
{
	// 이 시점에는 아직 Owner가 설정되지 않았을 수도 있기 때문에, 웬만하면 Deploy() 함수에서 처리
}

void AWeapon::Enter_Holstering()
{
	OnHolster();
}

void AWeapon::Exit_Ready()
{
}

void AWeapon::Exit_Reloading()
{
	if (LastStateElapsedTime != 0.0f)
	{
		OnReloadCancelled();
	}
}

void AWeapon::Exit_Deploying()
{
}

void AWeapon::Exit_Holstering()
{
}

void AWeapon::BeginFiring(int32 RandSeed)
{
	CurBurstCount = 0;
	FireRand.Initialize(RandSeed);
	bFiring = true;
	
	const auto FireDelay = GetFireDelay();
	if (FireLag >= FireDelay)
	{
		FireLag -= FireDelay;

		if (!Fire())
			EndFiring();
	}
}

void AWeapon::EndFiring()
{
	bFiring = false;
	CurBurstCount = 0;
}

bool AWeapon::Fire()
{
	if (Clip == 0)
	{
		OnDryFire();
		return false;
	}

	SetClip(Clip - 1);

	if (FireMode == EWeaponFireMode::Burst)
		CurBurstCount++;

	OnFire();

	if (Clip == 0)
	{
		OnDryFire();
		return false;
	}

	switch (FireMode)
	{
	case EWeaponFireMode::SemiAuto:
		return false;
	case EWeaponFireMode::Burst:
		return CurBurstCount < BurstCount;
	default:
		return true;
	}
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
	if (State == NewState)
		return;
	
	switch (State)
	{
	case EWeaponState::Ready:
		Exit_Ready();
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
	LastStateElapsedTime = 0.0f;
	
	switch (State)
	{
	case EWeaponState::Ready:
		Enter_Ready();
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

void AWeapon::SetFireMode(EWeaponFireMode NewFm)
{
	FireMode = NewFm;
	FireMode_LastModified = GetWorld()->GetRealTimeSeconds();
}

void AWeapon::CorrectClientState()
{
	if (GetOwner()->GetRemoteRole() != ROLE_AutonomousProxy)
		return;

	const auto Now = GetWorld()->GetRealTimeSeconds();
	if (NextCorrection <= Now)
	{
		Client_CorrectState({FireMode, bAiming});
		Multicast_CorrectState({Clip, State});
		NextCorrection = Now + 0.5f;
	}
}

bool AWeapon::IsExpired(float LastModified) const
{
	if (const auto Char = GetCharOwner())
	{
		if (const auto PS = Char->GetPlayerState())
		{
			const auto PingMs = PS->GetPing() * (Char->IsLocallyControlled() ? 4.0f : 2.0f);
			const auto Now = GetWorld()->GetRealTimeSeconds();
			return (Now - LastModified) * 1000.0f >= PingMs;
		}
	}
	return true;
}

void AWeapon::Client_CorrectState_Implementation(FClientWeaponCorrectionData Data)
{
	if (FireMode != Data.FireMode && IsExpired(FireMode_LastModified))
		FireMode = Data.FireMode;

	if (bAiming != Data.bAiming && IsExpired(Aiming_LastModified))
		bAiming = Data.bAiming;
}

void AWeapon::Multicast_CorrectState_Implementation(FMulticastWeaponCorrectionData Data)
{
	if (Clip != Data.Clip && IsExpired(Clip_LastModified))
		Clip = Data.Clip;
	
	if (State != Data.State && IsExpired(State_LastModified))
		SetState(Data.State);
}

void AWeapon::Server_StartFiring_Implementation(int32 RandSeed)
{
	BeginFiring(RandSeed);
	Multicast_StartFiring(RandSeed);
}

bool AWeapon::Server_StartFiring_Validate(int32 RandSeed)
{
	return true;
}

void AWeapon::Multicast_StartFiring_Implementation(int32 RandSeed)
{
	if (HasAuthority())
		return;
	
	const auto Char = GetCharOwner();
	if (Char && !Char->IsLocallyControlled())
	{
		BeginFiring(RandSeed);
	}
}

void AWeapon::Server_StopFiring_Implementation()
{
	EndFiring();
	Multicast_StopFiring();
}

bool AWeapon::Server_StopFiring_Validate()
{
	return true;
}

void AWeapon::Multicast_StopFiring_Implementation()
{
	if (HasAuthority())
		return;
	
	const auto Char = GetCharOwner();
	if (Char && !Char->IsLocallyControlled())
	{
		EndFiring();
	}
}

void AWeapon::OnRep_State(EWeaponState OldState)
{
	const auto NewState = State;
	State = OldState;
	SetState(NewState);
}

