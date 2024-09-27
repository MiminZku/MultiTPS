// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "MultiTPSCharacter.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	mPlayer = Cast<AMultiTPSCharacter>(TryGetPawnOwner());
	OnMontageEnded.AddDynamic(this, &UPlayerAnimInstance::OnMontageEnd);
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (mPlayer)
	{
		//float Speed = mPlayer->GetVelocity().Length();
		//float Direction = CalculateDirection(mPlayer->GetVelocity(), mPlayer->GetActorForwardVector().Rotation());

		//mForwardSpeed = Speed * FMath::Cos(FMath::DegreesToRadians(Direction));
		//mRightSpeed = Speed * FMath::Sin(FMath::DegreesToRadians(Direction));
		
		mForwardSpeed = FVector::DotProduct(mPlayer->GetVelocity(), mPlayer->GetActorForwardVector());
		mRightSpeed = FVector::DotProduct(mPlayer->GetVelocity(), mPlayer->GetActorRightVector());

		bHasPistol = mPlayer->HasPistol();

		mPitchAngle = -mPlayer->GetBaseAimRotation().GetNormalized().Pitch;	// normalize => -180 ~ 180
		mPitchAngle = FMath::Clamp(mPitchAngle, -60.f, 60.f);

		bIsDead = mPlayer->IsDead();
	}
}

void UPlayerAnimInstance::OnMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{

}

void UPlayerAnimInstance::AnimNotify_Reload()
{
	if (mPlayer)
	{
		mPlayer->ReloadBullets();
	}
}

void UPlayerAnimInstance::AnimNotify_DieEnd()
{
	if (mPlayer)
	{
		mPlayer->OnDieEnd();
	}
}
