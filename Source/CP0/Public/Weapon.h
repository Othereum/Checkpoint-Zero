// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "GameFramework/Actor.h"
#include "WeaponAnimInst.h"
#include "Weapon.generated.h"

class ACP0Character;
class UWeaponComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    Idle,
    Firing,
    Reloading,
    Deploying,
    Holstering
};

UENUM(BlueprintType, meta = (Bitflags))
enum class EWeaponFireMode : uint8
{
    SemiAuto,
    Burst,
    FullAuto
};

UCLASS()
class CP0_API AWeapon : public AActor
{
    GENERATED_BODY()

  public:
    AWeapon();
    ACP0Character* GetCharOwner() const;
    UWeaponComponent* GetWeaponComp() const;
    TSubclassOf<UWeaponAnimInst> GetArmsAnimClass() const { return ArmsAnimClass; }

    void StartFiring();
    void StopFiring();

    void SetAiming(bool bNewAiming);
    void Reload();
    void SwitchFiremode();

    bool CanFire() const;
    bool IsAiming() const { return bAiming; }
    EWeaponState GetState() const { return State; }
    float GetFireDelay() const { return 60.0f / RPM; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform ArmsOffset;

  protected:
    void BeginPlay() override;
    void Tick(float DeltaTime) override;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintImplementableEvent)
    void OnFire();

    UFUNCTION(BlueprintImplementableEvent)
    void OnDryFire();

    UFUNCTION(BlueprintImplementableEvent)
    void OnReloadStart(bool bEmpty);

    UFUNCTION(BlueprintImplementableEvent)
    void OnFiremodeSwitched();

  private:
    void Tick_Idle(float DeltaTime);
    void Tick_Firing(float DeltaTime);
    void Tick_Reloading(float DeltaTime);
    void Tick_Deploying(float DeltaTime);
    void Tick_Holstering(float DeltaTime);

    void Fire();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    USkeletalMeshComponent* Mesh;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UWeaponAnimInst> ArmsAnimClass;

    UPROPERTY(EditAnywhere, meta = (UIMin = 1, ClampMin = 1))
    float RPM = 650.0f;
    float FireLag;

    UPROPERTY(EditAnywhere)
    float ReloadTime_Tactical = 2.0f;

    UPROPERTY(EditAnywhere)
    float ReloadTime_Empty = 3.0f;

    float CurrentReloadTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    uint8 ClipSize = 30;

    UPROPERTY(Replicated, Transient, EditInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    uint8 Clip;

    UPROPERTY(EditAnywhere, BlueprintReadOnly,
              meta = (AllowPrivateAccess = true, Bitmask, BitmaskEnum = EWeaponFireMode))
    uint8 FireModes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    uint8 BurstCount = 3;

    UPROPERTY(Replicated, Transient, EditInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    EWeaponFireMode FireMode;

    UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    EWeaponState State;

    UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bAiming;
};
