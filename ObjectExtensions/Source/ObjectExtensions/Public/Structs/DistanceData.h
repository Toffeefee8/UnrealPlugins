#pragma once

#include "DistanceData.generated.h"

UENUM(Blueprintable)
enum class EDistanceType : uint8
{
	Centimeters = 0,
	Meters = 2,
	Kilometers = 3,
	Millimeters = 1,
};

USTRUCT(Blueprintable)
struct OBJECTEXTENSIONS_API FDistanceData
{
	GENERATED_BODY()

public:

	FDistanceData() { }
	FDistanceData(float InDistance, EDistanceType InDistanceType)
	{
		this->Distance = InDistance;
		this->DistanceType = InDistanceType;
	}
	FDistanceData(float InDistance)
	{
		this->Distance = InDistance;
		this->DistanceType = EDistanceType::Centimeters;
	}
	FDistanceData(EDistanceType InDistanceType)
	{
		this->Distance = 1;
		this->DistanceType = InDistanceType;
	}
	
	float GetAsDistance(EDistanceType NewDistanceType) const
	{
		switch (NewDistanceType) {
		case EDistanceType::Centimeters:
			return ConvertToCentimeters();
		case EDistanceType::Meters:
			return ConvertToCentimeters()/100;
		case EDistanceType::Kilometers:
			return ConvertToCentimeters()/100000;
		case EDistanceType::Millimeters:
			return ConvertToCentimeters()*10;
		}
		return -1;
	}
	float ConvertToCentimeters() const
	{
		switch (DistanceType)
		{
		case EDistanceType::Millimeters:
			return Distance * 0.1f;
		case EDistanceType::Centimeters:
			return Distance;
		case EDistanceType::Meters:
			return Distance * 100;
		case EDistanceType::Kilometers:
			return Distance * 100000;
		default:
			break;
		}
		return -1;
	}

	operator float() const
	{
		return ConvertToCentimeters();
	}

protected:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = 0))
	float Distance = 1;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EDistanceType DistanceType = EDistanceType::Centimeters;
	
	friend uint32 GetTypeHash(const FDistanceData& Data);
};

FORCEINLINE uint32 GetTypeHash(const FDistanceData& Data)
{
	uint32 Hash = GetTypeHash(Data.Distance);
	Hash = HashCombine(Hash, GetTypeHash(static_cast<uint8>(Data.DistanceType)));
	return Hash;
}

UCLASS()
class OBJECTEXTENSIONS_API UDistanceHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetAsDistance(FDistanceData DistanceData, EDistanceType DistanceType);

	UFUNCTION(BlueprintCallable)
	static void SetAsDistance(UPARAM(ref) float& DistanceFloat, FDistanceData DistanceData);

	UFUNCTION(BlueprintCallable)
	static void AddDistance(UPARAM(ref) float& DistanceFloat, FDistanceData DistanceData);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float ConvertToCentimeters(const FDistanceData DistanceData);
};
