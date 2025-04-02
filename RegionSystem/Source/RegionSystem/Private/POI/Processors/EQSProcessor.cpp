#include "POI/Processors/EQSProcessor.h"

#include "EnvironmentQuery/EnvQueryManager.h"

#if WITH_EDITOR
void UEQSProcessor::CalculateRelevantPoints_Implementation(ARegionPOI* POI, TArray<FVector>& OutPoints, bool& OutIsLocal) const
{
	FEnvQueryRequest QueryRequest (EnvQuery, POI);
	TSharedPtr<FEnvQueryResult> Result = UEnvQueryManager::GetCurrent(this)->RunInstantQuery(QueryRequest, EEnvQueryRunMode::AllMatching);
	
	if(Result.IsValid() && !Result->IsAborted() && Result->Items.Num() > 0)
	{
		if(Result.IsValid() && !Result->IsAborted() && Result->Items.Num() > 0)
		{
			for(int32 i = 0; i < Result->Items.Num(); i++)
			{
				OutPoints.Add(Result->GetItemAsLocation(i));
				OutIsLocal = false;
			}
		}
	}
}
#endif