// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "FileDisplayTile.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class ETileTargetType : uint8
{
	PE_FOLDER 	UMETA(DisplayName = "Folder"),
	PE_GCODE 	UMETA(DisplayName = "GCODE File"),
	PE_STL 	UMETA(DisplayName = "STL File"),
	PE_STEP	UMETA(DisplayName = "STEP File")
};

UCLASS()
class GCODEVIEWER_API AFileDisplayTile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFileDisplayTile();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	void update();

	/** Maximum model scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FileExplorer)
	FString filePath;

	/** Maximum model scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FileExplorer)
	ETileTargetType targetType;

	FString fileName;

	FString displayName;

	int displayNameMaxLength = 15;

	int folderTreeLevel;
	int folderTreeIndex;


	UTextRenderComponent* textComponent;
	UBoxComponent* collisionComponent;
	
	//AFileManagerActor* parentFileManager;
	class AFileManagerActor* parentFileManager;
};
