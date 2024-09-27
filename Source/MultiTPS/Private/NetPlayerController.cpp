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
	// ���� Pawn ���
	auto CurPlayer = GetPawn();
	// UnPossess
	UnPossess();
	// ���� ��� Pawn �޸𸮿��� ����
	CurPlayer->Destroy();
	// ���Ӱ� �ٽ� spawn (GameMode->RestartPlayer)
	mGameMode->RestartPlayer(this);
}

void ANetPlayerController::ClientRPC_RespawnPlayer_Implementation()
{

}