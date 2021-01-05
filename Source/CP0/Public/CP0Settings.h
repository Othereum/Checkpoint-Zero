// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#pragma once

#include "UObject/NoExportTypes.h"
#include "CP0Settings.generated.h"

/**
 *
 */
UCLASS()
class CP0_API UCP0Settings : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Save()
	{
		SaveConfig();
	}
};
