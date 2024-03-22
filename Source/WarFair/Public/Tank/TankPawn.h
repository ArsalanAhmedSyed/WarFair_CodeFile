#pragma once

#include "CoreMinimal.h"
#include "BaseTank.h"
#include "WheeledVehiclePawn.h"
#include "InputActionValue.h"
#include "Enums/ECameraState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "widget/PlayerWidget.h"
#include "TankPawn.generated.h"

class UArrowComponent;
class UHandlePartsComponent;
enum class ECameraState : uint8;
class UCameraComponent;
class USpringArmComponent;

DECLARE_DELEGATE(FOnTankDestroyedSignature)

UCLASS()
class WARFAIR_API ATankPawn : public ABaseTank
{
	GENERATED_BODY()

public:

	FOnTankDestroyedSignature OnTankDestroyed{};
	
	ATankPawn();
	void Initialise(UPlayerWidget* CrosshairWidget);

	UFUNCTION(BlueprintCallable, Category = "Player Data | Audio")
	void StopTurretSFX();
	UFUNCTION(BlueprintCallable, Category = "Player Data | Audio")
	void StartTurretSFX();

#pragma region // Input Function calls
	void PerformTankMovement(const FInputActionValue& Value);
	void PerformStopTankMovement(const FInputActionValue& Value);
	void PerformStartTankMovement();
	
	void PerformTankTurn(const FInputActionValue& Value);
	void PerformStartTankTurn();
	void PerformStopTankTurn();
	
	void PerformLook(const FInputActionValue& Value);
	void PerformStopLook();
	void ChangeCameraSpeedTo(float NewCameraSpeed, float InterpSpeed);

	void PerformFire(const FInputActionValue& Value);
	void PerformCameraStatSwitch(const FInputActionValue& Value);
	
	void PerformBrake(const FInputActionValue& Value);
	void PerformCancelBrake() const;
#pragma endregion // Input Function calls
	
protected:

	virtual void OnConstruction(const FTransform& Transform) override;
	
	/**
	 * Allows smoke to be applied to Tracks.
	 * The particle setup should be in blueprint.
	 * 
	 * @param Intensity How much smoke effect should take place.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ApplyTrackSmoke(float Intensity);
	/**
	 * Leave Track marks 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tank")
	void ApplyDecal();

#pragma region //Damage Tank Part Performance Effect
	virtual void ApplyCrewEffect() override;
	virtual void ApplyTurretEffect() override;
	virtual void ApplyEngineEffect() override;
	virtual void ApplyLeftTrackEffect() override;
	virtual void ApplyRightTrackEffect() override;
#pragma endregion //Damage Tank Part Performance Effect

	virtual void UpdateHealthBar(float HealthPercent) override;
	virtual void DestroyTank() override;
private:

#pragma region // Components
	UPROPERTY(VisibleAnywhere, Category = "Components | Camera")
	TObjectPtr<USpringArmComponent> _SpringArmComponent{};
	UPROPERTY(VisibleAnywhere, Category = "Components | Camera")
	TObjectPtr<UCameraComponent> _CameraComponent{};

	/*Store component value *NOT CREATED**/
	TObjectPtr<UChaosVehicleMovementComponent> _VehicleMovementComponent{};
	TObjectPtr<USkeletalMeshComponent> _SkeletalMeshComponent{};
#pragma endregion // Components

#pragma region // SpringArm
	UPROPERTY(EditDefaultsOnly, Category = "Player Data | Camera")
	FVector _DefaultTargetOffset{0,0,400.f};
	UPROPERTY(EditDefaultsOnly, Category = "Player Data | Camera")
	float _DefaultTargetArmLength{600.f};
#pragma endregion // SpringArm

#pragma region // Camera Properties 
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Camera")
	float _ViewPitchMax{15};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Camera")
	float _ViewPitchMin{-60};
	float _MaxCameraYaw{};
	float _MinCameraYaw{};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Camera", meta=(ToolTip="Adjust how fast should the camera move with the mouse"))
	float _CameraSpeed{0.8f};

	/*Scroll Zoom in & out*/
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Camera | Zoom", meta=(ToolTip= "Sets how fast the Camera Zoom in and out should be for third person and top down camera state."))
	float _ZoomRate{90};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Camera | Zoom", meta=(ToolTip= "Sets how fast the FOV of the camera should Zoom in and out."))	
	float _FOVZoomRate{0.1f};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Camera | Zoom")
	float _MinFOV{10};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Camera | Zoom")	
	float _MaxFOV{90};
	bool _bCameraAttachedToGun{false};

	/*Camera Forward & Backward Zoom Properties*/
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Camera | Zoom", meta=(ToolTip="Set how far back the camera can zoom out."))
	float _MaxForwardZoom{2500.f};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Camera | Zoom", meta=(ToolTip="Set how far forward the camera can zoom in"))
	float _MinForwardZoom{0.f};

	/*Camera Up & Down Zoom Properties*/
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Camera | Zoom", meta=(ToolTip="Set how far up the camera should Zoom out."))
	float _MaxUpZoom{2000.f};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Camera | Zoom", meta=(ToolTip="Set how far down the camera should Zoom in."))
	float _MinUpZoom{400.f};
	
	ECameraState _CameraState{ECameraState::ECS_ThirdPerson};
	
	void SetCameraState(ECameraState InCameraState);
	void ChangeCameraAttachment(FName SocketName, float TargetArmLength, FVector TargetOffset, bool bIsCameraAttachedToGun);
	void UpdateCameraFOV(float ZoomValue);
	void UpdateSpringArmLength(float ZoomValue);
	void UpdateSpringArmTargetOffset(float ZoomValue);


	/**
	 * This section is related to the Turret and camera offset
	 * during the calculate turret turn function.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Turret",
		meta=(ToolTip = "How much the camera speed should be reduced, when it is far away from the gun \n (This is for first person camera state only"))
	float _MinCameraSpeed{0.01f};
	//Represents default camera speed, **DO NOT CHANGE**
	float _MaxCameraSpeed{1.f};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Turret", meta=(ToolTip="How fast should the mouse speed reduce."))
	float _MinCameraInterpSpeed{50.f};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Turret", meta=(ToolTip="How fast should the mouse speed increase."))
	float _MaxCameraInterpSpeed{1.f};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Turret",
		meta=(ToolTip="At what distance range should the camera speed be reduced. \n This will allow the turret to catch up with the camera rotation."))
	float _MaxCameraDistanceFromSocket{50.f};
#pragma endregion // Camera Properties

#pragma region //Tank Turning and Movement
	void SetMovementInput(float ThrottleInput, float BrakeInput) const;
	bool IsTankMoving() const;
	
	UPROPERTY(VisibleAnywhere, Category = "Tank | Test")
	bool _bIsTankMovingInputApplied{false};
	UPROPERTY(VisibleAnywhere, Category = "Tank | Test")
	bool _bIsTankTurning{false};
	bool _bCanTankTurn{true};
	
	/*These Turning Speed should be adjusted
	 *when adjusting the Turning speed of the tank*/
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Hull", meta=(ToolTip="Set how fast should the turning be reduced."))
	float _ReduceTurnLerpRate{0.8f};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Hull", meta=(ToolTip = "Set how much tank should move forward when turning in place."))
	float _TurnThrottleInput{0.1f};

	UPROPERTY(EditDefaultsOnly, Category = "Tank | Hull", meta=(ToolTip="Set value to reduce speed of tank when a track is damaged. \n The higher the value the slower the tank will move"))
	float _IncreaseDragCoefficient{1000.f};
#pragma endregion //Tank Turning and Movement

#pragma region // Turret Turn
	UFUNCTION(BlueprintCallable, BlueprintPure , Category = "Tank | Turret")
	float CalculateTurretTurn();
	bool _bIsTurretTurning{false};
	bool _bCanTurretUpdate{true};
	bool _bHasTurningSpeedReduced{false};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Turret")
	float _ReduceTurretRotSpeed{0.5};
	float _DefaultTurretSpeed{40.f};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Turret")
	float _TurretMaxAudioDistance{1.f};
#pragma endregion // Turret Turn

#pragma region // Gun Elevation properties
	virtual float CalculateGunPitch() override;
	void  StartGunSFX();
	void StopGunSFX();
	bool _bIsGunSFXPlaying{false};
	/*Trace Properties*/
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Gun | Trace", meta=(ToolTip = "Set what trace should be detected"))
	TEnumAsByte<ETraceTypeQuery> _GunTraceTypeQuery{};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Gun | Trace")
	TEnumAsByte<EDrawDebugTrace::Type> _GunDrawDebugTrace{};
	
#pragma endregion // Gun Elevation properties

#pragma region // Widget Sight (ScreenPos) Properties
	FVector2D CalculateScreenPos();
	float _ScreenPoseTraceRange{7000.f};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Widget | Trace", meta=(ToolTip = "Set what trace should be detected"))
	TEnumAsByte<ETraceTypeQuery> _SPTraceTypeQuery{};
	UPROPERTY(EditDefaultsOnly, Category = "Tank | Widget | Trace")
	TEnumAsByte<EDrawDebugTrace::Type> _SPDrawDebugTrace{};
#pragma endregion // Widget Sight (ScreenPos) Properties
	
	FTimerHandle TH_WidgetUpdate{};
	void CustomTick();

	UPROPERTY()
	TObjectPtr<UPlayerWidget> _PlayerWidget{};
	void PartDamagedUpdate(EPartDamaged PartDamaged) const;

	inline static FName _TurretSkeletalName	{"turret_jnt"};

#pragma region //Audio 
	UPROPERTY(EditDefaultsOnly, Category="Tank || Audio")
	float _DefaultPitch{0};
	UPROPERTY(EditDefaultsOnly, Category="Tank || Audio")
	float _FirstPersonPitch{-10};
	
	bool _bIsEngineSFXPlaying{false};
	bool _bisTurretSFXPlaying{false};
	bool _bIsTankTurningSFXPlaying{false};
	bool _bHasSFXPitchLowered{false};

	void ChangeSoundPitch();
	void CheckForTankIdleSFX() const;
	/*Sound Effect*/
	inline static FName _SFXStartEngine		{"StartEngine"};
	inline static FName _SFXStartMoving		{"StartMoving"};
	inline static FName _SFXStopMoving		{"StopMoving"};
	inline static FName _SFXStartTurretTurn	{"StartTurretTurn"};
	inline static FName _SFXStopTurretTurn	{"StopTurretTurn"};
	inline static FName _SFXTankTurning		{"StartTankTurn"};
	inline static FName _SFXStopTankTurning	{"StopTankTurn"};
#pragma endregion //Audio 
};
 