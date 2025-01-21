#include "ReplicatedObject/ObjectReplicator.h"
#include "ReplicatedObject/ReplicatedObject.h"

UObjectReplicator::UObjectReplicator()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	
	bReplicateUsingRegisteredSubObjectList = true;
}

bool UObjectReplicator::RegisterReplicatedObject(UReplicatedObject* ObjectToRegister)
{
	if (IsValid(ObjectToRegister))
	{
		if (!ObjectToRegister->IsSupportedForNetworking())
			return false;
		
		ReplicatedObjects.AddUnique(ObjectToRegister);
		AddReplicatedSubObject(ObjectToRegister);
		return true;
	}

	return false;
}

bool UObjectReplicator::UnregisterReplicatedObject(UReplicatedObject* ObjectToUnregister, bool bDestroyObject)
{
	if (IsValid(ObjectToUnregister))
	{
		if (!ObjectToUnregister->IsSupportedForNetworking())
			return false;
		
		ReplicatedObjects.Remove(ObjectToUnregister);
		RemoveReplicatedSubObject(ObjectToUnregister);

		if (bDestroyObject)
		{
			ObjectToUnregister->DestroyObject();
		}
		
		return true;
	}

	return false;
}
