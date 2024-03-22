#pragma once

UENUM(BlueprintType)
enum class EPartDamaged : uint8 
{
	EPD_Turret		UMETA(DisplayName="Turret"),
	EPD_Crew		UMETA(DisplayName="Crew"),
	EPD_Engine		UMETA(DisplayName="Engine"),
	EPD_RightTrack  UMETA(DisplayName="RightTrack"),
	EPD_LeftTrack	UMETA(DisplayName="LeftTrack")
};
