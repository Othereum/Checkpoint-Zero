// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "CP0Settings.h"
#include "CP0InputSettings.generated.h"

UENUM(BlueprintType)
enum class EPressType : uint8
{
	Press,
	Release,
	Continuous,
	DoubleClick
};

/**
 *
 */
UCLASS(Config = Input)
class CP0_API UCP0InputSettings final : public UCP0Settings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, BlueprintReadWrite)
	float DoubleClickTimeout = 0.3;

	UPROPERTY(Config, BlueprintReadWrite)
	TMap<FName, EPressType> PressTypes;
};
