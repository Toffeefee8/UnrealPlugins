#include "Structs/DistanceData.h"

float UDistanceHelpers::GetAsDistance(FDistanceData DistanceData, EDistanceType DistanceType)
{
	return DistanceData.GetAsDistance(DistanceType);
}

void UDistanceHelpers::SetAsDistance(float& DistanceFloat, FDistanceData DistanceData)
{
	DistanceFloat = ConvertToCentimeters(DistanceData);
}

void UDistanceHelpers::AddDistance(float& DistanceFloat, FDistanceData DistanceData)
{
	DistanceFloat += ConvertToCentimeters(DistanceData);
}

float UDistanceHelpers::ConvertToCentimeters(const FDistanceData DistanceData)
{
	return DistanceData.ConvertToCentimeters();
}
