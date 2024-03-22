// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "TankController.generated.h"

class UPlayerDead;
class UPlayerWidget;
class ATankPawn;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

DECLARE_DELEGATE(FOnPauseGameSignature)

UCLASS()
class WARFAIR_API ATankController : public APlayerController
{
	GENERATED_BODY()

public:
	FOnPauseGameSignature OnPauseGame;
	
	void Initialise();

protected:
	virtual void SetupInputComponent() override;

private:

	UPROPERTY(EditDefaultsOnly, Category = "Player Data | EnhanceInput")
	TObjectPtr<UInputMappingContext> _InputMappingContext{};

	UPROPERTY(EditDefaultsOnly, Category = "Player | EnhanceInput")
	TObjectPtr<UInputAction> _IA_Move{};
	UPROPERTY(EditDefaultsOnly, Category = "Player | EnhanceInput")
	TObjectPtr<UInputAction> _IA_Turn{};
	UPROPERTY(EditDefaultsOnly, Category = "Player | EnhanceInput")
	TObjectPtr<UInputAction> _IA_Look{};
	UPROPERTY(EditDefaultsOnly, Category = "Player | EnhanceInput")
	TObjectPtr<UInputAction> _IA_Fire{};
	UPROPERTY(EditDefaultsOnly, Category = "Player | EnhanceInput")
	TObjectPtr<UInputAction> _IA_CameraControl{};
	UPROPERTY(EditDefaultsOnly, Category = "Player | EnhanceInput")
	TObjectPtr<UInputAction> _IA_Brake{};
	UPROPERTY(EditDefaultsOnly, Category = "Player | EnhanceInput")
	TObjectPtr<UInputAction> _IA_Pause{};

	void PerformTankMovement(const FInputActionValue& Value);
	void PerformStopTankMovement(const FInputActionValue& Value);
	void PerformStartTankMovement();
	
	void PerformTankTurn(const FInputActionValue& Value);
	void PerformStartTankTurn();
	void PerformStopTankTurn();
	
	void PerformLook(const FInputActionValue& Value);
	void PerformStopLook();
	void PerformFire(const FInputActionValue& Value);
	void PerformCameraControl(const FInputActionValue& Value);
	
	void PerformTankBrake(const FInputActionValue& Value);
	void PerformCancelBrake();

	void PerformPauseGame();

	TObjectPtr<ATankPawn> _TankPawn{};

	UPROPERTY(EditDefaultsOnly, Category = "Player | Widget")
	TSubclassOf<UPlayerWidget> _PlayerWidget{};
	TObjectPtr<UPlayerWidget> _PlayerWidgetSpawned{};

	UPROPERTY(EditDefaultsOnly, Category = "Player | Widget")
	TSubclassOf<UPlayerDead> _PlayerDeadWidget{};
	TObjectPtr<UPlayerDead> _PlayerDeadWidgetSpawned{};

	UPROPERTY(EditDefaultsOnly, Category = "Player | Widget")
	TSubclassOf<UUserWidget> _PauseWidget{};
	TObjectPtr<UUserWidget> _PauseWidgetSpawned{};

	void HandlePawnTankDestroyed();

};
