// Fill out your copyright notice in the Description page of Project Settings.


#include "MainUI.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "NetPlayerController.h"

UMainUI::UMainUI(const FObjectInitializer& ObjectInitializer) :	UUserWidget(ObjectInitializer)
{

}

void UMainUI::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_Retry->OnClicked.AddDynamic(this, &UMainUI::OnRetry);
	Btn_Exit->OnClicked.AddDynamic(this, &UMainUI::OnExit);
}

void UMainUI::ShowCrosshair(bool Enable)
{
	if (Enable)
	{
		Img_CrossHair->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Img_CrossHair->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainUI::AddBulletUI()
{
	UWidget* BulletWidget = CreateWidget(GetWorld(), BulletUIClass);
	BulletPanel->AddChildToUniformGrid(BulletWidget, 0, BulletPanel->GetChildrenCount());
}

void UMainUI::PopBulletUI()
{
	BulletPanel->RemoveChildAt(BulletPanel->GetChildrenCount() - 1);
}

void UMainUI::PlayDamageAnimation()
{
	PlayAnimation(DamageAnim);
}

void UMainUI::ShowGameoverUI()
{
	if (GameoverUI)
	{
		GameoverUI->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMainUI::OnRetry()
{
	if (GameoverUI)
	{
		GameoverUI->SetVisibility(ESlateVisibility::Hidden);
	}
	auto PlayerController = Cast<ANetPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController)
	{
		PlayerController->SetShowMouseCursor(false);
		PlayerController->ServerRPC_RespawnPlayer();
	}
}

void UMainUI::OnExit()
{
}
