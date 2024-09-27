// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "MultiTPSCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AMultiTPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMultiTPSCharacter();
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	virtual void Tick(float DeltaSeconds) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	void InitUiWidget();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void ReleasePistol(const FInputActionValue& Value);
	void Fire(const FInputActionValue& Value);
	void Reload(const FInputActionValue& Value);

	void AttachPistol(const AActor* Pistol);
	void DetachPistol(const AActor* Pistol);

	void Die();

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return mCameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return mFollowCamera; }

	FORCEINLINE bool IsDead() const { return bIsDead; }
	
	UFUNCTION()
	void OnInteractionAbled(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnInteractionDisabled(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool HasPistol() { return bHasPistol; }

	void ReloadBullets();

	void OnDieEnd();

	// 네트워크 상태 로그 출력 함수
	void PrintNetLog();

public:
	UFUNCTION(Server, Reliable)
	void ServerRPC_TakePistol();

	UFUNCTION(NetMulticast, Reliable)
	void ClientRPC_TakePistol(AActor* Pistol);

	UFUNCTION(Server, Reliable)
	void ServerRPC_ReleasePistol();

	UFUNCTION(NetMulticast, Reliable)
	void ClientRPC_ReleasePistol(AActor* Pistol);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPC_Fire();

	UFUNCTION(NetMulticast, Reliable)
	void ClientRPC_Fire(bool bHit, const FHitResult& HitResult);

	UFUNCTION(Server, Reliable)
	void ServerRPC_Reload();

	UFUNCTION(NetMulticast, Reliable)
	void ClientRPC_Reload();

	UFUNCTION()
	void OnRep_ChangeHP();

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* mCameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* mFollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* mDefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* mJumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* mMoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* mLookAction;

	/** Interaction Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* mInteractAction;

	/** Release Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* mReleaseAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* mFireAction;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* mReloadAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> mInteractionRange;

	// 총을 자식으로 붙일 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gun, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> mGunComp;

	UPROPERTY(Replicated)
	bool bHasPistol = false;

	UPROPERTY()
	TObjectPtr<AActor> mOwnedPistol = nullptr;

	TArray<TObjectPtr<AActor>> mInteractableActors;

	UPROPERTY(EditDefaultsOnly, Category = Gun)
	TObjectPtr<UParticleSystem> mGunHitEffect;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<class UMainUI> mMainUIWidget;

	UPROPERTY()
	TObjectPtr<UMainUI> mMainUI;

	UPROPERTY(EditAnywhere, Category = Bullet)
	int32 mMaxBulletNum = 10;

	UPROPERTY(Replicated)
	int32 mCurrentBulletNum = mMaxBulletNum;

	bool bIsReloading = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = HP)
	float mMaxHP = 5.f;

	UPROPERTY(BlueprintReadOnly, Category = HP, ReplicatedUsing = OnRep_ChangeHP)
	float mCurHP = mMaxHP;

	__declspec(property(get = GetHP, put = SetHP)) float HP;
	float GetHP() { return mCurHP; }
	void SetHP(float Value);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UWidgetComponent> mHPBarComp;

	UPROPERTY()
	bool bIsDead = false;

	UPROPERTY(EditDefaultsOnly, Category = Damage)
	TSubclassOf<class UCameraShakeBase> mDamageCameraShake;
};

