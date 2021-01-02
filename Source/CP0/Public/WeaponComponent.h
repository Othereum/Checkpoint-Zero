// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "Components/SkeletalMeshComponent.h"
#include "WeaponComponent.generated.h"

class ACP0Character;
class AWeapon;

/**
 *
 */
UCLASS()
class CP0_API UWeaponComponent : public USkeletalMeshComponent
{
    GENERATED_BODY()

  public:
    UWeaponComponent();
    ACP0Character* GetCharOwner() const;
    const UWeaponComponent* GetDefaultSelf() const;
    void UpdateTransform(float DeltaTime);

    UFUNCTION(BlueprintCallable)
    void SetWeapon(AWeapon* NewWeapon);
    AWeapon* GetWeapon() const { return Weapon; }

  protected:
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  private:
    UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    AWeapon* Weapon;

    FTransform LocalOffset;
    FRotator PrevAimRot;
    FRotator AimRotSpeed;
};

struct CP0_API FInputAction_Fire
{
    static void Enable(ACP0Character* Character);
    static void Disable(ACP0Character* Character);
};

struct CP0_API FInputAction_Aim
{
    static void Enable(ACP0Character* Character);
    static void Disable(ACP0Character* Character);
    static void Toggle(ACP0Character* Character);
};

struct CP0_API FInputAction_Reload
{
    static void Enable(ACP0Character* Character);
};

struct CP0_API FInputAction_SwitchFiremode
{
    static void Enable(ACP0Character* Character);
};

