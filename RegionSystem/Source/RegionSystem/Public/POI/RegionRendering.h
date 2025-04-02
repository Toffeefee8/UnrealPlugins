#pragma once

#include "CoreMinimal.h"
#include "RegionPOI.h"
#include "Debug/DebugDrawComponent.h"
#include "RegionRendering.generated.h"

class FPOISceneProxy;

UCLASS(ClassGroup = Debug)
class UPOIRenderingComponent: public UDebugDrawComponent
{
	GENERATED_BODY()

protected:
	UPOIRenderingComponent();

#if WITH_EDITOR
	virtual FBoxSphereBounds CalcBounds(const FTransform &LocalToWorld) const override;
	virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
	
	friend FPOISceneProxy;
#endif

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bOnlyShowWhenSelected = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float DiamondSize = 10.0f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FLinearColor DiamondColor = FLinearColor::Red;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float LineThickness = 1.0f;
#endif
};

#if UE_ENABLE_DEBUG_DRAWING

#if	WITH_EDITOR

class FPrimitiveSceneProxy;
class APlayerController;
class FMeshElementCollector;
class UCanvas;
struct FNodeDebugData;

class FPOISceneProxy final : public FDebugRenderSceneProxy
{
public:

	FPOISceneProxy(const UPOIRenderingComponent* InComponent): FDebugRenderSceneProxy(InComponent)
	{
		if (InComponent && InComponent->GetOwner())
		{
			bOnlyShowWhenSelected = InComponent->bOnlyShowWhenSelected;
			DiamondColor = InComponent->DiamondColor;
			DiamondSize = InComponent->DiamondSize;
			LineThickness = InComponent->LineThickness;
			if (ARegionPOI* POIActor = Cast<ARegionPOI>(InComponent->GetOwner()))
			{
				CachedPoints = POIActor->GetData();
			}
		}
	}

	//~ Begin FPrimitiveSceneProxy interface.
	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views, 
		const FSceneViewFamily& ViewFamily, 
		uint32 VisibilityMap, 
		FMeshElementCollector& Collector) const override
	{
		if (!IsSelected() && bOnlyShowWhenSelected)
			return;
		
		//Define Depth
		const uint8 DepthPriority = SDPG_Foreground;

		//Iterate over all views.
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
				for (const FVector& Center : CachedPoints)
				{
					const FVector TrueCenter = Center + CachedPoints.Location;
					//Define vertices
					const FVector Top     = TrueCenter + FVector(0.0f, 0.0f, DiamondSize);
					const FVector Bottom  = TrueCenter + FVector(0.0f, 0.0f, -DiamondSize);
					const FVector Right   = TrueCenter + FVector(DiamondSize, 0.0f, 0.0f);
					const FVector Left    = TrueCenter + FVector(-DiamondSize, 0.0f, 0.0f);
					const FVector Forward = TrueCenter + FVector(0.0f, DiamondSize, 0.0f);
					const FVector Back    = TrueCenter + FVector(0.0f, -DiamondSize, 0.0f);

					//Draw edges from Top to each midpoint.
					PDI->DrawLine(Top, Right, DiamondColor, DepthPriority, LineThickness);
					PDI->DrawLine(Top, Left, DiamondColor, DepthPriority, LineThickness);
					PDI->DrawLine(Top, Forward, DiamondColor, DepthPriority, LineThickness);
					PDI->DrawLine(Top, Back, DiamondColor, DepthPriority, LineThickness);

					//Draw edges from Bottom to each midpoint.
					PDI->DrawLine(Bottom, Right, DiamondColor, DepthPriority, LineThickness);
					PDI->DrawLine(Bottom, Left, DiamondColor, DepthPriority, LineThickness);
					PDI->DrawLine(Bottom, Forward, DiamondColor, DepthPriority, LineThickness);
					PDI->DrawLine(Bottom, Back, DiamondColor, DepthPriority, LineThickness);

					//Draw edges connecting the midpoints to form a ring.
					PDI->DrawLine(Right, Forward, DiamondColor, DepthPriority, LineThickness);
					PDI->DrawLine(Forward, Left, DiamondColor, DepthPriority, LineThickness);
					PDI->DrawLine(Left, Back, DiamondColor, DepthPriority, LineThickness);
					PDI->DrawLine(Back, Right, DiamondColor, DepthPriority, LineThickness);
				}
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = true;
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = false;
		return Result;
	}
	//~ End FPrimitiveSceneProxy interface.

private:

	FRegionPOIData CachedPoints;

	//Settings
	bool bOnlyShowWhenSelected = true;
	float DiamondSize = 10.0f;  
	FLinearColor DiamondColor = FLinearColor::White;
	float LineThickness = 1.0f;
};

#endif

#endif