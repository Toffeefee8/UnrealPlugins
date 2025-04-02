#if WITH_EDITOR

#include "POI/Processors/POITypeProcessor.h"

void UPOITypeProcessor::CalculateRelevantPoints_Implementation(ARegionPOI* POI, TArray<FVector>& OutPoints, bool& OutIsLocal) const
{
	
}

FLocationCache UPOITypeProcessor::GetPOIRelevantPoints(ARegionPOI* POI) const
{
	TArray<FVector> Points {};
	bool bIsLocal {};
	CalculateRelevantPoints(POI, Points, bIsLocal);

	if (!bIsLocal)
		MakeArrayLocal(Points, POI->GetActorLocation());

	if (bLineOfSight)
	{
		FVector UpOffset = FVector(0, 0, HeightOffset);
		for (int i = Points.Num() - 1; i >= 0; --i)
		{
			FHitResult Result;
			POI->GetWorld()->LineTraceSingleByChannel(Result, POI->GetActorLocation() + UpOffset, POI->GetActorLocation() + Points[i] + UpOffset, ECC_Visibility);
			if (Result.bBlockingHit)
			{
				Points.RemoveAt(i);
			}
		}
	}
	
	FLocationCache Cache = FLocationCache(Points, POI->GetActorLocation(), true);
	return Cache;
}

void UPOITypeProcessor::MakeArrayLocal(TArray<FVector>& Array, const FVector& Origin) const
{
	for (auto& Vector : Array)
	{
		Vector -= Origin;
	}
}

#endif