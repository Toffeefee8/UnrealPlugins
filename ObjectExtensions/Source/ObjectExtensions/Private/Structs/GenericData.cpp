#include "Structs/GenericData.h"

float FGenericData::GetAllNumbers() const
{
	return FloatValue + IntValue;
}

FString FGenericData::ToString() const
{
	FString BoolString = bBoolValue ? TEXT("true") : TEXT("false");
	FString ObjectName = ObjectValue ? ObjectValue->GetName() : TEXT("null");

	return FString::Printf(TEXT("BoolValue: %s, FloatValue: %f, IntValue: %d, StringValue: %s, TransformValue: %s, ObjectValue: %s"),
		*BoolString,
		FloatValue,
		IntValue,
		*StringValue,
		*VectorValue.ToString(),
		*ObjectName);
}
