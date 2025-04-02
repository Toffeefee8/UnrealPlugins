#include "ObjectWrapper/ObjectWrapperArray.h"
#include "Extensions/ArrayAndMapExtensions.h"

FObjectWrapper* FObjectWrapperArray::GetObjectWrapper(const TSubclassOf<UObject>& ShardClass, bool AllowChildClass)
{
	for (auto& ShardWrapper : ObjectWrappers)
	{
		if (!ShardWrapper.IsValid())
			continue;

		if (ShardWrapper.IsClass(ShardClass, AllowChildClass))
			return &ShardWrapper;
	}

	return nullptr;
}

FObjectWrapper FObjectWrapperArray::GetObjectWrapper(const TSubclassOf<UObject>& ShardClass, bool AllowChildClass) const
{
	for (auto& ShardWrapper : ObjectWrappers)
	{
		if (!ShardWrapper.IsValid())
			continue;

		if (ShardWrapper.IsClass(ShardClass, AllowChildClass))
			return ShardWrapper;
	}

	return FObjectWrapper();
}

TArray<FObjectWrapper*> FObjectWrapperArray::GetObjectWrappers(const TSubclassOf<UObject>& ShardClass, bool AllowChildClass)
{
	TArray<FObjectWrapper*> ReturnArray;
	for (auto& ShardWrapper : ObjectWrappers)
	{
		if (!ShardWrapper.IsValid())
			continue;

		if (ShardWrapper.IsClass(ShardClass, AllowChildClass))
			ReturnArray.Add(&ShardWrapper);
	}

	return ReturnArray;
}

TArray<FObjectWrapper> FObjectWrapperArray::GetObjectWrappers(const TSubclassOf<UObject>& ShardClass, bool AllowChildClass) const
{
	TArray<FObjectWrapper> ReturnArray;
	for (auto& ShardWrapper : ObjectWrappers)
	{
		if (!ShardWrapper.IsValid())
			continue;

		if (ShardWrapper.IsClass(ShardClass, AllowChildClass))
			ReturnArray.Add(ShardWrapper);
	}

	return ReturnArray;
}

UObject* FObjectWrapperArray::GetObject(const TSubclassOf<UObject>& ShardClass, bool AllowChildClass, UObject* Outer) const
{
	const FObjectWrapper Wrapper = GetObjectWrapper(ShardClass, AllowChildClass);
	return Wrapper.GetObject();
}

TArray<UObject*> FObjectWrapperArray::GetObjects(const TSubclassOf<UObject>& ShardClass, bool AllowChildClass, UObject* Outer) const
{
	TArray<FObjectWrapper> Wrappers = GetObjectWrappers(ShardClass, AllowChildClass);
	return UnpackWrappers(Wrappers, Outer);
}

TArray<FObjectWrapper*> FObjectWrapperArray::GetAllAsPointers()
{
	return ConvertToPointerArray(ObjectWrappers);
}

TArray<FObjectWrapper> FObjectWrapperArray::GetCopyArray() const
{
	return ObjectWrappers;
}

TArray<UObject*> FObjectWrapperArray::GetAllObjects(UObject* Outer) const
{
	return UnpackWrappers(ObjectWrappers, Outer);
}

bool FObjectWrapperArray::ContainsOrContainsChild(const TArray<TSubclassOf<UObject>>& Array,
                                               const TSubclassOf<UObject>& ClassToCheck, bool AllowChildClasses)
{
	for (auto Element : Array)
	{
		EClassInheritanceType Type = GetClassInheritanceType(Element, ClassToCheck);
		if (Type == EClassInheritanceType::Same || Type == EClassInheritanceType::Child && AllowChildClasses)
			return true;
	}
	return false;
}

void FObjectWrapperArray::RemoveParents(TArray<TSubclassOf<UObject>>& Array,
                                           const TSubclassOf<UObject>& Class, bool RemoveSame)
{
	TArray<int> ToRemoveIndexes {};
	for (int i = 0; i < Array.Num(); ++i)
	{
		EClassInheritanceType Type = GetClassInheritanceType(Class, Array[i]);
		if (Type == EClassInheritanceType::Child || RemoveSame && Type == EClassInheritanceType::Same)
			ToRemoveIndexes.Add(i);
	}
	for (int i = ToRemoveIndexes.Num() - 1; i >= 0; --i)
		Array.RemoveAt(ToRemoveIndexes[i]);
}

EClassInheritanceType FObjectWrapperArray::GetClassInheritanceType(const TSubclassOf<UObject>& ClassToCheck,
                                                                      const TSubclassOf<UObject>& BaseClass)
{
	if (BaseClass == ClassToCheck)
		return EClassInheritanceType::Same;
		
	if (BaseClass->IsChildOf(ClassToCheck))
		return EClassInheritanceType::Parent;

	if (ClassToCheck->IsChildOf(BaseClass))
		return EClassInheritanceType::Child;

	return EClassInheritanceType::Different;
}

TArray<UObject*> FObjectWrapperArray::UnpackWrappers(const TArray<FObjectWrapper>& Wrappers, UObject* Outer)
{
	TArray<UObject*> Shards;
	for (const auto& Wrapper : Wrappers)
	{
		if (Wrapper.IsValid())
			Shards.Add(Wrapper.GetObject(Outer));
	}

	return Shards;
}

TArray<TSubclassOf<UObject>> FObjectWrapperArray::GetCurrentCachedClasses() const
{
	TArray<TSubclassOf<UObject>> Classes;
	for (auto Wrapper : ObjectWrappers)
	{
		if (!Wrapper.IsValid())
			continue;

		Classes.AddUnique(Wrapper.GetSelectedClass());
	}

	return Classes;
}

bool FObjectWrapperArray::AddNewObjectWrapperOfClass(const TSubclassOf<UObject>& ShardClass, bool ForceLock)
{
	if (GetCurrentCachedClasses().Contains(ShardClass))
		return false;

	ObjectWrappers.Add(FObjectWrapper(ShardClass, LockedShards || ForceLock));
	
	return true;
}

void FObjectWrapperArray::ResetWrapper()
{
	ResetWrapper(InitialClasses);
}

void FObjectWrapperArray::ResetWrapper(TArray<TSubclassOf<UObject>> StartingClasses)
{
	ObjectWrappers.Empty();
	for (const auto Class : StartingClasses)
		AddNewObjectWrapperOfClass(Class);
}
