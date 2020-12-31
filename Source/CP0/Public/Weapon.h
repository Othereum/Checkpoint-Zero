// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class CP0_API AWeapon : public AActor
{
    GENERATED_BODY()

  public:
    AWeapon();

    TSubclassOf<UAnimInstance> GetArmsAnimClass() const { return ArmsAnimClass; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform ArmsOffset;

  protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

  private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    USkeletalMeshComponent* Mesh;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UAnimInstance> ArmsAnimClass;
};

