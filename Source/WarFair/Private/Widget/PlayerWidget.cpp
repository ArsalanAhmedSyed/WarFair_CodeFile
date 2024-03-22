// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PlayerWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"

void UPlayerWidget::DisplaySightToScreen(FVector2D ScreenPos) const
{
	UWidgetLayoutLibrary::SlotAsCanvasSlot(Sight_img)->SetPosition(ScreenPos);
}

void UPlayerWidget::ChangeAccuracySize(bool isAnyMovementApplied) const
{
	(isAnyMovementApplied == true) ? InterpAccuracy(_MaxAimSize) : InterpAccuracy(_MinAimSize);
}

void UPlayerWidget::InterpAccuracy(FVector2D Target) const
{
	const FVector2D CurrentAimImgSize = UWidgetLayoutLibrary::SlotAsCanvasSlot(Aim_img)->GetSize();
	const FVector2D NewSize = FMath::Vector2DInterpConstantTo(CurrentAimImgSize, Target, GetWorld()->GetDeltaSeconds(), 150.f);
	UWidgetLayoutLibrary::SlotAsCanvasSlot(Aim_img)->SetSize(NewSize);
}
