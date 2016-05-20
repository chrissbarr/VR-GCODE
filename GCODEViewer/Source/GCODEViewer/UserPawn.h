// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "UserPawnMovementComponent.h"
#include "UserPawn.generated.h"

UCLASS()
class GCODEVIEWER_API AUserPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AUserPawn();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;


	class UUserPawnMovementComponent* OurMovementComponent;
	
	virtual UPawnMovementComponent* GetMovementComponent() const override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void MoveVertically(float AxisValue);
	void Turn(float AxisValue);
	void Pitch(float AxisValue);
	
};
