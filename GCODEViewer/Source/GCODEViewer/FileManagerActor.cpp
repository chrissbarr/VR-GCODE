// Fill out your copyright notice in the Description page of Project Settings.

#include "GCODEViewer.h"
#include "FileManagerActor.h"


// Sets default values
AFileManagerActor::AFileManagerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFileManagerActor::BeginPlay()
{
	Super::BeginPlay();

	TArray<FString> currentDirectory = GetDrivePaths();
	FString totalPath;

	for (int j = 0; j < 10; j++) {

		UE_LOG(LogTemp, Warning, TEXT("Folders in directory:"));
		for (int i = 0; i < currentDirectory.Num(); i++) {
			UE_LOG(LogTemp, Warning, TEXT("                       %s"), *currentDirectory[i]);
		}

		FFileStructureLevel currentLevel;



		for (int i = 0; i < currentDirectory.Num(); i++) {
			float rotationalPosition = (float)i * 360 / FMath::Min(currentDirectory.Num(), maxTilesPerCircumference);
			FVector spawnLocation = FVector(GetActorLocation().X + spawnRadius * FMath::Cos(FMath::DegreesToRadians(rotationalPosition)), GetActorLocation().Y + spawnRadius * FMath::Sin(FMath::DegreesToRadians(rotationalPosition)), 0-(j*100 + FMath::Floor(currentLevel.tileArray.Num()/ maxTilesPerCircumference)*35));
			FRotator spawnRotator = FRotationMatrix::MakeFromX(FVector(GetActorLocation().X, GetActorLocation().Y,0) - FVector(spawnLocation.X,spawnLocation.Y,0)).Rotator();
			AFileDisplayTile* tile = (AFileDisplayTile*)GetWorld()->SpawnActor<AFileDisplayTile>(spawnLocation, spawnRotator);
			tile->fileName = currentDirectory[i];
			tile->filePath = FString(totalPath).Append("\\").Append(currentDirectory[i]);;
			tile->parentFileManager = this;
			tile->update();
			tile->folderTreeLevel = j;
			tile->folderTreeIndex = i;

			currentLevel.tileArray.Add(tile);

		}
		
		levelStructures.Add(currentLevel);

		//choose a random directory
		int randInt = FMath::RandRange(0, currentDirectory.Num() - 1);

		if (currentDirectory.Num() > 0) {
			FString nextDirectory = currentDirectory[randInt];

			UE_LOG(LogTemp, Warning, TEXT("Randomly selected directory: %s"), *nextDirectory);

			totalPath.Append(nextDirectory);

			if (j != 0) {
				totalPath.Append("\\");
			}

			UE_LOG(LogTemp, Warning, TEXT("New total path: %s"), *totalPath);

			currentDirectory.Empty();
			GetDirectories(totalPath, currentDirectory, false, "");

			//UE_LOG(LogTemp, Warning, TEXT("New folders in path: %s"), *currentDirectory);
		}
		
	}




	
}

// Called every frame
void AFileManagerActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

/* folder navigation is composed of moving up or down
the user can select a folder several folders up, in which case
we would need to move up until we're at the right level,
then down into the selected folder */

void AFileManagerActor::downDirectoryLevel(FString folderName) {
	//levelStructures[currentLevel].tileArray
}

void AFileManagerActor::upDirectoryLevel() {

}


TArray<FString> AFileManagerActor::GetDrivePaths()
{
	TArray<FString> DrivePaths;
	uint32 CharCount = 0;
	WCHAR DeviceName[MAX_PATH] = L"";
	uint32 Error = ERROR_SUCCESS;
	HANDLE FindHandle = INVALID_HANDLE_VALUE;
	BOOL Found = false;
	size_t Index = 0;
	BOOL Success = false;
	WCHAR VolumeName[MAX_PATH] = TEXT("");

	// Enumerate all volumes in the system
	FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

	if (FindHandle == INVALID_HANDLE_VALUE)
	{
		Error = GetLastError();
		// wprintf(L"FindFirstVolumeW failed with error code %d", Error);
		DrivePaths.Empty();
		return DrivePaths;
	}

	do
	{
		// Skip the \\?\\ prefix and remove the trailing backslash
		Index = wcslen(VolumeName) - 1;
		if (VolumeName[0] != L'\\' ||
			VolumeName[1] != L'\\' ||
			VolumeName[2] != L'?' ||
			VolumeName[3] != L'\\' ||
			VolumeName[Index] != L'\\')
		{
			Error = ERROR_BAD_PATHNAME;
			//  wprintf(L"FindFirsVolumeW/FindNextVolumeW returned a bad path: %s\n", VolumeName);
			break;
		}

		// QueryDosDeviceW does not allow a trailing backslash, so temporarily remove it.
		VolumeName[Index] = L'\0';

		CharCount = QueryDosDeviceW(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName));

		VolumeName[Index] = L'\\';

		if (CharCount == 0)
		{
			Error = GetLastError();
			// wprintf(L"QueryDosDeviceW failed with error code %d\n", Error);
			break;
		}

		/*
		wprintf(L"\nFound a device: %s", DeviceName);
		wprintf(L"\nVolume name: %s", VolumeName);
		wprintf(L"\nPaths: ");
		*/

		uint32 chCount = MAX_PATH + 1;
		PWCHAR Names = NULL;
		PWCHAR NameIdx = NULL;
		BOOL bSuccess = false;

		for (;;)
		{
			// Allocate a buffer to hold the paths
			Names = (PWCHAR) new BYTE[chCount * sizeof(WCHAR)];

			if (!Names) break; // If memory can't be allocated, break

							   // Obtain all of the paths for this volume
			bSuccess = GetVolumePathNamesForVolumeNameW(VolumeName,
				Names,
				chCount,
				(PDWORD)&chCount);

			if (bSuccess) break;

			if (GetLastError() != ERROR_MORE_DATA) break;

			delete[] Names;
			Names = NULL;
		}

		if (bSuccess)
		{
			// "Display" the various paths
			for (NameIdx = Names;
				NameIdx[0] != L'\0';
				NameIdx += wcslen(NameIdx) + 1)
			{
				DrivePaths.Add(NameIdx);
			}
		}

		if (Names != NULL)
		{
			delete[] Names;
			Names = NULL;
		}

		// Move on to the next Volume
		Success = FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName));

		if (!Success)
		{
			Error = GetLastError();
			if (Error != ERROR_NO_MORE_FILES)
			{
				// wprintf(L"FindNextVolumeW failed with error code %d\n", Error);
				break;
			}

			// Finished iterating through all the volumes
			Error = ERROR_SUCCESS;
			break;
		}

	} while (Error != ERROR_NO_MORE_FILES);

	FindVolumeClose(FindHandle);
	FindHandle = INVALID_HANDLE_VALUE;

	DrivePaths.Sort();
	return DrivePaths;
 }


 ////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ////  Victory Get All Files
 ////      Optional File Extension Filter!!!  by Rama
 ////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 //bool AFileManagerActor::GetFiles(const FString& FullPathOfBaseDir, TArray<FString>& FilenamesOut, bool Recursive = false, const FString& FilterByExtension = "")
 //{
	// //Format File Extension, remove the "." if present
	// const FString FileExt = FilterByExtension.Replace(TEXT("."), TEXT("")).ToLower();

	// FString Str;
	// auto FilenamesVisitor = MakeDirectoryVisitor(
	//	 [&](const TCHAR* FilenameOrDirectory, bool bIsDirectory)
	// {
	//	 //Files
	//	 if (!bIsDirectory)
	//	 {
	//		 //Filter by Extension
	//		 if (FileExt != "")
	//		 {
	//			 Str = FPaths::GetCleanFilename(FilenameOrDirectory);

	//			 //Filter by Extension
	//			 if (FPaths::GetExtension(Str).ToLower() == FileExt)
	//			 {
	//				 if (Recursive)
	//				 {
	//					 FilenamesOut.Push(FilenameOrDirectory); //need whole path for recursive
	//				 }
	//				 else
	//				 {
	//					 FilenamesOut.Push(Str);
	//				 }
	//			 }
	//		 }

	//		 //Include All Filenames!
	//		 else
	//		 {
	//			 //Just the Directory
	//			 Str = FPaths::GetCleanFilename(FilenameOrDirectory);

	//			 if (Recursive)
	//			 {
	//				 FilenamesOut.Push(FilenameOrDirectory); //need whole path for recursive
	//			 }
	//			 else
	//			 {
	//				 FilenamesOut.Push(Str);
	//			 }
	//		 }
	//	 }
	//	 return true;
	// }
	// );
	// if (Recursive)
	// {
	//	 return FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*FullPathOfBaseDir, FilenamesVisitor);
	// }
	// else
	// {
	//	 return FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*FullPathOfBaseDir, FilenamesVisitor);
	// }
 //}

 //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 //  Victory Get Directories
 //      Optional Search SubString!!!  by Rama
 //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 //Get Directories
 bool AFileManagerActor::GetDirectories(const FString& FullPathOfBaseDir, TArray<FString>& DirsOut, bool Recursive, const FString& ContainsStr)
 {
	 FString Str;
	 auto FilenamesVisitor = MakeDirectoryVisitor(
		 [&](const TCHAR* FilenameOrDirectory, bool bIsDirectory)
	 {
		 if (bIsDirectory)
		 {
			 //Using a Contains Filter?
			 if (ContainsStr != "")
			 {
				 Str = FPaths::GetCleanFilename(FilenameOrDirectory);

				 //Only if Directory Contains Str
				 if (Str.Contains(ContainsStr))
				 {
					 if (Recursive) DirsOut.Push(FilenameOrDirectory); //need whole path for recursive
					 else DirsOut.Push(Str);
				 }
			 }

			 //Get ALL Directories!
			 else
			 {
				 //Just the Directory
				 Str = FPaths::GetCleanFilename(FilenameOrDirectory);

				 if (Recursive) DirsOut.Push(FilenameOrDirectory); //need whole path for recursive
				 else DirsOut.Push(Str);
			 }
		 }
		 return true;
	 }
	 );
	 if (Recursive)
	 {
		 return FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*FullPathOfBaseDir, FilenamesVisitor);
	 }
	 else
	 {
		 return FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*FullPathOfBaseDir, FilenamesVisitor);
	 }
 }
