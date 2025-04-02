#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PropertyPackingLibrary.generated.h"

/**
 * 
 */
UCLASS()
class OBJECTEXTENSIONS_API UPropertyPackingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "Property Packing")
	static TArray<uint8> PackFloat(const float& InValue);

	UFUNCTION(BlueprintCallable, Category = "Property Packing")
	static float UnpackFloat(const TArray<uint8>& InValue);

	template<typename T>
	static void PackValue(const T& Value, TArray<uint8>& OutBytes);
	template<typename T>
	static void UnpackValue(const TArray<uint8>& Bytes, T& OutValue);
};

template<typename T>
inline void UPropertyPackingLibrary::PackValue(const T& Value, TArray<uint8>& OutBytes)
{
	OutBytes.Empty();
	FMemoryWriter Writer(OutBytes);
	Writer << const_cast<T&>(Value);
}

template<typename T>
inline void UPropertyPackingLibrary::UnpackValue(const TArray<uint8>& Bytes, T& OutValue)
{
	FMemoryReader Reader(Bytes);
	Reader << OutValue;
}