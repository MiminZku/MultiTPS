// Fill out your copyright notice in the Description page of Project Settings.


#include "MainUI.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"

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
