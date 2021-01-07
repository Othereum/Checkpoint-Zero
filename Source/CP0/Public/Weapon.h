// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CP0.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class AWeapon;
class ACP0Character;
class UWeaponComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Ready,
	Reloading,
	Deploying,
	Holstering
};

USTRUCT()
struct FClientWeaponCorrectionData
{
	GENERATED_BODY()

	UPROPERTY()
	EWeaponFireMode FireMode;

	UPROPERTY()
	uint8 bAiming : 1;
};

USTRUCT()
struct FMulticastWeaponCorrectionData
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 Clip;

	UPROPERTY()
	EWeaponState State;
};

UCLASS()
class CP0_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	UWeaponComponent* GetWeaponComp() const;
	TSubclassOf<UAnimInstance> GetArmsAnimClass() const { return ArmsAnimClass; }

	UFUNCTION(BlueprintCallable)
	ACP0Character* GetCharOwner() const;

	void Deploy(ACP0Character* Char);
	void Holster(AWeapon* SwitchTo);

	// 클라이언트 전용. 키 입력시 호출됨.
	void StartFiring();
	void StopFiring();

	void SetAiming(bool bNewAiming);
	void Reload();
	void SwitchFireMode();

	bool IsAiming() const { return bAiming; }
	EWeaponState GetState() const { return State; }
	EWeaponFireMode GetFireMode() const { return FireMode; }
	float GetFireDelay() const { return 60.0f / Rpm; }

	UFUNCTION(BlueprintCallable)
	void PlayMontage(UAnimMontage* Montage) const;

	UFUNCTION(BlueprintCallable)
	void StopMontage(float BlendOutTime = 0.0f, UAnimMontage* Montage = nullptr) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform ArmsOffset;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnFire();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDryFire();

	UFUNCTION(BlueprintImplementableEvent)
	void OnReloadStart(bool bEmpty);

	UFUNCTION(BlueprintImplementableEvent)
	void OnFireModeSwitched();

	UFUNCTION(BlueprintImplementableEvent)
	void OnReloadCancelled();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDeploy();

	UFUNCTION(BlueprintImplementableEvent)
	void OnHolster();

private:
	void Tick_Ready(float DeltaTime);
	void Tick_Reloading(float DeltaTime);
	void Tick_Deploying(float DeltaTime);
	void Tick_Holstering(float DeltaTime);

	void Enter_Ready();
	void Enter_Reloading();
	void Enter_Deploying();
	void Enter_Holstering();

	void Exit_Ready();
	void Exit_Reloading();
	void Exit_Deploying();
	void Exit_Holstering();

	void BeginFiring(int32 RandSeed);
	void EndFiring();

	bool Fire();
	bool CanDoCommonAction() const;

	void SetClip(uint8 NewClip);
	void SetState(EWeaponState NewState);
	void SetFireMode(EWeaponFireMode NewFm);

	void CorrectClientState();
	bool IsExpired(float LastModified) const;

	UFUNCTION(Client, Unreliable)
	void Client_CorrectState(FClientWeaponCorrectionData Data);
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_CorrectState(FMulticastWeaponCorrectionData Data);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartFiring(int32 RandSeed);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartFiring(int32 RandSeed);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StopFiring();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopFiring();

	UFUNCTION()
	void OnRep_State(EWeaponState OldState);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	USkeletalMeshComponent* Mesh1P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	USkeletalMeshComponent* Mesh3P;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAnimInstance> ArmsAnimClass;

	UPROPERTY(Transient)
	AWeapon* SwitchingTo;
	
	FRandomStream FireRand;

	UPROPERTY(EditAnywhere, meta = (UIMin = 1, ClampMin = 1))
	float Rpm = 650.0f;
	float FireLag;

	UPROPERTY(EditAnywhere)
	float ReloadTime_Tactical = 2.0f;

	UPROPERTY(EditAnywhere)
	float ReloadTime_Empty = 3.0f;

	UPROPERTY(EditAnywhere)
	float DeployTime = 0.67f;

	UPROPERTY(EditAnywhere)
	float HolsterTime = 0.67f;

	float LastStateElapsedTime;
	
	float Clip_LastModified;
	float FireMode_LastModified;
	float State_LastModified;
	float Aiming_LastModified;
	float NextCorrection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	uint8 ClipSize = 30;

	UPROPERTY(Transient, EditInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	uint8 Clip;
	uint8 CurBurstCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (AllowPrivateAccess = true, Bitmask, BitmaskEnum = EWeaponFireMode))
	uint8 FireModes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	uint8 BurstCount = 3;

	UPROPERTY(Replicated, Transient, EditInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EWeaponFireMode FireMode;

	UPROPERTY(ReplicatedUsing = OnRep_State, Transient, EditInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EWeaponState State;

	UPROPERTY(Replicated, Transient, EditInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	uint8 bAiming : 1;
	uint8 bFiring : 1;
};
