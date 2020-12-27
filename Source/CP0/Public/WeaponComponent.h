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

  protected:
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  private:
    UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
    AWeapon* Weapon;

    FTransform LocalOffset;
    FRotator PrevAimRot;
    FRotator AimRotSpeed;
};
