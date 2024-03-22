
#include "Tank/AI/TankAI.h"

#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ATankAI::ATankAI()
{
	PrimaryActorTick.bCanEverTick = false;

	_ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow Component"));
	_HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidget Component"));
	
	_ArrowComponent->SetupAttachment(GetRootComponent());
	_HealthBarWidget->SetupAttachment(GetRootComponent());
}

void ATankAI::BeginPlay()
{
	Super::BeginPlay();

	_TurretInterpRotationSpeed = _TankAttributes._TurretRotationSpeed;
}

void ATankAI::DestroyTank()
{
	Super::DestroyTank();
	Destroy();
}

float ATankAI::CalculateTurretAngle()
{
	const USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
	const UWorld* World = GetWorld();
	if(_ArrowComponent == nullptr || SkeletalMeshComponent == nullptr || World == nullptr) { return float(); }

	const FVector CameraForwardVector = _ArrowComponent->GetForwardVector();
	FVector NewCameraVector = FVector(CameraForwardVector.X,CameraForwardVector.Y,0.f);
	NewCameraVector.Normalize();
	const float CosAngle = FVector::DotProduct(FVector::ForwardVector, NewCameraVector);
	float Angle =  FMath::RadiansToDegrees(FMath::Acos(CosAngle));
	const FVector CrossProduct = FVector::CrossProduct(FVector::ForwardVector, NewCameraVector);

	//Check if turret is rotating to left or right
	(CrossProduct.Z > 0) ? Angle : Angle *= -1;
	
	const FRotator TargetYawRotation =  FRotator(0.f,Angle - GetActorRotation().Yaw,  0.f);
	const FRotator CurrentYawRotation = FRotator(0.f, _TurretAngle, 0.f);
	const FRotator InterpRotation = FMath::RInterpConstantTo(CurrentYawRotation, TargetYawRotation, World->GetDeltaSeconds(), _TurretInterpRotationSpeed);
	
	return _TurretAngle = InterpRotation.Yaw;
}

float ATankAI::CalculateGunPitch()
{
	USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
	UWorld* World = GetWorld();
	
	if(_ArrowComponent == nullptr ||
		 SkeletalMeshComponent == nullptr ||
		 World == nullptr) { return float(); }

#pragma region // Trace Parameters
	FVector TraceStart = _ArrowComponent->GetComponentLocation(); 
	FVector TraceEnd = TraceStart + (_ArrowComponent->GetForwardVector() * _GunPitchTraceDistance);
	TArray<AActor*> ActorsToIgnore{};
	ActorsToIgnore.Add(this);
	FHitResult OutHit;
#pragma endregion // Trace Parameters
	
	bool TraceImpact = UKismetSystemLibrary::LineTraceSingle(this, TraceStart, TraceEnd, TraceTypeQuery1, false,
		ActorsToIgnore, EDrawDebugTrace::None, OutHit, true);
	
	const FVector TraceVector = (TraceImpact) ? OutHit.Location : OutHit.TraceEnd;
	const FVector VectorOffset = TraceVector - GetMesh()->GetSocketLocation(_GunSkeletalName);
	const FRotator TargetRotation = FRotationMatrix::MakeFromX(VectorOffset).Rotator();
	const float ClampPitchAngle = FMath::Clamp(TargetRotation.Pitch, _MinGunPitch, _MaxGunPitch);

	_GunPitchAngle = FMath::FInterpConstantTo(_GunPitchAngle, ClampPitchAngle, GetWorld()->GetDeltaSeconds(), _GunPitchInterpSpeed);
	return _GunPitchAngle;
	
}

void ATankAI::ApplyTurretEffect()
{
	Super::ApplyTurretEffect();
	_TurretInterpRotationSpeed = _TankAttributes._TurretRotationSpeed;
}

void ATankAI::UpdateHealthBar(float HealthPercent)
{
	UpdateWidgetHealth(HealthPercent);
}

void ATankAI::PerformFire()
{
	SpawnShell(false);
}

