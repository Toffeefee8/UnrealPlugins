#pragma once

#include "CoreMinimal.h"
#include "ElectricityProviderType.h"
#include "UObject/Interface.h"
#include "ElectricityProviderInterface.generated.h"


USTRUCT(BlueprintType)
struct FPowerProviderData
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PowerProvision = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TotalPowerProvision = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bEnabled = false;

	// FUTURE
	// UPROPERTY(BlueprintReadWrite, EditAnywhere)
	// EElectricityProviderType ConsumerType = EElectricityProviderType::None;

	
	FString ToString() const
	{
		return FString::Printf(TEXT("PowerProvision: %f, TotalPowerProvision: %f, bEnabled: %s"), PowerProvision, TotalPowerProvision, bEnabled ? TEXT("true") : TEXT("false"));
	}
};

// This class does not need to be modified.
UINTERFACE()
class UElectricityProviderInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class REGIONSYSTEM_API IElectricityProviderInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Regions|Modules|Electricity|Provider")
	void GetPowerProviderData(FPowerProviderData& OutProviderData);
};
