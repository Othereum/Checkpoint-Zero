// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0Character.h"
#include "CP0.h"
#include "CP0CharacterMovement.h"
#include "CP0GameInstance.h"
#include "CP0InputSettings.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

template <class Action>
static void DispatchInputActionByType(ACP0Character* Character, EInputAction Type);

#define MAKE_ACTION(Name) {TEXT(#Name), &DispatchInputActionByType<F##Name##Action>}

static TMap<FName, void (*)(ACP0Character*, EInputAction)> InputActionMap{
    MAKE_ACTION(Sprint),
    MAKE_ACTION(Crouch),
    MAKE_ACTION(Prone),
};

#undef MAKE_ACTION

ACP0Character::ACP0Character(const FObjectInitializer& Initializer)
    : Super{Initializer.SetDefaultSubobjectClass<UCP0CharacterMovement>(ACharacter::CharacterMovementComponentName)}
{
    PrimaryActorTick.bCanEverTick = true;
    BaseEyeHeight = 150.0f;
    CrouchedEyeHeight = 100.0f;
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

    EyeHeightAlpha = FMath::Clamp(EyeHeightAlpha + DeltaTime / EyeHeightBlendTime, 0.0f, 1.0f);
    SetEyeHeight(FMath::CubicInterp(PrevEyeHeight, 0.0f, TargetEyeHeight, 0.0f, EyeHeightAlpha));
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class Action>
static void DispatchInputActionByType(ACP0Character* Character, EInputAction Type)
{
    switch (Type)
    {
    case EInputAction::Enable:
        Action::Enable(GetActionObject<Action>(Character));
        break;
    case EInputAction::Disable:
        Action::Disable(GetActionObject<Action>(Character));
        break;
    case EInputAction::Toggle:
        Action::Toggle(GetActionObject<Action>(Character));
        break;
    default:
        ensureNoEntry();
    }
}

template <class Action>
[[nodiscard]] static decltype(Action::GetObject(DeclVal<ACP0Character*>())) GetActionObject(ACP0Character* Character)
{
    return Action::GetObject(Character);
}

template <class Action>
[[nodiscard]] static
    typename TEnableIf<TIsPointer<decltype(Action::Enable(DeclVal<ACP0Character*>()))*>::Value, ACP0Character*>::Type
    GetActionObject(ACP0Character* Character)
{
    return Character;
}

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
    const auto Dispatcher = InputActionMap.Find(Name);
    if (ensure(Dispatcher))
        (*Dispatcher)(this, Type);

    if (!HasAuthority())
        ServerInputAction(Name, Type);
}

void ACP0Character::BindInputAction(UInputComponent* Input, FName Name)
{
    FInputActionBinding Pressed{Name, IE_Pressed};
    Pressed.ActionDelegate.GetDelegateForManualSet().BindWeakLambda(this, [=, LastTime = -1.0f]() mutable {
        const auto GI = CastChecked<UCP0GameInstance>(GetGameInstance());
        const auto Settings = GI->GetInputSettings();

        switch (Settings->PressTypes[Name])
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
        const auto GI = CastChecked<UCP0GameInstance>(GetGameInstance());
        const auto Settings = GI->GetInputSettings();

        switch (Settings->PressTypes[Name])
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
