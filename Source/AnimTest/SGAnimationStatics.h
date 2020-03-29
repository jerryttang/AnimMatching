// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SGAnimTypes.h"
#include "AnimTagMetaData.h"

#include "SGAnimationStatics.generated.h"

UCLASS()
class ANIMTEST_API USGAnimationStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UAnimTagMetaData* GetMetaData(UAnimSequence* AnimSeq);

	static bool GetLayerBlendAnimInfo(UAnimSequence* AnimSeq, FName& OutBoneName, int32& OutBlendIndex);
};