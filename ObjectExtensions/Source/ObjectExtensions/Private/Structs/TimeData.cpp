#include "Structs/TimeData.h"

float UTimeHelpers::GetAsTime(FTimeData TimeData, ETimeMode TimeMode)
{
	return TimeData.GetAsTime(TimeMode);
}

void UTimeHelpers::SetAsTime(float& TimeFloat, FTimeData TimeData)
{
	TimeFloat = ConvertToSeconds(TimeData);
}

void UTimeHelpers::AddTime(float& TimeFloat, FTimeData TimeData)
{
	TimeFloat += ConvertToSeconds(TimeData);
}

float UTimeHelpers::ConvertToSeconds(const FTimeData TimeData)
{
	return TimeData.ConvertToSeconds();
}
