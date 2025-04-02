#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RegionObjectInterface.h"
#include "Components/PlayerStateComponent.h"
#include "RegionTracker.generated.h"


class URegion;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRegionChange, FGameplayTag, RegionTag);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REGIONSYSTEM_API URegionTracker : public UGameFrameworkComponent, public IRegionObject
{
	GENERATED_BODY()

	//Region Object Interface
	virtual void ForceSetRegion_Implementation(FGameplayTag NewRegion) override;
	virtual FGameplayTag GetRegionTag_Implementation() const override;
	virtual void GetCheckData_Implementation(FVector& CheckLocation, ERegionTypes& DesiredType, bool& bDisableRuntimeChecks) const override;
	//Region Object Interface

public:

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintAssignable, Category="Regions")
	FOnRegionChange OnRegionEnter;
	UPROPERTY(BlueprintAssignable, Category="Regions")
	FOnRegionChange OnRegionExit;
	
	UFUNCTION(BlueprintCallable)
	bool IsInRegion(FGameplayTag Tag) const;
	UFUNCTION(BlueprintCallable)
	FGameplayTagContainer GetRegionTags() const;
	UFUNCTION(BlueprintCallable)
	FGameplayTag GetContainingRegionTag() const;
	UFUNCTION(BlueprintCallable)
	FGameplayTag GetRegionTagOfType(ERegionTypes DesiredType) const;
	UFUNCTION(BlueprintCallable)
	TArray<URegion*> GetRegionRefs() const;
	UFUNCTION(BlueprintCallable)
	bool IsInValidRegion() const;

protected:

	friend URegion;

	UFUNCTION()
	void AddRegion(URegion* Region);
	UFUNCTION()
	void RemoveRegion(URegion* Region);
	UFUNCTION()
	FGameplayTag CalculateRelevantRegion() const;
	
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<URegion>> RegionRefs;
	UPROPERTY(Transient)
	mutable FGameplayTag CachedRegionTag;
	
};