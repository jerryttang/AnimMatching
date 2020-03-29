// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNode_AnimMatching.h"
#include "Engine/Engine.h"
#include "AnimationRuntime.h"
#include "CharacterAnimationComponent.h"
#include "Animation/AnimSequence.h"

float FAnimNode_AnimMatching::GetCurrentAssetTime()
{
	return InternalTimeAccumulator;
}

float FAnimNode_AnimMatching::GetCurrentAssetTimePlayRateAdjusted()
{
	float AnimTime = 0;
	UAnimSequence* Sequence = GetCurrentAnim(AnimTime);

	float PlayRate = 1.f;
	float EffectivePlayrate = PlayRate * (Sequence ? Sequence->RateScale : 1.0f);
	return (EffectivePlayrate < 0.0f) ? GetCurrentAssetLength() - InternalTimeAccumulator : InternalTimeAccumulator;
}

float FAnimNode_AnimMatching::GetCurrentAssetLength()
{
	float AnimTime = 0;
	UAnimSequence* Sequence = GetCurrentAnim(AnimTime);

	return Sequence ? Sequence->SequenceLength : 0.0f;

}

void FAnimNode_AnimMatching::Initialize_AnyThread(const FAnimationInitializeContext & Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);

	GetEvaluateGraphExposedInputs().Execute(Context);
	
	InternalTimeAccumulator = 0.f;
}

void FAnimNode_AnimMatching::Evaluate_AnyThread(FPoseContext & Output)
{
	Character = Cast<ACharacter>(Output.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner());
	if (Character)
	{
		UCharacterAnimationComponent *AnimComp = Character->FindComponentByClass<UCharacterAnimationComponent>();
		if (AnimComp)
		{
			if (AnimComp->TestShouldBlend())
			{
				GetBlendPose(Output.AnimInstanceProxy->GetDeltaSeconds(), Output.Pose, Output.Curve);
			}
			else
			{
				if (AnimComp->ChoosenAnimSeq)
				{
					AnimComp->ChoosenAnimSeq->GetAnimationPose(Output.Pose, Output.Curve, FAnimExtractContext(AnimComp->AnimTime, false));
				}
			}

			// test if should blend layer animation
			if (AnimComp->TestShouldBlendLayeredAnimation())
			{
				EvaluateLayeredAnimation_AnyThread(AnimComp, Output);
			}
		}
	}
}

void FAnimNode_AnimMatching::EvaluateLayeredAnimation_AnyThread(UCharacterAnimationComponent* AnimComp, FPoseContext & Output)
{
	if (!IsValid(AnimComp))
	{
		return;
	}

	FPoseToLayerBlend& LayeredAnimSeq = AnimComp->ChoosenLayeredAnimSeqInfo;
	if (LayeredAnimSeq.bShouldStopBlend || AnimComp->ChoosenLayeredAnimSeqInfo.BoneBlendWeights.Num() <= 0)
	{
		return;
	}

	FBoneContainer& RequiredBones = Output.AnimInstanceProxy->GetRequiredBones();

	USkeleton* Skeleton = Output.AnimInstanceProxy->GetSkeleton();

	// build desired bone weights
	const TArray<FBoneIndexType>& RequiredBoneIndices = RequiredBones.GetBoneIndicesArray();
	const int32 NumRequiredBones = RequiredBoneIndices.Num();
	DesiredBoneBlendWeights.SetNumZeroed(NumRequiredBones);
	for (int32 RequiredBoneIndex = 0; RequiredBoneIndex < NumRequiredBones; RequiredBoneIndex++)
	{
		const int32 SkeletonBoneIndex = RequiredBones.GetSkeletonIndex(FCompactPoseBoneIndex(RequiredBoneIndex));
		if (ensure(SkeletonBoneIndex != INDEX_NONE))
		{
			DesiredBoneBlendWeights[RequiredBoneIndex] = AnimComp->ChoosenLayeredAnimSeqInfo.BoneBlendWeights[SkeletonBoneIndex];
		}
	}

	CurrentBoneBlendWeights.Reset(DesiredBoneBlendWeights.Num());
	CurrentBoneBlendWeights.AddZeroed(DesiredBoneBlendWeights.Num());

	//Reinitialize bone blend weights now that we have cleared them
	TArray<float> BlendWeights;
	BlendWeights.Add(1.f);
	FAnimationRuntime::UpdateDesiredBoneWeight(DesiredBoneBlendWeights, CurrentBoneBlendWeights, BlendWeights);




	//FPoseContext BasePoseContext(Output);

	int32 NumPoses = 1;

	TArray<FCompactPose> TargetBlendPoses;
	TargetBlendPoses.SetNum(NumPoses);

	TArray<FBlendedCurve> TargetBlendCurves;
	TargetBlendCurves.SetNum(NumPoses);


	//FPoseContext CurrentPoseContext(Output);
	TargetBlendPoses[0] = Output.Pose;
	TargetBlendCurves[0] = Output.Curve;


	FCompactPose BasePoseToBlend = Output.Pose;
	FBlendedCurve BaseCurveToBlend = Output.Curve;

	AnimComp->ChoosenLayeredAnimSeqInfo.BlendAnim->GetAnimationPose(TargetBlendPoses[0], TargetBlendCurves[0], FAnimExtractContext(AnimComp->ChoosenLayeredAnimSeqInfo.CurrentAnimTime, false));
	

	TEnumAsByte<enum ECurveBlendOption::Type>	CurveBlendOption = ECurveBlendOption::MaxWeight;
	FAnimationRuntime::EBlendPosesPerBoneFilterFlags BlendFlags = FAnimationRuntime::EBlendPosesPerBoneFilterFlags::None;
	FAnimationRuntime::BlendPosesPerBoneFilter(BasePoseToBlend, TargetBlendPoses, BaseCurveToBlend, TargetBlendCurves, Output.Pose, Output.Curve, CurrentBoneBlendWeights, BlendFlags, CurveBlendOption);
	Output.Pose.NormalizeRotations();
}

void FAnimNode_AnimMatching::OverrideAsset(UAnimationAsset * NewAsset)
{
	Super::OverrideAsset(NewAsset);
}


void FAnimNode_AnimMatching::GatherDebugData(FNodeDebugData& DebugData)
{
	Super::GatherDebugData(DebugData);
}

UAnimSequence * FAnimNode_AnimMatching::GetCurrentAnim(float& OutAnimTime)
{
	if (Character)
	{
		UCharacterAnimationComponent *AnimComp = Character->FindComponentByClass<UCharacterAnimationComponent>();
		if (AnimComp)
		{
			OutAnimTime = AnimComp->AnimTime;
			return AnimComp->ChoosenAnimSeq;
		}
	}
	return NULL;
}



void FAnimNode_AnimMatching::GetBlendPose(const float DT, FCompactPose & OutPose, FBlendedCurve & OutCurve)
{
	if (Character)
	{
		UCharacterAnimationComponent *AnimComp = Character->FindComponentByClass<UCharacterAnimationComponent>();
		if (!AnimComp)
		{
			return;
		}
		FScopeLock(&AnimComp->Mutex);
		
		TArray<FPoseToBlend> BlendPosesCache = AnimComp->BlendPosesList;
		for (int32 Index = 0; Index < BlendPosesCache.Num(); ++Index)
		{
			if (BlendPosesCache[Index].bShouldStopBlend)
			{
				BlendPosesCache.RemoveAt(Index, 1);
				--Index;
			}
		}
		const int32 NumPoses = BlendPosesCache.Num();

		if (NumPoses > 0)
		{
			TArray<FCompactPose, TInlineAllocator<8>> ChildrenPoses;
			ChildrenPoses.AddZeroed(NumPoses);

			TArray<FBlendedCurve, TInlineAllocator<8>> ChildrenCurves;
			ChildrenCurves.AddZeroed(NumPoses);

			TArray<float, TInlineAllocator<8>> ChildrenWeights;
			ChildrenWeights.AddZeroed(NumPoses);

			TArray<FTransform> ChildrenRootMotions;
			ChildrenRootMotions.AddZeroed(NumPoses);

			for (int32 ChildrenIdx = 0; ChildrenIdx < ChildrenPoses.Num(); ++ChildrenIdx)
			{
				ChildrenPoses[ChildrenIdx].SetBoneContainer(&OutPose.GetBoneContainer());
				ChildrenCurves[ChildrenIdx].InitFrom(OutCurve);
			}

			// get all child atoms we interested in

			float Sum = 0.f;

			for (int32 Index = 0; Index < NumPoses; Index++)
			{
				FCompactPose& Pose = ChildrenPoses[Index];

				ChildrenWeights[Index] = BlendPosesCache[Index].CurrentWeight;
				Sum += ChildrenWeights[Index];

				float AnimTime = BlendPosesCache[Index].CurrentAnimTime;

				BlendPosesCache[Index].BlendAnim->GetAnimationPose(Pose, ChildrenCurves[Index], FAnimExtractContext(AnimTime, false));
			}


			if (Sum > 0.f)
			{

				TArrayView<FCompactPose> ChildrenPosesView(ChildrenPoses);

				for (int32 Index = 0; Index < ChildrenWeights.Num(); Index++)
				{
					ChildrenWeights[Index] = ChildrenWeights[Index] / Sum;
				}

				FAnimationRuntime::BlendPosesTogether(ChildrenPosesView, ChildrenCurves, ChildrenWeights, OutPose, OutCurve);
				OutPose.NormalizeRotations();
			}
		}
		
	}
}

void FAnimNode_AnimMatching::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
{
	Super::CacheBones_AnyThread(Context);
}

void FAnimNode_AnimMatching::UpdateAssetPlayer(const FAnimationUpdateContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);
}


