// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PawnMovementComponent.h"
#include "UserPawnMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class GCODEVIEWER_API UUserPawnMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
	
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

public:

	float defaultSpeed = 200;
	float fasterSpeed = 350;

	bool goFaster = false;
	
};
