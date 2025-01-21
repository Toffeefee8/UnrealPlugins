#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectReplicator.generated.h"

class UReplicatedObject;

UCLASS(ClassGroup=("Object Replication"), meta=(BlueprintSpawnableComponent), Blueprintable, BlueprintType, DisplayName = "UObject Replicator")
class OBJECTEXTENSIONS_API UObjectReplicator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UObjectReplicator();
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Object Replication", DisplayName = "Register Replicated UObject")
	virtual bool RegisterReplicatedObject(UPARAM(DisplayName = "Replicated UObject") UReplicatedObject* ObjectToRegister);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Object Replication", DisplayName = "Unregister Replicated UObject")
	virtual bool UnregisterReplicatedObject(UPARAM(DisplayName = "Replicated UObject") UReplicatedObject* ObjectToUnregister, UPARAM(DisplayName = "Destroy Replicated UObject") bool bDestroyObject = false);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Object Replication", DisplayName = "Get Registered Replicated UObjects")
	virtual TArray<UReplicatedObject*> GetRegisteredReplicatedObjects()
	{
		return ReplicatedObjects;
	};

	
protected:
	UPROPERTY()
	TArray<UReplicatedObject*> ReplicatedObjects;
};
