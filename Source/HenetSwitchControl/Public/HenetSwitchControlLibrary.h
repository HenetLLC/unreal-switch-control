// Copyright Henet LLC 2025
// NEW FILE
// Blueprint Function Library to provide Open/Close nodes.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HenetSerialConnection.h" // For UHenetSerialConnection
#include "HenetSwitchControlLibrary.generated.h"

/**
 * Provides nodes to open and close a Henet Serial Connection.
 */
UCLASS()
class HENETSWITCHCONTROL_API UHenetSwitchControlLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * (NODE 1)
     * Opens a new serial port connection and returns a reference to it.
     * You must listen for the "OnConnected" event (from Node 2) to know if it succeeded.
     * @param PortName The name of the serial port (e.g., "COM3").
     * @return A new UHenetSerialConnection object.
     */
    UFUNCTION(BlueprintCallable, Category = "Henet Switch Control", meta = (Keywords = "open serial com port henet"))
    static UHenetSerialConnection* OpenHenetSerialConnection(const FString& PortName);

    /**
     * (NODE 3)
     * Closes a serial port connection.
     * @param Connection The connection object returned from "OpenHenetSerialConnection".
     */
    UFUNCTION(BlueprintCallable, Category = "Henet Switch Control", meta = (Keywords = "close serial com port henet"))
    static void CloseHenetSerialConnection(UHenetSerialConnection* Connection);
};