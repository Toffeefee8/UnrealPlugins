#pragma once

#include "TimeData.generated.h"

UENUM(Blueprintable)
enum class ETimeMode : uint8
{
	Seconds = 0,
	Minutes = 2,
	Hours = 3,
	Milliseconds = 1,
};

USTRUCT(Blueprintable)
struct OBJECTEXTENSIONS_API FTimeData
{
	GENERATED_BODY()

public:

	FTimeData() { }
	FTimeData(float InTime, ETimeMode InTimeMode)
	{
		this->Time = InTime;
		this->TimeMode = InTimeMode;
	}
	FTimeData(float InTime)
	{
		this->Time = InTime;
		this->TimeMode = ETimeMode::Seconds;
	}
	FTimeData(ETimeMode InTimeMode)
	{
		this->Time = 1;
		this->TimeMode = InTimeMode;
	}
	
	float GetAsTime(ETimeMode NewTimeMode) const
	{
		switch (NewTimeMode) {
		case ETimeMode::Seconds:
			return ConvertToSeconds();
		case ETimeMode::Minutes:
			return ConvertToSeconds()/60;
		case ETimeMode::Hours:
			return ConvertToSeconds()/3600;
		case ETimeMode::Milliseconds:
			return ConvertToSeconds()*1000;
		}
		return -1;
	}
	float ConvertToSeconds() const
	{
		switch (TimeMode)
		{
		case ETimeMode::Milliseconds:
			return Time * 0.001f;
		case ETimeMode::Seconds:
			return Time;
		case ETimeMode::Minutes:
			return Time * 60;
		case ETimeMode::Hours:
			return Time * 3600;
		default:
			break;
		}
		return -1;
	}

	operator float() const
	{
		return ConvertToSeconds();
	}

protected:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = 0))
	float Time = 1;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ETimeMode TimeMode = ETimeMode::Seconds;

	friend uint32 GetTypeHash(const FTimeData& Data);
};

FORCEINLINE uint32 GetTypeHash(const FTimeData& Data)
{
	uint32 Hash = GetTypeHash(Data.Time);
	Hash = HashCombine(Hash, GetTypeHash(static_cast<uint8>(Data.TimeMode)));
	return Hash;
}


UCLASS()
class OBJECTEXTENSIONS_API UTimeHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetAsTime(FTimeData TimeData, ETimeMode TimeMode);

	UFUNCTION(BlueprintCallable)
	static void SetAsTime(UPARAM(ref) float& TimeFloat, FTimeData TimeData);

	UFUNCTION(BlueprintCallable)
	static void AddTime(UPARAM(ref) float& TimeFloat, FTimeData TimeData);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float ConvertToSeconds(const FTimeData TimeData);
};
