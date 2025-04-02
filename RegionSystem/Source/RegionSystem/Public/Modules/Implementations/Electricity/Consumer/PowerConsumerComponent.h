#pragma once

#include "CoreMinimal.h"
#include "ElectricityConsumerInterface.h"
#include "RegionObjectInterface.h"
#include "UObject/Object.h"
#include "PowerConsumerComponent.generated.h"

class UGlobalReplicator;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConsumerStateChange, UPowerConsumerComponent*, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPowerConsumptionChange, UPowerConsumerComponent*, Component, float, OldConsumption, float, NewConsumption);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPowerConsumptionTypeChange, UPowerConsumerComponent*, Component, EElectricityConsumerType, OldConsumptionType, EElectricityConsumerType, NewConsumptionType);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REGIONSYSTEM_API UPowerConsumerComponent : public UActorComponent, public IElectricityConsumerInterface, public IRegionObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FConsumerStateChange OnGainPower;
	UPROPERTY(BlueprintAssignable)
	FConsumerStateChange OnLosePower;

	UPROPERTY(BlueprintAssignable)
	FPowerConsumptionChange OnConsumptionChange;
	UPROPERTY(BlueprintAssignable)
	FPowerConsumptionTypeChange OnConsumptionTypeChange;

	//IElectricityConsumerInterface
	virtual void GetPowerConsumptionData_Implementation(TArray<FPowerConsumerData>& OutConsumptionData) const override;
	virtual void OnGainPower_Implementation(EElectricityConsumerType InConsumerType) override;
	virtual void OnLosePower_Implementation(EElectricityConsumerType InConsumerType) override;
	//IElectricityConsumerInterface

	//IRegionObject
	virtual void ForceSetRegion_Implementation(FGameplayTag NewRegion) override;
	virtual void GetCheckData_Implementation(FVector& CheckLocation, ERegionTypes& DesiredType, bool& bOutDisableRuntimeChecks) const override;
	virtual FGameplayTag GetRegionTag_Implementation() const override;
	//IRegionObject

	UFUNCTION(BlueprintCallable, Category = "PowerConsumerComponent")
	bool WantsPower() const;
	UFUNCTION(BlueprintCallable, Category = "PowerConsumerComponent")
	bool HasPower() const;
	
	UFUNCTION(BlueprintCallable, Category = "PowerConsumerComponent")
	void TurnOn();
	UFUNCTION(BlueprintCallable, Category = "PowerConsumerComponent")
	void TurnOff();

	UFUNCTION(BlueprintCallable, Category = "PowerConsumerComponent")
	void ChangePowerConsumption(float NewPowerConsumption);
	UFUNCTION(BlueprintCallable, Category = "PowerConsumerComponent")
	void ChangePowerType(EElectricityConsumerType NewPowerType); //Not Replicated

protected:
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UFUNCTION()
	void WaitForGlobalReplicator();
	UFUNCTION(BlueprintNativeEvent, Category = "PowerConsumerComponent")
	void InitializeReplication(UGlobalReplicator* Replicator);

	//Replication
	UFUNCTION()
	void SyncReplicatedState();
	UFUNCTION()
	void SyncReplicatedConsumption();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerConsumer", meta = (Categories = "Regions.Modules.Electricity.ConsumerTypes", ExposeOnSpawn))
	EElectricityConsumerType ConsumerType = EElectricityConsumerType::None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerConsumer", meta = (ExposeOnSpawn))
	float PowerConsumption = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerConsumer", meta = (ExposeOnSpawn))
	bool bStartEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerConsumer|Replication", meta = (ExposeOnSpawn))
	bool bReplicateState = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerConsumer|Replication", meta = (ExposeOnSpawn))
	bool bReplicateConsumption = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerConsumer|Region", meta = (ExposeOnSpawn))
	bool bDisableRuntimeChecks = false;

	UPROPERTY(VisibleAnywhere, Category = "PowerConsumer")
	bool bHasPower;
	
	UPROPERTY(VisibleAnywhere, Category = "PowerConsumer|Region")
	FGameplayTag RegionTag;

private:
	
	UPROPERTY()
	bool bReplicatedConsumptionState;
	UPROPERTY()
	bool bPowerConsumptionDesired;

	UPROPERTY()
	float ReplicatedConsumption;
	
};