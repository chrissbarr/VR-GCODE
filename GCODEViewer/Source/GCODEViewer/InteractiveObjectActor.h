// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "InteractiveObjectActor.generated.h"

UCLASS()
class GCODEVIEWER_API AInteractiveObjectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractiveObjectActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;


	/** Initial scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractiveObject)
	FVector initialScale = FVector(1,1,1);

	/** Initial rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractiveObject)
	FRotator initialRotation = FRotator(0, 0, 0);

	UFUNCTION(BlueprintCallable, Category = "InteractiveObject")
	void resetScale();

	UFUNCTION(BlueprintCallable, Category = "InteractiveObject")
	void resetOrientation();
	
	
};
