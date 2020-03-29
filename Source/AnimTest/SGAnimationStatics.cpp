// Fill out your copyright notice in the Description page of Project Settings.


#include "SGAnimationStatics.h"
#include "AnimTagMetaData.h"

UAnimTagMetaData* USGAnimationStatics::GetMetaData(UAnimSequence* AnimSeq)
{
	if (AnimSeq)
	{
		if (AnimSeq->GetMetaData().Num() > 0)
		{
			UAnimTagMetaData *MetaData = Cast<UAnimTagMetaData>(AnimSeq->GetMetaData()[0]);
			return MetaData;
		}
	}
	return nullptr;
}

bool USGAnimationStatics::GetLayerBlendAnimInfo(UAnimSequence* AnimSeq, FName& OutBoneName, int32& OutBlendIndex)
{
	if (AnimSeq)
	{
		if (AnimSeq->GetMetaData().Num() > 0)
		{
			UAnimTagMetaData *MetaData = Cast<UAnimTagMetaData>(AnimSeq->GetMetaData()[0]);
			if (MetaData)
			{
				OutBoneName = MetaData->BoneName;
				OutBlendIndex = MetaData->BlendDepth;
				return true;
			}
		}
	}
	return false;
}
