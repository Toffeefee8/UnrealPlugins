#if WITH_EDITOR
#include "POI/Processors/CustomGrid.h"

#include "NavigationSystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "POI/RegionPOI.h"

void UCustomGrid::CalculateRelevantPoints_Implementation(ARegionPOI* POI, TArray<FVector>& OutPoints, bool& OutIsLocal) const
{
	OutPoints.Empty();
	OutIsLocal = false;
	
	if (!POI)
	{
		return;
	}
	
	if (NavPointSpacing <= 0.0f)
	{
		return;
	}
	
	//Get bounds
	const FVector2D BoxExtent = GetBoxExtents(POI);
	const FVector Origin = POI->GetActorLocation() - FVector(0, 0, BottomOffset);
	
	const int32 NumCellsX = FMath::FloorToInt(BoxExtent.X / NavPointSpacing);
	const int32 NumCellsY = FMath::FloorToInt(BoxExtent.Y/ NavPointSpacing);
	
	int32 NumCellsZ = FMath::Max(1, HeightDivisions);
	float ToleranceHeight = ((MaxHeight + BottomOffset) * POI->GetActorScale().Z / NumCellsZ) / 2.f;
	
	if (MaxHeight <= 0)
	{
		NumCellsZ = 1;
		ToleranceHeight = NavXYTolerance;
	}
	
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return;
	}
	
	for (int32 X = -NumCellsX; X <= NumCellsX; ++X)
	{
		for (int32 Y = -NumCellsY; Y <= NumCellsY; ++Y)
		{
			for (int32 Z = 0; Z < NumCellsZ; ++Z)
			{
				FVector2D LocalGridPoint2D = FVector2D(NavPointSpacing * X, NavPointSpacing * Y);
				if (!bUseBox && LocalGridPoint2D.Size() > CircleRadius)
					continue;
				
				FVector GridPoint = Origin + FVector(LocalGridPoint2D, 0);
				GridPoint.Z += ToleranceHeight + (ToleranceHeight * 2 * Z);
				
				FNavLocation ProjectedLocation;
				// UKismetSystemLibrary::DrawDebugBox(this, GridPoint, FVector(NavXYTolerance, NavXYTolerance, ToleranceHeight), FLinearColor::Red, FRotator::ZeroRotator, 3, 0);
				if (NavSys->ProjectPointToNavigation(GridPoint, ProjectedLocation, FVector(NavXYTolerance, NavXYTolerance, ToleranceHeight)))
				{
					OutPoints.Add(ProjectedLocation.Location);
				}
			}
		}
	}

// #if WITH_EDITOR
// 	for (const FVector& Point : OutPoints)
// 	{
// 		UE_LOG(LogTemp, Log, TEXT("Cached Point: %s"), *Point.ToString());
// 		UKismetSystemLibrary::DrawDebugSphere(this, Point, 10, 4, FLinearColor::White, 5,  0);
// 	}
// #endif
}

FVector2D UCustomGrid::GetBoxExtents(ARegionPOI* POI) const
{
	if (!bUseBox)
		return FVector2D(CircleRadius) * FMath::Max(POI->GetActorScale3D().X, POI->GetActorScale3D().Y);

	return BoxExtents * FVector2D(POI->GetActorScale().X, POI->GetActorScale().Y);
}
#endif
