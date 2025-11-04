// Copyright Henet LLC 2025
// Private implementation for the HenetSwitchControl module

#include "HenetSwitchControlModule.h"

// Define a custom log category for easy debugging
DEFINE_LOG_CATEGORY_STATIC(LogHenetSwitchControl, Log, All);

void FHenetSwitchControlModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file
    UE_LOG(LogHenetSwitchControl, Log, TEXT("HenetSwitchControl module has started."));
}

void FHenetSwitchControlModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
    UE_LOG(LogHenetSwitchControl, Log, TEXT("HenetSwitchControl module has shut down."));
}

IMPLEMENT_MODULE(FHenetSwitchControlModule, HenetSwitchControl)

