#pragma once

#include "GenericArrayData.generated.h"

USTRUCT(BlueprintType)
struct OBJECTEXTENSIONS_API FGenericArrayData
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, Category = "Data")
	TArray<bool> bBoolValues;
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	TArray<float> FloatValues;
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	TArray<int32> IntValues;
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	TArray<FString> StringValues;
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	TArray<FVector> VectorValues;
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	TArray<TObjectPtr<UObject>> ObjectValues;

	void SetBool(bool bValue, int Index);
	bool GetBool(int Index) const;

	void SetFloat(float Value, int Index);
	float GetFloat(int Index) const;

	void SetInt(int32 Value, int Index);
	int32 GetInt(int Index) const;

	void SetString(const FString& Value, int Index);
	FString GetString(int Index) const;

	void SetVector(const FVector& Value, int Index);
	FVector GetVector(int Index) const;

	void SetObject(TObjectPtr<UObject> Value, int Index);
	TObjectPtr<UObject> GetObject(int Index) const;
	
	float GetAllNumbers() const;

	FString ToString() const;

};
