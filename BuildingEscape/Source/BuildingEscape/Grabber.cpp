// Copyright Sean Martin 2017

#include "Grabber.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Engine/World.h" //Not needed for compilation, only intellisense

#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	FindPhysicsHandleComponent();
	SetupInputComponent();
}

// Look for attached Input Component (only appears at run time)
void UGrabber::SetupInputComponent()
{
	PlayerInput = GetOwner()->FindComponentByClass<UInputComponent>();
	if (PlayerInput == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Falied to find Player Input Component on %s"), *GetOwner()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Player Input Component on %s"), *GetOwner()->GetName());
		PlayerInput->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		PlayerInput->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
}

//Look for attached Physics Handle
void UGrabber::FindPhysicsHandleComponent() 
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Falied to find Physics Handle Component on %s"), *GetOwner()->GetName());
	}
	else
	{

	}
}

void UGrabber::Grab() 
{
	UE_LOG(LogTemp, Warning, TEXT("Grab Pressed"));
	///Try and reach any actors with physics body collision channel set
	auto HitResult = GetFirstPhysicsBody();
	auto ComponentToGrab = HitResult.GetComponent();
	auto ActorHit = HitResult.GetActor();

	if (ActorHit)
	{
		PhysicsHandle->GrabComponentAtLocationWithRotation(
			ComponentToGrab,
			NAME_None,
			ComponentToGrab->GetOwner()->GetActorLocation(),
			ComponentToGrab->GetOwner()->GetActorRotation()
		);
		/*PhysicsHandle->GrabComponentAtLocation(
			ComponentToGrab,
			NAME_None,
			ComponentToGrab->GetOwner()->GetActorLocation()
		); This version is pretty funny and the object turns around its orbit*/
	}
}

void UGrabber::Release()
{
	UE_LOG(LogTemp, Warning, TEXT("Grab Released"));
	PhysicsHandle->ReleaseComponent();
}

FHitResult UGrabber::GetFirstPhysicsBody() const
{
	///Get player view point this tick
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);
	FVector LineTraceEnd = PlayerViewPointLocation
		+ PlayerViewPointRotation.Vector() * Reach;

	/////Draw a red trace in the world to visualise
	//DrawDebugLine(
	//	GetWorld(),
	//	PlayerViewPointLocation,
	//	LineTraceEnd,
	//	FColor(255, 0, 0),
	//	false,
	//	0.f,
	//	0.f,
	//	10.f
	//);

	//Setup query parameters - uses a constructor
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());

	///Ray cast out to reach distance
	FHitResult Hit;
	bool bHitBody = GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		PlayerViewPointLocation,
		LineTraceEnd,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParameters
	);

	///See what we hit
	if (bHitBody)
	{
		UE_LOG(LogTemp, Warning, TEXT("We hit the %s"), *Hit.GetActor()->GetName());
	}
	return Hit;
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	///Get player view point this tick
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);
	FVector LineTraceEnd = PlayerViewPointLocation
		+ PlayerViewPointRotation.Vector() * Reach;

	if (PhysicsHandle->GrabbedComponent)
	{
		PhysicsHandle->SetTargetLocation(LineTraceEnd);
	}
}

