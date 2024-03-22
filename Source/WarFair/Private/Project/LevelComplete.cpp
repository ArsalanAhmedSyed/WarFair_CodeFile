
#include "Project/LevelComplete.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/LevelWidget.h"

ALevelComplete::ALevelComplete()
{
	PrimaryActorTick.bCanEverTick = false;

	_BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider Component"));
	SetRootComponent(_BoxCollider);
}

void ALevelComplete::Initialise()
{
	_BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBoxOverlap);
}

void ALevelComplete::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent,
                                  AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult& SweepResult)
{
	if(_LevelWidget == nullptr) { return; }
	if(_Overlapped == true) { return;}
	
	if(OtherActor->ActorHasTag(FName("Player")))
	{
		_Overlapped = true;

		ULevelWidget* LevelWidgetSpawned = CreateWidget<ULevelWidget>(UGameplayStatics::GetPlayerController(this, 0), _LevelWidget);
		if(LevelWidgetSpawned != nullptr)
		{
			LevelWidgetSpawned->InitialiseTimer();
		}
	
		LevelWidgetSpawned->AddToViewport();

		OnLevelComplete.ExecuteIfBound();
	}
}


