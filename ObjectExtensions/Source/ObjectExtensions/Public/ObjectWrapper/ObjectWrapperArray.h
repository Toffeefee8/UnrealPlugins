#pragma once

#include "CoreMinimal.h"
#include "ObjectWrapper.h"
#include "Enums/ClassInheritanceType.h"
#include "ObjectWrapperArray.generated.h"

class UObject;

USTRUCT(BlueprintType)
struct OBJECTEXTENSIONS_API FObjectWrapperArray
{
	GENERATED_BODY()

public:
	
	FObjectWrapperArray() {  }
	FObjectWrapperArray(const TArray<FObjectWrapper>& Wrappers)
	{
		for (const auto& Wrapper : Wrappers)
		{
			if (!Wrapper.IsValid())
				return;
			
			ObjectWrappers.Add(Wrapper);
		}
	};
	FObjectWrapperArray(const TArray<FObjectWrapperArray>& ArrayArray)
	{
		for (const auto& WrapperArray : ArrayArray)
		{
			ObjectWrappers.Append(WrapperArray.GetCopyArray());
		}
	};
	explicit FObjectWrapperArray(const TArray<const FObjectWrapperArray*>& ArrayArray)
	{
		for (const auto& WrapperArray : ArrayArray)
		{
			if (!WrapperArray)
				return;
			
			ObjectWrappers.Append(WrapperArray->GetCopyArray());
		}
	};
	explicit FObjectWrapperArray(const TArray<TSubclassOf<UObject>>& InStartingClasses, const bool AllShardsLocked = true)
	{
		InitialClasses = InStartingClasses;
		LockedShards = AllShardsLocked;
		ResetWrapper(InStartingClasses);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite) 
	TArray<FObjectWrapper> ObjectWrappers {};
	
	UPROPERTY()
	bool LockedShards = false;
	
	bool AddNewObjectWrapperOfClass(const TSubclassOf<UObject>& ShardClass, bool ForceLock = false);
	void ResetWrapper();
	void ResetWrapper(TArray<TSubclassOf<UObject>> StartingClasses);

private:
	TArray<TSubclassOf<UObject>> InitialClasses {};
	
#pragma region Gets
public:
	TArray<TSubclassOf<UObject>> GetCurrentCachedClasses() const;
	
	//Single Gets
	FObjectWrapper* GetObjectWrapper(const TSubclassOf<UObject>& ShardClass, bool AllowChildClass = true);
	FObjectWrapper GetObjectWrapper(const TSubclassOf<UObject>& ShardClass, bool AllowChildClass = true) const;
	UObject* GetObject(const TSubclassOf<UObject>& ShardClass, bool AllowChildClass = true, UObject* Outer = nullptr) const;
	template <typename T>
	T* GetObject(bool AllowChildClass = true, UObject* Outer = nullptr) const;

	//Group Gets
	TArray<FObjectWrapper*> GetObjectWrappers(const TSubclassOf<UObject>& ShardClass, bool AllowChildClass = true);
	TArray<FObjectWrapper> GetObjectWrappers(const TSubclassOf<UObject>& ShardClass, bool AllowChildClass = true) const;
	TArray<UObject*> GetObjects(const TSubclassOf<UObject>& ShardClass, bool AllowChildClass = true, UObject* Outer = nullptr) const;
	template <typename T>
	TArray<T*> GetObjects(bool AllowChildClass = true, UObject* Outer = nullptr) const;

	//All Gets
	TArray<FObjectWrapper*> GetAllAsPointers();
	TArray<FObjectWrapper> GetCopyArray() const;
	TArray<UObject*> GetAllObjects(UObject* Outer = nullptr) const;

	//Helpers
	static bool ContainsOrContainsChild(const TArray<TSubclassOf<UObject>>& Array, const TSubclassOf<UObject>& ClassToCheck, bool AllowChildClasses);
	static void RemoveParents(TArray<TSubclassOf<UObject>>& Array, const TSubclassOf<UObject>& Class, bool OrSam = false);
	static EClassInheritanceType GetClassInheritanceType(const TSubclassOf<UObject>& ClassToCheck, const TSubclassOf<UObject>& BaseClass);

protected:
	static TArray<UObject*> UnpackWrappers(const TArray<FObjectWrapper>& Wrappers, UObject* Outer);
#pragma endregion
	
};

#pragma region Templates
template <typename T>
T* FObjectWrapperArray::GetObject(bool AllowChildClass, UObject* Outer) const
{
	return Cast<T>(GetObject(T::StaticClass(), AllowChildClass, Outer));
}

template <typename T>
TArray<T*> FObjectWrapperArray::GetObjects(bool AllowChildClass, UObject* Outer) const
{
	return GetItemShards<T>(T::StaticClass(), AllowChildClass, Outer);
}
#pragma endregion