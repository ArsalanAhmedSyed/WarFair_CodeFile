// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/LevelWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Tank/Enums/EPauseType.h"

void ULevelWidget::InitialiseTimer()
{
	Restart_btn->OnPressed.AddDynamic(this, &ThisClass::HandleRestart);
	Quit_btn->OnPressed.AddDynamic(this, &ThisClass::HandleQuitGame);
}

void ULevelWidget::HandleRestart()
{
	const FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(this));
	UGameplayStatics::OpenLevel(this, CurrentLevelName);
	Destruct();
}

void ULevelWidget::HandleQuitGame()
{
	UKismetSystemLibrary::QuitGame(this,UGameplayStatics::GetPlayerController(this, 0),EQuitPreference::Quit, true);
}
