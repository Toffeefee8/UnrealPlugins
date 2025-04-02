#include "RegionSystemEditor.h"

#include "RegionCompoundWidget.h"
#include "RegionPOIEditor.h"
#include "RegionVolumeEditor.h"

#define LOCTEXT_NAMESPACE "FRegionSystemEditorModule"

void FRegionSystemEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout("RegionVolume", FOnGetDetailCustomizationInstance::CreateStatic(&FRegionVolumeEditor::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout("RegionPOI", FOnGetDetailCustomizationInstance::CreateStatic(&FRegionPOIEditor::MakeInstance));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SRegionCompoundWidget::RegionTabName, FOnSpawnTab::CreateStatic(&SRegionCompoundWidget::MakeInstance));
}

void FRegionSystemEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.UnregisterCustomClassLayout("RegionVolume");
	PropertyEditorModule.UnregisterCustomClassLayout("RegionPOI");

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SRegionCompoundWidget::RegionTabName);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FRegionSystemEditorModule, RegionSystemEditor)