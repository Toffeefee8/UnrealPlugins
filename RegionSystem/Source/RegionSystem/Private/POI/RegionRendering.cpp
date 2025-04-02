#include "POI/RegionRendering.h"

UPOIRenderingComponent::UPOIRenderingComponent()
{
	bIsEditorOnly = true;
	PrimaryComponentTick.bCanEverTick = false;
}

#if	WITH_EDITOR
FBoxSphereBounds UPOIRenderingComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBox BoundsBox(ForceInit);
	
	if (ARegionPOI* POIActor = Cast<ARegionPOI>(GetOwner()))
	{
		if (POIActor->GetData().RelevantLocations.Num() > 0)
		{
			for (const FVector& Point : POIActor->GetData().RelevantLocations)
			{
				BoundsBox += Point;
			}
		}
		else
		{
			//Fallback: use the actor's location.
			BoundsBox += POIActor->GetActorLocation();
		}
	}
	else
	{
		//Fallback: if no valid owner is found, use a zero-size box.
		BoundsBox = FBox(FVector::ZeroVector, FVector::ZeroVector);
	}
	
	BoundsBox = BoundsBox.ExpandBy(15.0f);
	return FBoxSphereBounds(BoundsBox).TransformBy(LocalToWorld);
}

FDebugRenderSceneProxy* UPOIRenderingComponent::CreateDebugSceneProxy()
{
	return new FPOISceneProxy(this);
}
#endif