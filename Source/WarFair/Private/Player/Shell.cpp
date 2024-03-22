
#include "Tank/Shell.h"

#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AShell::AShell()
{
	PrimaryActorTick.bCanEverTick = false;

	_StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh Component"));
	_CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Component"));
	_ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement Component"));

	SetRootComponent(_StaticMeshComponent);
	_CapsuleComponent->SetupAttachment(GetRootComponent());
	
	_StaticMeshComponent->SetCollisionProfileName(FName("NoCollision"), false);
	_CapsuleComponent->SetCollisionProfileName(FName("OverlapAllDynamic"), true);

	AActor::SetLifeSpan(_ActorLifeSpan);
	_ProjectileMovementComponent->InitialSpeed = _ProjectileSpeed;
	_ProjectileMovementComponent->MaxSpeed = _ProjectileSpeed;
	_ProjectileMovementComponent->Velocity = FVector(_ProjectileVelocity, 0.f,0.f);
}

void AShell::BeginPlay()
{
	Super::BeginPlay();

	if(_CapsuleComponent)
		_CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnCapsuleBeginOverlap);
}

void AShell::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor == GetOwner()) { return; }

	if(OtherActor->ActorHasTag("Player"))
	{
		UAudioComponent* TankHitSFX = UGameplayStatics::SpawnSoundAtLocation(this, _TankHitSFX, GetActorLocation());
		TankHitSFX->SetTriggerParameter(FName("TankImpact"));
	}
	
	_ActorDamaged = OtherActor;

#pragma region // Trace Parameter
	const FVector TraceStart =  GetActorLocation();
	const FVector TraceEnd = TraceStart + GetActorForwardVector() * _TracePartCheckRange;

	TArray<AActor*> ActorsToIgnore{};
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(GetOwner());
	FHitResult HitResult{};
#pragma endregion // Trace Parameter
	
	bool bHitTankPart = UKismetSystemLibrary::LineTraceSingle(this, TraceStart, TraceEnd, _TraceTypeQuery, false, ActorsToIgnore, _DrawDebugTrace, HitResult, true);

	FVector ActorLocation = GetActorLocation();
	//Play shell impact particle effect
	UGameplayStatics::SpawnEmitterAtLocation(this, _ImpactParticle, GetActorLocation());	
	(bHitTankPart == false) ? DamageTankPart(nullptr) : FindTankPartHit(HitResult);

	Destroy();
}


void AShell::FindTankPartHit(const FHitResult& TankPart) const
{
	if(TankPart.Component->ComponentHasTag(FName("Crew")))
	{
		DamageTankPart(_Crew);
	}
	else if(TankPart.Component->ComponentHasTag(FName("Turret")))
	{
		DamageTankPart(_Turret);
	}
	else if(TankPart.Component->ComponentHasTag(FName("Engine")))
	{
		DamageTankPart(_Engine);
	}
	else if(TankPart.Component->ComponentHasTag(FName("LeftTrack")))
	{
		DamageTankPart(_LeftTrack);
	}
	else if(TankPart.Component->ComponentHasTag(FName("RightTrack")))
	{
		DamageTankPart(_RightTrack);
	}
}

void AShell::DamageTankPart(const TSubclassOf<UDamageType> DamageType) const
{
	UGameplayStatics::ApplyDamage(_ActorDamaged, _DamageAmount, GetOwner()->GetInstigatorController(), GetOwner(), DamageType);
}

