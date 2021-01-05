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
class CP0_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();
	ACP0Character* GetCharOwner() const;
	const UWeaponComponent* GetDefaultSelf() const;

	UFUNCTION(BlueprintCallable)
	void EquipWeapon(AWeapon* NewWeapon);
	
	AWeapon* GetWeapon() const { return Weapon; }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	friend AWeapon;
	
	UFUNCTION()
	void OnRep_Weapon();

	UPROPERTY(ReplicatedUsing = OnRep_Weapon, Transient, VisibleInstanceOnly, BlueprintReadOnly, meta = (
		AllowPrivateAccess = true))
	AWeapon* Weapon;
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
