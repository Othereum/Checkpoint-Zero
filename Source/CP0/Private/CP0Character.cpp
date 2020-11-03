// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0Character.h"
#include "CP0CharacterInput.inl"
#include "CP0CharacterMovement.h"
#include "Net/UnrealNetwork.h"

ACP0Character::ACP0Character(const FObjectInitializer& Initializer)
    : Super{Initializer.SetDefaultSubobjectClass<UCP0CharacterMovement>(ACharacter::CharacterMovementComponentName)}
{
    PrimaryActorTick.bCanEverTick = true;
    RegisterInputActions();
}

UCP0CharacterMovement* ACP0Character::GetCP0Movement() const
{
    return CastChecked<UCP0CharacterMovement>(GetCharacterMovement());
}

void ACP0Character::SetEyeHeightWithBlend(float NewEyeHeight, float BlendTime)
{
    TargetEyeHeight = NewEyeHeight;
    PrevEyeHeight = BaseEyeHeight;
    EyeHeightAlpha = 0.0f;
    EyeHeightBlendTime = BlendTime;
}

float ACP0Character::GetEyeHeight(EPosture Posture) const
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

void ACP0Character::BeginPlay()
{
    Super::BeginPlay();
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
    if (EyeHeightAlpha >= 1.0f)
        return;

    EyeHeightAlpha = FMath::Min(EyeHeightAlpha + DeltaTime * EyeHeightBlendTime, 1.0f);
    BaseEyeHeight = FMath::CubicInterp(PrevEyeHeight, 0.0f, TargetEyeHeight, 0.0f, EyeHeightAlpha);
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
