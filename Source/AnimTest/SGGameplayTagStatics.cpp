// Fill out your copyright notice in the Description page of Project Settings.


#include "SGGameplayTagStatics.h"
#include "SGAnimTypes.h"
#include "GameFramework/Character.h"

bool USGGameplayTagStatics::IsOnLandTag(const FGameplayTag& InTag)
{
	return InTag.MatchesTag(PoseTag_Stand) || InTag.MatchesTag(PoseTag_Crouch) || InTag.MatchesTag(PoseTag_Prone);
}

bool USGGameplayTagStatics::IsInAirTag(const FGameplayTag& InTag)
{
	return InTag.MatchesTag(PoseTag_InAir);
}

bool USGGameplayTagStatics::HasTransitionTag(const FGameplayTagContainer& TagContainer)
{
	return TagContainer.HasTag(TransitionTag);
}
