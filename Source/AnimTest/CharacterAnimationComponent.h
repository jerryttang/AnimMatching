// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SGAnimTypes.h"
#include "CharacterAnimationComponent.generated.h"


static const int32 MAX_COST = 10000;

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class ANIMTEST_API UCharacterAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterAnimationComponent();

	virtual void BeginPlay();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	ACharacter* GetCharacter();

	void TickMostRightAnimation(float DeltaTime);

	void OnChoosenAnimChanged(UAnimSequence* OldAnimSeq, UAnimSequence* NewAnimSeq);

	void OnPoseChanged(FGameplayTag& InLastPose, FGameplayTag& InCurrentPose);

	void TickPlayAnimTime(float DeltaTime);

	void OnAnimFinished();

	int32 GetAnimCost(UAnimSequence* AnimSeq, FAnimMatchDebugInfo& DebugInfo);

	void PreProcessAnimationSet();

	void TickDebugInfo();

	void RefreshBlendPoseList(float DeltaTime);

	bool TestShouldBlend();

protected:

	UPROPERTY(EditAnywhere)
		bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Asset")
		TArray<FAnimSetInfo> AnimationSet;

	UPROPERTY(EditAnywhere, Category = "Cost")
		int32 WrongPoseCost = 1000;

	UPROPERTY(EditAnywhere, Category = "Cost")
		float VelocitySizeDiffCostScale = 1.f;

	UPROPERTY(EditAnywhere, Category = "Cost")
		float VelocityDirDiffCostScale_Yaw = 1.f;

	UPROPERTY(EditAnywhere, Category = "Cost")
		float VelocityDirDiffCostScale_Pitch = 1.f;
	
protected:
	FGameplayTag CurrentPoseTag;
	FGameplayTag LastPoseTag;
	FGameplayTag TransitionTag;

	UPROPERTY(BlueprintReadOnly)
		float VelocityDirDiffAngle_Yaw;

	UPROPERTY(BlueprintReadOnly)
		float VelocityDirDiffAngle_Pitch;

	FRotator CharacterRotation;
	float CurrentVelocitySize;
	bool bAnimLoop = true;

	TArray<FAnimMatchDebugInfo> DebugInfoList;

public:
	UPROPERTY(BlueprintReadOnly)
		UAnimSequence* ChoosenAnimSeq;

	UPROPERTY(BlueprintReadOnly)
		TArray<FPoseToBlend> BlendPosesList;

	UPROPERTY(BlueprintReadOnly)
		float AnimTime;

	FCriticalSection Mutex;
};



