#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RegionSubsystem.h"
#include "RegionSystem.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Structs/RegionTypes.h"
#include "RegionVolume.generated.h"

USTRUCT(BlueprintType)
struct FRegionPOIData
{
	GENERATED_BODY()

public:
	bool IsValid() const
	{
		return Location != FVector::ZeroVector;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> RelevantLocations {};

	// Iterator functions
	TArray<FVector>::RangedForIteratorType begin() { return RelevantLocations.begin(); }
	TArray<FVector>::RangedForIteratorType end() { return RelevantLocations.end(); }
	TArray<FVector>::RangedForConstIteratorType begin() const { return RelevantLocations.begin(); }
	TArray<FVector>::RangedForConstIteratorType end() const { return RelevantLocations.end(); }
};

class USphereComponent;
class UTextRenderComponent;
class URegionEditorSubsystem;
class URegionSubsystem;
class UBoxComponent;

UCLASS(Blueprintable, PrioritizeCategories = ("Regions"))
class REGIONSYSTEM_API ARegionVolume : public AActor
{
	GENERATED_BODY()

public:
	ARegionVolume();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Destroyed() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma region Functions
	
	UFUNCTION(BlueprintCallable)
	TArray<UObject*> GetOverlappingInterfaceObjects(TSubclassOf<UInterface> InterfaceClass) const;
	template<typename T>
	TArray<UObject*> GetOverlappingInterfaceObjects() const;
	TArray<AActor*> GetAllActorsInRegion() const;
	
	//RegionTag
	UFUNCTION(BlueprintCallable)
	FGameplayTag GetRegionTag() const;
	UFUNCTION(BlueprintCosmetic)
	int8 GetRegionDepth() const;

	//RegionType
	UFUNCTION(BlueprintCallable)
	ERegionTypes GetRegionType() const;

	//Extents
	UFUNCTION(BlueprintCallable)
	FVector GetBoxExtent() const;
	UFUNCTION(BlueprintCallable)
	bool Contains(FVector Location) const;
	UFUNCTION(BlueprintCallable)
	bool ContainsFully(FVector Location, FVector BoxExtent) const;

	//POI
	UFUNCTION(BlueprintCallable)
	TArray<FRegionPOIData> GetPOIData() const;

protected:
		
	//Overlaps
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//Register
	UFUNCTION()
	void RegisterSelfWithSubsystem();
	UFUNCTION()
	void DeregisterSelfWithSubsystem();

	UFUNCTION()
	float GetTextScale() const;
	
#pragma endregion

#pragma region Properties

protected:
	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> RegionBox = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Regions", meta = (Categories = "Regions.Areas"))
	FGameplayTag RegionTag = FGameplayTag();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Regions|POI")
	TArray<FRegionPOIData> POIData {};

private:
	
	friend URegionSubsystem;
	
	//Do Not Modify Value, For Region Subsystem to Handle
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bRegistered = false;
	
#pragma endregion

#pragma region Editor
#if	WITH_EDITOR
	
public:
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	
	UFUNCTION(CallInEditor, Category = "Regions")
	void ForceAutoTag();
	void ForceParentTag();

	UFUNCTION(CallInEditor, Category = "Regions")
	void SetExtension();
	void Bake();
	
	void HideVolume();
	void ShowVolume();
	void SetHighlight(uint8 State);

	void SetRegionText();
	
#endif
#if WITH_EDITORONLY_DATA

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Regions")
	FString CustomRegionExtension = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Regions")
	bool bAutoSetRegion = true;
	UPROPERTY(VisibleAnywhere, Category = "Regions")
	FGameplayTag ParentRegionTag = FGameplayTag();
	UPROPERTY(VisibleAnywhere, Category = "Regions")
	ERegionTypes RegionType = ERegionTypes::Room;

	static const FColor DefaultVolumeColor;
	static const int32 DefaultVolumeThickness;

	UPROPERTY(BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> RegionText = nullptr;
	
#endif
#pragma endregion

#pragma region Validation
public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
#pragma endregion
};

template <typename T>
TArray<UObject*> ARegionVolume::GetOverlappingInterfaceObjects() const
{
	static_assert(TIsDerivedFrom<T, UInterface>::Value, "GetOverlappingInterfaceActors: Template parameter T must be a UInterface type");

	TArray<AActor*> AllActors = GetAllActorsInRegion();
	TArray<UObject*> ReturnArray;

	for (AActor* Actor : AllActors)
	{
		if (!Actor)
			continue;
		
		if (Actor->Implements<T>())
			ReturnArray.Add(Actor);
		
		TArray<UActorComponent*> Components;
		Actor->GetComponents(Components);
		for (UActorComponent* Component : Components)
		{
			if (Component && Component->Implements<T>())
				ReturnArray.Add(Component);
		}
	}
	return ReturnArray;
}
