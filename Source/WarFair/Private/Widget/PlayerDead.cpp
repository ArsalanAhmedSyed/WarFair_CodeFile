// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PlayerDead.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UPlayerDead::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UPlayerDead::NativeConstruct()
{
	Super::NativeConstruct();

	this->SetVisibility(ESlateVisibility::Hidden);

	Restart_btn->OnPressed.AddDynamic(this, &ThisClass::RestartPressed);

	GetWorld()->GetTimerManager().SetTimer(TH_DisplayUI, this, &ThisClass::DisplayUI, _DisplayDelay, false);
}

void UPlayerDead::RestartPressed()
{
	const FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(this));
	UGameplayStatics::OpenLevel(this, CurrentLevelName);
}

void UPlayerDead::DisplayUI()
{
	this->SetVisibility(ESlateVisibility::Visible);
}
