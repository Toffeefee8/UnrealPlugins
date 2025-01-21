#pragma once

#include "ValueRangeFloat.generated.h"

USTRUCT(BlueprintType)
struct OBJECTEXTENSIONS_API FValueRangeFloat
{
	GENERATED_BODY()
	
#pragma region Constructors
	FValueRangeFloat() {	}
	
	FValueRangeFloat(float InValue)
	{
		bUseRange = false;
		FixedValue = InValue;
		MinValue = InValue * 0.8f;
		MaxValue = InValue * 1.2f;
	}
	
	FValueRangeFloat(float InMinValue, float InMaxValue)
	{
		bUseRange = true;
		MinValue = InMinValue;
		MaxValue = InMaxValue;
		FixedValue = (InMinValue + InMaxValue) / 2;
	}

	explicit FValueRangeFloat(bool UseValueRange)
	{
		bUseRange = UseValueRange;
	}
#pragma endregion

#pragma region Float Extention
	operator float() const
	{
		return GetValue();
	}
		
	friend FValueRangeFloat operator+(const FValueRangeFloat& ValueRange, float FloatValue)
	{
		return FValueRangeFloat(ValueRange.GetValue() + FloatValue);
	}

	friend FValueRangeFloat operator+(const FValueRangeFloat& ValueRangeA, const FValueRangeFloat& ValueRangeB)
	{
		return FValueRangeFloat(ValueRangeA.GetValue() + ValueRangeB.GetValue());
	}
	
	friend FValueRangeFloat operator+(float FloatValue, const FValueRangeFloat& ValueRange)
	{
		return FValueRangeFloat(FloatValue + ValueRange.GetValue());
	}

#pragma endregion
	
	//Functions
	float CalculateNewValue() const
	{
		if (bUseRange)
			LiveValue = FMath::RandRange(MinValue, MaxValue);
		else
			LiveValue = FixedValue;
		
		bValueSet = true;
		
		return GetValue();
	}
	bool GetValueRange(float &OutMin, float &OutMax) const
	{
		OutMin = Min();
		OutMax = Max();
		return bUseRange;
	}
	float GetValue() const
	{
		if (bValueSet)
			return LiveValue;

		return CalculateNewValue();
	}
	float Max() const
	{
		return bUseRange ? MaxValue : FixedValue;
	}
	float Min() const
	{
		return bUseRange ? MinValue : FixedValue;
	}
	bool Contains(float Value) const
	{
		return Value >= Min() && Value <= Max();
	}
	float Difference() const
	{
		return Max() - Min();
	}

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bUseRange = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="bUseRange==false", EditConditionHides))
	float FixedValue = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="bUseRange", EditConditionHides))
	float MinValue = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="bUseRange", EditConditionHides))
	float MaxValue = 2;

private:
	UPROPERTY()
	mutable bool bValueSet = false;

	UPROPERTY()
	mutable float LiveValue = 0;
};
