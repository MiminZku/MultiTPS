// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTITPS_API ANetPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANetPlayerController();

public:
	virtual void BeginPlay() override;

public:
	UFUNCTION(Server, Reliable)
	void ServerRPC_RespawnPlayer();

	UFUNCTION(NetMulticast, Reliable)
	void ClientRPC_RespawnPlayer();


public:
	UPROPERTY()
	TObjectPtr<class AMultiTPSGameMode> mGameMode;


};
