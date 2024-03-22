
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Shell.generated.h"

class UProjectileMovementComponent;
class UCapsuleComponent;

UCLASS()
class WARFAIR_API AShell : public AActor
{
	GENERATED_BODY()

public:
	AShell();

protected:
	virtual void BeginPlay() override;
	
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCapsuleComponent> _CapsuleComponent{};
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> _StaticMeshComponent{};
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> _ProjectileMovementComponent{};

	UPROPERTY(EditDefaultsOnly, Category = "Shell")
	float _ActorLifeSpan{3.f};
	UPROPERTY(EditDefaultsOnly, Category = "Shell")
	float _ProjectileSpeed{50000.f};
	UPROPERTY(EditDefaultsOnly, Category = "Shell")
	float _ProjectileVelocity{2.f};
	UPROPERTY(EditDefaultsOnly, Category = "Shell")
	float _DamageAmount{20.f};

	UPROPERTY(EditDefaultsOnly, Category = "Shell | Trace", meta=(ToolTip = "The trace type should be set to TankPart."))
	TEnumAsByte<ETraceTypeQuery> _TraceTypeQuery{};
	UPROPERTY(EditDefaultsOnly, Category = "Shell | Trace")
	TEnumAsByte<EDrawDebugTrace::Type> _DrawDebugTrace{};
	UPROPERTY(EditDefaultsOnly, Category = "Shell | Trace")
	float _ScreenPoseTraceRange{7000.f};
	UPROPERTY(EditDefaultsOnly, Category = "Shell | Trace")
	float _TracePartCheckRange{100.f};

	UPROPERTY(EditDefaultsOnly, Category = "Shell | DamageType")
	TSubclassOf<UDamageType> _Crew{};
	UPROPERTY(EditDefaultsOnly, Category = "Shell | DamageType")
	TSubclassOf<UDamageType> _Turret{};
	UPROPERTY(EditDefaultsOnly, Category = "Shell | DamageType")
	TSubclassOf<UDamageType> _Engine{};
	UPROPERTY(EditDefaultsOnly, Category = "Shell | DamageType")
	TSubclassOf<UDamageType> _RightTrack{};
	UPROPERTY(EditDefaultsOnly, Category = "Shell | DamageType")
	TSubclassOf<UDamageType> _LeftTrack{};
	
	UPROPERTY(EditDefaultsOnly, Category = "Shell | Particle")
	TObjectPtr<UParticleSystem> _ImpactParticle{};
	UPROPERTY(EditDefaultsOnly, Category = "Shell | SoundEffects")
	TObjectPtr<USoundBase> _TankHitSFX{};
	
	UPROPERTY()
	TObjectPtr<AActor> _ActorDamaged{};
	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void FindTankPartHit(const FHitResult& TankPart) const;
	void DamageTankPart(const TSubclassOf<UDamageType>  DamageType) const;
};
