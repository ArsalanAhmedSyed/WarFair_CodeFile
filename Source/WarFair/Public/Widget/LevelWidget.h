// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LevelWidget.generated.h"

enum class EPauseType;
class UButton;
class UTextBlock;

UCLASS(Blueprintable)
class WARFAIR_API ULevelWidget : public UUserWidget
{
	GENERATED_BODY()

		
public:
	UFUNCTION(BlueprintCallable)
	void InitialiseTimer();

protected:
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	TObjectPtr<UTextBlock> PauseType_txt{};
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Restart_btn{};
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Quit_btn{};

	FSlateColor _PauseColour{};
	FSlateColor _LevelCompleteColour{};
private:
	UFUNCTION()
	void HandleRestart();
	UFUNCTION()
	void HandleQuitGame();
	bool _Overlapped{false};
};
