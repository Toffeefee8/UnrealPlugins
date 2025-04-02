#include "ObjectExtensionsEditor.h"

#include "Common/CompactRowEditor.h"
#include "ObjectWrapper/ObjectWrapperArrayEditor.h"
#include "ObjectWrapper/ObjectWrapperEditor.h"
#include "Structs/ValueRangeEditor.h"

#define LOCTEXT_NAMESPACE "FObjectExtensionsEditorModule"

void FObjectExtensionsEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout("TimeData", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FCompactRowEditor::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout("DistanceData", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FCompactRowEditor::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout("Probability", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FCompactRowEditor::MakeInstance));

	PropertyEditorModule.RegisterCustomPropertyTypeLayout("ValueRangeInt", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FValueRangeEditor::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout("ValueRangeFloat", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FValueRangeEditor::MakeInstance));
	
	PropertyEditorModule.RegisterCustomPropertyTypeLayout("ObjectWrapper", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FObjectWrapperEditor::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout("ObjectWrapperArray", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FObjectWrapperArrayEditor::MakeInstance));
}

void FObjectExtensionsEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("TimeData");
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("DistanceData");
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("Probability");

	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("ValueRangeInt");
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("ValueRangeFloat");

	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("ObjectWrapper");
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("ObjectWrapperArray");
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FObjectExtensionsEditorModule, ObjectExtensionsEditor)