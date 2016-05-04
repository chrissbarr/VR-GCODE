// Fill out your copyright notice in the Description page of Project Settings.

#include "GCODEViewer.h"
#include "MyBlueprintFunctionLibrary.h"


TArray<FPrintMoveStructCPP> UMyBlueprintFunctionLibrary::parseGcode(const TArray<FString> gcodeArray, const int prevIndex, const int linesPerTick, FVector& lastPoint, TArray<FPrintMoveStructCPP> moveStruct, int& newIndex, bool& completed, float& percentageProgress) {
	int index = prevIndex;

	if (index < gcodeArray.Num()) {
		for (int i = index; i < index + FMath::Min(linesPerTick,gcodeArray.Num()-index); i++) {
			//UE_LOG(LogTemp, Warning, TEXT("Parsing line %d a"), i);
			FString line = gcodeArray[i];

			bool newCoordinateExists;

			FVector newCoordVector = getNewCoordinate(line, lastPoint, newCoordinateExists);

			if (newCoordinateExists) {
				//UE_LOG(LogTemp, Warning, TEXT("Coordinate %f,%f,%f"), newCoordVector.X,newCoordVector.Y,newCoordVector.Z);
				lastPoint = newCoordVector;

				EPrintMoveEnum newMoveType;

				if (!line.Contains("E", ESearchCase::CaseSensitive, ESearchDir::FromStart)) {
					newMoveType = EPrintMoveEnum::PE_Travel;
					//UE_LOG(LogTemp, Warning, TEXT("line %d is a travel move"), i);
				}
				else 
				{
					newMoveType = EPrintMoveEnum::PE_Perimeter;
				}

				FPrintMoveStructCPP newMove;

				newMove.endPoint = newCoordVector;
				newMove.moveType = newMoveType;
				newMove.layerChange = line.Contains("Z", ESearchCase::CaseSensitive, ESearchDir::FromStart);

				moveStruct.Add(newMove);
			}
		}
		newIndex = index + linesPerTick;

		percentageProgress = (float) ((float) newIndex / (float) gcodeArray.Num()) * 100;
		UE_LOG(LogTemp, Warning, TEXT("progress %f"), percentageProgress);
		completed = false;
	}
	else {
		//UE_LOG(LogTemp, Warning, TEXT("Parsing completed"));
		completed = true;
		newIndex = index;
	}
	return moveStruct;
}

FVector UMyBlueprintFunctionLibrary::getNewCoordinate(const FString gcodeLine, const FVector lastPoint, bool & success) {
	FVector newPoint;
	
	if (gcodeLine.StartsWith("G1", ESearchCase::IgnoreCase)) {
		float xCoord, yCoord, zCoord;
		bool xFound, yFound, zFound;

		xCoord = getAxisCoordinateCPP(gcodeLine, "X", xFound);
		yCoord = getAxisCoordinateCPP(gcodeLine, "Y", yFound);
		zCoord = getAxisCoordinateCPP(gcodeLine, "Z", zFound);

		if (!xFound) { xCoord = lastPoint.X; }
		if (!yFound) { yCoord = lastPoint.Y; }
		if (!zFound) { zCoord = lastPoint.Z; }

		success = true;
		newPoint.Set(xCoord, yCoord, zCoord);

		return newPoint;
	}
	else {
		success = false;
		newPoint.Set(0, 0, 0);
		return newPoint;
	}
}

float UMyBlueprintFunctionLibrary::getAxisCoordinateCPP(const FString gcodeLine, const FString axisPrefix, bool& foundAxis) {
	int axisIndex = gcodeLine.Find(axisPrefix, ESearchCase::IgnoreCase, ESearchDir::FromStart, -1);
	float axisCoordinate = 0;

	if (axisIndex > 0) {
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "found axis letter");
		//UE_LOG(LogTemp, Warning, TEXT("found axis letter %s"), *axisPrefix);
		FString tempCoordString = "";

		for (int i = axisIndex+1; i < axisIndex + 6; i++) {
			FString substring = gcodeLine.Mid(i, 1);

			if (substring.Compare(" ", ESearchCase::IgnoreCase) != 0) {
				tempCoordString.Append(substring);
			}
			else {
				break;
			}
		}

		//UE_LOG(LogTemp, Warning, TEXT("coordinate %s"), *tempCoordString);
		axisCoordinate = FCString::Atof(*tempCoordString);
		foundAxis = true;
		return axisCoordinate;
	}
	else {
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "no axis letter");
		//UE_LOG(LogTemp, Warning, TEXT("no axis letter %s"), *axisPrefix);

		foundAxis = false;
		return axisCoordinate;
	}
}

FTransform UMyBlueprintFunctionLibrary::extrudeTransform(const FVector startPoint, const FVector endPoint, const FVector modelCentre) {
	
	FRotator lineRotation = FRotationMatrix::MakeFromX(endPoint - startPoint).Rotator();

	FTransform outTransform;

	FVector lineLocation = ((endPoint + startPoint).operator*(.5)).operator-(modelCentre);

	FVector lineScale;

	lineScale.Y = 0.004;
	lineScale.Z = 0.004;

	lineScale.X = ((startPoint.operator-(endPoint)).Size())/100;

	lineLocation.X = -lineLocation.X;
	//lineLocation.Y = -lineLocation.Y;

	//lineRotation.Yaw += 3.14159/2;


	outTransform.SetLocation(lineLocation);
	outTransform.SetRotation(lineRotation.Quaternion());
	outTransform.SetScale3D(lineScale);

	return outTransform;
}


bool UMyBlueprintFunctionLibrary::loadBinarySTL(
	const FString& FullFilePath,
	TArray<uint8> & binaryData,
	int & triangleCount
) {

	UE_LOG(LogTemp, Warning, TEXT("Loading Binary STL %s"), *FullFilePath);

	TArray<uint8> TheBinaryArray;

	
	if (!FFileHelper::LoadFileToArray(TheBinaryArray, *FullFilePath))
	{
		//ClientMessage("FFILEHELPER:>> Invalid File");
		UE_LOG(LogTemp, Warning, TEXT("Invalid File!"));
		return false;
		//~~
	}

	//File Load Error
	if (TheBinaryArray.Num() <= 0) {
		UE_LOG(LogTemp, Warning, TEXT("Empty File!"));
		return false;
	}

	binaryData = TheBinaryArray;

	UE_LOG(LogTemp, Warning, TEXT("Binary STL Loaded"));

	//Read triangle count from header
	int index = 80;

	triangleCount = binaryData[index] | (uint32_t)binaryData[index + 1] << 8 | ((uint32_t)binaryData[index + 2] << 16 | (uint32_t)binaryData[index + 3] << 24);
	UE_LOG(LogTemp, Warning, TEXT("Triangle Count: %d"), triangleCount);

	return true;
}

bool UMyBlueprintFunctionLibrary::parseBinarySTL(const TArray<uint8> binaryData, const int startingIndex, const int verticesToParse, TArray<FVector> &vertexArray, TArray<FVector> & normalArray, int & newIndex) {
	int index = startingIndex;
	int i;
	int arrayIndex = 0;
	bool finishedData = false;

	//for (int i = index; i < 80; i++) {
		//UE_LOG(LogTemp, Warning, TEXT("%d"), binaryData[i]);
	//}
	

	

	while (index < FMath::Min(binaryData.Num(), startingIndex + (verticesToParse*50))) {

		//UE_LOG(LogTemp, Warning, TEXT("Calculating Normal"));
		normalArray.Add(FVector(getFloat32FromByteArray(binaryData, index), getFloat32FromByteArray(binaryData, index + 4), getFloat32FromByteArray(binaryData, index + 8)));
		//UE_LOG(LogTemp, Warning, TEXT("Normal Calculated"));
		normalArray.Add(FVector(normalArray[arrayIndex]));
		normalArray.Add(FVector(normalArray[arrayIndex]));
		//UE_LOG(LogTemp, Warning, TEXT("Normal Copied"));
		
		index += 12;
		
		for (i = 0; i < 3; i++) {

			vertexArray.Add(FVector(getFloat32FromByteArray(binaryData, index + 8), getFloat32FromByteArray(binaryData, index + 4), getFloat32FromByteArray(binaryData, index)));
			//UE_LOG(LogTemp, Warning, TEXT("Vertex Calculated"));
			index += 12;
			arrayIndex += 1;
		}

		index += 2;

	}

	if (index >= binaryData.Num()-1) {
		finishedData = true;
		//UE_LOG(LogTemp, Warning, TEXT("Binary File Exhausted"));
	}

	newIndex = index;

	return finishedData;
}

float UMyBlueprintFunctionLibrary::getFloat32FromByteArray(const TArray<uint8> binaryData, const int startIndex) {
	static FloatUnionData tempFloat;

	tempFloat.byteData = getInt32FromByteArray(binaryData, startIndex);

	return tempFloat.f;

	//return 0.0f;
}

int UMyBlueprintFunctionLibrary::getInt32FromByteArray(const TArray<uint8> binaryData, const int startIndex) {
	return binaryData[startIndex] | (uint32_t)binaryData[startIndex + 1] << 8 | ((uint32_t)binaryData[startIndex + 2] << 16 | (uint32_t)binaryData[startIndex + 3] << 24);
}

//int UMyBlueprintFunctionLibrary::hmdType() {
//	if (GEngine->HMDDevice.IsValid() && GEngine->HMDDevice->IsHMDEnabled())
//	{
//		if (GEngine->HMDDevice->GetHMDDeviceType() == EHMDDeviceType::DT_SteamVR)
//		{
//			return 1;
//		}
//	}
//}
