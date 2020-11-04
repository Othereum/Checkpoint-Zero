// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0Character.h"
#include "CP0CharacterInput.inl"
#include "CP0CharacterMovement.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

ACP0Character::ACP0Character(const FObjectInitializer& Initializer)
    : Super{Initializer.SetDefaultSubobjectClass<UCP0CharacterMovement>(ACharacter::CharacterMovementComponentName)},
      Legs{CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Legs"))}
{
    PrimaryActorTick.bCanEverTick = true;
    BaseEyeHeight = 150.f;
    CrouchedEyeHeight = 100.f;

    GetMesh()->SetRelativeLocationAndRotation({0.f, 0.f, -88.f}, {0.f, -90.f, 0.f});
    GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
    GetMesh()->bOwnerNoSee = true;

    Legs->SetupAttachment(GetMesh());
    Legs->SetRelativeLocation({0.f, -30.f, 10.f});
    Legs->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
    Legs->bOnlyOwnerSee = true;
    Legs->bSelfShadowOnly = true;

    RegisterInputActions();
}

UCP0CharacterMovement* ACP0Character::GetCP0Movement() const
{
    return CastChecked<UCP0CharacterMovement>(GetCharacterMovement());
}

void ACP0Character::SetEyeHeight(float NewEyeHeight)
{
    BaseEyeHeight = NewEyeHeight - GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
}

void ACP0Character::SetEyeHeightWithBlend(float NewEyeHeight, float BlendTime)
{
    TargetEyeHeight = NewEyeHeight;
    EyeHeightBlendTime = BlendTime;
    PrevEyeHeight = GetEyeHeight();

    if (BlendTime > KINDA_SMALL_NUMBER)
    {
        EyeHeightAlpha = 0.f;
    }
    else
    {
        EyeHeightAlpha = 1.f;
        SetEyeHeight(NewEyeHeight);
    }
}

float ACP0Character::GetDefaultEyeHeight(EPosture Posture) const
{
    switch (Posture)
    {
    default:
        ensureNoEntry();
    case EPosture::Stand:
        return GetDefault<APawn>(GetClass())->BaseEyeHeight;
    case EPosture::Crouch:
        return CrouchedEyeHeight;
    case EPosture::Prone:
        return ProneEyeHeight;
    }
}

float ACP0Character::GetEyeHeight() const
{
    return BaseEyeHeight + GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
}

float ACP0Character::PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
    if (const auto AnimInst = Legs->GetAnimInstance())
    {
        const auto Duration = AnimInst->Montage_Play(AnimMontage, InPlayRate);
        if (Duration > 0.f)
        {
            if (StartSectionName != NAME_None)
                AnimInst->Montage_JumpToSection(StartSectionName, AnimMontage);
        }
    }

    return Super::PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}

void ACP0Character::BeginPlay()
{
    Super::BeginPlay();
    SetEyeHeight(BaseEyeHeight);
}

void ACP0Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    InterpEyeHeight(DeltaTime);
}

void ACP0Character::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);

    Input->BindAxis(TEXT("MoveForward"), this, &ACP0Character::MoveForward);
    Input->BindAxis(TEXT("MoveRight"), this, &ACP0Character::MoveRight);
    Input->BindAxis(TEXT("Turn"), this, &ACP0Character::Turn);
    Input->BindAxis(TEXT("LookUp"), this, &ACP0Character::LookUp);

    for (const auto& Action : InputActionMap)
        BindInputAction(Input, Action.Key);
}

void ACP0Character::InterpEyeHeight(float DeltaTime)
{
    if (EyeHeightAlpha >= 1.f)
        return;

    EyeHeightAlpha = FMath::Clamp(EyeHeightAlpha + DeltaTime / EyeHeightBlendTime, 0.f, 1.f);
    SetEyeHeight(FMath::CubicInterp(PrevEyeHeight, 0.f, TargetEyeHeight, 0.f, EyeHeightAlpha));
}

void ACP0Character::RegisterInputActions()
{
    RegisterInputAction<FSprintAction>(TEXT("Sprint"));
    RegisterInputAction<FCrouchAction>(TEXT("Crouch"));
    RegisterInputAction<FProneAction>(TEXT("Prone"));
}

void ACP0Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ACP0Character::MoveForward(float AxisValue)
{
    AddMovementInput(GetActorForwardVector(), AxisValue);
}

void ACP0Character::MoveRight(float AxisValue)
{
    AddMovementInput(GetActorRightVector(), AxisValue);
}

void ACP0Character::Turn(float AxisValue)
{
    AddControllerYawInput(AxisValue);
}

void ACP0Character::LookUp(float AxisValue)
{
    AddControllerPitchInput(AxisValue);
}
