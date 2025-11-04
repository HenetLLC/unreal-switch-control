// Copyright Henet LLC 2025
// Public header for the HenetSwitchControl module

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FHenetSwitchControlModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

