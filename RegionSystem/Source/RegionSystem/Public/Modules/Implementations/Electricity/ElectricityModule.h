#pragma once

#include "CoreMinimal.h"
#include "Consumer/ElectricityConsumerInterface.h"
#include "Provider/ElectricityProviderInterface.h"
#include "GameplayTagContainer.h"
#include "FuzeBox/FuzeBoxInterface.h"
#include "Modules/RegionModule.h"
#include "ElectricityModule.generated.h"

class UGlobalReplicator;

USTRUCT(BlueprintType)
struct FConsumerBundledData
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PowerConsumption = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TotalPowerConsumption = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<TObjectPtr<UObject>, bool> RegisteredPowerConsumers {};
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct FProviderBundledData
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PowerProvision = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TotalPowerProvision = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<TObjectPtr<UObject>, bool> RegisteredPowerProviders {};
};

USTRUCT(BlueprintType)
struct FFuzeBoxBundledData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FFuzeBoxData CachedData {};
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UObject> FuzeBox = nullptr;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FTimerHandle ActivationTimerHandle {};
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FTimerHandle DeactivationTimerHandle {};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FElectricityEvent, UElectricityModule*, Module, EElectricityConsumerType, ConsumerType, bool, bEnable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FElectricityStateEvent, UElectricityModule*, Module, bool, bEnable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FConsumptionChangeEvent, UElectricityModule*, Module, EElectricityConsumerType, ConsumerType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProvisionChangeEvent, UElectricityModule*, Module);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FElectricityUserEvent, UElectricityModule*, Module);

UCLASS(Blueprintable)
class REGIONSYSTEM_API UElectricityModule : public URegionModule, public IElectricityProviderInterface, public IElectricityConsumerInterface
{
	GENERATED_BODY()

public:

	//Static Helpers
	UFUNCTION(BlueprintCallable, Category = "Regions|Modules|Electricity", meta = (WorldContext = "WorldContextObject"))
	static void TryReevaluatePowerConsumers(UObject* WorldContextObject, FGameplayTag RegionTag, EElectricityConsumerType ConsumerType);
	UFUNCTION(BlueprintCallable, Category = "Regions|Modules|Electricity", meta = (WorldContext = "WorldContextObject"))
	static void TryRefreshConsumerData(UObject* WorldContextObject, FGameplayTag RegionTag, UObject* Consumer);
	UFUNCTION(BlueprintCallable, Category = "Regions|Modules|Electricity", meta = (WorldContext = "WorldContextObject"))
	static void TryRefreshPowerProviderData(UObject* WorldContextObject, FGameplayTag RegionTag);

	//IElectricityConsumerInterface
	virtual void GetPowerConsumptionData_Implementation(TArray<FPowerConsumerData>& OutConsumptionData) const override;
	virtual void OnGainPower_Implementation(EElectricityConsumerType ConsumerType) override;
	virtual void OnLosePower_Implementation(EElectricityConsumerType ConsumerType) override;
	//IElectricityConsumerInterface

	//IElectricityProviderInterface
	virtual void GetPowerProviderData_Implementation(FPowerProviderData& OutProvisionData) override;
	//IElectricityProviderInterface

	//Region Extensions
	virtual void StartModule_Implementation(URegion* OwningRegion) override;
	virtual void EndModule_Implementation() override;
	virtual void RefreshModule_Implementation() override;
	virtual void NewParent_Implementation(URegion* OldParentRegion, URegion* NewParentRegion) override;
	//Region Extensions

	UPROPERTY(BlueprintAssignable, Category="Regions|Modules|Electricity")
	FElectricityEvent OnConsumerTypePowerChange;
	UPROPERTY(BlueprintAssignable, Category="Regions|Modules|Electricity")
	FElectricityStateEvent OnStateChange;

	UPROPERTY(BlueprintAssignable, Category="Regions|Modules|Electricity")
	FConsumptionChangeEvent OnConsumptionChange;
	UPROPERTY(BlueprintAssignable, Category="Regions|Modules|Electricity")
	FProvisionChangeEvent OnProvisionChange;

	UPROPERTY(BlueprintAssignable, Category="Regions|Modules|Electricity")
	FElectricityUserEvent OnChangePowerConsumers;
	UPROPERTY(BlueprintAssignable, Category="Regions|Modules|Electricity")
	FElectricityUserEvent OnChangePowerProviders;

	//Consumers
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity", meta = (AdvancedDisplay = 1))
	void ActivateType(EElectricityConsumerType ConsumerType, bool bRepairModule = false);
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity")
	void DeactivateType(EElectricityConsumerType ConsumerType);
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity", meta = (AdvancedDisplay = 1))
	void ActivateTypes(TArray<EElectricityConsumerType> ConsumerTypes, bool bRepairModule = false);
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity")
	void DeactivateTypes(TArray<EElectricityConsumerType> ConsumerTypes);
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity")
	void ActivateAllTypes();
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity")
	void DeactivateAllTypes();
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity")
	void FullActivate();
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity")
	void FullDeactivate();

	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	float GetPowerConsumption() const;
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	float GetTotalPowerConsumption() const;
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	float GetPowerConsumptionOfType(EElectricityConsumerType ConsumerType) const;
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	float GetTotalPowerConsumptionOfType(EElectricityConsumerType ConsumerType) const;
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	bool IsTypeEnabled(EElectricityConsumerType ConsumerType) const;
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	bool IsTypeEnabledIgnoreState(EElectricityConsumerType ConsumerType) const;
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	bool AreTypesEnabled(TArray<EElectricityConsumerType> ConsumerType) const;

	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	TArray<EElectricityConsumerType> GetAllConsumerTypes() const;

	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	void ReevaluatePowerConsumersMulti(TArray<EElectricityConsumerType> ConsumerTypes);
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	void ReevaluatePowerConsumers(EElectricityConsumerType ConsumerType = EElectricityConsumerType::None);
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	void RefreshConsumerData(UObject* Consumer);
	
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	void RegisterPowerConsumer(UObject* Consumer);
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Consumers")
	void DeregisterPowerConsumer(UObject* Consumer);

	//Providers
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Providers")
	float GetPowerProvisions() const;
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Providers")
	float GetTotalPowerProvisions() const;
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Providers")
	bool IsProvidingPower() const;

	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Providers")
	void RefreshPowerProviderData();

	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Providers")
	void RegisterPowerProvider(UObject* Provider);
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|Providers")
	void DeregisterPowerProvider(UObject* Provider);

	//FuzeBox
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity")
	void Repair();
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity")
	void Break();
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity")
	bool IsRepaired() const;
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity")
	bool IsBroken() const;
	
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|FuzeBox")
	bool SetFuzeBox(UObject* FuzeBox);

	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity|FuzeBox")
	void RefreshFuzeData();

	//Checks
	UFUNCTION(BlueprintCallable, Category="Regions|Modules|Electricity")
	bool CanActivate(EElectricityConsumerType ConsumerType) const;
	

protected:
	
	UFUNCTION(Category="Regions|Modules|Electricity")
	void InternalDeactivate(EElectricityConsumerType ConsumerType);
	UFUNCTION(Category="Regions|Modules|Electricity")
	void InternalActivate(EElectricityConsumerType ConsumerType);
	UFUNCTION(Category="Regions|Modules|Electricity")
	void NotifyParentAboutConsumerChange(bool bAllowLocalReevaluation = true);
	UFUNCTION(Category="Regions|Modules|Electricity")
	void NotifyParentAboutProviderChange(bool bAllowLocalReevaluation = true);
	UFUNCTION(Category="Regions|Modules|Electricity")
	void ReevaluateState();

	//Consumers
	UFUNCTION(Category="Regions|Modules|Electricity|Consumers")
	FConsumerBundledData GetNewDefaultBundledData(EElectricityConsumerType Type) const;
	UFUNCTION(Category="Regions|Modules|Electricity|Consumers")
	void SetupBundledConsumerDatas();

	//FuzeBox
	UFUNCTION(Category="Regions|Modules|Electricity|FuzeBox")
	bool HasFuzeBox() const;
	UFUNCTION(Category="Regions|Modules|Electricity|FuzeBox")
	float GetActivationDelay() const;
	UFUNCTION(Category="Regions|Modules|Electricity|FuzeBox")
	bool HasActivationDelay() const;
	UFUNCTION(Category="Regions|Modules|Electricity|FuzeBox")
	bool IsIndependent() const;

protected:

	//Consumers
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<EElectricityConsumerType, FConsumerBundledData> ConsumerDataByType {};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<TObjectPtr<UObject>, FPowerConsumerDataArray> RegisteredPowerConsumers {};

	//Providers
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FProviderBundledData ProviderData;

	//FuzeBox
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FFuzeBoxBundledData FuzeBoxData;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bFuze = true;

	//Caches
	UPROPERTY()
	TWeakObjectPtr<UElectricityModule> PreviousParentModule;

	//Replication
	UFUNCTION()
	void TryInitializeReplication();
	UFUNCTION()
	void OnInitializeReplication(UGlobalReplicator* Replicator);
	UFUNCTION()
	void OnRep_ReplicatedState();
	
	void UnpackReplicatedState(TMap<EElectricityConsumerType, bool>& ConsumerTypeStates, bool& bState) const;
	int PackReplicatedState() const;
	int16 GetTypePackedIndex(EElectricityConsumerType ConsumerType) const;
	FName GetReplicationKey() const;
	
	UPROPERTY()
	int ReplicatedState = 0;
};