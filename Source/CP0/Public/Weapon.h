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
    Ready,
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

    bool TrySetFiring(bool bNewFiring);
    bool IsFiring() const { return bFiring; }
    bool CanFire() const;
    bool TrySetAiming(bool bNewAiming);
    bool IsAiming() const { return bAiming; }
    EWeaponState GetState() const { return State; }

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

  private:
    void Fire();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    USkeletalMeshComponent* Mesh;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UWeaponAnimInst> ArmsAnimClass;

    UPROPERTY(EditAnywhere, meta = (UIMin = 1, ClampMin = 1))
    float RPM = 650.0f;
    float FireLag;

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
    bool bFiring;

    UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bAiming;
};
