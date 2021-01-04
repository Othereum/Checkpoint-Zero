// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0Character.h"
#include "CP0.h"
#include "CP0CharacterMovement.h"
#include "CP0GameInstance.h"
#include "CP0InputSettings.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "WeaponComponent.h"

template <class...>
using TBool = bool;

template <int I>
struct TChoice : TChoice<I + 1>
{
};

template <>
struct TChoice<10>
{
};

template <class Action>
static auto DispatchInputByTypeImpl(ACP0Character* Character, EInputAction Type, TChoice<0>)
    -> TBool<decltype(Action::Toggle(DeclVal<ACP0Character*>()))>
{
    switch (Type)
    {
    case EInputAction::Enable:
        Action::Enable(Character);
        return true;
    case EInputAction::Disable:
        Action::Disable(Character);
        return true;
    case EInputAction::Toggle:
        Action::Toggle(Character);
        return true;
    }
    return false;
}

template <class Action>
static auto DispatchInputByTypeImpl(ACP0Character* Character, EInputAction Type, TChoice<1>)
    -> TBool<decltype(Action::Disable(DeclVal<ACP0Character*>()))>
{
    switch (Type)
    {
    case EInputAction::Enable:
        Action::Enable(Character);
        return true;
    case EInputAction::Disable:
        Action::Disable(Character);
        return true;
    }
    return false;
}

template <class Action>
static bool DispatchInputByTypeImpl(ACP0Character* Character, EInputAction Type, TChoice<2>)
{
    switch (Type)
    {
    case EInputAction::Enable:
        Action::Enable(Character);
        return true;
    }
    return false;
}

template <class Action>
static bool DispatchInputByType(ACP0Character* Character, EInputAction Type)
{
    return DispatchInputByTypeImpl<Action>(Character, Type, TChoice<0>{});
}

struct FInputAction
{
    FName Name;
    bool (*Dispatcher)(ACP0Character*, EInputAction);
    bool bSendToServer;
};

#define MAKE_INPUT_ACTION(Name, bSendToServer)                                                                         \
    {                                                                                                                  \
        TEXT(#Name), &DispatchInputByType<FInputAction_##Name>, bSendToServer                                          \
    }

static const FInputAction InputActions[]{
    MAKE_INPUT_ACTION(Sprint, true),    MAKE_INPUT_ACTION(Crouch, true),         MAKE_INPUT_ACTION(Prone, true),
    MAKE_INPUT_ACTION(WalkSlow, false), MAKE_INPUT_ACTION(Fire, true),           MAKE_INPUT_ACTION(Aim, true),
    MAKE_INPUT_ACTION(Reload, true),    MAKE_INPUT_ACTION(SwitchFiremode, true),
};

template <class T, size_t N>
static constexpr size_t Size(const T (&)[N])
{
    return N;
}

static_assert(Size(InputActions) <= MAX_uint8, "Too many input actions");

#undef MAKE_INPUT_ACTION

ACP0Character::ACP0Character(const FObjectInitializer& Initializer)
    : Super{Initializer.SetDefaultSubobjectClass<UCP0CharacterMovement>(ACharacter::CharacterMovementComponentName)},
      Camera{CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"))},
      WeaponComp{CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComp"))},
      LegsMesh{CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh"))}
{
    PrimaryActorTick.bCanEverTick = true;
    BaseEyeHeight = 150.0f;
    CrouchedEyeHeight = 100.0f;
    bUseControllerRotationYaw = false;

    Camera->SetupAttachment(RootComponent);
    WeaponComp->SetupAttachment(RootComponent);
    LegsMesh->SetupAttachment(GetMesh());
}

UCP0CharacterMovement* ACP0Character::GetCP0Movement() const
{
    return CastChecked<UCP0CharacterMovement>(GetCharacterMovement());
}

bool ACP0Character::IsMoveInputIgnored() const
{
    return GetCP0Movement()->IsProneSwitching() || Super::IsMoveInputIgnored();
}

static constexpr auto AngleCompressRatio = (1 << 16) / 360.0f;

FRotator ACP0Character::GetBaseAimRotation() const
{
    if (Controller)
        return Controller->GetControlRotation();

    return {RemoteViewPitch16 / AngleCompressRatio, RemoteViewYaw16 / AngleCompressRatio, 0.0f};
}

FRotator ACP0Character::GetViewRotation() const
{
    auto Rotation = Super::GetViewRotation();
    Rotation += WeaponComp->GetSocketRotation(TEXT("CameraSocket")) - WeaponComp->GetComponentRotation();
    Rotation.Roll = 0.0f;
    return Rotation;
}

void ACP0Character::SetRemoteViewRotation(FRotator Rotation)
{
    RemoteViewPitch16 = static_cast<uint16>(FRotator::ClampAxis(Rotation.Pitch) * AngleCompressRatio);
    RemoteViewYaw16 = static_cast<uint16>(FRotator::ClampAxis(Rotation.Yaw) * AngleCompressRatio);
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
        EyeHeightAlpha = 0.0f;
    }
    else
    {
        EyeHeightAlpha = 1.0f;
        SetEyeHeight(NewEyeHeight);
    }
}

float ACP0Character::GetDefaultEyeHeight(EPosture Posture) const
{
    switch (Posture)
    {
    case EPosture::Crouch:
        return CrouchedEyeHeight;
    case EPosture::Prone:
        return ProneEyeHeight;
    }
    return GetDefault<APawn>(GetClass())->BaseEyeHeight;
}

float ACP0Character::GetEyeHeight() const
{
    return BaseEyeHeight + GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
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
    UpdateLegsTransform();
    WeaponComp->UpdateTransform(DeltaTime);
    Camera->SetWorldLocationAndRotation(GetPawnViewLocation(), GetViewRotation());
}

void ACP0Character::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);

    Input->BindAxis(TEXT("MoveForward"), this, &ACP0Character::MoveForward);
    Input->BindAxis(TEXT("MoveRight"), this, &ACP0Character::MoveRight);
    Input->BindAxis(TEXT("Turn"), this, &ACP0Character::Turn);
    Input->BindAxis(TEXT("LookUp"), this, &ACP0Character::LookUp);

    Input->BindAction(TEXT("Jump"), IE_Pressed, this, &ACP0Character::Jump);

    for (size_t i = 0; i < Size(InputActions); ++i)
    {
        FInputActionBinding Pressed{InputActions[i].Name, IE_Pressed};
        Pressed.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(this, [this, i, LastTime = -1.0f]() mutable {
            const auto GI = CastChecked<UCP0GameInstance>(GetGameInstance());
            const auto Settings = GI->GetInputSettings();
            const auto TypePtr = Settings->PressTypes.Find(InputActions[i].Name);
            switch (TypePtr ? *TypePtr : EPressType::Continuous)
            {
            case EPressType::Press:
                DispatchInputAction(i, EInputAction::Toggle);
                break;
            case EPressType::Continuous:
                DispatchInputAction(i, EInputAction::Enable);
                break;
            case EPressType::DoubleClick: {
                const auto CurTime = GetGameTimeSinceCreation();
                if (CurTime - LastTime <= Settings->DoubleClickTimeout)
                {
                    DispatchInputAction(i, EInputAction::Toggle);
                    LastTime = -1.0f;
                }
                else
                {
                    LastTime = CurTime;
                }
                break;
            }
            }
        });
        Input->AddActionBinding(MoveTemp(Pressed));

        FInputActionBinding Released{InputActions[i].Name, IE_Released};
        Released.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(this, [this, i] {
            const auto GI = CastChecked<UCP0GameInstance>(GetGameInstance());
            const auto Settings = GI->GetInputSettings();
            const auto TypePtr = Settings->PressTypes.Find(InputActions[i].Name);
            switch (TypePtr ? *TypePtr : EPressType::Continuous)
            {
            case EPressType::Release:
                DispatchInputAction(i, EInputAction::Toggle);
                break;
            case EPressType::Continuous:
                DispatchInputAction(i, EInputAction::Disable);
                break;
            }
        });
        Input->AddActionBinding(MoveTemp(Released));
    }
}

void ACP0Character::InterpEyeHeight(float DeltaTime)
{
    if (EyeHeightAlpha >= 1.0f)
        return;

    EyeHeightAlpha = FMath::Clamp(EyeHeightAlpha + DeltaTime / EyeHeightBlendTime, 0.0f, 1.0f);
    SetEyeHeight(FMath::CubicInterp(PrevEyeHeight, 0.0f, TargetEyeHeight, 0.0f, EyeHeightAlpha));
}

void ACP0Character::UpdateLegsTransform()
{
    const auto Default = GetDefault<ACP0Character>(GetClass())->LegsMesh;
    const auto OffsetX = Default->GetRelativeLocation().Y;

    const FRotator ViewYaw{0.0f, GetBaseAimRotation().Yaw, 0.0f};
    const auto BaseLoc = GetMesh()->GetComponentLocation();

    LegsMesh->SetWorldLocation(BaseLoc + ViewYaw.Vector() * OffsetX);
}

void ACP0Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ACP0Character, RemoteViewPitch16, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(ACP0Character, RemoteViewYaw16, COND_SkipOwner);
}

void ACP0Character::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
    Super::PreReplication(ChangedPropertyTracker);

    if (HasAuthority() && GetController())
    {
        SetRemoteViewRotation(GetController()->GetControlRotation());
    }
}

void ACP0Character::ServerInputAction_Implementation(uint8 Idx, EInputAction Type)
{
    DispatchInputAction(Idx, Type);
}

bool ACP0Character::ServerInputAction_Validate(uint8 Idx, EInputAction Type)
{
    return true;
}

void ACP0Character::DispatchInputAction(size_t Idx, EInputAction Type)
{
    if (Idx < Size(InputActions))
    {
        const auto bExecuted = InputActions[Idx].Dispatcher(this, Type);

        if (bExecuted && InputActions[Idx].bSendToServer && !HasAuthority())
            ServerInputAction(static_cast<uint8>(Idx), Type);
    }
}

void ACP0Character::MoveForward(float AxisValue)
{
    if (!FMath::IsNearlyZero(AxisValue))
    {
        const FRotator Rotation{0.0f, GetControlRotation().Yaw, 0.0f};
        AddMovementInput(Rotation.Vector(), AxisValue);
    }
}

void ACP0Character::MoveRight(float AxisValue)
{
    if (!FMath::IsNearlyZero(AxisValue))
    {
        const FRotator Rotation{0.0f, GetControlRotation().Yaw + 90.0f, 0.0f};
        AddMovementInput(Rotation.Vector(), AxisValue);
    }
}

void ACP0Character::Turn(float AxisValue)
{
    AddControllerYawInput(AxisValue);
}

void ACP0Character::LookUp(float AxisValue)
{
    AddControllerPitchInput(AxisValue);
}

