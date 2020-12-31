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
using TVoid = void;

template <class Action, class = void>
struct TInputDispatcher
{
    static constexpr auto bHasToggle = false;

    static void Dispatch(ACP0Character* Character, EInputAction Type)
    {
        switch (Type)
        {
        case EInputAction::Enable:
            Action::Enable(Character);
            break;
        case EInputAction::Disable:
            Action::Disable(Character);
            break;
        }
    }
};

template <class Action>
struct TInputDispatcher<Action, TVoid<decltype(Action::Toggle(DeclVal<ACP0Character*>()))>>
{
    static constexpr auto bHasToggle = true;

    static void Dispatch(ACP0Character* Character, EInputAction Type)
    {
        switch (Type)
        {
        case EInputAction::Enable:
            Action::Enable(Character);
            break;
        case EInputAction::Disable:
            Action::Disable(Character);
            break;
        case EInputAction::Toggle:
            Action::Toggle(Character);
            break;
        }
    }
};

struct FInputAction
{
    template <class Action>
    static FInputAction Create(bool bSendToServer)
    {
        using Dispatcher = TInputDispatcher<Action>;
        return {&Dispatcher::Dispatch, bSendToServer, Dispatcher::bHasToggle};
    }

    void (*Dispatcher)(ACP0Character*, EInputAction);
    bool bSendToServer;
    bool bHasToggle;
};

#define MAKE_INPUT_ACTION(Name, bSendToServer)                                                                         \
    {                                                                                                                  \
        TEXT(#Name), FInputAction::Create<FInputAction_##Name>(bSendToServer)                                          \
    }

static const TMap<FName, FInputAction> InputActionMap{
    MAKE_INPUT_ACTION(Sprint, true),    MAKE_INPUT_ACTION(Crouch, true), MAKE_INPUT_ACTION(Prone, true),
    MAKE_INPUT_ACTION(WalkSlow, false), MAKE_INPUT_ACTION(Fire, true),   MAKE_INPUT_ACTION(Aim, true),
};

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

FRotator ACP0Character::GetBaseAimRotation() const
{
    if (Controller)
        return Controller->GetControlRotation();

    return {RemoteViewPitch * 360.0f / 255.0f, RemoteViewYaw * 360.0f / 255.0f, 0.0f};
}

FRotator ACP0Character::GetViewRotation() const
{
    auto Rotation = Super::GetViewRotation();
    Rotation += WeaponComp->GetSocketRotation(TEXT("CameraSocket")) - WeaponComp->GetComponentRotation();
    Rotation.Roll = 0.0f;
    return Rotation;
}

void ACP0Character::SetRemoteViewYaw(float NewRemoteViewYaw)
{
    NewRemoteViewYaw = FRotator::ClampAxis(NewRemoteViewYaw);
    RemoteViewYaw = static_cast<uint8>(NewRemoteViewYaw * 255.0f / 360.0f);
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

    for (const auto& Action : InputActionMap)
        BindInputAction(Input, Action.Key, Action.Value.bHasToggle);
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

    DOREPLIFETIME_CONDITION(ACP0Character, RemoteViewYaw, COND_SkipOwner);
}

void ACP0Character::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
    Super::PreReplication(ChangedPropertyTracker);

    if (HasAuthority() && GetController())
    {
        SetRemoteViewYaw(GetController()->GetControlRotation().Yaw);
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACP0Character::ServerInputAction_Implementation(FName Name, EInputAction Type)
{
    DispatchInputAction(Name, Type);
}

bool ACP0Character::ServerInputAction_Validate(FName Name, EInputAction Type)
{
    return true;
}

void ACP0Character::DispatchInputAction(FName Name, EInputAction Type)
{
    const auto Action = InputActionMap.Find(Name);
    if (ensure(Action != nullptr))
    {
        Action->Dispatcher(this, Type);

        if (Action->bSendToServer && !HasAuthority())
            ServerInputAction(Name, Type);
    }
}

void ACP0Character::BindInputAction(UInputComponent* Input, FName Name, bool bHasToggle)
{
    if (bHasToggle)
    {
        const auto GI = CastChecked<UCP0GameInstance>(GetGameInstance());
        const auto Settings = GI->GetInputSettings();
        const auto Type = Settings->PressTypes.Find(Name);
        if (!ensure(Type))
            return;

        FInputActionBinding Pressed{Name, IE_Pressed};
        Pressed.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(this, [=, LastTime = -1.0f]() mutable {
            switch (*Type)
            {
            case EPressType::Press:
                DispatchInputAction(Name, EInputAction::Toggle);
                break;
            case EPressType::Continuous:
                DispatchInputAction(Name, EInputAction::Enable);
                break;
            case EPressType::DoubleClick: {
                const auto CurTime = GetGameTimeSinceCreation();
                if (CurTime - LastTime <= Settings->DoubleClickTimeout)
                {
                    DispatchInputAction(Name, EInputAction::Toggle);
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

        FInputActionBinding Released{Name, IE_Released};
        Released.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(this, [=] {
            switch (*Type)
            {
            case EPressType::Release:
                DispatchInputAction(Name, EInputAction::Toggle);
                break;
            case EPressType::Continuous:
                DispatchInputAction(Name, EInputAction::Disable);
                break;
            }
        });
        Input->AddActionBinding(MoveTemp(Released));
    }
    else
    {
        FInputActionBinding Pressed{Name, IE_Pressed};
        Pressed.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(this, [=] {
            DispatchInputAction(Name, EInputAction::Enable);
        });
        Input->AddActionBinding(MoveTemp(Pressed));

        FInputActionBinding Released{Name, IE_Released};
        Released.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(this, [=] {
            DispatchInputAction(Name, EInputAction::Disable);
        });
        Input->AddActionBinding(MoveTemp(Released));
    }
}
