// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimGraphNode_AnimMatching.h"


#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_AnimMatching::UAnimGraphNode_AnimMatching(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

}

FLinearColor UAnimGraphNode_AnimMatching::GetNodeTitleColor() const
{
	return FLinearColor::Blue;
}

FText UAnimGraphNode_AnimMatching::GetTooltipText() const
{
	return LOCTEXT("Generates_A_Pose_By_Matching_Animation_Sets", "Generates a pose by Matching Animation Sets");
}

FText UAnimGraphNode_AnimMatching::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Anim_Matching", "Anim Matching");
}

FString UAnimGraphNode_AnimMatching::GetNodeCategory() const
{
	return TEXT("Tools");
}

#undef LOCTEXT_NAMESPACE

