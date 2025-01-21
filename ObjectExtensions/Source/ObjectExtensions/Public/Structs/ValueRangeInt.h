#pragma once

#include "ValueRangeInt.generated.h"

USTRUCT(BlueprintType)
struct OBJECTEXTENSIONS_API FValueRangeInt
{
	GENERATED_BODY()

#pragma region Constructors
	FValueRangeInt() {	}
	
	FValueRangeInt(int InValue)
	{
		bUseRange = false;
		FixedValue = InValue;
		MinValue = FMath::TruncToInt(InValue * 0.8f);
		MaxValue = FMath::TruncToInt(InValue * 1.2f);
	}
	
	FValueRangeInt(int InMinValue, int InMaxValue)
	{
		bUseRange = true;
		MinValue = InMinValue;
		MaxValue = InMaxValue;
		FixedValue = FMath::TruncToInt((InMinValue + InMaxValue) / 2.f);
	}

	explicit FValueRangeInt(bool UseValueRange)
	{
		bUseRange = UseValueRange;
	}
#pragma endregion

#pragma region Int Extention
	operator int() const
	{
		return GetValue();
	}
		
	friend FValueRangeInt operator+(const FValueRangeInt& ValueRange, int IntValue)
	{
		return FValueRangeInt(ValueRange.GetValue() + IntValue);
	}

	friend FValueRangeInt operator+(const FValueRangeInt& ValueRangeA, const FValueRangeInt& ValueRangeB)
	{
		return FValueRangeInt(ValueRangeA.GetValue() + ValueRangeB.GetValue());
	}
	
	friend FValueRangeInt operator+(int IntValue, const FValueRangeInt& ValueRange)
	{
		return FValueRangeInt(IntValue + ValueRange.GetValue());
	}

#pragma endregion
	
	//Functions
	int CalculateNewValue() const
	{
		if (bUseRange)
			LiveValue = FMath::RandRange(MinValue, MaxValue);
		else
			LiveValue = FixedValue;
		
		bValueSet = true;
		
		return GetValue();
	}
	bool GetValueRange(int &OutMin, int &OutMax) const
	{
		OutMin = Min();
		OutMax = Max();
		return bUseRange;
	}
	int GetValue() const
	{
		if (bValueSet)
			return LiveValue;

		return CalculateNewValue();
	}
	int Max() const
	{
		return bUseRange ? MaxValue : FixedValue;
	}
	int Min() const
	{
		return bUseRange ? MinValue : FixedValue;
	}
	bool Contains(int Value) const
	{
		return Value >= Min() && Value <= Max();
	}
	int Difference() const
	{
		return Max() - Min();
	}

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bUseRange = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="bUseRange==false", EditConditionHides))
	int FixedValue = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="bUseRange", EditConditionHides))
	int MinValue = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="bUseRange", EditConditionHides))
	int MaxValue = 2;

private:
	UPROPERTY()
	mutable bool bValueSet = false;

	UPROPERTY()
	mutable int LiveValue = 0;
};
