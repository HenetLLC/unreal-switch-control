// Copyright Henet LLC 2025
// NEW FILE
// Implementation for the Blueprint Function Library.

#include "HenetSwitchControlLibrary.h"
#include "HenetSerialConnection.h"

UHenetSerialConnection* UHenetSwitchControlLibrary::OpenHenetSerialConnection(const FString& PortName)
{
	// Create a new UObject to hold the connection
	UHenetSerialConnection* ConnectionObject = NewObject<UHenetSerialConnection>();
	
	// Start the connection process (this spawns the thread)
	ConnectionObject->Open(PortName);
	
	// Return the object to Blueprints
	return ConnectionObject;
}

void UHenetSwitchControlLibrary::CloseHenetSerialConnection(UHenetSerialConnection* Connection)
{
	if (IsValid(Connection))
	{
		Connection->Close();
	}
}