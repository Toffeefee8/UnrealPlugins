#include "ObjectExtensionsEditor.h"

#include "Common/CompactRowEditor.h"
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
}

void FObjectExtensionsEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("TimeData");
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("DistanceData");
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("Probability");

	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("ValueRangeInt");
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("ValueRangeFloat");
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FObjectExtensionsEditorModule, ObjectExtensionsEditor)