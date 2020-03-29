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
static const FGameplayTag TransitionTag_StandToCrouch = FGameplayTag::RequestGameplayTag(FName("AnimTag.Transition.StandToCrouch"));
static const FGameplayTag TransitionTag_StandToProne = FGameplayTag::RequestGameplayTag(FName("AnimTag.Transition.StandToProne"));
static const FGameplayTag TransitionTag_CrouchToStand = FGameplayTag::RequestGameplayTag(FName("AnimTag.Transition.CrouchToStand"));
static const FGameplayTag TransitionTag_CrouchToProne = FGameplayTag::RequestGameplayTag(FName("AnimTag.Transition.CrouchToProne"));
static const FGameplayTag TransitionTag_ProneToStand = FGameplayTag::RequestGameplayTag(FName("AnimTag.Transition.ProneToStand"));
static const FGameplayTag TransitionTag_ProneToCrouch = FGameplayTag::RequestGameplayTag(FName("AnimTag.Transition.ProneToCrouch"));
static const FGameplayTag VelocityTag_SizeSkip = FGameplayTag::RequestGameplayTag(FName("AnimTag.Veloctiy.Size.Skip"));
static const FGameplayTag VelocityTag_DirSkip = FGameplayTag::RequestGameplayTag(FName("AnimTag.Veloctiy.Dir.Skip"));

static const FGameplayTag UpperBodyTag_Aim = FGameplayTag::RequestGameplayTag(FName("AnimTag.UpperBody.Aim"));
static const FGameplayTag UpperBodyTag_Fire = FGameplayTag::RequestGameplayTag(FName("AnimTag.UpperBody.Fire"));

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

	virtual float UpdateWeight(float DeltaTime)
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

USTRUCT(BlueprintType)
struct FPoseToLayerBlend : public FPoseToBlend
{
	GENERATED_USTRUCT_BODY()
public:

	FPoseToLayerBlend() {}
	FPoseToLayerBlend(UAnimSequence* InSeq, bool bInBlendIn, FName InBoneName, int32 InBlendDepth) : FPoseToBlend(InSeq, bBlendIn, 0), BoneName(InBoneName), BlendDepth(InBlendDepth)
	{
		bShouldStopBlend = true;
	}
	
	virtual ~FPoseToLayerBlend() {}

	void BuildCacheData(USkeleton* InSkeleton)
	{
		const FReferenceSkeleton& RefSkeleton = InSkeleton->GetReferenceSkeleton();
		const int32 MaskBoneIndex = RefSkeleton.FindBoneIndex(BoneName);

		const int32 NumBones = RefSkeleton.GetNum();
		BoneBlendWeights.Reset(NumBones);
		BoneBlendWeights.AddZeroed(NumBones);

		if (MaskBoneIndex != INDEX_NONE)
		{
			// how much weight increase Per depth
			const float IncreaseWeightPerDepth = (BlendDepth != 0) ? (1.f / ((float)BlendDepth)) : 1.f;

			// go through skeleton bone hierarchy.
			// Bones are ordered, parents before children. So we can start looking at MaskBoneIndex for children.
			for (int32 BoneIndex = MaskBoneIndex; BoneIndex < NumBones; ++BoneIndex)
			{
				// if Depth == -1, it's not a child
				const int32 Depth = RefSkeleton.GetDepthBetweenBones(BoneIndex, MaskBoneIndex);
				if (Depth != -1)
				{
					// when you write to buffer, you'll need to match with BasePoses BoneIndex
					FPerBoneBlendWeight& BoneBlendWeight = BoneBlendWeights[BoneIndex];

					BoneBlendWeight.SourceIndex = 0;
					const float BlendIncrease = IncreaseWeightPerDepth * (float)(Depth + 1);
					BoneBlendWeight.BlendWeight = FMath::Clamp<float>(BoneBlendWeight.BlendWeight + BlendIncrease, 0.f, 1.f);
				}
			}
		}
	}

	virtual float UpdateWeight(float DeltaTime) override
	{
		if (!bShouldStopBlend)
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
					bShouldStopBlend = true;
				}
			}
			return 1.f;
		}
		return 0;
	}

public:
	FName BoneName;
	int32 BlendDepth;
	UPROPERTY()
		TArray<FPerBoneBlendWeight>	BoneBlendWeights;
};