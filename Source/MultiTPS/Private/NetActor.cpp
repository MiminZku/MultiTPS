// Fill out your copyright notice in the Description page of Project Settings.


#include "NetActor.h"
#include "MultiTPS.h"
#include "EngineUtils.h"
#include "MultiTPSCharacter.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ANetActor::ANetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(mMeshComp);
	mMeshComp->SetRelativeScale3D(FVector(0.5f));

	// ��Ʈ��ũ ����ȭ �ɼ� Ȱ��ȭ
	SetReplicates(true);
}

// Called when the game starts or when spawned
void ANetActor::BeginPlay()
{
	Super::BeginPlay();
	
	mMat = mMeshComp->CreateDynamicMaterialInstance(0);

	// ������ ���� (����) �� ������ ��������
	if (HasAuthority())
	{
		FTimerHandle TimerHandle;

		GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda(
			[&]
			{
				FLinearColor NewMatColor
				/*mMatColor*/ = FLinearColor(FMath::RandRange(0.f, 0.3f), FMath::RandRange(0.f, 0.3f),
					FMath::RandRange(0.f, 0.3f), 1);

				//OnRep_ChangeMatColor();	// ������ �ڵ� ���� �ȵǹǷ� ���� ��������
				ServerRPC_ChangeColor(NewMatColor);
			}),
			1, true);
	}
}

void ANetActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetActor, RotYaw);
	//DOREPLIFETIME(ANetActor, mMatColor);
}

// Called every frame
void ANetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FindOwner();
	PrintNetLog();	

	if (HasAuthority())
	{
		// ȸ�� ó��
		AddActorLocalRotation(FRotator(0, DeltaTime * 50.f, 0));
		RotYaw = GetActorRotation().Yaw;
	}
}

void ANetActor::FindOwner()
{
	// ���������� ó��
	if (HasAuthority())
	{
		// ���� ����� Pawn�� Owner�� ����, �� ���� �ȿ� ������ ��
		AActor* NewOwner = nullptr;
		float MinDist = mSearchDistance;

		for (TActorIterator<AMultiTPSCharacter> Iter(GetWorld()); Iter; ++Iter)
		{
			AActor* OtherActor = *Iter;
			float Dist = GetDistanceTo(OtherActor);

			if (Dist < MinDist)
			{
				MinDist = Dist;
				NewOwner = OtherActor;
			}
		}

		if (GetOwner() != NewOwner)	SetOwner(NewOwner);
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), mSearchDistance, 30,
		FColor::Yellow, false, 0, 0, 1);
}

void ANetActor::PrintNetLog()
{
	const FString conStr = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");
	const FString ownerName = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");

	const FString logStr = FString::Printf(TEXT("Connection : %s\nOwner Name : %s\nLocal Role : %s\nRemote Role : %s"),
		*conStr, *ownerName, *LOCALROLE, *REMOTEROLE);

	DrawDebugString(GetWorld(), GetActorLocation() + FVector::UpVector * 100, logStr,
		nullptr, FColor::White, 0, true, 1);
}

void ANetActor::OnRep_RotYaw()
{
	// ȸ���� ����ȭ
	FRotator NewRot = GetActorRotation();
	NewRot.Yaw = RotYaw;
	SetActorRotation(NewRot);
}

void ANetActor::OnRep_ChangeMatColor()
{
	if (mMat)
	{
		mMat->SetVectorParameterValue(TEXT("FloorColor"), mMatColor);
	}
}

void ANetActor::ServerRPC_ChangeColor_Implementation(const FLinearColor& NewColor)
{
	ClientRPC_ChangeColor(NewColor);
}

void ANetActor::ClientRPC_ChangeColor_Implementation(const FLinearColor& NewColor)
{
	if (mMat)
	{
		mMat->SetVectorParameterValue(TEXT("FloorColor"), NewColor);
	}
}