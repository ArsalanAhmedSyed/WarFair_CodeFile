// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "PlayerWidget.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class WARFAIR_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void DisplaySightToScreen(FVector2D ScreenPos) const;
	void ChangeAccuracySize(bool isAnyMovementApplied) const;

	/**
	 * Pass it to Tank Part UI to update visual of part damaged.
	 * 
	 * @param PartDamaged The Part that has been damaged
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Tank | Widget", DisplayName = "Update Part Damaged UI")
	void PartDamageUIUpdate(EPartDamaged PartDamaged);

	/**
	 * Update the players health UI.
	 * 
	 * @param HealthPercent New Health bar percent value.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Tank | Widget")
	void UpdateHealthUI(float HealthPercent);
	
protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> Sight_img{};
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite, EditDefaultsOnly)
	TObjectPtr<UImage> Camera_img{};
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite, EditDefaultsOnly)
	TObjectPtr<UImage> Aim_img{};

private:
	void InterpAccuracy(FVector2D Target) const;

	const FVector2D _MaxAimSize{125.f, 125.f};
	const FVector2D _MinAimSize{50.f, 50.f};
};
