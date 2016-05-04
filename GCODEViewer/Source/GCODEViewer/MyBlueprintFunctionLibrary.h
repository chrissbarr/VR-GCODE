// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyBlueprintFunctionLibrary.generated.h"

/**
 * 
 */

typedef union floatdata {
	float f;
	unsigned long byteData;
} FloatUnionData;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EPrintMoveEnum : uint8
{
	PE_Travel 	UMETA(DisplayName = "Travel"),
	PE_Perimeter 	UMETA(DisplayName = "Perimeter"),
	PE_Infill	UMETA(DisplayName = "Infill"),
	PE_Support	UMETA(DisplayName = "Support")
};
//
//USTRUCT()
//struct PrintMoveStruct
//{
//	GENERATED_USTRUCT_BODY()
//
//	//Always make USTRUCT variables into UPROPERTY()
//	//    any non-UPROPERTY() struct vars are not replicated
//
//	// So to simplify your life for later debugging, always use UPROPERTY()
//	UPROPERTY()
//	FVector EndPoint;
//
//	UPROPERTY()
//	EPrintMoveEnum moveType;
//
//}; 

USTRUCT(BlueprintType)
struct GCODEVIEWER_API FPrintMoveStructCPP
{
	GENERATED_USTRUCT_BODY()

	/** Min range value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Range)
	FVector endPoint;

	/** Min range value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Range)
	bool layerChange;

	/** Max range value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Range)
	EPrintMoveEnum moveType;

#if CPP
	FPrintMoveStructCPP() :
		endPoint(0.f,0.f,0.f), moveType(EPrintMoveEnum::PE_Travel), layerChange(false)
	{}
#endif


};

UCLASS()
class GCODEVIEWER_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enum)
	EPrintMoveEnum PrintMoveEnum;

	UFUNCTION(BlueprintCallable, Category="GCODE")
	static float getAxisCoordinateCPP(const FString gcodeLine, const FString axisPrefix, bool& foundAxis);

	UFUNCTION(BlueprintCallable, Category = "GCODE")
	static TArray<FPrintMoveStructCPP> parseGcode(const TArray<FString> gcodeArray, const int prevIndex, const int linesPerTick, FVector& lastPoint, TArray<FPrintMoveStructCPP> moveStruct, int& newIndex, bool& completed, float& percentageProgress);
	
	UFUNCTION(BlueprintCallable, Category = "GCODE")
	static FVector getNewCoordinate(const FString gcodeLine, const FVector lastPoint, bool & success);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCODE")
	static FTransform extrudeTransform(const FVector startPoint, const FVector endPoint, const FVector modelCentre);

	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCODE")
	//static int hmdType();
	//

	//
	UFUNCTION(BlueprintCallable, Category = "STL")
	static bool loadBinarySTL(const FString& FullFilePath, TArray<uint8>& binaryData, int & triangleCount);

	//
	UFUNCTION(BlueprintCallable, Category = "STL")
	static bool parseBinarySTL(const TArray<uint8> binaryData, const int startingIndex, const int linesToParse, TArray<FVector> &vertexArray, TArray<FVector> & normalArray, int & newIndex);

	UFUNCTION(BlueprintCallable, Category = "STL")
	static int getInt32FromByteArray(const TArray<uint8> binaryData, const int startIndex);


	UFUNCTION(BlueprintCallable, Category = "STL")
	static float getFloat32FromByteArray(const TArray<uint8> binaryData, const int startIndex);
	

};


