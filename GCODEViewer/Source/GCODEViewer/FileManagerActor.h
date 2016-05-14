// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "FileDisplayTile.h"
#include "FileManagerActor.generated.h"


USTRUCT(BlueprintType)
struct GCODEVIEWER_API FFileStructureLevel
{
	GENERATED_USTRUCT_BODY()

	/** Min range value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Range)
	TArray<AFileDisplayTile*> tileArray;
	//int type;

};

UCLASS()
class GCODEVIEWER_API AFileManagerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFileManagerActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	/** Current depth in the file tree (0 = drives) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FileExplorer)
	int currentLevel = 0;

	float spawnRadius = 450;
	int maxTilesPerCircumference = 16;

	/** Maximum model scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FileExplorer)
	TArray<FFileStructureLevel> levelStructures;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Drive Paths", Keywords = "Array String Drive Paths"), Category = "MyNodes|FileManagement")
	static TArray<FString> GetDrivePaths();

	//bool GetFiles(const FString& FullPathOfBaseDir, TArray<FString>& FilenamesOut, bool Recursive = false, const FString& FilterByExtension = "");
	bool GetDirectories(const FString& FullPathOfBaseDir, TArray<FString>& DirsOut, bool Recursive = false, const FString& ContainsStr = "");

	void downDirectoryLevel(FString folderName);
	void upDirectoryLevel();
	
};

template <class FunctorType>
class PlatformFileFunctor : public IPlatformFile::FDirectoryVisitor	//GenericPlatformFile.h
{
public:

	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
	{
		return Functor(FilenameOrDirectory, bIsDirectory);
	}

	PlatformFileFunctor(FunctorType&& FunctorInstance)
		: Functor(MoveTemp(FunctorInstance))
	{
	}

private:
	FunctorType Functor;
};

template <class Functor>
PlatformFileFunctor<Functor> MakeDirectoryVisitor(Functor&& FunctorInstance)
{
	return PlatformFileFunctor<Functor>(MoveTemp(FunctorInstance));
}