#pragma once

#include "Probability.generated.h"

USTRUCT(BlueprintType)
struct OBJECTEXTENSIONS_API FProbability
{
	GENERATED_BODY()

public:
	enum EModificationType
	{
		Addition,
		Multiplication,
		Power
	};
	
	FProbability()
	{
		Probability = InitialProbability;
	}
	FProbability(double InProbability)
	{
		Probability = InitialProbability = InProbability;
	}

	//Get / Set
	void SetProbability(double InProbability, double MaxValue = 1);
	double GetProbability(double RangeMax = 1) const;
	double GetPercent(int Decimals = 2) const;

	//Evaluate 
	bool Evaluate() const;
	bool EvaluateBySeed(FRandomStream Seed) const;

	void ResetProbability();
	void Modify(float Modifier, EModificationType ModType = Multiplication);

	operator double() const
	{
		return Probability;
	}

	FProbability& operator*=(FProbability Value)
	{
		Probability *= Value;
		return *this;
	}

protected:

	UPROPERTY(BlueprintReadOnly)
	double Probability = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1, NoAutoSize))
	double InitialProbability = 1.f;

	double GetProbabilityClamped() const;
	
};
