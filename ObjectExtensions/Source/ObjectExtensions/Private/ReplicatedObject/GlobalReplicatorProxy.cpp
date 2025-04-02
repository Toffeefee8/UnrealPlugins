#include "ReplicatedObject/GlobalReplicatorProxy.h"

#include "ReplicatedObject/GlobalReplicator.h"

UGlobalReplicatorProxy::UGlobalReplicatorProxy()
{
	SetIsReplicatedByDefault(true);
}

void UGlobalReplicatorProxy::Server_ForwardChangeValue_Implementation(FReplicatedKey ReplicationKey, const TArray<uint8>& NewData)
{
	if (UGlobalReplicator* Replicator = UGlobalReplicator::Get(this))
	{
		Replicator->Server_UpdateData(ReplicationKey, NewData, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGlobalReplicatorProxy: GlobalReplicator component not found."));
	}
}

void UGlobalReplicatorProxy::Server_ForwardRequestValue_Implementation(FName ReplicationKey)
{
	if (UGlobalReplicator* Replicator = UGlobalReplicator::Get(this))
	{
		Replicator->Server_RequestData(ReplicationKey);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGlobalReplicatorProxy: GlobalReplicator component not found."));
	}
}

void UGlobalReplicatorProxy::Server_ForwardDereplicate_Implementation(FName ReplicationKey)
{
	if (UGlobalReplicator* Replicator = UGlobalReplicator::Get(this))
	{
		Replicator->Server_DereplicateData(ReplicationKey);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGlobalReplicatorProxy: GlobalReplicator component not found."));
	}
}
