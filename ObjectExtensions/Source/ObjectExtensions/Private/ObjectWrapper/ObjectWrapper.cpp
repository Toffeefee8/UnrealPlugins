#include "ObjectWrapper/ObjectWrapper.h"
#include "Save/ObjectSerializationLibrary.h"

DEFINE_LOG_CATEGORY(LogObjectWrapper);

FObjectWrapper::FObjectWrapper(UObject* Object)
{
	SelectedClass = Object->GetClass();
	UObjectSerializationLibrary::CaptureObject(ObjectData, Object);
}

TSubclassOf<UObject> FObjectWrapper::GetSelectedClass() const
{
	return SelectedClass;
}

bool FObjectWrapper::IsValid() const
{
	return SelectedClass != nullptr;
}

bool FObjectWrapper::IsClass(const TSubclassOf<UObject>& Class, const bool AllowChildClass) const
{
	if (!IsValid())
		return false;
	
	if (AllowChildClass)
		return GetSelectedClass()->IsChildOf(Class);

	return GetSelectedClass() == Class;
}

bool FObjectWrapper::IsDataSet() const
{
	return GetSelectedClass() == ObjectData.ObjectClass;
}

UObject* FObjectWrapper::GetObject(UObject* Outer) const
{
	if (!IsValid())
	{
		UE_LOG(LogObjectWrapper, Warning, TEXT("Object Wrapper has no selected class!"))
		return nullptr;
	}

	if (!Outer)
		Outer = GetTransientPackage();
	
	return UObjectSerializationLibrary::GetObject<UObject>(GetSelectedClass(), ObjectData, Outer, false);
}

FString FObjectWrapper::GetSelectedName() const
{
	if (SelectedClass)
		return GetSelectedClass()->GetName();
	return "NULL";
}
