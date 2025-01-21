

#include "Structs/Probability.h"

void FProbability::SetProbability(double InProbability, double MaxValue)
{
	Probability = InitialProbability = FMath::Clamp(InProbability / MaxValue, 0.f, 1.f);
}

double FProbability::GetProbability(double RangeMax) const
{
	return GetProbabilityClamped() * RangeMax;
}

double FProbability::GetPercent(int Decimals) const
{
	double ScaleFactor = pow(10, Decimals);
	return FMath::TruncToDouble(Probability * 100 * ScaleFactor) / ScaleFactor;
}

bool FProbability::Evaluate() const
{
	return EvaluateBySeed(FRandomStream(FMath::Rand()));
}

bool FProbability::EvaluateBySeed(FRandomStream Seed) const
{
	if (Probability == 1)
		return true;
	if (Probability == 0)
		return false;
		
	return Seed.GetFraction() < Probability;
}

void FProbability::ResetProbability()
{
	Probability = InitialProbability;
}

void FProbability::Modify(float Modifier, EModificationType ModType)
{
	switch (ModType) {
		case Addition:
			Probability += Modifier;
			break;
		case Multiplication:
			Probability *= Modifier;
			break;
		case Power:
			pow(Probability, Modifier);
			break;
	}
}

double FProbability::GetProbabilityClamped() const
{
	return FMath::Clamp(Probability, 0, 1);
}
