
#include "Project/TankGameMode.h"
#include "Widget/LevelWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Project/LevelComplete.h"
#include "Tank/TankController.h"
#include "Tank/Enums/EPauseType.h"

void ATankGameMode::BeginPlay()
{
	Super::BeginPlay();

	AActor* ControllerFound = UGameplayStatics::GetActorOfClass(this, PlayerControllerClass);

	_TankController = Cast<ATankController>(ControllerFound);
	if(_TankController)
	{
		_TankController->Initialise();
		_TankController->OnPauseGame.BindUObject(this, &ThisClass::HandlePauseGame);
	}

	if(AActor* ActorFound = UGameplayStatics::GetActorOfClass(this, _LevelCompleteActor))
	{
		_LevelComplete = Cast<ALevelComplete>(ActorFound);
		if(_LevelComplete)
		{
			_LevelComplete->Initialise();
			_LevelComplete->OnLevelComplete.BindUObject(this, &ThisClass::HandlePauseGame);
		}
	}
}

void ATankGameMode::HandlePauseGame() const
{
	(UGameplayStatics::IsGamePaused(this)) ? SetPauseGame(EInputMode::EIM_Game, false) : SetPauseGame(EInputMode::EIM_UIAndGame, true);
}

void ATankGameMode::SetPauseGame(const EInputMode InputModeType, bool SetPause) const
{
	UGameplayStatics::SetGamePaused(this, SetPause);
	const FInputModeGameOnly InputModeGameOnly;
	const FInputModeGameAndUI InputModeGameAndUI;

	switch(InputModeType)
	{
	case EInputMode::EIM_Game:
		_TankController->SetInputMode(InputModeGameOnly);
		_TankController->SetShowMouseCursor(false);
		break;
	case EInputMode::EIM_UIAndGame:
		_TankController->SetInputMode(InputModeGameAndUI);
		_TankController->SetShowMouseCursor(true);
		break;
	default: break;
	}
}
