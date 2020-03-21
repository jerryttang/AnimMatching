// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SGAnimTypes.h"

#include "SGCharacterStatics.generated.h"

UCLASS()
class ANIMTEST_API USGCharacterStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool IsStanding(ACharacter* Character);
	static bool IsCrouching(ACharacter* Character);
	static bool IsInAir(ACharacter* Character);
	static FGameplayTag GetAnimPoseTag(ACharacter* Character);
	static FVector GetVelocity(ACharacter* Character);
	static FRotator GetCharacterRotation(ACharacter* Character);
	static FRotator GetViewRotation(ACharacter* Character);
};