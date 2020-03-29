// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SGAnimTypes.h"

#include "SGGameplayTagStatics.generated.h"

UCLASS()
class ANIMTEST_API USGGameplayTagStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool IsOnLandTag(const FGameplayTag& InTag);
	static bool IsInAirTag(const FGameplayTag& InTag);
	static bool IsStandTag(const FGameplayTag& InTag);
	static bool IsCrouchTag(const FGameplayTag& InTag);
	static bool IsProneTag(const FGameplayTag& InTag);
	static bool HasTransitionTag(const FGameplayTagContainer& TagContainer);
};