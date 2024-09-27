// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "MainUI.generated.h"

/**
 * 
 */
UCLASS()
class MULTITPS_API UMainUI : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowCrosshair(bool Enable);

	void AddBulletUI();
	void PopBulletUI();
	void SetHP(float InHP) { HP = InHP; }

	void PlayDamageAnimation();

	void ShowGameoverUI() { GameoverUI->SetVisibility(ESlateVisibility::Visible); }

public:
	UPROPERTY(BlueprintReadWrite, Category = UI, meta = (BindWidget))
	TObjectPtr<class UImage> Img_CrossHair;

	UPROPERTY(BlueprintReadWrite, Category = UI, meta = (BindWidget))
	TObjectPtr<class UUniformGridPanel> BulletPanel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Bullet)
	TSubclassOf<class UUserWidget> BulletUIClass;
	
	UPROPERTY(BlueprintReadWrite, Category = UI, meta = (BindWidget))
	TObjectPtr<class UProgressBar> PB_HPbar;

	UPROPERTY(EditDefaultsOnly, Category = UI, meta = (BindWidgetAnim), Transient)
	TObjectPtr<class UWidgetAnimation> DamageAnim;

	UPROPERTY(BlueprintReadWrite, Category = UI, meta = (BindWidget))
	TObjectPtr<class UHorizontalBox> GameoverUI;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = HP)
	float HP = 1.f;
};
