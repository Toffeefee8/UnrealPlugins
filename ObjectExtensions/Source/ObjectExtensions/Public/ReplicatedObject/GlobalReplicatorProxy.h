#pragma once

#include "CoreMinimal.h"
#include "GlobalReplicator.h"
#include "Components/ActorComponent.h"
#include "GlobalReplicatorProxy.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OBJECTEXTENSIONS_API UGlobalReplicatorProxy : public UActorComponent
{
	GENERATED_BODY()

public:
	UGlobalReplicatorProxy();
	
	UFUNCTION(Server, Reliable)
	void Server_ForwardChangeValue(FReplicatedKey ReplicationKey, const TArray<uint8>& NewData);
	
	UFUNCTION(Server, Reliable)
	void Server_ForwardRequestValue(FName ReplicationKey);

	UFUNCTION(Server, Reliable)
	void Server_ForwardDereplicate(FName ReplicationKey);
};
