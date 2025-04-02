#pragma once

#include "CoreMinimal.h"
#include "ElectricityConsumerType.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "ElectricityConsumerInterface.generated.h"

USTRUCT(BlueprintType)
struct FPowerConsumerData
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PowerConsumption = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TotalPowerConsumption = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Categories = "Regions.Modules.Electricity.ConsumerTypes"))
	EElectricityConsumerType ConsumerType = EElectricityConsumerType::None;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bEnabled = false;

	
	FString ToString() const
	{
		return FString::Printf(TEXT("PowerConsumption: %f, TotalPowerConsumption: %f, ConsumerType: %s, bEnabled: %s"),
			PowerConsumption, TotalPowerConsumption, *UEnum::GetValueAsString(ConsumerType), bEnabled ? TEXT("true") : TEXT("false"));
	}
};


USTRUCT(BlueprintType)
struct FPowerConsumerDataArray
{
	GENERATED_BODY()

public:
	FPowerConsumerDataArray() {  }
	FPowerConsumerDataArray(const TArray<FPowerConsumerData>& InData) : Data(InData) {  }

	void GetDiff(TArray<FPowerConsumerData>& OutNew, 
				 TArray<FPowerConsumerData>& OutRemoved, 
				 TArray<FPowerConsumerData>& OutUnchanged, 
				 const TArray<FPowerConsumerData>& OtherData)
	{
		// Clear the output arrays.
		OutNew.Empty();
		OutRemoved.Empty();
		OutUnchanged.Empty();
    
		// Build a set of ConsumerType tags from the old data (Data array).
		TSet<EElectricityConsumerType> OldTags;
		for (const FPowerConsumerData& OldItem : Data)
		{
			OldTags.Add(OldItem.ConsumerType);
		}
    
		// Build a set of ConsumerType tags from the new data (OtherData array).
		TSet<EElectricityConsumerType> NewTags;
		for (const FPowerConsumerData& NewItem : OtherData)
		{
			NewTags.Add(NewItem.ConsumerType);
		}
    
		// Process the new data: determine which items are new vs. unchanged.
		for (const FPowerConsumerData& NewItem : OtherData)
		{
			if (OldTags.Contains(NewItem.ConsumerType))
			{
				OutUnchanged.Add(NewItem);
			}
			else
			{
				OutNew.Add(NewItem);
			}
		}
    
		// Process the old data: determine which items were removed.
		for (const FPowerConsumerData& OldItem : Data)
		{
			if (!NewTags.Contains(OldItem.ConsumerType))
			{
				OutRemoved.Add(OldItem);
			}
		}
	}
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FPowerConsumerData> Data {};
};

// This class does not need to be modified.
UINTERFACE()
class UElectricityConsumerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class REGIONSYSTEM_API IElectricityConsumerInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Regions|Modules|Electricity|Consumer")
	void GetPowerConsumptionData(TArray<FPowerConsumerData>& OutConsumptionData) const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Regions|Modules|Electricity|Consumer")
	void OnGainPower(EElectricityConsumerType ConsumerType);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Regions|Modules|Electricity|Consumer")
	void OnLosePower(EElectricityConsumerType ConsumerType);
};
