// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ImportedObject.h"
#include "modelViewer.generated.h"



typedef union floatdata {
	float f;
	unsigned long byteData;
} FloatUnionData;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EModelFileType : uint8
{
	PE_STL 	UMETA(DisplayName = "STL File"),
	PE_GCODE 	UMETA(DisplayName = "GCODE File"),
	PE_STEP	UMETA(DisplayName = "STEP File")
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class ESTLFileType : uint8
{
	PE_BINARY 	UMETA(DisplayName = "Binary STL"),
	PE_ASCII 	UMETA(DisplayName = "ASCII STL")
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EGcodePrintMoveEnum : uint8
{
	PE_Travel 	UMETA(DisplayName = "Travel"),
	PE_Perimeter 	UMETA(DisplayName = "Perimeter"),
	PE_Infill	UMETA(DisplayName = "Infill"),
	PE_Support	UMETA(DisplayName = "Support")
};

USTRUCT(BlueprintType)
struct GCODEVIEWER_API FPrintMove
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
	EGcodePrintMoveEnum moveType;

#if CPP
	FPrintMove() :
		endPoint(0.f, 0.f, 0.f), moveType(EGcodePrintMoveEnum::PE_Travel), layerChange(false)
	{}
#endif


};

UCLASS()
class GCODEVIEWER_API AmodelViewer : public AImportedObject
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AmodelViewer();

	//------------- File Loading --------------------------//

	/** File has been read in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loading)
	bool fileLoaded = false;

	/** File has been parsed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loading)
	bool fileParsed = false;

	/** Model has been constructed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loading)
	bool modelConstructed = false;

	/** Progress made in loading file */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loading)
	float loadingProgress = 0.0;

	/** Type of file to load */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = File)
	EModelFileType fileType;

	/** If STL, Binary or ASCII */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = STL)
	ESTLFileType stlType;

	/** String array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = File)
	TArray<FString> fileStringArray;

	/** Binary array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = File)
	TArray<uint8> fileBinaryArray;

	/** Binary array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = File)
	TArray<FVector> stlNormalArray;

	/** Binary array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = File)
	TArray<FVector> stlVertexArray;

	/** Binary array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = File)
	TArray<int> stlTriangleArray;

	/** Binary array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = STL)
	int stlTriangleIndex = 0;

	/** Binary array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = STL)
	int stlTriangleCount;

	/** Binary array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GCODE)
	TArray<FPrintMove> gcodePrintMoveArray;

	/** Binary array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GCODE)
	TArray<FTransform> gcodeTransformArray;

	/** Binary array of file contents */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GCODE)
	TArray<UInstancedStaticMeshComponent*> gcodePerimeterInstances;

	/** Binary array of file contents */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GCODE)
	TArray<UInstancedStaticMeshComponent*> gcodeTravelInstances;

	/** Binary array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GCODE)
	UProceduralMeshComponent* modelProceduralMesh;
	

	/** SM Asset for gcode extrusion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GCODE)
	UStaticMesh* AssetSM_PrintExtrusionLine;

	/** Material Asset for gcode extrusion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GCODE)
	UMaterialInterface* AssetM_ExtrusionMaterialDefault;

	/** Material Asset for gcode extrusion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GCODE)
	UMaterialInterface* AssetM_ExtrusionMaterialActive;

	/** Material Asset for STL Models */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GCODE)
	UMaterialInterface* AssetM_ModelMaterial;

	/** Binary array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GCODE)
	int gcodeLayerRenderIndex;

	int gcodeLayerRenderIndexPrev;

	int gcodeExtrusionRenderIndex = 0;
	int gcodeExtrusionRenderIndexPrev = 0;

	int gcodeExtrusionTotalCount = 0;

	bool subScrubbingEnabled = false;

	/** Binary array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GCODE)
	int gcodeLayerTotalCount;

	/** Binary array of file contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GCODE)
	FVector currentExtrusionScrubLocation;

	/** Is selected by user? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interactivity)
	bool selected = true;

	/** Is selected by user? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loading)
	float loadRateModifier = 1.0;

	/** Time loading started */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loading)
	double loadingStartedMillis;

	int parsingIndex = 0;
	FVector gcodeLastPoint = FVector(0, 0, 0);

	FVector maxModelBounds = FVector(0,0,0);
	FVector secondMaxModelBounds = FVector(0,0,0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Model)
	UBoxComponent* CollisionBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Model)
	USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loading)
	UMaterialInstance* loadingMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loading)
	UMaterial* loadingMaterialParent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loading)
	UStaticMeshComponent* LoadingBox;

	UMaterialInstanceDynamic* MaterialInst;

	void updatePercentageProgress();
	 

	bool loadFile();

	bool loadGcodeFile();
	bool loadStlFile();
	bool loadStepFile();

	bool parseFile();

	bool parseBinaryStlFile();
	bool parseAsciiStlFile();

	bool parseGcodeFile();
	bool parseStlFile();
	bool parseStepFile();

	bool constructModel();

	bool constructGcodeModel();
	bool constructStlModel();
	bool constructStepModel();
	
	void updateModel();
	void updateGcodeModel();
	void scrubThroughExtrusion();
	void updateStlModel();

	FTransform extrudeTransform(const FVector startPoint, const FVector endPoint, const FVector modelCentre);

	FPrintMove gcodeParseG1Line(FString & gcodeLine);
	float gcodeGetAxisCoordFromLine(FString & gcodeLine, const FString axisPrefix, bool& foundAxis);
	void gcodeNewInstanceLayer(TArray<UInstancedStaticMeshComponent*> & addToArray);

	bool stlFileIsAscii(TArray<FString> & fileStringArray);
	bool readTextFile();
	bool readBinaryFile();
	int stlGetTriangleCount();

	void updateLoadingRate(float DeltaTime);

	float getFloat32FromByteArray(const TArray<uint8> & binaryData, const int startIndex);


	//---------------------- Model Manipulation ---------------//

	/** Minimum model scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Model)
	float minSize = 0.01;

	/** Maximum model scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Model)
	float maxSize = 1000;

	/** Adjusts speed of operations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Model)
	bool doFaster;

	/** Centre coordinate of model */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Model)
	FVector modelCentreOffset;

	UFUNCTION(BlueprintCallable, Category = "Input")
	void resetScaleAndOrientation();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void adjustOperationSpeed();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void scaleModel(float val);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void rotateModel(float pitch, float yaw);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void jogGcodeLayers(int layersToJog);



	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	//virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	
	
};
