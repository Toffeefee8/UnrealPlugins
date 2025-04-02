#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_WaitForGlobalReplicator.generated.h"

class UGlobalReplicator;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitForGlobalReplicatorDelegate, UGlobalReplicator*, Replicator);

UCLASS()
class OBJECTEXTENSIONS_API UAsyncAction_WaitForGlobalReplicator : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
	
public:

	UPROPERTY(BlueprintAssignable)
	FWaitForGlobalReplicatorDelegate OnReplicatorFound;
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UAsyncAction_WaitForGlobalReplicator* WaitForGlobalReplicator(UObject* WorldContextObject);
	
	virtual void Activate() override;

private:

	UPROPERTY()
	TObjectPtr<UObject> WorldContext;
	
	void PollForReplicator();
};
