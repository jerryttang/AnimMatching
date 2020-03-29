// Fill out your copyright notice in the Description page of Project Settings.


#include "SGCharacterStatics.h"
#include "GameFramework/Character.h"
#include "AnimTestCharacter.h"

bool USGCharacterStatics::IsStanding(ACharacter* Character)
{
	// default is standing
	AAnimTestCharacter *TestCharacter = Cast<AAnimTestCharacter>(Character);
	return TestCharacter ? !TestCharacter->bIsCrouch && !TestCharacter->bIsProne: true;
}

bool USGCharacterStatics::IsCrouching(ACharacter* Character)
{
	AAnimTestCharacter *TestCharacter = Cast<AAnimTestCharacter>(Character);
	return TestCharacter ? TestCharacter->bIsCrouch : false;
}

bool USGCharacterStatics::IsInAir(ACharacter* Character)
{
	return Character ? Character->GetCharacterMovement()->IsFalling() : false;
}

bool USGCharacterStatics::IsProne(ACharacter* Character)
{
	AAnimTestCharacter *TestCharacter = Cast<AAnimTestCharacter>(Character);
	return TestCharacter ? TestCharacter->bIsProne : false;
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
	else if (IsProne(Character))
	{
		ResultTag = PoseTag_Prone;
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

bool USGCharacterStatics::IsFiring(ACharacter* Character)
{
	AAnimTestCharacter *TestCharacter = Cast<AAnimTestCharacter>(Character);
	return TestCharacter ? TestCharacter->bIsFiring : false;
}

bool USGCharacterStatics::IsAiming(ACharacter* Character)
{
	AAnimTestCharacter *TestCharacter = Cast<AAnimTestCharacter>(Character);
	return TestCharacter ? TestCharacter->bIsAiming : false;
}

