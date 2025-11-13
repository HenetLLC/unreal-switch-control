// Copyright Henet LLC 2025
// NEW FILE
// Implementation for the UHenetSerialConnection object.

#include "HenetSerialConnection.h"
#include "HenetSerialPortReader.h"
#include "HenetSwitchControlModule.h" // For logging

UHenetSerialConnection::UHenetSerialConnection()
{
	Worker = nullptr;
}

void UHenetSerialConnection::Open(const FString& PortName)
{
	if (Worker)
	{
		UE_LOG(LogHenetSwitchControl, Warning, TEXT("UHenetSerialConnection::Open called, but connection is already open."));
		return;
	}

	// --- NEW: Protect this object from the Garbage Collector ---
	// This prevents the "Connection object is invalid" error.
	AddToRoot();
	// --- End of new code ---

	UE_LOG(LogHenetSwitchControl, Log, TEXT("UHenetSerialConnection: Opening connection to %s..."), *PortName);
	// The FHenetSerialPortReader constructor spawns the thread.
	// We pass it *our* event queue for it to push events to.
	Worker = new FHenetSerialPortReader(PortName, EventQueue);
}

void UHenetSerialConnection::Close()
{
	if (Worker)
	{
		UE_LOG(LogHenetSwitchControl, Log, TEXT("UHenetSerialConnection: Closing connection..."));
		Worker->EnsureCompletion();
		delete Worker;
		Worker = nullptr;

		// --- NEW: Allow the Garbage Collector to clean up this object ---
		RemoveFromRoot();
		// --- End of new code ---
	}
}

void UHenetSerialConnection::BeginDestroy()
{
	// This ensures the thread is cleaned up if the object is garbage collected.
	UE_LOG(LogHenetSwitchControl, Log, TEXT("UHenetSerialConnection: BeginDestroy called, ensuring connection is closed."));
	Close();
	Super::BeginDestroy();
}