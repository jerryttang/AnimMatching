// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "Animation/AnimNode_AssetPlayerBase.h"

#include "Animation/AnimInstanceProxy.h"
#include "AnimNode_AnimMatching.generated.h"


/*Currently This is where the process gets done, Both Finding the best Candidate Motion Key and Blending the Animations together along with their Root Motion*/
USTRUCT(BlueprintInternalUseOnly)
struct ANIMTEST_API FAnimNode_AnimMatching : public FAnimNode_AssetPlayerBase
{
	GENERATED_BODY()

public:
	// FAnimNode_AssetPlayerBase interface
	virtual float GetCurrentAssetTime();
	virtual float GetCurrentAssetTimePlayRateAdjusted();
	virtual float GetCurrentAssetLength();
	// End of FAnimNode_AssetPlayerBase interface

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override; ////-Unused
	virtual void UpdateAssetPlayer(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void OverrideAsset(UAnimationAsset* NewAsset) override; ////-Unused 
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface

	void GetBlendPose(const float DT, FCompactPose & OutPose, FBlendedCurve & OutCurve);
	UAnimSequence * GetCurrentAnim(float& OutAnimTime);
protected:
	UPROPERTY()
		ACharacter* Character;
};
