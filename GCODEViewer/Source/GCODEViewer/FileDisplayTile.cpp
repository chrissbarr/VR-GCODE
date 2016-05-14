// Fill out your copyright notice in the Description page of Project Settings.

#include "GCODEViewer.h"
#include "FileManagerActor.h"
#include "FileDisplayTile.h"


// Sets default values
AFileDisplayTile::AFileDisplayTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Our root component will be a sphere that reacts to physics
	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(2.0f);
	SphereComponent->SetVisibility(true, false);
	SphereComponent->SetHiddenInGame(false);
	SphereComponent->SetCollisionProfileName(TEXT("Pawn"));


	textComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComponent"));

	textComponent->RegisterComponent();
	textComponent->AttachTo(GetRootComponent(), NAME_None);
	textComponent->AttachParent = RootComponent;

	textComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);

	collisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	collisionComponent->RegisterComponent();
	collisionComponent->AttachTo(GetRootComponent(), NAME_None);
	collisionComponent->AttachParent = RootComponent;
	collisionComponent->SetBoxExtent(FVector(5, 100, 30),true);
	collisionComponent->SetVisibility(true, false);
	collisionComponent->SetHiddenInGame(false);
}

// Called when the game starts or when spawned
void AFileDisplayTile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFileDisplayTile::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	

}

void AFileDisplayTile::update() {


	if (fileName.Contains(".", ESearchCase::IgnoreCase, ESearchDir::FromStart)) {
		if (filePath.EndsWith("gcode", ESearchCase::IgnoreCase)) {
			targetType = ETileTargetType::PE_GCODE;
		}
		else if (filePath.EndsWith("stl", ESearchCase::IgnoreCase)) {
			targetType = ETileTargetType::PE_STL;
		}
		else {
			targetType = ETileTargetType::PE_FOLDER;
		}
	}

	if (targetType != ETileTargetType::PE_FOLDER) {
		fileName.Split(".", &displayName, NULL, ESearchCase::IgnoreCase, ESearchDir::FromStart);
	}
	else {
		displayName = FString(fileName);
	}

	if (displayName.Len() > displayNameMaxLength) {
		displayName = displayName.Mid(0, displayNameMaxLength);
		displayName.Append("...");
	}

	
	textComponent->SetText(displayName);

	collisionComponent->SetBoxExtent(FVector(5, ((parentFileManager->spawnRadius) * PI) / parentFileManager->maxTilesPerCircumference, 30), true);
}

