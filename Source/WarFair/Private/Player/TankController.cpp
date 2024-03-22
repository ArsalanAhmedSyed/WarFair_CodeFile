// Fill out your copyright notice in the Description page of Project Settings.


#include "Tank/TankController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Widget/PlayerWidget.h"
#include "Tank/TankPawn.h"
#include "Widget/PlayerDead.h"

void ATankController::Initialise()
{
	if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(_InputMappingContext, 1);
	}

	_PlayerWidgetSpawned = CreateWidget<UPlayerWidget>(this, _PlayerWidget);
	if(_PlayerWidgetSpawned) { _PlayerWidgetSpawned->AddToViewport(); }

	//Reset Input mode to Game mode after level is restarted
	const FInputModeGameOnly GameOnlyInput;
	SetInputMode(GameOnlyInput);
	bShowMouseCursor = false;

	_TankPawn = Cast<ATankPawn>(GetPawn());
	if(_TankPawn  == nullptr) { return; }
	Possess(_TankPawn);

	_TankPawn->Initialise(_PlayerWidgetSpawned);
	_TankPawn->OnTankDestroyed.BindUObject(this, &ThisClass::HandlePawnTankDestroyed);
}

void ATankController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		/*Movement Input*/
		EnhancedInputComponent->BindAction(_IA_Move, ETriggerEvent::Started, this, &ThisClass::PerformStartTankMovement);
		EnhancedInputComponent->BindAction(_IA_Move, ETriggerEvent::Triggered, this, &ThisClass::PerformTankMovement);
		EnhancedInputComponent->BindAction(_IA_Move, ETriggerEvent::Canceled, this, &ThisClass::PerformStopTankMovement);
		EnhancedInputComponent->BindAction(_IA_Move, ETriggerEvent::Completed, this, &ThisClass::PerformStopTankMovement);

		/*Turn Input*/
		EnhancedInputComponent->BindAction(_IA_Turn, ETriggerEvent::Started, this, &ThisClass::PerformStartTankTurn);
		EnhancedInputComponent->BindAction(_IA_Turn, ETriggerEvent::Triggered, this, &ThisClass::PerformTankTurn);
		EnhancedInputComponent->BindAction(_IA_Turn, ETriggerEvent::Canceled, this, &ThisClass::PerformStopTankTurn);
		EnhancedInputComponent->BindAction(_IA_Turn, ETriggerEvent::Completed, this, &ThisClass::PerformStopTankTurn);

		/*Mouse look Input*/
		EnhancedInputComponent->BindAction(_IA_Look, ETriggerEvent::Triggered, this, &ThisClass::PerformLook);
		EnhancedInputComponent->BindAction(_IA_Look, ETriggerEvent::Canceled, this, &ThisClass::PerformStopLook);
		EnhancedInputComponent->BindAction(_IA_Look, ETriggerEvent::Completed, this, &ThisClass::PerformStopLook);

		/*Brake Input*/
		EnhancedInputComponent->BindAction(_IA_Brake, ETriggerEvent::Started, this, &ThisClass::PerformTankBrake);
		EnhancedInputComponent->BindAction(_IA_Brake, ETriggerEvent::Completed, this, &ThisClass::PerformCancelBrake);

		/*Other Input*/
		EnhancedInputComponent->BindAction(_IA_Fire, ETriggerEvent::Triggered, this, &ThisClass::PerformFire);
		EnhancedInputComponent->BindAction(_IA_CameraControl, ETriggerEvent::Triggered, this, &ThisClass::PerformCameraControl);
		EnhancedInputComponent->BindAction(_IA_Pause, ETriggerEvent::Triggered, this, &ThisClass::PerformPauseGame);
	}
}

void ATankController::PerformTankMovement(const FInputActionValue& Value) 
{
	_TankPawn->PerformTankMovement(Value);
}

void ATankController::PerformTankTurn(const FInputActionValue& Value) 
{
	_TankPawn->PerformTankTurn(Value);
}

void ATankController::PerformStartTankTurn()
{
	_TankPawn->PerformStartTankTurn();
}

void ATankController::PerformStopTankTurn()
{
	_TankPawn->PerformStopTankTurn();
}

void ATankController::PerformLook(const FInputActionValue& Value)
{
	_TankPawn->PerformLook(Value);
}

void ATankController::PerformStopLook()
{
	_TankPawn->PerformStopLook();
}

void ATankController::PerformFire(const FInputActionValue& Value)
{
	_TankPawn->PerformFire(Value);
}

void ATankController::PerformCameraControl(const FInputActionValue& Value)
{
	_TankPawn->PerformCameraStatSwitch(Value);
}

void ATankController::PerformTankBrake(const FInputActionValue& Value)
{
	_TankPawn->PerformBrake(Value);
}

void ATankController::PerformCancelBrake()
{
	_TankPawn->PerformCancelBrake();
}

void ATankController::PerformPauseGame()
{
	if(_PauseWidgetSpawned)
	{
		_PauseWidgetSpawned->RemoveFromParent();
		_PauseWidgetSpawned->Destruct();
	}
	else
	{
		if(_PauseWidget)
		{
			_PauseWidgetSpawned = CreateWidget<UUserWidget>(this,_PauseWidget);
			_PauseWidgetSpawned->AddToViewport();
		}
	}
	
	OnPauseGame.ExecuteIfBound();
}

void ATankController::PerformStopTankMovement(const FInputActionValue& Value)
{
	_TankPawn->PerformStopTankMovement(Value);
}

void ATankController::PerformStartTankMovement()
{
	_TankPawn->PerformStartTankMovement();
}

void ATankController::HandlePawnTankDestroyed()
{
	if(_PlayerWidgetSpawned)
	{
		_PlayerWidgetSpawned->RemoveFromParent();
		_PlayerWidgetSpawned->Destruct();
	}

	_PlayerDeadWidgetSpawned = CreateWidget<UPlayerDead>(this, _PlayerDeadWidget);
	_PlayerDeadWidgetSpawned->AddToViewport();
	
	FInputModeUIOnly UIInputMode;
	UIInputMode.SetWidgetToFocus(_PlayerDeadWidgetSpawned->TakeWidget());
	SetInputMode(UIInputMode);
	bShowMouseCursor = true;
}
