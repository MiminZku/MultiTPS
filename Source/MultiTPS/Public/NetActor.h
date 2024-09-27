// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetActor.generated.h"

UCLASS()
class MULTITPS_API ANetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void PrintNetLog();

	void FindOwner();

	// ReplicatedUsing 콜백 함수는 클라이언트에서만 호출됨
	UFUNCTION()
	void OnRep_RotYaw();

	UFUNCTION()
	void OnRep_ChangeMatColor();

	// RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_ChangeColor(const FLinearColor& NewColor);
	//void ServerRPC_ChangeColor_Implementation(const FLinearColor NewColor);
	
	UFUNCTION(NetMulticast, Unreliable)
	void ClientRPC_ChangeColor(const FLinearColor& NewColor);
	//void ClientRPC_ChangeColor_Implementation(const FLinearColor NewColor);

protected:
	UPROPERTY(VisibleAnyWhere)
	TObjectPtr<UStaticMeshComponent> mMeshComp;

	UPROPERTY(EditAnywhere)
	float mSearchDistance = 200.f;

	//회전값 동기화 변수
	UPROPERTY(ReplicatedUsing = OnRep_RotYaw)
	float RotYaw = 0.f;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> mMat;

	UPROPERTY(ReplicatedUsing = OnRep_ChangeMatColor)
	FLinearColor mMatColor;
};
