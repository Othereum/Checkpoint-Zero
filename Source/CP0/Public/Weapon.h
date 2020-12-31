// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class ACP0Character;
class UWeaponComponent;

UCLASS()
class CP0_API AWeapon : public AActor
{
    GENERATED_BODY()

  public:
    AWeapon();
    ACP0Character* GetCharOwner() const;
    UWeaponComponent* GetWeaponComp() const;
    TSubclassOf<UAnimInstance> GetArmsAnimClass() const { return ArmsAnimClass; }

    bool IsFiring() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform ArmsOffset;

  protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintImplementableEvent)
    void OnFire();

  private:
    void Fire();
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    USkeletalMeshComponent* Mesh;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UAnimInstance> ArmsAnimClass;

    UPROPERTY(EditAnywhere, meta = (UIMin = 1, ClampMin = 1))
    float RPM = 600.0f;
    float FireLag = 60.0f / RPM;

    UPROPERTY(EditAnywhere)
    bool bAutomatic = true;
};

