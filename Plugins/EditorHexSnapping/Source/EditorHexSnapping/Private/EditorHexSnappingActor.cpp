// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorHexSnappingActor.h"
#include "CoreMinimal.h"
#include "Editor.h"
#include "uobject/constructorhelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Math/NumericLimits.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Misc/CString.h"

// Sets default values
AEditorHexSnappingActor::AEditorHexSnappingActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bIsEditorOnlyActor = true;
	bForceFullRebuildGrid = true;

	DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	RootComponent = DefaultRoot;
#ifdef WITH_EDITOR
	gridComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Grid"));	
	gridComponent->SetupAttachment(RootComponent);	
	gridComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	previewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMeshComponent"));	
	previewMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));	
	previewMeshComponent->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>CubeMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (CubeMeshAsset.Object != NULL)
	{
		previewMeshComponent->SetStaticMesh(CubeMeshAsset.Object);		
	}
	previewMeshComponent->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	if (SphereMeshAsset.Object != NULL)
	{
		this->defaultmesh = SphereMeshAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/Engine/EditorMaterials/PhAT_UnselectedMaterial.PhAT_UnselectedMaterial'"));
	if (Material.Object != NULL)
	{
		defaultmeshmaterial = (UMaterial*)Material.Object;
	}
#endif
}

#ifdef WITH_EDITOR
void AEditorHexSnappingActor::InitializeHexGrid()
{
	bForceFullRebuildGrid = true;
	RebuildGrid();

	if (GEditor)
	{
		SetSnapingMode(bEnabledSnapping);
	}
}

void AEditorHexSnappingActor::RebuildGrid()
{
	if (bForceFullRebuildGrid)
	{
		bForceFullRebuildGrid = false;

		TArray<USceneComponent*> arr;
		gridComponent->GetChildrenComponents(true, arr);		
		for (auto item : arr)
		{
			item->UnregisterComponent();
			item->DestroyComponent();			
		}

		for (int i = 0; i < GridX; i++) {
			for (int j = 0; j < GridY; j++) {
				FName name = *FString::Printf(TEXT("%i_%i"), i, j);

				FVector pos = FVector(.0f, .0f, 0.f);
				pos.X = i * GridItemSize * ScaleXY + (j % 2 == 0 ? 0 : GridItemSize * ScaleXY * 0.5f) + (i * Spacing);
				pos.Y = j * GridItemSize * ScaleXY * sqrt(3) * 0.5f + (j * Spacing);

				UStaticMeshComponent* item = NewObject<UStaticMeshComponent>(this, name);
				if (HexagonMeshGrid) {
					item->SetStaticMesh(HexagonMeshGrid);
					item->SetWorldScale3D(FVector(ScaleXY, ScaleXY, 1.f));
				}				
				else if (defaultmesh) {
					item->SetStaticMesh(defaultmesh);
					item->SetWorldScale3D(FVector(0.25f * ScaleXY, 0.25f * ScaleXY, 0.25f * ScaleXY));
					if (defaultmeshmaterial) {
						item->SetMaterial(0, (UMaterialInterface*)defaultmeshmaterial);
					}
				}
				
				item->SetRelativeLocation(pos);								
				item->AttachToComponent(gridComponent, FAttachmentTransformRules::KeepRelativeTransform);								
				item->RegisterComponent();
			}
		}
	}

	TArray<USceneComponent*> arr;
	gridComponent->GetChildrenComponents(true, arr);
	for (const auto item : arr)
	{
		TArray<FString> Out;
		item->GetName().ParseIntoArray(Out, TEXT("_"), true);
		int i = FCString::Atoi(*Out[0]);
		int j = FCString::Atoi(*Out[1]);
		FVector pos = FVector(.0f, .0f, 0.f);
		pos.X = i * GridItemSize * ScaleXY + (j % 2 == 0 ? 0 : GridItemSize * ScaleXY * 0.5f) + (i * Spacing);
		pos.Y = j * GridItemSize * ScaleXY * sqrt(3) * 0.5f + (j * Spacing);
		item->SetRelativeLocation(pos);		
	}
}

void AEditorHexSnappingActor::OnActorMoving(AActor* actor)
{
	if (actor == this || !bEnabledSnapping || actor->GetClass()->GetName() != "StaticMeshActor") return;

	if (actor != currentactor)
	{
		currentactor = actor;

		TArray<UStaticMeshComponent*> Components;
		actor->GetComponents<UStaticMeshComponent>(Components);
		for (int32 i = 0; i < Components.Num(); i++)
		{
			UStaticMeshComponent* StaticMeshComponent = Components[i];
			auto mesh = StaticMeshComponent->GetStaticMesh();
			if (mesh) {
				previewMeshComponent->SetStaticMesh(mesh);
				previewMeshComponent->SetMaterial(0, (UMaterialInterface*)defaultmeshmaterial);
				break;
			}
		}
		previewMeshComponent->SetVisibility(true);
	}
	else
	{
		float nearest = MAX_flt;
		USceneComponent* nearestcomp = nullptr;

		TArray<USceneComponent*> arr;
		gridComponent->GetChildrenComponents(true, arr);		
		for (USceneComponent* item : arr)
		{
			float distance = (actor->GetTransform().GetLocation() - item->GetComponentTransform().GetLocation()).Size();
			if (distance < nearest)
			{
				nearest = distance;
				nearestcomp = item;
			}
		}

		if (nearestcomp)
		{
			FVector snapdestination = nearestcomp->GetComponentTransform().GetLocation();
			if (FVector::Dist2D(lastsnap, snapdestination) > KINDA_SMALL_NUMBER)
			{
				lastsnap = snapdestination;
				previewMeshComponent->SetWorldLocation(snapdestination);
			}
		}
	}
}

void AEditorHexSnappingActor::OnActorMoved(AActor* actor)
{
	if (currentactor != actor) return;

	if (actor != this && bEnabledSnapping && actor->GetClass()->GetName() == "StaticMeshActor")
	{
		float distance = MAX_flt;
		USceneComponent* nearest = nullptr;

		TArray<USceneComponent*> arr;
		gridComponent->GetChildrenComponents(true, arr);
		for (USceneComponent* item : arr)
		{
			float currentdistance = (actor->GetTransform().GetLocation() - item->GetComponentTransform().GetLocation()).Size();
			if (currentdistance < distance)
			{
				distance = currentdistance;
				nearest = item;
			}
		}

		if (nearest)
		{
			FVector snapdestination = nearest->GetComponentTransform().GetLocation();
			if (!bAutosnapToZAxis)
			{
				snapdestination.Z = actor->GetActorLocation().Z;
			}
			actor->SetActorLocation(snapdestination);
		}
	}

	//previewMeshComponent->SetStaticMesh(nullptr);
	previewMeshComponent->SetVisibility(false);
	currentactor = nullptr;
}

void AEditorHexSnappingActor::SetSnapingMode(bool isEnable)
{
	if (isEnable)
	{
		OnActorMovedDelegate = GEditor->OnActorMoved().AddUObject(this, &AEditorHexSnappingActor::OnActorMoved);
		OnActorMovingDelegate = GEditor->OnActorMoving().AddUObject(this, &AEditorHexSnappingActor::OnActorMoving);
	}
	else
	{
		OnActorMovedDelegate.Reset();
		OnActorMovingDelegate.Reset();
		GEditor->OnActorMoved().Remove(OnActorMovedDelegate);
		GEditor->OnActorMoving().Remove(OnActorMovingDelegate);
	}
}

void AEditorHexSnappingActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{		
	if ((PropertyChangedEvent.Property != nullptr) && (PropertyChangedEvent.Property->GetName() == "HexagonMeshGrid"))
	{
		if (HexagonMeshGrid) {
			auto hexagonBounds = HexagonMeshGrid->GetExtendedBounds();
			GridItemSize = hexagonBounds.GetBox().GetSize().X;
		}
		else
		{
			GridItemSize = DefaultGridItemSize;
		}
		bForceFullRebuildGrid = true;
	};

	if ((PropertyChangedEvent.Property != nullptr) && (PropertyChangedEvent.Property->GetName() == "GridX" || PropertyChangedEvent.Property->GetName() == "GridY"))
	{
		bForceFullRebuildGrid = true;
	};

	if ((PropertyChangedEvent.Property != nullptr) &&
		(PropertyChangedEvent.Property->GetName() == "bEnabledSnapping"))
	{
		if (bEnabledSnapping) {
			TArray<AActor*> arr;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEditorHexSnappingActor::StaticClass(), arr);
			for (AActor* item : arr)
			{
				if (item != this) {
					Cast<AEditorHexSnappingActor>(item)->bEnabledSnapping = false;
					item->PostEditChange();
				}
			}
		}
		SetSnapingMode(bEnabledSnapping);
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AEditorHexSnappingActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();	
	RebuildGrid();
}

void AEditorHexSnappingActor::PostEditImport()
{	
	Super::PostEditImport();	
	bEnabledSnapping = false;
}

#endif // WITH_EDITOR