#pragma once

#include "CoreMinimal.h"
#include "ElectricityProviderInterface.h"
#include "RegionObjectInterface.h"
#include "Components/ActorComponent.h"
#include "PowerProviderComponent.generated.h"

class UGlobalReplicator;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProviderStateChange, UPowerProviderComponent*, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPowerProvisionChange, UPowerProviderComponent*, Component, float, OldProvision, float, NewProvision);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REGIONSYSTEM_API UPowerProviderComponent : public UActorComponent, public IElectricityProviderInterface, public IRegionObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FProviderStateChange OnEnable;
	UPROPERTY(BlueprintAssignable)
	FProviderStateChange OnDisable;

	UPROPERTY(BlueprintAssignable)
	FPowerProvisionChange OnConsumptionChange;

	//IElectricityProviderInterface
	virtual void GetPowerProviderData_Implementation(FPowerProviderData& OutProvisionData) override;
	//IElectricityProviderInterface

	//IRegionObject
	virtual void ForceSetRegion_Implementation(FGameplayTag NewRegion) override;
	virtual void GetCheckData_Implementation(FVector& CheckLocation, ERegionTypes& DesiredType, bool& bOutDisableRuntimeChecks) const override;
	virtual FGameplayTag GetRegionTag_Implementation() const override;
	//IRegionObject

	UFUNCTION(BlueprintCallable, Category = "PowerConsumerComponent")
	bool IsProvidingPower() const;

	UFUNCTION(BlueprintCallable, Category = "PowerProviderComponent")
	void TurnOn();
	UFUNCTION(BlueprintCallable, Category = "PowerProviderComponent")
	void TurnOff();

	UFUNCTION(BlueprintCallable, Category = "PowerProviderComponent")
	void ChangePowerProvision(float NewPowerProvision);

protected:

	//Startup
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UFUNCTION()
	void WaitForGlobalReplicator();
	UFUNCTION(BlueprintNativeEvent, Category = "PowerProviderComponent")
	void InitializeReplication(UGlobalReplicator* Replicator);

	//Replication
	UFUNCTION()
	void SyncReplicatedState();
	UFUNCTION()
	void SyncReplicatedProvision();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerProvider", meta = (ExposeOnSpawn))
	float ProvidedPower = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerProvider", meta = (ExposeOnSpawn))
	bool bStartEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerProvider|Replication", meta = (ExposeOnSpawn))
	bool bReplicateState = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerProvider|Replication", meta = (ExposeOnSpawn))
	bool bReplicateProvision = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerProvider|Region", meta = (ExposeOnSpawn))
	bool bDisableRuntimeChecks = false;

	UPROPERTY(VisibleAnywhere, Category = "PowerProvider|Region")
	FGameplayTag RegionTag;

private:

	UPROPERTY()
	bool bReplicatedProvisionState;
	UPROPERTY()
	bool bPowerProvisionEnabled;

	UPROPERTY()
	float ReplicatedProvidedPower;

};
