// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CP0.h"
#include "GameFramework/Character.h"
#include "CP0Character.generated.h"

class UCP0CharacterMovement;
class UWeaponComponent;
class UCameraComponent;

UENUM()
enum class EInputAction : uint8
{
	Enable,
	Disable,
	Toggle
};

UCLASS()
class CP0_API ACP0Character : public ACharacter
{
	GENERATED_BODY()

public:
	ACP0Character(const FObjectInitializer& Initializer);
	UCP0CharacterMovement* GetCP0Movement() const;
	UWeaponComponent* GetWeaponComp() const { return WeaponComp; }
	UCameraComponent* GetCamera() const { return Camera; }
	USkeletalMeshComponent* GetArms() const { return ArmsMesh; }

	virtual void RecalculateBaseEyeHeight() override
	{
	}

	virtual bool IsMoveInputIgnored() const override;
	virtual FRotator GetBaseAimRotation() const override;
	virtual FRotator GetViewRotation() const override;

	void SetRemoteViewRotation(FRotator Rotation);
	void SetEyeHeight(float NewEyeHeight);
	void SetEyeHeightWithBlend(float NewEyeHeight, float BlendTime);
	float GetDefaultEyeHeight(EPosture Posture) const;
	float GetEyeHeight() const;

	UFUNCTION(BlueprintImplementableEvent)
	void OnPostureChanged(EPosture PrevPosture, EPosture NewPosture);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComp) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

private:
	friend UCP0CharacterMovement;

	void InterpEyeHeight(float DeltaTime);
	void UpdateLegsTransform() const;
	void UpdateArmsTransform(float DeltaTime);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInputAction(uint8 Idx, EInputAction Type);
	void DispatchInputAction(size_t Idx, EInputAction Type);

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UWeaponComponent* WeaponComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	USkeletalMeshComponent* LegsMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	USkeletalMeshComponent* ArmsMesh;

	FTransform ArmsLocalOffset;
	FRotator PrevAimRot;
	FRotator AimRotSpeed;
	
	UPROPERTY(EditAnywhere, Category = "Camera")
	float ProneEyeHeight = 35.0f;

	float TargetEyeHeight = 150.0f;
	float PrevEyeHeight = 150.0f;
	float EyeHeightAlpha = 1.0f;
	float EyeHeightBlendTime = 1.0f;

	UPROPERTY(Replicated, Transient)
	uint16 RemoteViewPitch16;

	UPROPERTY(Replicated, Transient)
	uint16 RemoteViewYaw16;
};
