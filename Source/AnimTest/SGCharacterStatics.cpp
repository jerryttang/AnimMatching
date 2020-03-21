// Fill out your copyright notice in the Description page of Project Settings.


#include "SGCharacterStatics.h"
#include "GameFramework/Character.h"

bool USGCharacterStatics::IsStanding(ACharacter* Character)
{
	// default is standing
	return Character ? !Character->bIsCrouched : true;
}

bool USGCharacterStatics::IsCrouching(ACharacter* Character)
{
	return Character ? Character->bIsCrouched : false;
}

bool USGCharacterStatics::IsInAir(ACharacter* Character)
{
	return Character ? Character->GetCharacterMovement()->IsFalling() : false;
}

FGameplayTag USGCharacterStatics::GetAnimPoseTag(ACharacter* Character)
{
	FGameplayTag ResultTag = FGameplayTag::EmptyTag;
	if (IsInAir(Character))
	{
		ResultTag = PoseTag_InAir;
	}
	else if (IsStanding(Character))
	{
		ResultTag = PoseTag_Stand;
	}
	else if (IsCrouching(Character))
	{
		ResultTag = PoseTag_Crouch;
	}
	
	return ResultTag;
}

FVector USGCharacterStatics::GetVelocity(ACharacter* Character)
{
	return Character ? Character->GetVelocity() : FVector::ZeroVector;
}

FRotator USGCharacterStatics::GetCharacterRotation(ACharacter* Character)
{
	return Character ? Character->GetActorRotation() : FRotator::ZeroRotator;
}

FRotator USGCharacterStatics::GetViewRotation(ACharacter* Character)
{
	return Character ? Character->GetViewRotation() : FRotator::ZeroRotator;
}

