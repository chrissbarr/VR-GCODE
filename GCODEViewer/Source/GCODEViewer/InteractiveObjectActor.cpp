// Fill out your copyright notice in the Description page of Project Settings.

#include "GCODEViewer.h"
#include "InteractiveObjectActor.h"


// Sets default values
AInteractiveObjectActor::AInteractiveObjectActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AInteractiveObjectActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInteractiveObjectActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// reset scale
void AInteractiveObjectActor::resetScale()
{

	SetActorScale3D(initialScale);

}

// Called every frame
void AInteractiveObjectActor::resetOrientation()
{

	SetActorRotation(initialRotation);

}



