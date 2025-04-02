#pragma once

#include "CoreMinimal.h"
#include "FuzeBoxInterface.h"
#include "RegionObjectInterface.h"
#include "Modules/Implementations/Electricity/ElectricityModule.h"
#include "UObject/Object.h"
#include "FuzeBoxComponent.generated.h"

class UGlobalReplicator;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFuzeBoxStateChange, UFuzeBoxComponent*, Component);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REGIONSYSTEM_API UFuzeBoxComponent : public UActorComponent, public IFuzeBoxInterface, public IRegionObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category="FuzeBox")
	FElectricityEvent OnConsumerTypePowerChange;
	UPROPERTY(BlueprintAssignable, Category="FuzeBox")
	FElectricityStateEvent OnStateChange;

	//IFuzeBoxInterface
	virtual void GetFuzeBoxData_Implementation(FFuzeBoxData& OutData) override;
	virtual void OnReceiveModule_Implementation(UElectricityModule* Module) override;
	//IFuzeBoxInterface
	
	//IRegionObject
	virtual void ForceSetRegion_Implementation(FGameplayTag NewRegion) override;
	virtual void GetCheckData_Implementation(FVector& OutCheckLocation, ERegionTypes& OutDesiredType, bool& bOutDisableRuntimeChecks) const override;
	virtual FGameplayTag GetRegionTag_Implementation() const override;
	//IRegionObject

	UFUNCTION(BlueprintCallable, Category = "FuzeBoxComponent")
	bool HasPowerModule() const;
	UFUNCTION(BlueprintCallable, Category = "FuzeBoxComponent")
	UElectricityModule* GetPowerModule() const;

	UFUNCTION(BlueprintCallable, Category = "FuzeBoxComponent")
	void Repair();
	UFUNCTION(BlueprintCallable, Category = "FuzeBoxComponent")
	void Break();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FuzeBox")
	bool bStartEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FuzeBox")
	float ActivationDelay = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FuzeBox")
	bool bIndependent = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FuzeBox|Region")
	ERegionTypes DesiredRegion = ERegionTypes::Room;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FuzeBox|Region")
	bool bDisableRuntimeChecks = false;
	
protected:
	
	UPROPERTY(VisibleAnywhere, Category = "FuzeBox|Region")
	FGameplayTag RegionTag;

	UPROPERTY()
	TWeakObjectPtr<UElectricityModule> PowerModule = nullptr;
};