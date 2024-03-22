#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelComplete.generated.h"

class UBoxComponent;
class ULevelWidget;

DECLARE_DELEGATE(FOnLevelCompleteSignature);

UCLASS()
class WARFAIR_API ALevelComplete : public AActor
{
	GENERATED_BODY()

public:

	FOnLevelCompleteSignature OnLevelComplete{};
	
	ALevelComplete();
	void Initialise();
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> _BoxCollider{};

	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<ULevelWidget> _LevelWidget{};
	
	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult& SweepResult);

	bool _Overlapped{false};
};
