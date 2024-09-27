// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerController.h"
#include "MultiTPSGameMode.h"

ANetPlayerController::ANetPlayerController()
{

}

void ANetPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		mGameMode = Cast<AMultiTPSGameMode>(GetWorld()->GetAuthGameMode());
	}
}

void ANetPlayerController::ServerRPC_RespawnPlayer_Implementation()
{
	// 현재 Pawn 기억
	auto CurPlayer = GetPawn();
	// UnPossess
	UnPossess();
	// 이전 사용 Pawn 메모리에서 제거
	CurPlayer->Destroy();
	// 새롭게 다시 spawn (GameMode->RestartPlayer)
	mGameMode->RestartPlayer(this);
}

void ANetPlayerController::ClientRPC_RespawnPlayer_Implementation()
{

}