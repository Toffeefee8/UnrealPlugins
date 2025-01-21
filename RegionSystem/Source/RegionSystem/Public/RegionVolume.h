// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "RegionVolume.generated.h"

class USphereComponent;
class UTextRenderComponent;
class URegionEditorSubsystem;
class URegionSubsystem;
class UBoxComponent;

UCLASS(Blueprintable, PrioritizeCategories = ("Region"))
class REGIONSYSTEM_API ARegionVolume : public AActor
{
	GENERATED_BODY()

public:
	ARegionVolume();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	FGameplayTag GetRegionTag() const;
	UFUNCTION(BlueprintCosmetic)
	int32 GetRegionDepth() const;
	UFUNCTION(BlueprintCallable)
	bool IsParentRegion(UPARAM(meta=(Categories="Region")) FGameplayTag InRegionTag) const;
	UFUNCTION(BlueprintCallable)
	bool IsChildRegion(UPARAM(meta=(Categories="Region")) FGameplayTag InRegionTag) const;
	UFUNCTION(BlueprintCallable)
	bool IsRegionExact(UPARAM(meta=(Categories="Region")) FGameplayTag InRegionTag) const;

	UFUNCTION(BlueprintCallable)
	bool Contains(FVector Location) const;

	UFUNCTION(BlueprintCallable)
	FVector GetBoxExtent() const;
	UFUNCTION(BlueprintCallable)
	FVector GetCenterPoint() const;

#pragma region Operators
	friend bool operator<(const ARegionVolume& Lhs, const ARegionVolume& RHS)
	{
		if (Lhs.GetRegionDepth() == RHS.GetRegionDepth())
			return Lhs.Priority < RHS.Priority;
		
		return Lhs.GetRegionDepth() < RHS.GetRegionDepth();
	}

	friend bool operator<=(const ARegionVolume& Lhs, const ARegionVolume& RHS)
	{
		return !(RHS < Lhs);
	}

	friend bool operator>(const ARegionVolume& Lhs, const ARegionVolume& RHS)
	{
		return RHS < Lhs;
	}

	friend bool operator>=(const ARegionVolume& Lhs, const ARegionVolume& RHS)
	{
		return !(Lhs < RHS);
	}
#pragma endregion

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* RegionBox;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* RegionCenter;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTextRenderComponent* RegionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region", meta = (Categories = "Region"))
	FGameplayTag RegionTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region", meta = (ClampMin = 0))
	int Priority = 0;
	
	//Overlaps
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//Register
	UFUNCTION()
	void RegisterSelfWithSubsystem();
	UFUNCTION()
	void DeregisterSelfWithSubsystem();

	UFUNCTION()
	float GetTextScale() const;

private:

	friend URegionSubsystem;
	
	//Do Not Modify Value, For Region Subsystem to Handle
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bRegistered = false;

#pragma region Editor Tools
public:
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Region")
	void DisplayHierarchy();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Region")
	void ClearHighlights();
#endif
#pragma endregion

#pragma region Validation
public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
#pragma endregion
};
