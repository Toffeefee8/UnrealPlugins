#pragma once

#include "CoreMinimal.h"
#include "Save/ObjectSerializationLibrary.h"
#include "Templates/TypeHash.h"
#include "ObjectWrapper.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogObjectWrapper, Log, All);

USTRUCT(BlueprintType)
struct OBJECTEXTENSIONS_API FObjectWrapper
{
	friend uint32 GetTypeHash(const FObjectWrapper& Arg)
	{
		return GetTypeHash(Arg.SelectedClass);
	}

	GENERATED_BODY()

public:
	FObjectWrapper() {  }

	explicit FObjectWrapper(const TSubclassOf<UObject>& StartingClass, const bool InLockClass = true) :
		LockClass(InLockClass),
		SelectedClass(StartingClass)
	{  }

	explicit FObjectWrapper(UObject* Object);

	UPROPERTY()
	bool LockClass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "LockClass==false"))
	TSubclassOf<UObject> SelectedClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FObjectData ObjectData = FObjectData();

	//Gets the selected or override class
	TSubclassOf<UObject> GetSelectedClass() const;
	//Checks if selected class is null
	bool IsValid() const;
	//Checks if selected class is null
	bool IsClass(const TSubclassOf<UObject>& Class, bool AllowChildClass) const;
	//Checks if object data has correct class
	bool IsDataSet() const;
	//Error safe way to get selected class name
	FString GetSelectedName() const;
	
	//Builds object with specified outer
	UObject* GetObject(UObject* Outer = nullptr) const;
	template <typename T>
	T* GetObject(UObject* Outer = nullptr, bool AllowChildClass = true) const;
};

template <typename T>
T* FObjectWrapper::GetObject(UObject* Outer, bool AllowChildClass) const
{
	if (IsClass(T::StaticClass(), AllowChildClass))
		return static_cast<T*>(GetObject(Outer));
		
	return nullptr;
}
