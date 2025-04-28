// Universal Camera Plugin - Mathieu Jacq 2021

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "SharedStructs.generated.h"

UENUM(BlueprintType)
enum ETargetMod
{
	TargetMod_None					UMETA(DisplayName = "None"),
	TargetMod_CustomValue			UMETA(DisplayName = "CustomValue"),
	TargetMod_Spline				UMETA(DisplayName = "Spline"),
	TargetMod_Actor					UMETA(DisplayName = "Actor"),
	TargetMod_Socket				UMETA(DisplayName = "Socket"),
	TargetMod_SceneComponent		UMETA(DisplayName = "SceneComponent")
};

USTRUCT(Blueprintable)
struct UNIVERSALCAMERAPLUGIN_API FUniversalCameraPositionSaveFormat
{
	GENERATED_BODY()

		FUniversalCameraPositionSaveFormat() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector DesiredLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector DesiredSocketOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector DesiredTargetOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator DesiredRotation = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator DesiredRotationOffset = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DesiredZoom = 0.f;
};

USTRUCT(Blueprintable)
struct UNIVERSALCAMERAPLUGIN_API FTargetSettings
{
	GENERATED_BODY()

	FTargetSettings() {}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Universal Camera Plugin|Restrictions")
		AActor* Actor;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Universal Camera Plugin|Restrictions")
		USceneComponent* SceneComponent;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Universal Camera Plugin|Restrictions")
		UMeshComponent* Mesh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Universal Camera Plugin|Restrictions")
		FName Socket;

	bool IsValidActor() const;
	//	Check for validity before calling those
	FORCEINLINE FVector GetActorLocation() const { return Actor->GetActorLocation(); }
	FORCEINLINE FRotator GetActorRotation() const { return Actor->GetActorRotation(); }

	bool IsValidSocket() const;
	//	Check for validity before calling those
	FORCEINLINE FVector GetSocketLocation() const { return Mesh->GetSocketLocation(Socket); }
	FORCEINLINE FRotator GetSocketRotation() const { return Mesh->GetSocketRotation(Socket); }

	bool IsValidSceneComponent() const;
	//	Check for validity before calling those
	FORCEINLINE FVector GetSceneComponentLocation() const { return SceneComponent->GetComponentLocation(); }
	FORCEINLINE FRotator GetSceneComponentRotation() const { return SceneComponent->GetComponentRotation(); }


	//	Check for validity before calling those
	float GetTargetActorZoom(bool& IsValidTarget);

	AActor* GetOwnerActor() const;
};