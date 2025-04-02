#include "ObjectWrapper/ObjectWrapperFunctionLibrary.h"
#include "ObjectWrapper/ObjectWrapper.h"

bool UObjectWrapperFunctionLibrary::IsValid(const FObjectWrapper& Wrapper)
{
	return Wrapper.IsValid();
}

bool UObjectWrapperFunctionLibrary::IsClass(const FObjectWrapper& Wrapper, const TSubclassOf<UObject> Class, bool AllowChildClasses)
{
	return Wrapper.IsClass(Class, AllowChildClasses);
}

FString UObjectWrapperFunctionLibrary::GetSelectedName(const FObjectWrapper& Wrapper)
{
	return Wrapper.GetSelectedName();
}

UObject* UObjectWrapperFunctionLibrary::GetObject(const FObjectWrapper& Wrapper, UObject* Outer)
{
	return Wrapper.GetObject(Outer);
}

UObject* UObjectWrapperFunctionLibrary::GetObjectAs(const FObjectWrapper& Wrapper,
	TSubclassOf<UObject> WrapperClass, UObject* Outer)
{
	return Wrapper.GetObject(Outer);
}

FObjectWrapper UObjectWrapperFunctionLibrary::CreateObjectWrapper(UObject* Object)
{
	return FObjectWrapper(Object);
}

bool UObjectWrapperFunctionLibrary::AddNewObjectWrapperOfClass(FObjectWrapperArray& WrapperArray,
	const TSubclassOf<UObject> ObjectClass, const bool IncludeRequired)
{
	return WrapperArray.AddNewObjectWrapperOfClass(ObjectClass, IncludeRequired);
}

TArray<TSubclassOf<UObject>> UObjectWrapperFunctionLibrary::GetCurrentCachedClasses(
	const FObjectWrapperArray& WrapperArray)
{
	return WrapperArray.GetCurrentCachedClasses();
}

FObjectWrapper UObjectWrapperFunctionLibrary::GetObjectWrapper(const FObjectWrapperArray& WrapperArray,
	TSubclassOf<UObject> ObjectClass)
{
	return WrapperArray.GetObjectWrapper(ObjectClass);
}

UObject* UObjectWrapperFunctionLibrary::GetObjectFromArray(const FObjectWrapperArray& WrapperArray,
	const TSubclassOf<UObject> ObjectClass, UObject* Outer)
{
	return WrapperArray.GetObject(ObjectClass, false, Outer);
}

TArray<FObjectWrapper> UObjectWrapperFunctionLibrary::GetObjectWrappers(
	const FObjectWrapperArray& WrapperArray, const TSubclassOf<UObject> ObjectClass)
{
	return WrapperArray.GetObjectWrappers(ObjectClass);
}

TArray<UObject*> UObjectWrapperFunctionLibrary::GetObjects(const FObjectWrapperArray& WrapperArray,
	const TSubclassOf<UObject> ObjectClass, UObject* Outer)
{
	return WrapperArray.GetObjects(ObjectClass, false, Outer);
}

TArray<FObjectWrapper> UObjectWrapperFunctionLibrary::GetCopyArray(const FObjectWrapperArray& WrapperArray)
{
	return WrapperArray.GetCopyArray();
}

TArray<UObject*> UObjectWrapperFunctionLibrary::GetAllObjects(const FObjectWrapperArray& WrapperArray, UObject* Outer)
{
	return WrapperArray.GetAllObjects(Outer);
}