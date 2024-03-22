
#include "Tank/TankPawn.h"
#include "ChaosVehicleMovementComponent.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Tank/Enums/EPartDamaged.h"

ATankPawn::ATankPawn()
{
	PrimaryActorTick.bCanEverTick = false;
	
	_SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm Component"));
	_CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));

	SetRootComponent(GetMesh());
	
	_SpringArmComponent->SetupAttachment(GetRootComponent());
	_CameraComponent->SetupAttachment(_SpringArmComponent);

	_SkeletalMeshComponent = GetMesh();
	
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	_VehicleMovementComponent = GetVehicleMovementComponent();
}

void ATankPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	_TurretInterpRotationSpeed = _TankAttributes._TurretRotationSpeed;

	/*Clamp the camera pitch values*/
	if(APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		CameraManager->ViewPitchMax = _ViewPitchMax;
		CameraManager->ViewPitchMin = _ViewPitchMin;
	}

	_SpringArmComponent->TargetOffset = _DefaultTargetOffset;
	_SpringArmComponent->TargetArmLength = _DefaultTargetArmLength;
}

void ATankPawn::Initialise(UPlayerWidget* CrosshairWidget)
{
	if(CrosshairWidget == nullptr) { return; }
	_PlayerWidget = CrosshairWidget;

	GetWorldTimerManager().SetTimer(TH_WidgetUpdate, this, &ATankPawn::CustomTick, 0.01, true);
}

void ATankPawn::CustomTick()
{
	/*Update gun aim location widget*/
	_PlayerWidget->DisplaySightToScreen(CalculateScreenPos());

	(_bIsTankTurning || IsTankMoving() || _bIsTurretTurning) ? _PlayerWidget->ChangeAccuracySize(true) : _PlayerWidget->ChangeAccuracySize(false);
}


FVector2D ATankPawn::CalculateScreenPos()
{
	if(_SkeletalMeshComponent == nullptr) { return FVector2D(); }

#pragma region // Trace Parameter
	const FVector TraceStart = _SkeletalMeshComponent->GetSocketLocation(_GunSocketName);
	const FVector TraceEnd = TraceStart + _SkeletalMeshComponent->GetSocketRotation(_GunSocketName).Vector() * _ScreenPoseTraceRange;

	TArray<AActor*> ActorToIgnore{};
	ActorToIgnore.Add(this);
	FHitResult OutHit{};
#pragma endregion // Trace Parameter
	
	bool bDidTraceImpact = UKismetSystemLibrary::LineTraceSingle(this, TraceStart, TraceEnd, _SPTraceTypeQuery, false,
		ActorToIgnore, _SPDrawDebugTrace, OutHit, true);

	const FVector WorldLocation = (bDidTraceImpact == true) ? OutHit.ImpactPoint : OutHit.TraceEnd;
	FVector2D ScreenPos{};
	/*Convert the World Location to Screen Position*/
	UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(UGameplayStatics::GetPlayerController(this, 0), WorldLocation, ScreenPos, true);
	
	return ScreenPos;
}

void ATankPawn::PerformStartTankMovement()
{
	if(_AudioComponent == nullptr) { return; }
	
	_bIsTankMovingInputApplied = true;
	_VehicleMovementComponent->SetHandbrakeInput(false);
	_TurretInterpRotationSpeed = _TankAttributes._TurretRotationSpeed;

	if(_bIsEngineSFXPlaying == false)
	{
		TriggerSFXInput(_SFXStartMoving);
        _bIsEngineSFXPlaying = true;
	}
}

void ATankPawn::PerformTankMovement(const FInputActionValue& Value)
{
	const float ForwardDirection = Value.Get<float>();
	
	const float OverallSpeed = FMath::Abs(ForwardDirection * _TankAttributes._MovementSpeed);
	(ForwardDirection >= 0) ? SetMovementInput(OverallSpeed, 0.f) : SetMovementInput(0.f, OverallSpeed);
}

void ATankPawn::PerformStopTankMovement(const FInputActionValue& Value)
{
	_bIsTankMovingInputApplied = false;
	SetMovementInput(0.f,0.f);

	/*Reduce turret rotation based on if the tank is turning, while it isn't moving.
	 *Turret rotation speed should be reduced if it is only turning.*/
	_TurretInterpRotationSpeed = (_bIsTankTurning == false) ? _TankAttributes._TurretRotationSpeed :  _TurretInterpRotationSpeed * _ReduceTurretRotSpeed;

	if(_bIsEngineSFXPlaying == true)
	{
		TriggerSFXInput(_SFXStopMoving);
		_bIsEngineSFXPlaying = false;
		CheckForTankIdleSFX();
	}
}

void ATankPawn::PerformStartTankTurn()
{
	const FVector CrossProduct = FVector::CrossProduct(_SkeletalMeshComponent->GetForwardVector(), _SkeletalMeshComponent->GetRightVector());
	if(CrossProduct.Z < 0)
	{
		_bCanTankTurn = false;
	}
	
	_bIsTankTurning = true;
	//Set turret rotation speed based on if the tank is moving forward or backward.
	_TurretInterpRotationSpeed = (_bIsTankMovingInputApplied) ? _TankAttributes._TurretRotationSpeed : _TurretInterpRotationSpeed * _ReduceTurretRotSpeed;

	if(_bIsTankTurningSFXPlaying == false)
	{
		TriggerSFXInput(_SFXTankTurning);
		_bIsTankTurningSFXPlaying = true;
	}
}

void ATankPawn::PerformTankTurn(const FInputActionValue& Value)
{
	if(_bCanTankTurn == false) { return; }
	const float ActionValue = Value.Get<float>();
	
	ApplyTrackSmoke(ActionValue);

	/*Checks if the tank is rotating too fast
	 *If true, Slow down the tank turning speed*/
	if(_SkeletalMeshComponent->GetPhysicsAngularVelocityInDegrees().Length() > 30)
	{
		const float LerpActionValue = FMath::LerpStable(ActionValue, 0.f, _ReduceTurnLerpRate);
		_VehicleMovementComponent->SetYawInput(LerpActionValue);
		return;
	}

	/*If tank is not moving apply small amount of throttle input
	 *This will allow the tank to turn both ways while trying to turn in place.*/
	if(_bIsTankMovingInputApplied == false)
	{
		_VehicleMovementComponent->SetHandbrakeInput(true);
		_VehicleMovementComponent->SetThrottleInput(_TurnThrottleInput);
	}
	
	const float DirectionTurnSpeed = (ActionValue <= 0) ? _TankAttributes._RightTrackTurn : _TankAttributes._LeftTrackTurn;
	_VehicleMovementComponent->SetYawInput(ActionValue * DirectionTurnSpeed);
}

void ATankPawn::PerformStopTankTurn()
{
	_bIsTankTurning = false;
	_VehicleMovementComponent->SetYawInput(0.f);
	SetMovementInput(0.f,0.f);

	//Reset Turret rotation speed to default.
	_TurretInterpRotationSpeed = _TankAttributes._TurretRotationSpeed;

	if(_bIsTankTurningSFXPlaying == true)
	{
		TriggerSFXInput(_SFXStopTankTurning);
		_bIsTankTurningSFXPlaying = false;
		CheckForTankIdleSFX();
	}
}

void ATankPawn::PerformLook(const FInputActionValue& Value)
{
	const FVector2D ViewDirection  = Value.Get<FVector2D>();
	
	_bIsTurretTurning = true;
	AddControllerPitchInput(ViewDirection.Y * _CameraSpeed);
	AddControllerYawInput(ViewDirection.X * _CameraSpeed);
}

void ATankPawn::PerformStopLook()
{
	_bIsTurretTurning = false;
}

float ATankPawn::CalculateTurretTurn()
{
	if(_bCanTurretUpdate == false)  { return _TurretAngle; }
	
	/*Checks if the components exist*/
	const UWorld* World = GetWorld();
	if(_CameraComponent == nullptr || _SkeletalMeshComponent == nullptr || World == nullptr) { return float(); }

#pragma region //Calculate the cameras view direction
	const FVector CameraForwardVector = _CameraComponent->GetForwardVector();
	FVector NewCameraVector = FVector(CameraForwardVector.X,CameraForwardVector.Y,0.f);
	NewCameraVector.Normalize();
	const float CosAngle = FVector::DotProduct(FVector::ForwardVector, NewCameraVector);
	float Angle =  FMath::RadiansToDegrees(FMath::Acos(CosAngle));
	const FVector CrossProduct = FVector::CrossProduct(FVector::ForwardVector, NewCameraVector);

	//Check if turret is rotating to left or right
	(CrossProduct.Z > 0) ? Angle : Angle *= -1;
#pragma endregion //Calculate the cameras view direction
	
	const FRotator TargetYawRotation =  FRotator(0.f,Angle - GetActorRotation().Yaw,  0.f);
	const FRotator CurrentYawRotation = FRotator(0.f, _TurretAngle, 0.f);
	const FRotator InterpRotation = FMath::RInterpConstantTo(CurrentYawRotation, TargetYawRotation, World->GetDeltaSeconds(), _TurretInterpRotationSpeed);
	_TurretAngle = InterpRotation.Yaw;


#pragma region //Adjust camera speed according to the distance between camera and socket yaw rotation
	if(_CameraState == ECameraState::ECS_FirstPerson)
	{
		const float SocketYaw = _SkeletalMeshComponent->GetSocketRotation(_TurretSkeletalName).Yaw;
		const float CameraYaw = _CameraComponent->GetComponentRotation().Yaw;

		const float YawOffset = FMath::Abs(SocketYaw - CameraYaw);
		
		(YawOffset > _TurretMaxAudioDistance) ? StartTurretSFX() : StopTurretSFX();
		
		(YawOffset > _MaxCameraDistanceFromSocket) ?
			ChangeCameraSpeedTo(_MinCameraSpeed, _MinCameraInterpSpeed) : ChangeCameraSpeedTo(_MaxCameraSpeed, _MaxCameraInterpSpeed);
	}
#pragma endregion //Adjust camera speed according to the distance between camera and socket yaw rotation
	
	return _TurretAngle;
}

void ATankPawn::ChangeCameraSpeedTo(float NewCameraSpeed, float InterpSpeed)
{
	if(const UWorld* World = GetWorld())
		_CameraSpeed = FMath::FInterpConstantTo(_CameraSpeed, NewCameraSpeed, World->GetDeltaSeconds(), InterpSpeed);
}

float ATankPawn::CalculateGunPitch()
{
	if(_bCanTurretUpdate == false)  { return _GunPitchAngle; }

	if(_CameraState == ECameraState::ECS_TopDown)
	{
		return 0.f;
	}
	
	/*Checking if the components are accessible*/ 
	USkeletalMeshComponent* SkeletalMeshComponent = _SkeletalMeshComponent;
	UWorld* World = GetWorld();
	if(_CameraComponent == nullptr || SkeletalMeshComponent == nullptr || World == nullptr) { return float(); }

#pragma region // Trace Parameters
	FVector TraceStart = _CameraComponent->GetComponentLocation(); 
	FVector TraceEnd = TraceStart + (_CameraComponent->GetForwardVector() * _GunPitchTraceDistance);
	TArray<AActor*> ActorsToIgnore{};
	ActorsToIgnore.Add(this);
	FHitResult OutHit;
#pragma endregion // Trace Parameters
	
	bool TraceImpact = UKismetSystemLibrary::LineTraceSingle(this, TraceStart, TraceEnd, _GunTraceTypeQuery, false,
		ActorsToIgnore, _GunDrawDebugTrace, OutHit, true);
	
	const FVector TraceVector = (TraceImpact) ? OutHit.Location : OutHit.TraceEnd;
	const FVector VectorOffset = TraceVector - (_SkeletalMeshComponent->GetSocketLocation(_GunSkeletalName));
	const FRotator TargetRotation = FRotationMatrix::MakeFromX(VectorOffset).Rotator();
	const float ClampPitchAngle = FMath::Clamp(TargetRotation.Pitch - GetActorRotation().Pitch, _MinGunPitch, _MaxGunPitch);

	_GunPitchAngle = FMath::FInterpConstantTo(_GunPitchAngle, ClampPitchAngle, GetWorld()->GetDeltaSeconds(), _GunPitchInterpSpeed);
	
	if(_CameraState == ECameraState::ECS_FirstPerson)
	{
		if(ClampPitchAngle <= _MinGunPitch || ClampPitchAngle >= _MaxGunPitch)
		{
			StopGunSFX();
		}
		else
		{
			const float SocketPitch = SkeletalMeshComponent->GetSocketRotation(_GunSkeletalName).Pitch;
			const float CameraPitch = _CameraComponent->GetComponentRotation().Pitch;

			const float PitchOffset = FMath::Abs(SocketPitch - CameraPitch);
			(PitchOffset > _TurretMaxAudioDistance) ? StartGunSFX() : StopGunSFX();
		}
	}
	return _GunPitchAngle;
}

void ATankPawn::PerformFire(const FInputActionValue& Value)
{
	if(_bCanTurretUpdate == false) { return; }
	(_bIsTankTurning || _bIsTankMovingInputApplied || _bIsTurretTurning) ? SpawnShell(true) : SpawnShell(false);
}

void ATankPawn::PerformCameraStatSwitch(const FInputActionValue& Value)
{
	const float ZoomValue = Value.Get<float>() * _ZoomRate;
	
	switch(_CameraState)
	{
	case ECameraState::ECS_FirstPerson:
		UpdateCameraFOV(ZoomValue);
		ChangeSoundPitch();
		break;
	case ECameraState::ECS_ThirdPerson:
		UpdateSpringArmLength(ZoomValue);
		ChangeSoundPitch();
		StopTurretSFX();
		StopGunSFX();
		break;
	case ECameraState::ECS_TopDown:
		UpdateSpringArmTargetOffset(ZoomValue);
		break;
	}
	
}

void ATankPawn::UpdateCameraFOV(float ZoomValue)
{
	if(_CameraComponent == nullptr) { return; }

	float NewCameraFOV = _CameraComponent->FieldOfView;
	
	if(_bCameraAttachedToGun == false)
	{
		ChangeCameraAttachment(_GunSocketName, 0.f, FVector::ZeroVector, true);
	}
	
	NewCameraFOV = FMath::Clamp(NewCameraFOV + (ZoomValue * _FOVZoomRate), _MinFOV, _MaxFOV);
	if(FMath::IsNearlyEqual(NewCameraFOV, _MaxFOV))
	{
		SetCameraState(ECameraState::ECS_ThirdPerson);
	}

	_CameraComponent->FieldOfView = NewCameraFOV;
}

void ATankPawn::UpdateSpringArmLength(float ZoomValue)
{
	if(_SpringArmComponent == nullptr) { return; }

	_CameraSpeed = _MaxCameraSpeed;

	float& TargetArmLength = _SpringArmComponent->TargetArmLength;
	if(_bCameraAttachedToGun)
	{
		ChangeCameraAttachment(FName(), _DefaultTargetArmLength, _DefaultTargetOffset, false);
	}
	
	TargetArmLength = FMath::Clamp(TargetArmLength + ZoomValue, _MinForwardZoom, _MaxForwardZoom);

	if(FMath::IsNearlyEqual(TargetArmLength, _MaxForwardZoom))
	{
		SetCameraState(ECameraState::ECS_TopDown);
	}

	if(FMath::IsNearlyEqual(TargetArmLength, _MinForwardZoom))
	{
		SetCameraState(ECameraState::ECS_FirstPerson);
	}
	
}

/**
 * Moves the camera upwards by adjusting the target offset.
 * 
 * @param ZoomValue How fast should the camera zoom in or out.
 */
void ATankPawn::UpdateSpringArmTargetOffset(float ZoomValue)
{
	if(_SpringArmComponent == nullptr) { return; }
	
	FVector& TargetOffset = _SpringArmComponent->TargetOffset;
	TargetOffset.Z = FMath::Clamp(TargetOffset.Z + ZoomValue, _MinUpZoom, _MaxUpZoom);

	if(TargetOffset.Z <= _MinUpZoom)
	{
		SetCameraState(ECameraState::ECS_ThirdPerson);
	}
}

/**
 *	Allows the camera to be attached to two locations:
 *	Custom Gun Socket location.
 *	Back to Tank mesh center point.
 *	
 * @param SocketName Socket to attach to
 * @param TargetArmLength Set new Target Arm length
 * @param TargetOffset Set new target offset
 * @param bIsCameraAttachedToGun update if the camera is attached to gun or not.
 */
void ATankPawn::ChangeCameraAttachment(FName SocketName, float TargetArmLength, FVector TargetOffset, bool bIsCameraAttachedToGun)
{
	_bCameraAttachedToGun = bIsCameraAttachedToGun;
	const FAttachmentTransformRules AttachmentTransformRules{EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false};
	_SpringArmComponent->AttachToComponent(_SkeletalMeshComponent, AttachmentTransformRules, SocketName);
	_SpringArmComponent->TargetArmLength = TargetArmLength;
	_SpringArmComponent->TargetOffset = TargetOffset;
}

void ATankPawn::PerformBrake(const FInputActionValue& Value)
{
	_VehicleMovementComponent->SetHandbrakeInput(true);
	_bIsTankMovingInputApplied = false;
}

void ATankPawn::PerformCancelBrake() const
{
	_VehicleMovementComponent->SetHandbrakeInput(false);
}

void ATankPawn::SetMovementInput(float ThrottleInput, float BrakeInput) const
{
	//Move tank forward
	_VehicleMovementComponent->SetThrottleInput(ThrottleInput);
	//Move Tank Backwards
	_VehicleMovementComponent->SetBrakeInput(BrakeInput);
}

bool ATankPawn::IsTankMoving() const
{
	if(_VehicleMovementComponent == nullptr) { return false; }
	bool bIsMoving = false;
	
	const float TankAccel = _VehicleMovementComponent->GetForwardSpeed();
	if(FMath::IsNearlyZero(TankAccel) == false)
	{
		bIsMoving = true;
	}

	return bIsMoving;
}

void ATankPawn::SetCameraState(ECameraState InCameraState)
{
	_CameraState = InCameraState;
}

void ATankPawn::StopTurretSFX()
{
	if(_bisTurretSFXPlaying == true)
	{
		_bisTurretSFXPlaying = false;
		TriggerSFXInput(_SFXStopTurretTurn);
	}
}

void ATankPawn::StartTurretSFX()
{
	if(_bisTurretSFXPlaying == false)
	{
		_bisTurretSFXPlaying = true;
		TriggerSFXInput(_SFXStartTurretTurn);
	}
}

void ATankPawn::StartGunSFX()
{
	if(_bIsGunSFXPlaying == false)
	{
		TriggerSFXInput(FName("StartGunTurn"));
		_bIsGunSFXPlaying = true;
	}
}

void ATankPawn::StopGunSFX()
{
	if(_bIsGunSFXPlaying == true)
	{
		TriggerSFXInput(FName("StopGunTurn"));
		_bIsGunSFXPlaying = false;
	}
}

#pragma region //Apply Damage Effect and update UI for Widget parts damage display.
void ATankPawn::ApplyCrewEffect()
{
	Super::ApplyCrewEffect();
	PartDamagedUpdate(EPartDamaged::EPD_Crew);
}

void ATankPawn::ApplyTurretEffect()
{
	Super::ApplyTurretEffect();
	PartDamagedUpdate(EPartDamaged::EPD_Turret);
	_TurretInterpRotationSpeed = _TankAttributes._TurretRotationSpeed;
}

void ATankPawn::ApplyEngineEffect()
{
	Super::ApplyEngineEffect();
	PartDamagedUpdate(EPartDamaged::EPD_Engine);
	_bCanTurretUpdate = false;
}

void ATankPawn::ApplyLeftTrackEffect()
{
	Super::ApplyLeftTrackEffect();
	PartDamagedUpdate(EPartDamaged::EPD_LeftTrack);
	/*Reset VehicleState to Apply Drag Coefficient.
	 *This will reduce the speed of the tank.*/
	_VehicleMovementComponent->DragCoefficient += _IncreaseDragCoefficient;
	_VehicleMovementComponent->ResetVehicleState();
}

void ATankPawn::ApplyRightTrackEffect()
{
	Super::ApplyRightTrackEffect();
	PartDamagedUpdate(EPartDamaged::EPD_RightTrack);
	/*Reset VehicleState to Apply Drag Coefficient.
	 *This will reduce the speed of the tank.*/
	_VehicleMovementComponent->DragCoefficient += _IncreaseDragCoefficient;
	_VehicleMovementComponent->ResetVehicleState();
}
#pragma endregion //Apply Damage Effect and update UI for Widget parts damage display.

void ATankPawn::UpdateHealthBar(float HealthPercent)
{
	Super::UpdateHealthBar(HealthPercent);

	_PlayerWidget->UpdateHealthUI(HealthPercent);
}

void ATankPawn::DestroyTank()
{
	Super::DestroyTank();

	OnTankDestroyed.ExecuteIfBound();

	AActor::Destroy();
}

void ATankPawn::PartDamagedUpdate(EPartDamaged PartDamaged) const
{
	_PlayerWidget->PartDamageUIUpdate(PartDamaged);
}

void ATankPawn::ChangeSoundPitch()
{
	if(_CameraState == ECameraState::ECS_FirstPerson && _bHasSFXPitchLowered == false)
	{
		_AudioComponent->SetFloatParameter(FName("TankPitch"), -10.f);
		_bHasSFXPitchLowered = true;
	}
	else if(_CameraState == ECameraState::ECS_ThirdPerson && _bHasSFXPitchLowered == true)
	{
		_AudioComponent->SetFloatParameter(FName("TankPitch"), 0.f);
		_bHasSFXPitchLowered = false;
	}
}

void ATankPawn::CheckForTankIdleSFX() const
{
	if(_bIsTankTurning == false && _bIsTankMovingInputApplied == false)
	{
		TriggerSFXInput(_SFXStartEngine);	
	}
}
