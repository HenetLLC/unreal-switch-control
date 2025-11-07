// Copyright Henet LLC 2025
// Public header for the HenetSwitchControl module

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Declare the module's log category
DECLARE_LOG_CATEGORY_EXTERN(LogHenetSwitchControl, Log, All);

class FHenetSwitchControlModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};