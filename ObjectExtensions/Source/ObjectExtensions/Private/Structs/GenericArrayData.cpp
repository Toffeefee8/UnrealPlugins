#include "Structs/GenericArrayData.h"

void FGenericArrayData::SetBool(bool bValue, int Index)
{
	if (bBoolValues.IsValidIndex(Index))
	{
		bBoolValues[Index] = bValue;
	}
	else
	{
		bBoolValues.SetNum(Index + 1);
		bBoolValues[Index] = bValue;
	}
}

bool FGenericArrayData::GetBool(int Index) const
{
	if (bBoolValues.IsValidIndex(Index))
		return bBoolValues[Index];
	
	return false;
}

void FGenericArrayData::SetFloat(float Value, int Index)
{
    if (FloatValues.IsValidIndex(Index))
    {
        FloatValues[Index] = Value;
    }
    else
    {
        FloatValues.SetNum(Index + 1);
        FloatValues[Index] = Value;
    }
}

float FGenericArrayData::GetFloat(int Index) const
{
    if (FloatValues.IsValidIndex(Index))
        return FloatValues[Index];

    return 0.0f;
}

void FGenericArrayData::SetInt(int32 Value, int Index)
{
    if (IntValues.IsValidIndex(Index))
    {
        IntValues[Index] = Value;
    }
    else
    {
        IntValues.SetNum(Index + 1);
        IntValues[Index] = Value;
    }
}

int32 FGenericArrayData::GetInt(int Index) const
{
    if (IntValues.IsValidIndex(Index))
        return IntValues[Index];

    return 0;
}

void FGenericArrayData::SetString(const FString& Value, int Index)
{
    if (StringValues.IsValidIndex(Index))
    {
        StringValues[Index] = Value;
    }
    else
    {
        StringValues.SetNum(Index + 1);
        StringValues[Index] = Value;
    }
}

FString FGenericArrayData::GetString(int Index) const
{
    if (StringValues.IsValidIndex(Index))
        return StringValues[Index];

    return FString();
}

void FGenericArrayData::SetVector(const FVector& Value, int Index)
{
    if (VectorValues.IsValidIndex(Index))
    {
        VectorValues[Index] = Value;
    }
    else
    {
        VectorValues.SetNum(Index + 1);
        VectorValues[Index] = Value;
    }
}

FVector FGenericArrayData::GetVector(int Index) const
{
    if (VectorValues.IsValidIndex(Index))
        return VectorValues[Index];

    return FVector::ZeroVector;
}

void FGenericArrayData::SetObject(TObjectPtr<UObject> Value, int Index)
{
    if (ObjectValues.IsValidIndex(Index))
    {
        ObjectValues[Index] = Value;
    }
    else
    {
        ObjectValues.SetNum(Index + 1);
        ObjectValues[Index] = Value;
    }
}

TObjectPtr<UObject> FGenericArrayData::GetObject(int Index) const
{
    if (ObjectValues.IsValidIndex(Index))
        return ObjectValues[Index];

	return nullptr;
}

float FGenericArrayData::GetAllNumbers() const
{
	float AllNumbers = 0.0f;

	for (auto Element : FloatValues)
		AllNumbers += Element;

	for (auto Element : IntValues)
		AllNumbers += Element;

	return AllNumbers;
}

FString FGenericArrayData::ToString() const
{
	FString Result;

	// Convert bool array to string
	Result += "BoolValues: [";
	for (const bool& BoolVal : bBoolValues)
	{
		Result += BoolVal ? "true" : "false";
		Result += ", ";
	}
	Result.RemoveFromEnd(", ");
	Result += "]\n";

	// Convert float array to string
	Result += "FloatValues: [";
	for (const float& FloatVal : FloatValues)
	{
		Result += FString::SanitizeFloat(FloatVal);
		Result += ", ";
	}
	Result.RemoveFromEnd(", ");
	Result += "]\n";

	// Convert int array to string
	Result += "IntValues: [";
	for (const int32& IntVal : IntValues)
	{
		Result += FString::FromInt(IntVal);
		Result += ", ";
	}
	Result.RemoveFromEnd(", ");
	Result += "]\n";

	// Convert FString array to string
	Result += "StringValues: [";
	for (const FString& StrVal : StringValues)
	{
		Result += StrVal;
		Result += ", ";
	}
	Result.RemoveFromEnd(", ");
	Result += "]\n";

	// Convert FVector array to string
	Result += "VectorValues: [";
	for (const FVector& VecVal : VectorValues)
	{
		Result += VecVal.ToString();
		Result += ", ";
	}
	Result.RemoveFromEnd(", ");
	Result += "]\n";

	// Convert UObject array to string
	Result += "ObjectValues: [";
	for (const TObjectPtr<UObject>& ObjVal : ObjectValues)
	{
		Result += ObjVal ? ObjVal->GetName() : "null";
		Result += ", ";
	}
	Result.RemoveFromEnd(", ");
	Result += "]\n";

	return Result;
}
