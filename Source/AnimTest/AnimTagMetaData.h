// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMetaData.h"
#include "GameplayTagContainer.h"
#include "AnimTagMetaData.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ANIMTEST_API UAnimTagMetaData : public UAnimMetaData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		FGameplayTagContainer TagList;

	UPROPERTY(EditAnywhere)
		float VeloctiySize = 0;

	UPROPERTY(EditAnywhere)
		float VelocityDirDiff_Yaw = 0;

	UPROPERTY(EditAnywhere)
		float VelocityDirDiff_Pitch = 0;

	UPROPERTY(EditAnywhere)
		bool bShouldLoop = true;

	UPROPERTY(EditAnywhere, Category = "Blend")
		float BlendInTime = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Blend")
		float BlendOutTime = 0.2f;
};
