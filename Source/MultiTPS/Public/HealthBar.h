// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBar.generated.h"

/**
 * 
 */
UCLASS()
class MULTITPS_API UHealthBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetHP(float InHP) { HP = InHP; }

public:
	UPROPERTY(BlueprintReadWrite, Category = UI, meta = (BindWidget))
	TObjectPtr<class UProgressBar> PB_HPbar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = HP)
	float HP = 1.f;
};
