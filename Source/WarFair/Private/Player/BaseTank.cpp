
#include "Tank/BaseTank.h"
#include "Components/AudioComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"

ABaseTank::ABaseTank()
{
	PrimaryActorTick.bCanEverTick = false;

	_AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio Component"));
	_AudioComponent->SetupAttachment(GetRootComponent());
	
	_PerformanceEffectAttributes._CoolDown = 3.f;
	_PerformanceEffectAttributes._MovementSpeed = 0.f;
	_PerformanceEffectAttributes._LeftTrackTurn = 0.4f;
	_PerformanceEffectAttributes._RightTrackTurn = 0.4f;
	_PerformanceEffectAttributes._TurretRotationSpeed = 25.f;

	//Get the tank destroyed actor that will be spawned after a tank health reaches zero.
	static ConstructorHelpers::FClassFinder<AActor> ItemClass(TEXT("/Game/Blueprints/CodeWork/Actor/BPA_DestroyedTank"));
	if(ItemClass.Class != nullptr)
	{
		_DestroyedTankActor =  ItemClass.Class;
	}
}

void ABaseTank::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	_Health = _MaxHealth;
}

void ABaseTank::SpawnShell(bool bRandomize)
{
	if(_bCanFire == false) { return; }

#pragma region // Spawn Properties
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this->GetInstigator();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	const FRotator GunSocketRotation = GetMesh()->GetSocketRotation(_GunSocketName);
	const FVector SpawnLocation = GetMesh()->GetSocketLocation(_GunSocketName);
	const FVector ConDirection = GunSocketRotation.Vector();
	const FRotator SpawnRotation = (bRandomize) ? 
		  FMath::VRandCone(ConDirection, FMath::DegreesToRadians(_AccuracyRandomise),FMath::DegreesToRadians(_AccuracyRandomise)).Rotation() :  GunSocketRotation;
#pragma endregion // Spawn Properties

	GetWorld()->SpawnActor<AActor>(_Shell, SpawnLocation, SpawnRotation ,SpawnParameters);
	//Spawn Particle effect 
	UGameplayStatics::SpawnEmitterAtLocation(this, _ShellFireParticle, GetMesh()->GetSocketLocation(_GunSocketName));
	TriggerSFXInput(_SFXFire);

	_bCanFire = false;
	GetWorldTimerManager().SetTimer(TH_FireCoolDown, this, &ThisClass::FireCoolDownComplete, _TankAttributes._CoolDown, false);
}

void ABaseTank::FireCoolDownComplete()
{
	_bCanFire = true;
	
	if(TH_FireCoolDown.IsValid())
	{
		TH_FireCoolDown.Invalidate();
	}
}

void ABaseTank::TriggerSFXInput(FName TriggerInputName) const
{
	if(_AudioComponent != nullptr)
	{
		_AudioComponent->SetTriggerParameter(TriggerInputName);
	}
}

float ABaseTank::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                            AActor* DamageCauser)
{
	TakeHealthDamage(DamageAmount);
	
	if(_Crew || _Engine || _Turret || _RightTrack || _LeftTrack)
	{
		const TSubclassOf<UDamageType> PartDamaged = DamageEvent.DamageTypeClass;
		
		if(_Crew == DamageEvent.DamageTypeClass && !_bIsCewDamaged)
		{
			ApplyCrewEffect();
		}
		else if(_Turret == PartDamaged && !_bIsTurretDamaged)
		{
			ApplyTurretEffect();
		}
		else if(_Engine == PartDamaged && !_bIsEngineDamaged)
		{
			ApplyEngineEffect();
		}
		else if(_LeftTrack == PartDamaged && !_bIsLeftTrackDamaged)
		{
			ApplyLeftTrackEffect();
		}
		else if(_RightTrack == PartDamaged && !_bIsRightTrackDamaged)
		{
			ApplyRightTrackEffect();
		}
	}
	
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

/**
 * Increase the reload timer.
 */
void ABaseTank::ApplyCrewEffect()
{
	_bIsCewDamaged = true;
	_TankAttributes._CoolDown = _PerformanceEffectAttributes._CoolDown;
}

/**
 * Reduce the turret rotation speed
 */
void ABaseTank::ApplyTurretEffect()
{
	_bIsTurretDamaged = true;
	_TankAttributes._TurretRotationSpeed = _PerformanceEffectAttributes._TurretRotationSpeed;
}

/**
 * Stop the tank from moving.
 */
void ABaseTank::ApplyEngineEffect()
{
	_bIsEngineDamaged = true;
	_TankAttributes._MovementSpeed = _PerformanceEffectAttributes._MovementSpeed;
	_TankAttributes._LeftTrackTurn = 0.f;
	_TankAttributes._RightTrackTurn = 0.f;
}

/**
 * Reduce right turn speed.
 */
void ABaseTank::ApplyLeftTrackEffect()
{
	_bIsLeftTrackDamaged = true;
	_TankAttributes._LeftTrackTurn = _PerformanceEffectAttributes._LeftTrackTurn;
}

/**
 * Reduce left turn speed.
 */
void ABaseTank::ApplyRightTrackEffect()
{
	_bIsRightTrackDamaged = true;
	_TankAttributes._RightTrackTurn = _PerformanceEffectAttributes._RightTrackTurn;
}

void ABaseTank::TakeHealthDamage(float DamageAmount)
{
	_Health = FMath::Max(_Health - DamageAmount, 0);

	/*Update for UI health bar*/
	const float HealthPercent = _Health / _MaxHealth;
	UpdateHealthBar(HealthPercent);

	if(_Health <= 0)
	{
		DestroyTank();
	}
}

/**
 * Start the initiate of destroying the tank.
 * This is virtual function to allow child classes to perform their things before calling destroy.
 */
void ABaseTank::DestroyTank()
{
	//Spawn destroyed tank actor before removing the actual tank who's health has reached zero.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	GetWorld()->SpawnActor<AActor>(_DestroyedTankActor , GetActorLocation(), GetActorRotation(), SpawnParameters);
}



