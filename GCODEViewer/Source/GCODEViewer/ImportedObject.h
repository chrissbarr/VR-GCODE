// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InteractiveObjectActor.h"
#include "ImportedObject.generated.h"

/**
 * 
 */
UCLASS()
class GCODEVIEWER_API AImportedObject : public AInteractiveObjectActor
{
	GENERATED_BODY()
	
public:

	/** Path to file to load */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = File)
	FString fullFilePath;
	
	/** Set the path to the file to load, including the file name and extension */
	UFUNCTION(BlueprintCallable, Category = "ImportedObject")
	void setFullFilePath(FString path);
	
};
