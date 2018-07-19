// Fill out your copyright notice in the Description page of Project Settings.

#include "GCODEViewer.h"
#include "modelViewer.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "ImportedObject.h"


// Sets default values
AmodelViewer::AmodelViewer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//InputComponent->BindAction("ResetModel", IE_Pressed, this, &AmodelViewer::resetScaleAndOrientation);
	UE_LOG(LogTemp, Warning, TEXT("Model spawned"));

	// Our root component will be a sphere that reacts to physics
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(2.0f);
	SphereComponent->SetVisibility(true, false);
	SphereComponent->SetHiddenInGame(false);
	SphereComponent->SetCollisionProfileName(TEXT("Custom"));
	SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComponent->SetSimulatePhysics(true);
	SphereComponent->WakeRigidBody();
	SphereComponent->SetEnableGravity(false);
	SphereComponent->SetLinearDamping(1.0);
	SphereComponent->SetAngularDamping(10.0);


	CollisionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBounds"));
	//CollisionBounds->SetupAttachment(RootComponent);
	CollisionBounds->RegisterComponent();
	CollisionBounds->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	CollisionBounds->SetBoxExtent(FVector(25, 10, 10));
	CollisionBounds->SetVisibility(true, false);
	CollisionBounds->SetHiddenInGame(true);
	CollisionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBounds->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBounds->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBounds->SetCollisionObjectType(ECollisionChannel::ECC_Visibility);
	CollisionBounds->CanCharacterStepUpOn = ECB_No;
	
	modelProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));

	modelProceduralMesh->RegisterComponent();
	modelProceduralMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform); //SetupAttachment(CollisionBounds);
}

// Called when the game starts or when spawned
void AmodelViewer::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()->GetPawn()->GetController());

	if (PlayerController) {
		EnableInput(PlayerController);

		//InputComponent->BindAction("ResetModel", IE_Pressed, this, &AmodelViewer::resetScaleAndOrientation);
		//InputComponent->BindAction("SpeedModifier",IE_Pressed, this, &AmodelViewer::adjustOperationSpeed);
		//InputComponent->BindAction("SpeedModifier", IE_Released, this, &AmodelViewer::adjustOperationSpeed);
		//InputComponent->BindAxis("Scale", this, &AmodelViewer::scaleModel);
	}

	EInputConsumeOptions::ICO_ConsumeNone;
	//EnableInput(GetWorld()->GetFirstPlayerController());

	
}

// Called every frame
void AmodelViewer::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	
	
	updatePercentageProgress();
	updateModel();

	

	if (!fileLoaded) {
		fileLoaded = loadFile();
	}
	else if (fileLoaded && !fileParsed) {
		fileParsed = parseFile();
		updateLoadingRate(DeltaTime);
		if (MaterialInst) {
			MaterialInst->SetScalarParameterValue("Alpha", loadingProgress / 100);
		}

	}
	else if (fileLoaded && fileParsed && !modelConstructed) {
		if (MaterialInst) {
			MaterialInst->SetScalarParameterValue("Alpha", 0);
		}
		modelConstructed = constructModel();
		updateLoadingRate(DeltaTime);
	}


}

void AmodelViewer::resetScaleAndOrientation() {
	Super::resetScale();
	Super::resetOrientation();
	UE_LOG(LogTemp, Warning, TEXT("Model scale and rotation reset."));

	// TODO : reset vertical position to maintain bed offset on reset
}

void AmodelViewer::adjustOperationSpeed() {
	if (!doFaster) {
		doFaster = true;
		UE_LOG(LogTemp, Warning, TEXT("Do Faster!"));
	}
	else if (doFaster) {
		doFaster = false;
		UE_LOG(LogTemp, Warning, TEXT("Don't do Faster!"));
	}
}

void AmodelViewer::scaleModel(float val) {
	FVector currentScale = GetActorScale3D();
	FVector oldScale = FVector(currentScale);

	currentScale = currentScale.operator+(currentScale.operator/(200).operator*((val + (FMath::Sign(val) * 5 * (int) doFaster)))).GetClampedToSize(0.1, 10);

	if (currentScale != GetActorScale3D()) {
		UE_LOG(LogTemp, Warning, TEXT("Model scale adjusted."));
		SetActorScale3D(currentScale);
	}	

	

	//fix base of gcode files in place (so model scales 'upwards')
	if (fileType == EModelFileType::PE_GCODE) {
		float scaleDelta = oldScale.Z - GetActorScale3D().Z;
		if (FMath::Abs(scaleDelta) > 0.001) {
			AddActorWorldOffset(FVector(0, 0, -scaleDelta * modelCentreOffset.Z));
		}
	}


}

void AmodelViewer::rotateModel(float pitch, float yaw) {
	if (pitch != 0 || yaw != 0) {
		UE_LOG(LogTemp, Warning, TEXT("Model rotated."));
	}
	float mul = 1 + (FMath::Sign(FMath::Max(FMath::Abs(pitch), FMath::Abs(yaw)) * 1 * (int)doFaster));
	AddActorLocalRotation(FRotator(pitch*mul,0, yaw*mul), false, NULL, ETeleportType::None);
}

void AmodelViewer::jogGcodeLayers(int layersToJog) {
	//gcodeLayerRenderIndex = FMath::Clamp(gcodeLayerRenderIndex + (layersToJog + (FMath::Sign(layersToJog) * 4 * (int)doFaster)), 0, gcodeLayerTotalCount);
	int jogBy = 1;

	if (doFaster) {
		jogBy = 100;
	}


	gcodeExtrusionRenderIndex = FMath::Clamp(gcodeExtrusionRenderIndex + layersToJog * jogBy,0, gcodeExtrusionTotalCount);

	//if (gcodeExtrusionRenderIndex != gcodeExtrusionRenderIndexPrev) {
	//	UE_LOG(LogTemp, Warning, TEXT("Extrusion render index: %d"), gcodeExtrusionRenderIndex);
	//}

}


void AmodelViewer::updateModel() {

	switch (fileType) {
	case EModelFileType::PE_GCODE:
		updateGcodeModel();
		break;
	case EModelFileType::PE_STL:
		updateStlModel();
		break;
	}

}

void AmodelViewer::updateGcodeModel() {
	if (gcodeExtrusionRenderIndexPrev != gcodeExtrusionRenderIndex && subScrubbingEnabled) {
		scrubThroughExtrusion();
		gcodeExtrusionRenderIndexPrev = gcodeExtrusionRenderIndex;
	}
	//if (gcodeLayerRenderIndexPrev != gcodeLayerRenderIndex) {
	//	for (int i = 0; i < gcodePerimeterInstances.Num(); i++) {
	//		if (i < gcodeLayerRenderIndex) {
	//			gcodePerimeterInstances[i]->SetVisibility(true);
	//		} else {
	//			gcodePerimeterInstances[i]->SetVisibility(false);
	//		}
	//	}
	//}

	//gcodeLayerRenderIndexPrev = gcodeLayerRenderIndex;

	//if (subScrubbingEnabled) {
	//	//int sumTotal = 0;

	//	if (gcodeExtrusionRenderIndexPrev != gcodeExtrusionRenderIndex) {

	//		int currentLayer = 0;
	//		int sumTotal = 0;

	//		for (int i = 0; i < gcodePerimeterInstances.Num(); i++) {
	//			if (gcodeExtrusionRenderIndex < sumTotal + gcodePerimeterInstances[i]->GetInstanceCount()) {
	//				currentLayer = i;
	//				break;
	//			}

	//			sumTotal += gcodePerimeterInstances[i]->GetInstanceCount();
	//		}

	//		//currentLayer

	//		for (int i = FMath::Min(gcodeExtrusionRenderIndexPrev, gcodeExtrusionRenderIndex); i < FMath::Max(gcodeExtrusionRenderIndexPrev, gcodeExtrusionRenderIndex); i++) {

	//			if (sumTotal + i <= gcodeExtrusionRenderIndex) {
	//				gcodePerimeterInstances[currentLayer]->UpdateInstanceTransform(i - sumTotal, gcodeTransformArray[sumTotal], false, true);
	//			}
	//			else {
	//				gcodePerimeterInstances[currentLayer]->UpdateInstanceTransform(i - sumTotal, FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0)), false, true);
	//			}

	//		}


	//		/*for (int i = gcodeExtrusionRenderIndexPrev; i < gcodeExtrusionRenderIndex; i++) {
	//			for (int j = 0; j < gcodePerimeterInstances[i]->GetInstanceCount(); j++) {

	//				if (sumTotal <= gcodeExtrusionRenderIndex) {
	//					gcodePerimeterInstances[i]->UpdateInstanceTransform(j, gcodeTransformArray[sumTotal], false, true);
	//				}
	//				else {
	//					gcodePerimeterInstances[i]->UpdateInstanceTransform(j, FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0)), false, true);
	//				}

	//				sumTotal++;
	//			}

	//		}*/
	//	}

	//	gcodeExtrusionRenderIndexPrev = gcodeExtrusionRenderIndex;
	//}

}

void AmodelViewer::scrubThroughExtrusion() {
	bool forward;

	forward = gcodeExtrusionRenderIndex > gcodeExtrusionRenderIndexPrev;

	int startIndex = FMath::Min(gcodeExtrusionRenderIndex, gcodeExtrusionRenderIndexPrev);
	int stopIndex = FMath::Max(gcodeExtrusionRenderIndex, gcodeExtrusionRenderIndexPrev);

	FString dir;

	if (forward) {	dir = "forward";}
	else {	dir = "backwards";}

	UE_LOG(LogTemp, Warning, TEXT("Scrubbing %s from extrusion %d to extrusion %d"), *dir, startIndex, stopIndex);

	int startingLayer = -1;
	int stoppingLayer = -1;

	int totalPrevLayerCount = 0;

	int startingLayerTotalPrevCount = 0;
	int stoppingLayerTotalPrevCount = 0;
	
	//determine which layer the operation starts and stops on
	for (int i = 0; i < gcodePerimeterInstances.Num(); i++) {
		if (gcodePerimeterInstances[i]->GetInstanceCount() + totalPrevLayerCount > startIndex && startingLayer == -1) {
			startingLayer = i;
			startingLayerTotalPrevCount = totalPrevLayerCount;
		}
		if (gcodePerimeterInstances[i]->GetInstanceCount() + totalPrevLayerCount > stopIndex && stoppingLayer == -1) {
			stoppingLayer = i;
			stoppingLayerTotalPrevCount = totalPrevLayerCount;
		}

		if (startingLayer != -1 && stoppingLayer != -1) { 
			break; 
		}
		else {
			totalPrevLayerCount += gcodePerimeterInstances[i]->GetInstanceCount();	
		}
	}

	if (startingLayer == gcodePerimeterInstances.Num() - 1) {
		stoppingLayer = startingLayer;
	}

	if (startingLayer != -1 && stoppingLayer != -1) {

		UE_LOG(LogTemp, Warning, TEXT("Scrubbing %s from layer %d to layer %d"), *dir, startingLayer, stoppingLayer);

		int startingLayerStartIndex = startIndex - startingLayerTotalPrevCount;
		int startingLayerStopIndex = FMath::Min(stopIndex - startingLayerTotalPrevCount, gcodePerimeterInstances[startingLayer]->GetInstanceCount());

		UE_LOG(LogTemp, Warning, TEXT("min(%d,%d)"), stopIndex - startingLayerTotalPrevCount, gcodePerimeterInstances[startingLayer]->GetInstanceCount());

		UE_LOG(LogTemp, Warning, TEXT("First layer extrusion %d to %d"), startingLayerStartIndex, startingLayerStopIndex);

		int ticks = 0;

		for (int i = startingLayerStartIndex; i < startingLayerStopIndex; i++) {
			
			if (forward) {
				if (startIndex + ticks < gcodeTransformArray.Num()) {
					UE_LOG(LogTemp, Warning, TEXT("first layer, render extrusion %d"), startIndex + ticks);
					gcodePerimeterInstances[startingLayer]->UpdateInstanceTransform(i, gcodeTransformArray[startIndex + ticks], false, true);
				}
			}
			else {
				gcodePerimeterInstances[startingLayer]->UpdateInstanceTransform(i, FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0)), false, true);
			}

			ticks++;
		}

		if (startingLayer != stoppingLayer) {
			int stoppingLayerStartIndex = 0;
			int stoppingLayerStopIndex = stopIndex - stoppingLayerTotalPrevCount;

			for (int i = stoppingLayerStartIndex; i < stoppingLayerStopIndex; i++) {
				if (forward) {
					UE_LOG(LogTemp, Warning, TEXT("second layer, render extrusion %d"), startIndex + ticks);
					gcodePerimeterInstances[stoppingLayer]->UpdateInstanceTransform(i, gcodeTransformArray[startIndex + ticks], false, true);
				}
				else {
					gcodePerimeterInstances[stoppingLayer]->UpdateInstanceTransform(i, FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0)), false, true);
				}

				ticks++;

			}


			//if there are layers between where we started and stopped, set them either fully visible or invisible

			ticks = gcodePerimeterInstances[startingLayer]->GetInstanceCount();

			if (FMath::Abs(startingLayer - stoppingLayer) > 1) {
				for (int i = startingLayer + 1; i < stoppingLayer; i++) {
					for (int j = 0; j < gcodePerimeterInstances[i]->GetInstanceCount(); j++) {
						if (forward) {
							gcodePerimeterInstances[i]->UpdateInstanceTransform(j, gcodeTransformArray[startingLayerTotalPrevCount + ticks], false, true);
						}
						else {
							gcodePerimeterInstances[i]->UpdateInstanceTransform(j, FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0)), false, true);
						}
						ticks++;
					}
				}
			}

		}

		//set layer colours accordingly
		if (forward) {
			//highlight highest layer
			gcodePerimeterInstances[stoppingLayer]->SetMaterial(0, AssetM_ExtrusionMaterialActive);

			//return previous colours to normal
			for (int i = 0; i < FMath::Abs(startingLayer - stoppingLayer); i++) {
				gcodePerimeterInstances[stoppingLayer - (i+1)]->SetMaterial(0, AssetM_ExtrusionMaterialDefault);
			}
		}

		if (!forward) {
			gcodePerimeterInstances[startingLayer]->SetMaterial(0, AssetM_ExtrusionMaterialActive);
		}
		
		//update nozzle position
		currentExtrusionScrubLocation = gcodeTransformArray[FMath::Clamp(gcodeExtrusionRenderIndex,0, gcodeTransformArray.Num()-1)].GetLocation();


		
	}
	else {
		if(startingLayer == -1)
			UE_LOG(LogTemp, Warning, TEXT("Start layer not detected"));
		if(stoppingLayer == -1)
			UE_LOG(LogTemp, Warning, TEXT("Stop layer not detected"));
	}

	


	//gcodeExtrusionRenderIndexPrev != gcodeExtrusionRenderIndex
}

void AmodelViewer::updateStlModel() {

}

void AmodelViewer::updateLoadingRate(float DeltaTime) {
	static float loadRatePrev = 0;
	if ((1 / DeltaTime) < 90) {
		loadRateModifier = FMath::Clamp(loadRateModifier * 0.95, 0.9, 10.0);
		UE_LOG(LogTemp, Warning, TEXT("Minimum FPS reached!"));
	}
	else {
		loadRateModifier = FMath::Clamp(loadRateModifier * 1.05, 0.9, 10.0);
	}

	if (FMath::Abs(loadRateModifier - loadRatePrev) > 0.001) {
		UE_LOG(LogTemp, Warning, TEXT("Loading Rate: %f"), loadRateModifier);
		loadRatePrev = loadRateModifier;
	}


}

void AmodelViewer::updatePercentageProgress() {
	static float progressPrev = 0;

	if (FMath::Abs(progressPrev - loadingProgress) > 0.001) {
		UE_LOG(LogTemp, Warning, TEXT("Loading Progress: %f"), loadingProgress);
	}

	progressPrev = loadingProgress;
}

bool AmodelViewer::constructModel() {
	bool success = false;

	switch (fileType) {
	case EModelFileType::PE_GCODE:
		success = constructGcodeModel();
		break;
	case EModelFileType::PE_STL:
		success = constructStlModel();
		break;
	}

	if (success) {
		UE_LOG(LogTemp, Warning, TEXT("Model succesfully constructed"));
		double finishedTime = FDateTime::Now().GetTimeOfDay().GetTotalMilliseconds();
		UE_LOG(LogTemp, Warning, TEXT("Loading completed at %f"), loadingStartedMillis);
		UE_LOG(LogTemp, Warning, TEXT("Loading completed, total time %f"), finishedTime - loadingStartedMillis);

		//CollisionBounds->InitBoxExtent(FVector(maxModelBounds.X - minModelBounds.X, maxModelBounds.Y - minModelBounds.Y, maxModelBounds.Z - minModelBounds.Z));
		CollisionBounds->SetBoxExtent(FVector(maxModelBounds.X, maxModelBounds.Y, maxModelBounds.Z), true);
		CollisionBounds->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

		UE_LOG(LogTemp, Warning, TEXT("Object bounds calculated as %s"), *maxModelBounds.ToString());
	}


	return success;
}

bool AmodelViewer::constructGcodeModel() {
	int gcodeLinesPerTick = 500;

	//first we need to calculate the centre point
	if (parsingIndex == -1) {
		TArray<FVector> tempArray;
		
		for (int i = 0; i < gcodePrintMoveArray.Num(); i++) {
			tempArray.Add(gcodePrintMoveArray[i].endPoint);
		}

		modelCentreOffset = UKismetMathLibrary::GetVectorArrayAverage(tempArray);

		AddActorWorldOffset(FVector(0, 0, modelCentreOffset.Z));

		//next loop continue past this
		parsingIndex = 0;

		return false;
	}

	//now add the initial instanced mesh components
	if (parsingIndex == 0) {
		gcodeNewInstanceLayer(gcodePerimeterInstances);
		gcodeNewInstanceLayer(gcodeTravelInstances);
		gcodeTravelInstances[0]->SetVisibility(false, true);
		
	}

	int linesPerTick = (int) gcodeLinesPerTick * loadRateModifier;

	for (int i = parsingIndex; i < (parsingIndex + linesPerTick); i++) {
		if (i < gcodePrintMoveArray.Num()) {

			//if new layer, create a new instanced static mesh component
			if (gcodePrintMoveArray[i].layerChange) {
				gcodeNewInstanceLayer(gcodePerimeterInstances);
				gcodeLayerRenderIndex++;
				gcodeLayerTotalCount++;
			}

			FVector startPoint, endPoint;
			if (i > 0) {
				startPoint = gcodePrintMoveArray[i - 1].endPoint;
			}
			else { startPoint = FVector(0, 0, 0); }

			endPoint = gcodePrintMoveArray[i].endPoint;

			FTransform spawnTransform = extrudeTransform(startPoint, endPoint, modelCentreOffset);

			currentExtrusionScrubLocation = spawnTransform.GetLocation();
			

			if (gcodePrintMoveArray[i].moveType == EGcodePrintMoveEnum::PE_Perimeter) {
				gcodePerimeterInstances.Last()->AddInstance(spawnTransform);
				gcodeExtrusionTotalCount++;
				gcodeExtrusionRenderIndex++;
				gcodeExtrusionRenderIndexPrev = gcodeExtrusionRenderIndex;
				gcodeTransformArray.Add(spawnTransform);

				if (i > 3) {
					for (int j = 0; j < 3; j++) {
						if (FMath::Abs((endPoint- modelCentreOffset)[j]) > FMath::Abs(maxModelBounds[j])) {
							secondMaxModelBounds[j] = maxModelBounds[j];
							maxModelBounds[j] = FMath::Abs((endPoint - modelCentreOffset)[j]);
						}
					}
				}

				
			}
			else {
				gcodeTravelInstances.Last()->AddInstance(spawnTransform);
			}


		}
		else { 
			UE_LOG(LogTemp, Warning, TEXT("Total extrusions in gcode: %d"), gcodeExtrusionTotalCount); 
			subScrubbingEnabled = true;
			return true; 
		}
	}

	parsingIndex += linesPerTick;

	loadingProgress = (float)((float)parsingIndex / (float)gcodePrintMoveArray.Num()) * 100;

	return false;
}

void AmodelViewer::gcodeNewInstanceLayer(TArray<UInstancedStaticMeshComponent*> & addToArray) {

	UInstancedStaticMeshComponent* newInstance = NewObject<UInstancedStaticMeshComponent>(this);
	newInstance->RegisterComponentWithWorld(GetWorld());
	newInstance->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	AddOwnedComponent(newInstance);
	newInstance->SetRelativeLocation(FVector(0,0,0),false,NULL,ETeleportType::None);
	newInstance->SetMaterial(0, AssetM_ExtrusionMaterialActive);

	if (addToArray.Num() > 0) {
		addToArray.Last()->SetMaterial(0, AssetM_ExtrusionMaterialDefault);
	}

	addToArray.Add(newInstance);
	addToArray.Last()->SetStaticMesh(AssetSM_PrintExtrusionLine);

	


	//return newComponent;

	//UInstancedStaticMeshComponent* newInstance = NewObject<UInstancedStaticMeshComponent>(this);
	//newInstance->RegisterComponent();


}

FTransform AmodelViewer::extrudeTransform(const FVector startPoint, const FVector endPoint, const FVector modelCentre) {

	FRotator lineRotation = FRotationMatrix::MakeFromX(endPoint - startPoint).Rotator();

	FTransform outTransform;

	FVector lineLocation = ((endPoint + startPoint).operator*(.5)).operator-(modelCentre);

	FVector lineScale;

	lineScale.Y = 0.004;
	lineScale.Z = 0.004;

	lineScale.X = ((startPoint.operator-(endPoint)).Size()) / 100;

	lineLocation.X = -lineLocation.X;

	//lineRotation.Quaternion().Z;

	lineRotation.Yaw = 0 - lineRotation.Yaw;

	//lineRotation = FRotator(lineRotation.Quaternion().X, lineRotation.Quaternion().Y, 0 - lineRotation.Quaternion().Z);

	outTransform.SetLocation(lineLocation);
	outTransform.SetRotation(lineRotation.Quaternion());
	outTransform.SetScale3D(lineScale);

	return outTransform;
}

bool AmodelViewer::constructStlModel() {
	int verticesPerTickOriginal = 500;

	//first we need to calculate the centre point
	if (parsingIndex == -1) {

		modelCentreOffset = UKismetMathLibrary::GetVectorArrayAverage(stlVertexArray);
		UE_LOG(LogTemp, Warning, TEXT("Centre: %s"), *modelCentreOffset.ToString());

		//AddActorWorldOffset()
		for (int i = 0; i < stlVertexArray.Num(); i++) {
			stlVertexArray[i] = stlVertexArray[i].operator-(modelCentreOffset);// .operator+(GetActorLocation());
			//UE_LOG(LogTemp, Warning, TEXT("Vertex %d: %s"), i, *stlVertexArray[i].ToString());
		}

		//next loop continue past this
		parsingIndex = 0;

		return false;
	}

	//int verticesPerTick = (int)verticesPerTickOriginal * loadRateModifier;

	//for (int i = parsingIndex; i < (parsingIndex + verticesPerTick); i++) {
	//	if(i < )
	//}

	modelProceduralMesh->SetCastShadow(true);
	modelProceduralMesh->SetVisibility(true);
	modelProceduralMesh->SetMaterial(0, AssetM_ModelMaterial);
	modelProceduralMesh->CreateMeshSection(0, stlVertexArray, stlTriangleArray, stlNormalArray, TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), false);

	modelProceduralMesh->RegisterComponent();
	modelProceduralMesh->AttachToComponent(GetRootComponent(),FAttachmentTransformRules::KeepWorldTransform);

	return true;
}

bool AmodelViewer::loadFile() {

	//exit if no file path set
	if (fullFilePath.Len() < 0) {
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("Loading file %s"), *fullFilePath);
	loadingStartedMillis = FDateTime::Now().GetTimeOfDay().GetTotalMilliseconds();
	UE_LOG(LogTemp, Warning, TEXT("Loading starting at %f"), loadingStartedMillis);

	//First we need to determine the file type
	if (fullFilePath.EndsWith(".stl", ESearchCase::IgnoreCase)) {
		fileType = EModelFileType::PE_STL;
	}
	else if (fullFilePath.EndsWith(".gcode", ESearchCase::IgnoreCase)) {
		fileType = EModelFileType::PE_GCODE;
	} 
	else if (fullFilePath.EndsWith(".step", ESearchCase::IgnoreCase)) {
		fileType = EModelFileType::PE_STEP;
	}

	bool success = false;

	switch (fileType) {
	case EModelFileType::PE_GCODE:
		UE_LOG(LogTemp, Warning, TEXT("G-Code File Detected"));
		success = loadGcodeFile();
		break;
	case EModelFileType::PE_STL:
		UE_LOG(LogTemp, Warning, TEXT("STL File Detected"));
		success = loadStlFile();
		break;
	}

	if (success) {
		UE_LOG(LogTemp, Warning, TEXT("File succesfully loaded"));
	}

	return success;
}

bool AmodelViewer::parseFile() {
	bool success = false;

	switch (fileType) {
		case EModelFileType::PE_GCODE:
			success = parseGcodeFile();
			break;
		case EModelFileType::PE_STL:
			success = parseStlFile();
			break;
	}

	if (success) {
		UE_LOG(LogTemp, Warning, TEXT("File succesfully parsed"));
	}

	return success;
}

bool AmodelViewer::parseStlFile() {
	bool success = false;

	switch (stlType) {
		case ESTLFileType::PE_BINARY:
			success = parseBinaryStlFile();
			break;
		case ESTLFileType::PE_ASCII:
			success = parseAsciiStlFile();
			break;
	}
	return success;
}

bool AmodelViewer::parseBinaryStlFile() {
	int verticesPerTickOriginal = 10000;
	bool fileExhausted = false;

	int verticesPerTick = (int)verticesPerTickOriginal * loadRateModifier;
	int endingIndex = parsingIndex + (verticesPerTick * 50);

	if (parsingIndex == 0) { parsingIndex = 84; }

	while (parsingIndex != -1 && parsingIndex < FMath::Min(fileBinaryArray.Num(), endingIndex)) {
		if (parsingIndex < fileBinaryArray.Num() - 50) {
			stlNormalArray.Add(FVector(getFloat32FromByteArray(fileBinaryArray, parsingIndex), getFloat32FromByteArray(fileBinaryArray, parsingIndex + 4), getFloat32FromByteArray(fileBinaryArray, parsingIndex + 8)));
			//UE_LOG(LogTemp, Warning, TEXT("Normal Calculated"));
			stlNormalArray.Add(FVector(stlNormalArray[stlNormalArray.Num() - 1]));
			stlNormalArray.Add(FVector(stlNormalArray[stlNormalArray.Num() - 1]));
			//UE_LOG(LogTemp, Warning, TEXT("Normal Copied"));

			parsingIndex += 12;

			for (int i = 0; i < 3; i++) {

				stlVertexArray.Add(FVector(getFloat32FromByteArray(fileBinaryArray, parsingIndex + 8), getFloat32FromByteArray(fileBinaryArray, parsingIndex + 4), getFloat32FromByteArray(fileBinaryArray, parsingIndex)));
				//UE_LOG(LogTemp, Warning, TEXT("Vertex Calculated"));
				//UE_LOG(LogTemp, Warning, TEXT("Vertex %d: %s"), stlVertexArray.Num(), *stlVertexArray.Last().ToString());
				stlTriangleArray.Add(stlTriangleIndex);
				stlTriangleIndex += 1;

				parsingIndex += 12;
			}

			//stlTriangleIndex += 1;
			parsingIndex += 2;
		}
		else {
			fileExhausted = true;
			parsingIndex = -1;
		}
	}


	loadingProgress = (float)((float)parsingIndex / (float)fileBinaryArray.Num()) * 100;

	return fileExhausted;
}

bool AmodelViewer::parseAsciiStlFile() {
	int verticesPerTickOriginal = 10000;
	bool fileExhausted = false;

	int verticesPerTick = (int)verticesPerTickOriginal * loadRateModifier;
	int endingIndex = parsingIndex + (verticesPerTick * 50);

	TArray <float> tempVertex;
	TArray <FString> stringArray;

	for (int i = 0; i < fileStringArray.Num(); i++) {

		tempVertex.Empty();
		stringArray.Empty();

		int normalStringLocation = fileStringArray[i].Find("normal", ESearchCase::IgnoreCase, ESearchDir::FromStart, -1);

		if (normalStringLocation > -1) {

			fileStringArray[i].RightChop(normalStringLocation + 6).ParseIntoArray(stringArray, TEXT(" "), true);

			for (int j = 0; j < stringArray.Num(); j++) {
				float normalComp;

				normalComp = FCString::Atof(*stringArray[j]);
				tempVertex.Add(normalComp);

				//UE_LOG(LogTemp, Warning, TEXT("ASCII STL Normal section %s"), *stringArray[j]);
				//UE_LOG(LogTemp, Warning, TEXT("ASCII STL Normal convert %f"), normalComp);
			}

			stlNormalArray.Add(FVector(tempVertex[2], tempVertex[1], tempVertex[0]));
			stlNormalArray.Add(FVector(stlNormalArray[stlNormalArray.Num() - 1]));
			stlNormalArray.Add(FVector(stlNormalArray[stlNormalArray.Num() - 1]));

			//UE_LOG(LogTemp, Warning, TEXT("ASCII STL Normal %s"), *FVector(tempVertex[2], tempVertex[1], tempVertex[0]).ToString());
		}



		int vertexStringLocation = fileStringArray[i].Find("vertex", ESearchCase::IgnoreCase, ESearchDir::FromStart, -1);
		//if line contains vertex, extract
		if (vertexStringLocation > -1) {

			fileStringArray[i].RightChop(vertexStringLocation + 6).ParseIntoArray(stringArray, TEXT(" "),true);

			for (int j = 0; j < stringArray.Num(); j++) {
				
				tempVertex.Add(FCString::Atof(*stringArray[j]));

				stlTriangleArray.Add(stlTriangleIndex);
				stlTriangleIndex += 1;
			}

			stlVertexArray.Add(FVector(tempVertex[2], tempVertex[1], tempVertex[0]));
			//UE_LOG(LogTemp, Warning, TEXT("ASCII STL Vertex %s"), *FVector(tempVertex[2], tempVertex[1], tempVertex[0]).ToString());



		}
	}

	parsingIndex = -1;

	return true;
}

bool AmodelViewer::parseStepFile() {
	return true;
}

bool AmodelViewer::parseGcodeFile() {
	int parseLinesPerTick = 500;

	bool fileExhausted = false;

	int linesPerTick = (int)parseLinesPerTick * loadRateModifier;

	for (int i = parsingIndex; i < FMath::Min((parsingIndex + linesPerTick), fileStringArray.Num()); i++) {

		if (fileStringArray[i].StartsWith("G1", ESearchCase::IgnoreCase)) {
			gcodePrintMoveArray.Add(gcodeParseG1Line(fileStringArray[i]));
		}

	}

	parsingIndex += linesPerTick;

	loadingProgress = (float) ((float)parsingIndex / (float) fileStringArray.Num()) * 100;

	//Parsing completed
	if (parsingIndex >= fileStringArray.Num()) {
		fileExhausted = true;
		parsingIndex = -1;
	}

	return fileExhausted;
}

FPrintMove AmodelViewer::gcodeParseG1Line(FString & gcodeLine) {
	FPrintMove thisMove;

	float xCoord, yCoord, zCoord;
	bool xFound, yFound, zFound;

	xCoord = gcodeGetAxisCoordFromLine(gcodeLine, "X", xFound);
	yCoord = gcodeGetAxisCoordFromLine(gcodeLine, "Y", yFound);
	zCoord = gcodeGetAxisCoordFromLine(gcodeLine, "Z", zFound);

	if (!xFound) { xCoord = gcodeLastPoint.X; }
	if (!yFound) { yCoord = gcodeLastPoint.Y; }
	if (!zFound) { zCoord = gcodeLastPoint.Z; }
	else { thisMove.layerChange = true; }

	thisMove.endPoint.Set(xCoord, yCoord, zCoord);
	gcodeLastPoint = FVector(thisMove.endPoint);

	if (gcodeLine.Contains("E", ESearchCase::IgnoreCase, ESearchDir::FromStart)) {
		thisMove.moveType = EGcodePrintMoveEnum::PE_Perimeter;
	}
	else {
		thisMove.moveType = EGcodePrintMoveEnum::PE_Travel;
	}

	return thisMove;

}

bool AmodelViewer::loadGcodeFile() {
	return readTextFile();	
}

bool AmodelViewer::loadStlFile() { 
	if (!readTextFile()) {
		return false;
	}

	//Determine if STL file is ASCII or Binary
	if (stlFileIsAscii(fileStringArray)) {
		stlType = ESTLFileType::PE_ASCII;
		UE_LOG(LogTemp, Warning, TEXT("ASCII STL Detected"));
	}
	else {
		stlType = ESTLFileType::PE_BINARY;
		UE_LOG(LogTemp, Warning, TEXT("Binary STL Detected"));
	}

	//If ASCII, we've already loaded the file in the format we need
	//If binary, we need to load the raw binary data in

	if (stlType == ESTLFileType::PE_BINARY) {
		readBinaryFile();
	}

	//Now, get the total triangle count so we can estimate loading progress later
	stlTriangleCount = stlGetTriangleCount();
	
return	true;
}

float AmodelViewer::gcodeGetAxisCoordFromLine(FString & gcodeLine, const FString axisPrefix, bool& foundAxis) {
	int axisIndex = gcodeLine.Find(axisPrefix, ESearchCase::IgnoreCase, ESearchDir::FromStart, -1);
	float axisCoordinate = 0;

	if (axisIndex > 0) {
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "found axis letter");
		//UE_LOG(LogTemp, Warning, TEXT("found axis letter %s"), *axisPrefix);
		FString tempCoordString = "";

		for (int i = axisIndex + 1; i < axisIndex + 6; i++) {
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

bool AmodelViewer::stlFileIsAscii(TArray<FString> & fileStringArray) {
	if (fileStringArray.Num() > 1) {
		if (fileStringArray[0].Contains("solid", ESearchCase::IgnoreCase, ESearchDir::FromStart) && fileStringArray[1].Contains("facet", ESearchCase::IgnoreCase, ESearchDir::FromStart)) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

bool AmodelViewer::readBinaryFile() {

	if (!FFileHelper::LoadFileToArray(fileBinaryArray, *fullFilePath))
	{
		//ClientMessage("FFILEHELPER:>> Invalid File");
		UE_LOG(LogTemp, Warning, TEXT("Invalid File!"));
		return false;
		//~~
	}

	//File Load Error
	if (fileBinaryArray.Num() <= 0) {
		UE_LOG(LogTemp, Warning, TEXT("Empty File!"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("Binary File Loaded"));
	return true;

}

bool AmodelViewer::readTextFile() {
	if (!FFileHelper::LoadANSITextFileToStrings(*fullFilePath, NULL, fileStringArray)) {
		return false;
	}
	else {
		return true;
	}
}

int AmodelViewer::stlGetTriangleCount() {
	int triCount = 0;

	if (stlType == ESTLFileType::PE_BINARY) {
		triCount = fileBinaryArray[80] | (uint32_t)fileBinaryArray[81] << 8 | ((uint32_t)fileBinaryArray[82] << 16 | (uint32_t)fileBinaryArray[83] << 24);
	}
	else if (stlType == ESTLFileType::PE_ASCII) {
		triCount = (fileStringArray.Num() - 1) / 7;
	}

	UE_LOG(LogTemp, Warning, TEXT("Triangle Count: %d"), triCount);

	return triCount;
}

float AmodelViewer::getFloat32FromByteArray(const TArray<uint8> & binaryData, const int startIndex) {
	static FloatUnionData tempFloat;
	tempFloat.byteData = binaryData[startIndex] | (uint32_t)binaryData[startIndex + 1] << 8 | ((uint32_t)binaryData[startIndex + 2] << 16 | (uint32_t)binaryData[startIndex + 3] << 24);
	return tempFloat.f;
}