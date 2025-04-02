#include "Save/PropertyPackingLibrary.h"

TArray<uint8> UPropertyPackingLibrary::PackFloat(const float& InValue)
{
	TArray<uint8> OutBytes;
	PackValue<float>(InValue, OutBytes);
	return OutBytes;
}

float UPropertyPackingLibrary::UnpackFloat(const TArray<uint8>& InValue)
{
	float OutValue = 0.f;
	UnpackValue<float>(InValue, OutValue);
	return OutValue;
}
