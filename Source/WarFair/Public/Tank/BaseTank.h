#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Structs/FBasicAttributes.h"
#include "BaseTank.generated.h"

UCLASS(Abstract)
class WARFAIR_API ABaseTank : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	ABaseTank();
	
protected:
	virtual  void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY(VisibleAnywhere, Category = " Component | Audio")
	TObjectPtr<UAudioComponent> _AudioComponent{};
	
#pragma region //Health & Damage
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tank | Attributes")
	FBasicAttributes _TankAttributes{};
	virtual void UpdateHealthBar(float HealthPercent) {};
	virtual void DestroyTank();
#pragma endregion //Health & Damage

#pragma region // Functions to Apply Effect
	virtual void ApplyCrewEffect();
	virtual void ApplyTurretEffect();
	virtual void ApplyEngineEffect();
	virtual void ApplyLeftTrackEffect();
	virtual void ApplyRightTrackEffect();
#pragma endregion // Functions to Apply Effect

#pragma region // Gun Pitch
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tank | Gun")
	virtual float CalculateGunPitch() { return float(); }
	
	float _GunPitchAngle{};
	UPROPERTY(EditDefaultsOnly, Category = "Tank| Gun")
	float _MinGunPitch{-5.f};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Gun")
	float _MaxGunPitch{15.f};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Gun", meta=(ToolTip = "Sets how fast should the gun pitch adjust to camera view"))
	float _GunPitchInterpSpeed{10.f};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Gun | Trace")
	float _GunPitchTraceDistance{7000.f};
#pragma endregion // Gun Pitch

#pragma region // Turret
	/**
	 * Calculates the Pitch for the gun elevation
	 * 
	 * @return Returns Pitch for the Skeletal Mesh gun rotation
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Data | Turret")
	virtual float CalculateTurretAngle(){ return float(); }
	float _TurretAngle{};
	float _TurretInterpRotationSpeed{};
#pragma endregion // Turret

#pragma region // Firing
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Gun", meta=(ToolTip="Projectile to spawn"))
	TSubclassOf<AActor> _Shell{};
	
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Particle")
	TObjectPtr<UParticleSystem> _ShellFireParticle{};
	
	virtual void SpawnShell(bool bRandomize);
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Gun")
	float _AccuracyRandomise{2.f};
	
	FTimerHandle TH_FireCoolDown;
	void FireCoolDownComplete();
	bool _bCanFire{true};
#pragma endregion // Firing

	void TriggerSFXInput(FName TriggerInputName) const;
	inline static FName _SFXFire			{"Fire"};
	
	inline static FName _GunSkeletalName	{"Gun_jnt"};
	inline static FName _GunSocketName		{"gun_1_jntSocket"};
private:
	
#pragma region //Damage Type 
	UPROPERTY(EditDefaultsOnly, Category = "Tank | DamageTypes")
	TSubclassOf<UDamageType> _Crew{};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | DamageTypes")
	TSubclassOf<UDamageType> _Turret{};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | DamageTypes")
	TSubclassOf<UDamageType> _Engine{};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | DamageTypes")
	TSubclassOf<UDamageType> _RightTrack{};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | DamageTypes")
	TSubclassOf<UDamageType> _LeftTrack{};

	bool _bIsCewDamaged{};
	bool _bIsTurretDamaged{};
	bool _bIsEngineDamaged{};
	bool _bIsLeftTrackDamaged{};
	bool _bIsRightTrackDamaged{};
#pragma endregion //Damage Type
	
	UPROPERTY(EditDefaultsOnly, Category = "Tank | DamageTypes")
	FBasicAttributes _PerformanceEffectAttributes{};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Health")
	float _MaxHealth{100};
	
	UPROPERTY() /* <<< Health won't update properly for AI without UPROPERTY() */
	float _Health{};
	void TakeHealthDamage(float DamageAmount);
	
	UPROPERTY()
	TSubclassOf<AActor> _DestroyedTankActor{};
};
