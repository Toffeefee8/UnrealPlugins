#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "ReplicatedObject.generated.h"

// IMPORTANT!
// References to replicated objects should not be held on clients as these are invalidated when changing
// object replicators and thus their outers.


UENUM(BlueprintType)
enum EReplicationState
{
	Replicates,
	DoNotReplicate,
};

UCLASS(Blueprintable, BlueprintType, Abstract)
class OBJECTEXTENSIONS_API UReplicatedObject : public UObject
{
	GENERATED_BODY()

public:
	
	//UObject interface implementation
	virtual UWorld* GetWorld() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Replicated UObject")
	bool HasAuthority() const;
	
	//Will mark this UObject as garbage and will eventually get cleaned by the garbage collector.
	//Should only execute this on the server.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Replicated UObject", DisplayName = "Destroy Replicated UObject")
	void DestroyObject();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Replicated UObject")
	AActor* GetOwningActor() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Replicated UObject")
	APlayerController* GetOwningController() const;
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Replicated UObject")
	void SetOuter(AActor* NewOwner);
	
protected:

	UFUNCTION(BlueprintNativeEvent, Category = "Tick")
	void OnTick(float DeltaTime);

	//Will get called on the server once the Replicated UObject has been marked as garbage.
	UFUNCTION(BlueprintNativeEvent, Category = "Destructor")
	void OnExplicitDestroyed();

	//Only called with World Context and Not a template
	UFUNCTION(BlueprintNativeEvent, Category = "Constructor")
	void OnConstruct();

	//Called every time object is constructed, even in editor
	UFUNCTION(BlueprintNativeEvent, Category = "Constructor")
	void PreConstruct();
	
	virtual void BeginDestroy() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

	//Is this UObject replicated?
	UPROPERTY()
	bool bShowReplicationState = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replicated UObject", meta = (EditCondition = "bShowReplicationState", EditConditionHides))
	TEnumAsByte<EReplicationState> ReplicationState = EReplicationState::Replicates;

	UPROPERTY()
	bool bShowCanEverTick = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replicated UObject", meta = (EditCondition = "bShowCanEverTick", EditConditionHides))
	bool CanEverTick = false;

private:
	
	FTSTicker::FDelegateHandle TickDelegateHandle;

	UFUNCTION()
	bool InternalTick(float DeltaTime);
};