#pragma once

#include "CoreMinimal.h"
#include "FBasicAttributes.generated.h"

USTRUCT(BlueprintType)
struct FBasicAttributes
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tank | Attributes")
	float _MovementSpeed{1.f};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tank | Attributes")
	float _CoolDown{2.f};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tank | Attributes")
	float _RightTrackTurn{1.f};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tank | Attributes")
	float _LeftTrackTurn{1.f};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tank | Attributes")
	float _TurretRotationSpeed{60.f};
};
