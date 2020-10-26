// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0GameInstance.h"
#include "CP0InputSettings.h"

UCP0GameInstance::UCP0GameInstance() : InputSettings{CreateDefaultSubobject<UCP0InputSettings>(TEXT("InputSettings"))}
{
}
