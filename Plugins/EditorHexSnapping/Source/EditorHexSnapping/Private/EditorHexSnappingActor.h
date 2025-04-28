// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EditorHexSnappingActor.generated.h"

UCLASS(Blueprintable, ClassGroup = "Utility",
	hidecategories = (Collision, Physics, Object, LOD, Lighting, TextureStreaming, Replication, Actor, Rendering),
	meta = (DisplayName = "EditorHexSnapping", BlueprintSpawnableComponent))
class AEditorHexSnappingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEditorHexSnappingActor();

private:
#if WITH_EDITOR	

	UPROPERTY(EditAnywhere, Category = "SnappingMode")
		bool bEnabledSnapping = false;

	UPROPERTY(EditAnywhere, Category = "SnappingMode")
		bool bAutosnapToZAxis = false;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")	    		
		USceneComponent* DefaultRoot = nullptr;	

	UPROPERTY(EditAnywhere, Category = "Setup", meta = (ClampMin = 1))
		int GridX = 4;

	UPROPERTY(EditAnywhere, Category = "Setup", meta = (ClampMin = 1))
		int GridY = 4;

	UPROPERTY(EditAnywhere, Category = "Setup", meta = (ClampMin = 1.f, EditCondition = "HexagonMeshGrid == nullptr", ToolTip = "Automatic size when HexagonMeshGrid assigned"))
		float GridItemSize = 200.f; 

	UPROPERTY(EditAnywhere, Category = "Setup", meta = (ClampMin = 0.0f))
		float Spacing = 10.f; 

	UPROPERTY(EditDefaultsOnly, Category = "Setup", meta = (ClampMin = 0.1f))
		float ScaleXY = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Components")
		UStaticMesh* HexagonMeshGrid = nullptr;

	void InitializeHexGrid();
	void RebuildGrid();
	void SetSnapingMode(bool isEnable);	
	void OnActorMoved(AActor* actor);
	void OnActorMoving(AActor* actor);

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;			
	virtual void PostRegisterAllComponents() override;	
	virtual void PostEditImport() override;	

	const float DefaultGridItemSize = 200.f;
	bool bForceFullRebuildGrid = true;
	FDelegateHandle OnActorMovedDelegate;
	FDelegateHandle OnActorMovingDelegate;	
	UStaticMesh* defaultmesh = nullptr;
	UMaterial* defaultmeshmaterial = nullptr;
	AActor* currentactor = nullptr;	
	FVector lastsnap;	
	USceneComponent* gridComponent = nullptr;
	UStaticMeshComponent* previewMeshComponent = nullptr;		

#endif // WITH_EDITOR
};

