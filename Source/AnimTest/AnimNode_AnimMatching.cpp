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
				AnimComp->ChoosenAnimSeq->GetAnimationPose(Output.Pose, Output.Curve, FAnimExtractContext(AnimComp->AnimTime, false));
			}
		}
	}
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


