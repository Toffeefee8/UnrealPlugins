#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FObjectExtensionsEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
