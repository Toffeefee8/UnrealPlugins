#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class USaveGame;

class FSaveSystemEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

protected:

    TSharedRef<class SDockTab> OnSpawnSaveEditorTab(const class FSpawnTabArgs& SpawnTabArgs);
    
    static const FName SaveEditorTabName;

private:

    TArray<FName> GetAllOptions() const;

    void RefreshAllData();
    bool SetupModifiableSave();

    TSharedRef<SWidget> OnGenerateWidget(FName Selected);
    void OnSaveSelectionChanged(FName NewValue, ESelectInfo::Type SelectInfo);

    //TEMP REFS - NEED TO BE MOVED
    TWeakObjectPtr<USaveGame> CurrentSelectedObject = nullptr;
    FName CurrentSelectedPath = "";
    TSharedPtr<IDetailsView> DetailsView = nullptr;
    TArray<FName> FoundSavePaths;
};
