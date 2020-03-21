// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "AnimTagMetaData.h"
#include "Animation/AnimSequence.h"

#include "SGAnimTypes.generated.h"


static const FGameplayTag PoseTag_Stand = FGameplayTag::RequestGameplayTag(FName("AnimTag.Pose.Stand"));
static const FGameplayTag PoseTag_Crouch = FGameplayTag::RequestGameplayTag(FName("AnimTag.Pose.Crouch"));
static const FGameplayTag PoseTag_Prone = FGameplayTag::RequestGameplayTag(FName("AnimTag.Pose.Prone"));
static const FGameplayTag PoseTag_InAir = FGameplayTag::RequestGameplayTag(FName("AnimTag.Pose.InAir"));
static const FGameplayTag TransitionTag = FGameplayTag::RequestGameplayTag(FName("AnimTag.Transition"));
static const FGameplayTag TransitionTag_LandToAir = FGameplayTag::RequestGameplayTag(FName("AnimTag.Transition.LandToAir"));
static const FGameplayTag TransitionTag_AirToLand = FGameplayTag::RequestGameplayTag(FName("AnimTag.Transition.AirToLand"));
static const FGameplayTag VelocityTag_SizeSkip = FGameplayTag::RequestGameplayTag(FName("AnimTag.Veloctiy.Size.Skip"));
static const FGameplayTag VelocityTag_DirSkip = FGameplayTag::RequestGameplayTag(FName("AnimTag.Veloctiy.Dir.Skip"));

USTRUCT(BlueprintType)
struct FAnimSetInfo
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimSequence* AnimSeq;
};

UENUM(BlueprintType)
enum class EAnimPose : uint8
{
	EAnimPose_None,
	EAnimPose_Stand,
	EAnimPose_Crouch,
	EAnimPose_Prone,
};


USTRUCT(BlueprintType)
struct FAnimMatchDebugInfo
{
	GENERATED_USTRUCT_BODY()

public:
	FString AnimSeqName;

	int32 PoseCost;
	int32 VelocitySizeCost;
	int32 VelocityDirCost_Yaw;
	int32 VelocityDirCost_Pitch;
	int32 TotalCost;

	FString ToString() const
	{
		return FString::Printf(TEXT("AnimSeqName = %s, TotalCost = %d, PoseCost = %d, VelocitySizeCost = %d, VelocityDirCost_Yaw = %d, VelocityDirCost_Pitch = %d\n"), 
			*AnimSeqName, TotalCost, PoseCost, VelocitySizeCost, VelocityDirCost_Yaw, VelocityDirCost_Pitch);
	}

	bool operator<(const FAnimMatchDebugInfo& CompareInfo) const
	{
		return TotalCost < CompareInfo.TotalCost;
	}
};

USTRUCT(BlueprintType)
struct FPoseToBlend
{
	GENERATED_USTRUCT_BODY()

public:
	FPoseToBlend(UAnimSequence* InSeq, bool bInBlendIn, float InAnimTime) : BlendAnim(InSeq), bBlendIn(bInBlendIn), CurrentAnimTime(InAnimTime)
	{
		if (BlendAnim)
		{
			TArray<UAnimMetaData*> AnimMetaDataList = BlendAnim->GetMetaData();
			if (AnimMetaDataList.Num() > 0)
			{
				UAnimTagMetaData* MetaData = Cast<UAnimTagMetaData>(AnimMetaDataList[0]);
				if (MetaData)
				{
					if (bInBlendIn)
					{
						// is a blend in pose
						TotalBlendTime = MetaData->BlendInTime;
					}
					else
					{
						// is a blend out pose
						TotalBlendTime = MetaData->BlendOutTime;
					}
					bShouldLoop = MetaData->bShouldLoop;
				}
			}
		}
		CurrentTime = 0;
	}

	FPoseToBlend(){}

	float UpdateWeight(float DeltaTime)
	{
		CurrentWeight = (bBlendIn ? 1.f : 0);
		if (BlendAnim && !bShouldStopBlend)
		{
			CurrentAnimTime += DeltaTime;
			if (CurrentAnimTime > BlendAnim->GetPlayLength())
			{
				if (bShouldLoop)
				{
					CurrentAnimTime = CurrentAnimTime - BlendAnim->GetPlayLength();
				}
				else
				{
					CurrentAnimTime = BlendAnim->GetPlayLength();
				}
			}
			if (TotalBlendTime > 0)
			{
				CurrentTime = FMath::Clamp<float>(CurrentTime, CurrentTime + DeltaTime, TotalBlendTime);
				
				if (CurrentTime < TotalBlendTime)
				{
					if (bBlendIn)
					{
						CurrentWeight = CurrentTime / TotalBlendTime;
					}
					else
					{
						CurrentWeight = 1 - CurrentTime / TotalBlendTime;
					}
				}
				else
				{
					FinishBlend();
				}
			}
			else
			{
				FinishBlend();
			}
		}
		return CurrentWeight;
	}

	void FinishBlend()
	{
		bShouldStopBlend = true;
	}

	void Reset(bool bInBlendIn)
	{
		CurrentTime = 0;
		CurrentAnimTime = 0;
		bBlendIn = bInBlendIn;
		bShouldStopBlend = false;
	}

	bool operator==(const FPoseToBlend& Other) const
	{
		if (this == &Other)
		{
			return true;
		}
		return BlendAnim == Other.BlendAnim;
	}

	FString ToString()
	{
		return FString::Printf(TEXT("AnimName = %s, CurrentAnimTime = %f, Weight = %f"), 
			*BlendAnim->GetName(), CurrentAnimTime, CurrentWeight);
	}

	UPROPERTY()
		UAnimSequence* BlendAnim;

	bool bBlendIn = false;
	float CurrentTime = 0;
	float TotalBlendTime = 0;
	float CurrentAnimTime = 0;
	bool bShouldStopBlend = false;
	float CurrentWeight = 0;
	bool bShouldLoop = false;
};