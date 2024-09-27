// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiTPSCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerAnimInstance.h"
#include "MainUI.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "HealthBar.h"
#include "MultiTPS.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMultiTPSCharacter

AMultiTPSCharacter::AMultiTPSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	
	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	mCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	mCameraBoom->SetupAttachment(RootComponent);
	mCameraBoom->TargetArmLength = 150.0f; // The camera follows at this distance behind the character	
	mCameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	mCameraBoom->SetRelativeLocation(FVector(0.f, 45.f, 75.f));

	// Create a follow camera
	mFollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	mFollowCamera->SetupAttachment(mCameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	mFollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	
	mGunComp = CreateDefaultSubobject<USceneComponent>(TEXT("GumComp"));
	mGunComp->SetupAttachment(GetMesh(), TEXT("GunPosition"));
	mGunComp->SetRelativeLocation(FVector(-15.f, 3.f, 4.f));
	mGunComp->SetRelativeRotation(FRotator(10.f, 80.f, 0.f));

	mInteractionRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRange"));
	mInteractionRange->SetupAttachment(GetMesh());
	mInteractionRange->SetSphereRadius(100.f);

	mHPBarComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarComp"));
	mHPBarComp->SetupAttachment(GetMesh());
	
}

void AMultiTPSCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	mInteractionRange->OnComponentBeginOverlap.AddDynamic(this, &AMultiTPSCharacter::OnInteractionAbled);
	mInteractionRange->OnComponentEndOverlap.AddDynamic(this, &AMultiTPSCharacter::OnInteractionDisabled);

	// Client에서 Main 캐릭터일 때
	InitUiWidget();
}

void AMultiTPSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PrintNetLog();


}

void AMultiTPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiTPSCharacter, bHasPistol);
	DOREPLIFETIME(AMultiTPSCharacter, mCurHP);
	
}

void AMultiTPSCharacter::PossessedBy(AController* NewController)
{
	PRINTLOG(TEXT("Begin"));
	Super::PossessedBy(NewController);
	PRINTLOG(TEXT("End"));
}

void AMultiTPSCharacter::InitUiWidget()
{
	PRINTLOG(TEXT("[%s] Begin"), Controller ? TEXT("Player") : TEXT("No Player"));
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (nullptr == PlayerController)	return;

	if (IsValid(mMainUIWidget))
	{
		mMainUI = Cast<UMainUI>(CreateWidget(GetWorld(), mMainUIWidget));
		mMainUI->AddToViewport();
		mMainUI->ShowCrosshair(false);

		mCurrentBulletNum = mMaxBulletNum;
		for (int i = 0; i < mCurrentBulletNum; ++i)
		{
			mMainUI->AddBulletUI();
		}

		if (mHPBarComp)
		{
			mHPBarComp->SetVisibility(false);
		}
	}

}

//////////////////////////////////////////////////////////////////////////
// Input

void AMultiTPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(mDefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(mJumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(mJumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(mMoveAction, ETriggerEvent::Triggered, this, &AMultiTPSCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(mLookAction, ETriggerEvent::Triggered, this, &AMultiTPSCharacter::Look);
	
		// Interaction
		EnhancedInputComponent->BindAction(mInteractAction, ETriggerEvent::Started, this, &AMultiTPSCharacter::Interact);
	
		// Release
		EnhancedInputComponent->BindAction(mReleaseAction, ETriggerEvent::Started, this, &AMultiTPSCharacter::ReleasePistol);
	
		// Fire
		EnhancedInputComponent->BindAction(mFireAction, ETriggerEvent::Started, this, &AMultiTPSCharacter::Fire);

		// Reload
		EnhancedInputComponent->BindAction(mReloadAction, ETriggerEvent::Started, this, &AMultiTPSCharacter::Reload);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMultiTPSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMultiTPSCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMultiTPSCharacter::Interact(const FInputActionValue& Value)
{
	if (bHasPistol)	return;

	ServerRPC_TakePistol();
}

void AMultiTPSCharacter::ReleasePistol(const FInputActionValue& Value)
{
	if (false == bHasPistol || false == IsLocallyControlled() || bIsReloading)	return;

	ServerRPC_ReleasePistol();
}

void AMultiTPSCharacter::Fire(const FInputActionValue& Value)
{
	if (false == bHasPistol)	return;
	if (0 >= mCurrentBulletNum) return;
	if (bIsReloading)	return;
	if (bIsDead)	return;

	ServerRPC_Fire();
}

void AMultiTPSCharacter::Reload(const FInputActionValue& Value)
{
	if (bIsReloading)	return;
	if (!bHasPistol)	return;
	if (mCurrentBulletNum == mMaxBulletNum)	return;

	ServerRPC_Reload();
}

void AMultiTPSCharacter::AttachPistol(const AActor* Pistol)
{
	UStaticMeshComponent* PistolMesh = Pistol->GetComponentByClass<UStaticMeshComponent>();
	PistolMesh->SetSimulatePhysics(false);
	PistolMesh->AttachToComponent(mGunComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	if (mMainUI && IsLocallyControlled())
	{
		mMainUI->ShowCrosshair(true);
	}
}

void AMultiTPSCharacter::DetachPistol(const AActor* Pistol)
{
	UStaticMeshComponent* PistolMesh = Pistol->GetComponentByClass<UStaticMeshComponent>();
	PistolMesh->SetSimulatePhysics(true);
	PistolMesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	if (mMainUI && IsLocallyControlled())
	{
		mMainUI->ShowCrosshair(false);
	}
}

void AMultiTPSCharacter::Die()
{
	bIsDead = true;
}

float AMultiTPSCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	DamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	HP -= DamageAmount;
	
	return DamageAmount;
}

void AMultiTPSCharacter::OnInteractionAbled(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, OtherActor->GetName());
	if (OtherActor->GetName().Contains("BP_Pistol") && nullptr == OtherActor->GetOwner())
	{
		mInteractableActors.Add(OtherActor);
	}
}

void AMultiTPSCharacter::OnInteractionDisabled(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, OtherActor->GetName());
	if (OtherActor->GetName().Contains("BP_Pistol"))
	{
		mInteractableActors.Remove(OtherActor);
	}
}

void AMultiTPSCharacter::ReloadBullets()
{
	bIsReloading = false;
	if (mMainUI && IsLocallyControlled())
	{
		for (int i = mCurrentBulletNum; i < mMaxBulletNum; ++i)
		{
			mMainUI->AddBulletUI();
		}
	}
	mCurrentBulletNum = mMaxBulletNum;
}

void AMultiTPSCharacter::OnDieEnd()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

	if (IsLocallyControlled())
	{
		APlayerController* PlayerCtrl = Cast<APlayerController>(Controller);
		PlayerCtrl->SetShowMouseCursor(true);
		GetFollowCamera()->PostProcessSettings.ColorSaturation = FVector4(0, 0, 0, 1);
		mMainUI->ShowGameoverUI();
	}
}

void AMultiTPSCharacter::PrintNetLog()
{
	/*
	리슨 서버일 경우
	
	호스트는 Owner에 모든 Player Controller가 찍히지만
	게스트는 자신을 제외하곤 No Owner 라고 뜸

	호스트는 자기 자신이 클라이언트이므로 NetConnection이 필요가 없어서 Invalid Connection으로 뜸
	즉, 자신을 제외하곤 모두 Valid Connection으로 뜸
	게스트는 자신만 Valid Connection으로 뜸

	호스트는 Local Role이 모든 캐릭터 ROLE_Authority라고 뜸
	게스트는 본인 캐릭터는 AutonomousProxy, 다른 캐릭터는 SimulatedProxy 라고 뜸

	호스트는 Remote Role이 모든 캐릭터 ROLE_AutonomousProxy 라고 뜸 (모든 캐릭터 컨트롤러 있으니까)
	게스트는 Remote Role이 모든 캐릭터 ROLE_Authority 라고 뜸 (
	*/
	HasAuthority(); // 서버인지
	IsLocallyControlled(); // 클라인지

	const FString conStr = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");
	const FString ownerName = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");

	const FString logStr = FString::Printf(TEXT("Connection : %s\nOwner Name : %s\nLocal Role : %s\nRemote Role : %s"),
		*conStr, *ownerName, *LOCALROLE, *REMOTEROLE);

	DrawDebugString(GetWorld(), GetActorLocation() + FVector::UpVector * 100, logStr,
		nullptr, FColor::White, 0, true, 1);
}

void AMultiTPSCharacter::ServerRPC_TakePistol_Implementation()
{
	if (mInteractableActors.Num())
	{
		AActor* Actor = mInteractableActors[mInteractableActors.Num() - 1];
		if (Actor->GetName().Contains("BP_Pistol"))
		{
			ClientRPC_TakePistol(Actor);
		}
	}
}

void AMultiTPSCharacter::ClientRPC_TakePistol_Implementation(AActor* Pistol)
{
	AttachPistol(Pistol);
	Pistol->SetActorEnableCollision(false);
	bHasPistol = true;
	Pistol->SetOwner(this);
	mOwnedPistol = Pistol;
}

void AMultiTPSCharacter::ServerRPC_ReleasePistol_Implementation()
{
	if (mOwnedPistol)
	{
		ClientRPC_ReleasePistol(mOwnedPistol);
	}
}

void AMultiTPSCharacter::ClientRPC_ReleasePistol_Implementation(AActor* Pistol)
{
	DetachPistol(Pistol);
	Pistol->SetActorEnableCollision(true);
	bHasPistol = false;
	Pistol->SetOwner(nullptr);
	Pistol = nullptr;
}

bool AMultiTPSCharacter::ServerRPC_Fire_Validate()
{
	if (mCurrentBulletNum < 0 || mCurrentBulletNum > mMaxBulletNum) return false;
	return true;
}

void AMultiTPSCharacter::ServerRPC_Fire_Implementation()
{
	FHitResult HitResult;
	//FVector StartPos = mOwnedPistol->GetActorLocation();
	FVector StartPos = mFollowCamera->GetComponentLocation();
	FVector EndPos = StartPos + mFollowCamera->GetForwardVector() * 10000;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, ECC_Visibility, Params);
	if (bHit)
	{
		//DrawDebugLine(GetWorld(), StartPos, HitResult.Location, FColor::Red, false, 1.f);
		//DrawDebugLine(GetWorld(), HitResult.Location, EndPos, FColor::Green, false, 1.f);
		
		AMultiTPSCharacter* OtherPlayer = Cast<AMultiTPSCharacter>(HitResult.GetActor());
		if (OtherPlayer)
		{
			FDamageEvent DmgEvent;
			OtherPlayer->TakeDamage(1.f, DmgEvent, GetController(), mOwnedPistol);
		}
	}
	//else
	//{
	//	DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Red, false, 1.f);
	//}

	ClientRPC_Fire(bHit, HitResult);
}

void AMultiTPSCharacter::ClientRPC_Fire_Implementation(bool bHit, const FHitResult& HitResult)
{
	--mCurrentBulletNum;

	if (bHit)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
			mGunHitEffect, HitResult.Location, HitResult.ImpactNormal.Rotation());
	}

	if (mMainUI && IsLocallyControlled())
	{
		mMainUI->PopBulletUI();
	}

	UPlayerAnimInstance* AnimInst = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (AnimInst)
	{
		AnimInst->PlayFireAnimation();
	}
}

void AMultiTPSCharacter::ServerRPC_Reload_Implementation()
{
	ClientRPC_Reload();
}

void AMultiTPSCharacter::ClientRPC_Reload_Implementation()
{
	bIsReloading = true;
	UPlayerAnimInstance* AnimInst = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (AnimInst)
	{
		AnimInst->PlayReloadAnimation();
	}
}

void AMultiTPSCharacter::SetHP(float Value)
{	
	mCurHP = Value;
	OnRep_ChangeHP();
}

void AMultiTPSCharacter::OnRep_ChangeHP()
{
	if (HP <= 0)
	{
		Die();

		GetCharacterMovement()->DisableMovement();
		ReleasePistol(FInputActionValue());
	}

	if (IsValid(mMainUI))
	{
		mMainUI->SetHP(mCurHP / mMaxHP);
		mMainUI->PlayDamageAnimation();
		if (IsValid(mDamageCameraShake))
		{
			APlayerController* PlayerCtrl = Cast<APlayerController>(Controller);
			PlayerCtrl->ClientStartCameraShake(mDamageCameraShake);
		}
	}
	Cast<UHealthBar>(mHPBarComp->GetUserWidgetObject())->SetHP(mCurHP / mMaxHP);
}