#include "ReplicatedObject/AsyncAction_WaitForGlobalReplicator.h"

#include "ReplicatedObject/GlobalReplicator.h"

UAsyncAction_WaitForGlobalReplicator* UAsyncAction_WaitForGlobalReplicator::WaitForGlobalReplicator(UObject* WorldContextObject)
{
	UAsyncAction_WaitForGlobalReplicator* Node = NewObject<UAsyncAction_WaitForGlobalReplicator>();
	Node->WorldContext = WorldContextObject;
	return Node;
}

void UAsyncAction_WaitForGlobalReplicator::Activate()
{
	PollForReplicator();
}

void UAsyncAction_WaitForGlobalReplicator::PollForReplicator()
{
	if (!WorldContext)
	{
		return;
	}

	if (UGlobalReplicator* Replicator = UGlobalReplicator::Get(WorldContext))
	{
		OnReplicatorFound.Broadcast(Replicator);
		SetReadyToDestroy();
	}
	else
	{
		if (UWorld* World = WorldContext->GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(this, &UAsyncAction_WaitForGlobalReplicator::PollForReplicator);
		}
	}
}