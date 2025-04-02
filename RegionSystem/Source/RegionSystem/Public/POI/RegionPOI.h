#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RegionVolume.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "GameFramework/Actor.h"
#include "RegionPOI.generated.h"

class UPOIRenderingComponent;

USTRUCT(BlueprintType)
struct FLocationCache
{
	GENERATED_BODY()

public:
	FLocationCache() {  }
	FLocationCache(const TArray<FVector>& InRelevantPoints, FVector InOrigin, bool bInIsLocal) :
		RelevantPoints(InRelevantPoints),
		Origin(InOrigin),
		bIsLocal(bInIsLocal) {  }

	bool IsLocal() const;
	bool IsGlobal() const;
	TArray<FVector> GetAsLocal() const;
	TArray<FVector> GetAsGlobal() const;

	void SetNewOrigin(const FVector& InOrigin);
	void MakeLocal();
	void MakeGlobal();
	void Clear();
	void AddPoint(const FVector& Point, bool bLocal);

	operator TArray<FVector>() const
	{
		return GetAsLocal();
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FVector> RelevantPoints {};
	UPROPERTY(BlueprintReadOnly)
	FVector Origin {};
	UPROPERTY(BlueprintReadOnly)
	bool bIsLocal = true;
};

class UConvexPrimComponent;
class UPOITypeProcessor;

UCLASS(Blueprintable, PrioritizeCategories = ("Regions"))
class REGIONSYSTEM_API ARegionPOI : public AActor
{
	GENERATED_BODY()

public:

	ARegionPOI();
	virtual bool IsEditorOnly() const override { return true; }

#if WITH_EDITOR

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;

	FGameplayTag GetContainingRegion() const;
	FRegionPOIData GetData() const;
	
	void BakeAllRegions();
	void BakeRegion();
	
	UFUNCTION(CallInEditor, Category = "Regions")
	void ReevaluateRegion();

	UFUNCTION(CallInEditor, Category = "RegionPOI")
	void Recalculate();

#endif
	
protected:

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBillboardComponent> Billboard;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPOIRenderingComponent> POIRenderer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Regions")
	FGameplayTag ContainingRegion = FGameplayTag();

	//Custom EQS
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "RegionPOI")
	TObjectPtr<UPOITypeProcessor> POITypeProcessor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RegionPOI")
	FLocationCache RelevantPoints {};
#endif
};