// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MULTITPS_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPlayerAnimInstance();

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UFUNCTION()
	void OnMontageEnd(UAnimMontage* Montage, bool bInterrupted);

	void SetHasPistol(bool InBool) { bHasPistol = InBool; }

	void PlayFireAnimation()
	{
		if (false == bHasPistol)	return;
		if (!IsValid(mFireMontage))	return;

		Montage_Play(mFireMontage, 2);
	}

	void PlayReloadAnimation()
	{
		if (false == bHasPistol)	return;
		if (!IsValid(mReloadMontage))	return;

		Montage_Play(mReloadMontage);
	}

	UFUNCTION()
	void AnimNotify_Reload();

	UFUNCTION()
	void AnimNotify_DieEnd();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyAnimSettings")
	bool bHasPistol = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyAnimSettings")
	float mForwardSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyAnimSettings")
	float mRightSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyAnimSettings")
	float mPitchAngle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MyAnimSettings")
	bool bIsDead = false;

	UPROPERTY()
	TObjectPtr<class AMultiTPSCharacter> mPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	TObjectPtr<class UAnimMontage> mFireMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	TObjectPtr<class UAnimMontage> mReloadMontage;
};
