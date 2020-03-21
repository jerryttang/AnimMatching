// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CharacterAnimationComponent.h"
#include "AnimTestCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimSequence.h"
#include "SGAnimTypes.h"
#include "SGCharacterStatics.h"
#include "AnimTagMetaData.h"
#include "SGGameplayTagStatics.h"

UCharacterAnimationComponent::UCharacterAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterAnimationComponent::BeginPlay()
{
	Super::BeginPlay();
	PreProcessAnimationSet();
}

void UCharacterAnimationComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RefreshBlendPoseList(DeltaTime);
	TickMostRightAnimation(DeltaTime);
	TickPlayAnimTime(DeltaTime);
	TickDebugInfo();
	
}

ACharacter* UCharacterAnimationComponent::GetCharacter()
{
	return Cast<ACharacter>(GetOwner());
}

void UCharacterAnimationComponent::TickMostRightAnimation(float DeltaTime)
{
	// pose : stand/crouch/prone/fall
	CurrentPoseTag = USGCharacterStatics::GetAnimPoseTag(GetCharacter());
	

	// speed : walk/run/sprint
	FVector CharacterVeloctiy = USGCharacterStatics::GetVelocity(GetCharacter());
	CurrentVelocitySize = CharacterVeloctiy.Size();
	if (CurrentVelocitySize > 0)
	{
		// for breakpoints
		CurrentVelocitySize = CurrentVelocitySize;
	}
	CharacterRotation = USGCharacterStatics::GetCharacterRotation(GetCharacter());

	// to local dir
	FRotator DiffRotator = (CharacterRotation - CharacterVeloctiy.ToOrientationRotator()).GetNormalized();
	VelocityDirDiffAngle_Yaw = DiffRotator.Yaw;
	VelocityDirDiffAngle_Pitch = DiffRotator.Pitch;

	// transition : stand->crouch/crouch->prone etc.
	if (CurrentPoseTag != LastPoseTag)
	{
		OnPoseChanged(LastPoseTag, CurrentPoseTag);
	}

	// from all animation sets, choose the best one
	int32 WinnerIndex = INDEX_NONE;
	int32 MinCost = MAX_COST;
	DebugInfoList.Empty();
	for (int32 Index = 0; Index < AnimationSet.Num(); ++Index)
	{
		UAnimSequence* AnimSeq = AnimationSet[Index].AnimSeq;
		if (AnimSeq)
		{
			FAnimMatchDebugInfo DebugInfo;
			DebugInfo.AnimSeqName = AnimSeq->GetName();

			// calc pose cost
			int32 TotalCost = GetAnimCost(AnimSeq, DebugInfo);
			if (TotalCost < MinCost)
			{
				MinCost = TotalCost;
				WinnerIndex = Index;
			}

			// record for debug
			DebugInfo.AnimSeqName = AnimSeq->GetName();
			DebugInfo.TotalCost = TotalCost;
			DebugInfoList.Add(DebugInfo);
		}
	}

	if (WinnerIndex != INDEX_NONE)
	{
		// play anim with the winner index
		if (ChoosenAnimSeq != AnimationSet[WinnerIndex].AnimSeq)
		{
			UAnimSequence* LastAnimSeq = ChoosenAnimSeq;
			ChoosenAnimSeq = AnimationSet[WinnerIndex].AnimSeq;
			OnChoosenAnimChanged(LastAnimSeq, ChoosenAnimSeq);
		}
	}

	LastPoseTag = CurrentPoseTag;
}

void UCharacterAnimationComponent::OnChoosenAnimChanged(UAnimSequence* OldAnimSeq, UAnimSequence* NewAnimSeq)
{
	if (NewAnimSeq)
	{
		TArray<UAnimMetaData*> MetaDataList = ChoosenAnimSeq->GetMetaData();
		if (MetaDataList.Num() > 0)
		{
			UAnimTagMetaData* TagMetaData = Cast<UAnimTagMetaData>(MetaDataList[0]);
			if (TagMetaData)
			{
				bAnimLoop = TagMetaData->bShouldLoop;
			}
		}
	}

	FPoseToBlend NewPoseToBlend(NewAnimSeq, true, 0);
	int32 Index = BlendPosesList.Find(NewPoseToBlend);
	if (BlendPosesList.IsValidIndex(Index))
	{
		// not remove, just change value
		BlendPosesList[Index].Reset(true);
	}
	else
	{
		BlendPosesList.Add(NewPoseToBlend);
	}

	if (OldAnimSeq)
	{
		FPoseToBlend OldPoseToBlend(OldAnimSeq, false, AnimTime);
		Index = BlendPosesList.Find(OldPoseToBlend);
		if (BlendPosesList.IsValidIndex(Index))
		{
			// not remove, just change value
			BlendPosesList[Index].Reset(false);
			BlendPosesList[Index].CurrentAnimTime = AnimTime;
		}
		else
		{
			BlendPosesList.Add(OldPoseToBlend);
		}
	}

	AnimTime = 0;
}

void UCharacterAnimationComponent::OnPoseChanged(FGameplayTag& InLastPose, FGameplayTag& InCurrentPose)
{
	if (USGGameplayTagStatics::IsOnLandTag(InLastPose) && USGGameplayTagStatics::IsInAirTag(InCurrentPose))
	{
		TransitionTag = TransitionTag_LandToAir;
	}
	else if (USGGameplayTagStatics::IsInAirTag(InLastPose) && USGGameplayTagStatics::IsOnLandTag(InCurrentPose))
	{
		TransitionTag = TransitionTag_AirToLand;
	}
}

void UCharacterAnimationComponent::TickPlayAnimTime(float DeltaTime)
{
	if (ChoosenAnimSeq)
	{
		float AnimLength = ChoosenAnimSeq->GetPlayLength();
		AnimTime += DeltaTime;
		if (AnimTime > AnimLength)
		{
			// consider if should loop
			if (bAnimLoop)
			{
				AnimTime = 0;
			}
			else
			{
				// stop at the last frame
				AnimTime = AnimLength;
				OnAnimFinished();
			}
		}
	}
}

void UCharacterAnimationComponent::OnAnimFinished()
{
	if (ChoosenAnimSeq)
	{
		TArray<UAnimMetaData*> MetaDataList = ChoosenAnimSeq->GetMetaData();
		if (MetaDataList.Num() > 0)
		{
			UAnimTagMetaData* TagMetaData = Cast<UAnimTagMetaData>(MetaDataList[0]);
			if (TagMetaData)
			{
				if (USGGameplayTagStatics::HasTransitionTag(TagMetaData->TagList))
				{
					// untill now we clear the transition tag
					TransitionTag = FGameplayTag::EmptyTag;
				}
			}
		}
	}
}

int32 UCharacterAnimationComponent::GetAnimCost(UAnimSequence* AnimSeq, FAnimMatchDebugInfo& DebugInfo)
{
	int32 ResultCost = MAX_COST;
	if (AnimSeq)
	{
		ResultCost = 0;

		TArray<UAnimMetaData*> MetaDataList = AnimSeq->GetMetaData();
		if (MetaDataList.Num() > 0)
		{
			UAnimTagMetaData* TagMetaData = Cast<UAnimTagMetaData>(MetaDataList[0]);
			if (TagMetaData)
			{
				// test pose && transition
				if (TransitionTag == FGameplayTag::EmptyTag)
				{
					bool bHasTag = TagMetaData->TagList.HasTag(CurrentPoseTag);
					ResultCost += (bHasTag ? 0 : WrongPoseCost);
					DebugInfo.PoseCost = (bHasTag ? 0 : WrongPoseCost);
				}
				else
				{
					bool bHasTag = TagMetaData->TagList.HasTag(TransitionTag);
					ResultCost += (bHasTag ? 0 : WrongPoseCost);
					DebugInfo.PoseCost = (bHasTag ? 0 : WrongPoseCost);
				}

				// test velocity size
				int32 TempValue = FMath::Abs(CurrentVelocitySize - TagMetaData->VeloctiySize) * VelocitySizeDiffCostScale;
				ResultCost += TempValue;
				DebugInfo.VelocitySizeCost = TempValue;

				// test velocity dir -> rotation angle todo: GetManhattanDistance
				TempValue = FMath::Abs(TagMetaData->VelocityDirDiff_Yaw - VelocityDirDiffAngle_Yaw) * VelocityDirDiffCostScale_Yaw;
				ResultCost += TempValue;
				DebugInfo.VelocityDirCost_Yaw = TempValue;

				TempValue = FMath::Abs(TagMetaData->VelocityDirDiff_Pitch - VelocityDirDiffAngle_Pitch) * VelocityDirDiffCostScale_Pitch;
				ResultCost += TempValue;
				DebugInfo.VelocityDirCost_Pitch = TempValue;

				
			}
		}
	}
	return ResultCost;
}


void UCharacterAnimationComponent::PreProcessAnimationSet()
{
	
}

void UCharacterAnimationComponent::TickDebugInfo()
{
	if (bDebug)
	{
		FString DebugString = FString::Printf(TEXT("ChoosenAnim = %s\nAnimTime = %f\n\n"), *ChoosenAnimSeq->GetName(), AnimTime);
		DebugInfoList.Sort();
		for (const FAnimMatchDebugInfo& DebugInfo : DebugInfoList)
		{
			DebugString += DebugInfo.ToString();
		}
		for (FPoseToBlend& PoseToBlend : BlendPosesList)
		{
			DebugString += ("\n" + PoseToBlend.ToString());
		}
		GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Yellow, DebugString);
	}
}

void UCharacterAnimationComponent::RefreshBlendPoseList(float DeltaTime)
{
	// first check validation of the elements in BlendPosesList
	for (int32 Index = 0; Index < BlendPosesList.Num(); ++Index)
	{
		if (!BlendPosesList[Index].bShouldStopBlend)
		{
			BlendPosesList[Index].UpdateWeight(DeltaTime);
			if (BlendPosesList[Index].bShouldStopBlend && BlendPosesList[Index].bBlendIn)
			{
				AnimTime = BlendPosesList[Index].CurrentAnimTime;
			}
		}
	}
}

bool UCharacterAnimationComponent::TestShouldBlend()
{
	for (int32 Index = 0; Index < BlendPosesList.Num(); ++Index)
	{
		if (!BlendPosesList[Index].bShouldStopBlend)
		{
			return true;
		}
	}
	return false;
}
