#pragma once

#include "GenericData.generated.h"

USTRUCT(BlueprintType)
struct OBJECTEXTENSIONS_API FGenericData
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, Category = "Data")
	bool bBoolValue = false;
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	float FloatValue = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	int32 IntValue = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	FString StringValue  = "";
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	FVector VectorValue;
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	TObjectPtr<UObject> ObjectValue = nullptr;

	float GetAllNumbers() const;

	FString ToString() const;

};
