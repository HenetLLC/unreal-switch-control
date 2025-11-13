// Copyright Henet LLC 2025
// MODIFIED FILE
// Implementation of the Event Listener (Node 2)

#include "HenetSwitchMonitorNode.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HenetSwitchControlModule.h"
#include "HenetSerialConnection.h" // <-- NEW: Include for the connection object

// <-- MODIFIED: Function signature changed -->
UHenetSwitchMonitorNode* UHenetSwitchMonitorNode::ListenForHenetSwitchEvents(UObject* InWorldContextObject, UHenetSerialConnection* Connection)
{
	UHenetSwitchMonitorNode* Node = NewObject<UHenetSwitchMonitorNode>();
	Node->WorldContextObject = InWorldContextObject;
	Node->TargetConnection = Connection; // <-- Store the connection
	return Node;
}

void UHenetSwitchMonitorNode::Activate()
{
	// --- NEW: Added Log ---
	UE_LOG(LogHenetSwitchControl, Log, TEXT("HenetSwitchMonitorNode: Activate() called."));

	if (!WorldContextObject)
	{
		UE_LOG(LogHenetSwitchControl, Error, TEXT("HenetSwitchMonitorNode: Activate() FAILED. WorldContextObject is null."));
		SetReadyToDestroy();
		return;
	}

	// <-- NEW: Check if the connection object is valid -->
	if (!IsValid(TargetConnection))
	{
		UE_LOG(LogHenetSwitchControl, Error, TEXT("HenetSwitchMonitorNode: Activate() FAILED. Connection object is invalid."));
		SetReadyToDestroy();
		return;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogHenetSwitchControl, Error, TEXT("HenetSwitchMonitorNode: Activate() FAILED. Failed to get World."));
		SetReadyToDestroy();
		return;
	}

	// Set the initial connection state to false.
	// We will fire OnConnected if we get a connection event from the queue.
	bIsConnected = false;

	// --- REMOVED ---
	// Worker creation is now handled by Node 1
	// ---

	// Start a timer on the game thread to poll the queue
	// --- NEW: Added Log ---
	UE_LOG(LogHenetSwitchControl, Log, TEXT("HenetSwitchMonitorNode: Activate() success. Starting timer..."));
	World->GetTimerManager().SetTimer(TimerHandle, this, &UHenetSwitchMonitorNode::TimerCallback, 0.01f, true);

	UE_LOG(LogHenetSwitchControl, Log, TEXT("HenetSwitchMonitorNode activated. Listening for events..."));
}

void UHenetSwitchMonitorNode::SetReadyToDestroy()
{
	// --- REMOVED ---
	// Worker cleanup is now handled by the UHenetSerialConnection object
	// ---

	// Clear the timer
	if (WorldContextObject)
	{
		UWorld* World = WorldContextObject->GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
		}
	}
	
	UBlueprintAsyncActionBase::SetReadyToDestroy();
}

void UHenetSwitchMonitorNode::StopListening()
{
	UE_LOG(LogHenetSwitchControl, Log, TEXT("StopListening called on HenetSwitchMonitorNode. Stopping timer..."));
	// This just stops *this* listener, it does not close the connection.
	SetReadyToDestroy();
}

void UHenetSwitchMonitorNode::TimerCallback()
{
	// This function runs on the Game Thread
	// --- NEW: Added Verbose log ---
	CheckForUpdates();
}

void UHenetSwitchMonitorNode::CheckForUpdates()
{
	// <-- MODIFIED: Check TargetConnection instead of Worker -->
	if (!IsValid(TargetConnection))
	{
		// --- NEW: Added Warning log ---
		UE_LOG(LogHenetSwitchControl, Warning, TEXT("CheckForUpdates: TargetConnection is NOT valid. Skipping update."));
		return;
	}

	FHenetSwitchEvent Event;

	// Dequeue all events that have accumulated
	// <-- MODIFIED: Poll the TargetConnection's queue -->
	// --- NEW: Added Verbose log ---
	while (TargetConnection->EventQueue.Dequeue(Event))
	{
		// --- NEW: Added Log for dequeued event ---
		UE_LOG(LogHenetSwitchControl, Verbose, TEXT("CheckForUpdates: Dequeued event (Heartbeat: %s, ConnectionStatus: %s)"),
			Event.bIsHeartbeat ? TEXT("true") : TEXT("false"),
			Event.bIsConnectionStatus ? TEXT("true") : TEXT("false"));

		// --- Fire the "OnUpdate" (catch-all) Pin ---
		// This fires for *every* event, regardless of type.
		OnUpdate.Broadcast();

		// --- Fire Specific Event Pins ---
		// (This logic is identical to before)

		if (Event.bIsConnectionStatus)
		{
// ... (rest of the file is identical) ...
			// Check if the status has actually changed
			if (Event.bIsConnected && !bIsConnected)
			{
				// We have just connected
				bIsConnected = true;
				OnConnected.Broadcast();
				UE_LOG(LogHenetSwitchControl, Log, TEXT("Connection status: CONNECTED."));
			}
			else if (!Event.bIsConnected && bIsConnected)
			{
				// We have just disconnected
				bIsConnected = false;
				OnDisconnected.Broadcast();
				UE_LOG(LogHenetSwitchControl, Log, TEXT("Connection status: DISCONNECTED."));
			}
		}
		else if (Event.bIsHeartbeat)
		{
			// Fire the specific "OnHeartbeat" pin
			OnHeartbeat.Broadcast();
			UE_LOG(LogHenetSwitchControl, Verbose, TEXT("Heartbeat event fired."));
		}
		else if (Event.SwitchNumber >= 1 && Event.SwitchNumber <= 4)
		{
			// Fire the specific pin for the switch and its state (pressed/released)
			switch (Event.SwitchNumber)
			{
			case 1:
				if (Event.bIsPressed) OnSwitch1Pressed.Broadcast();
				else OnSwitch1Released.Broadcast();
				break;
			case 2:
				if (Event.bIsPressed) OnSwitch2Pressed.Broadcast();
				else OnSwitch2Released.Broadcast();
				break;
			case 3:
				if (Event.bIsPressed) OnSwitch3Pressed.Broadcast();
				else OnSwitch3Released.Broadcast();
				break;
			case 4:
				if (Event.bIsPressed) OnSwitch4Pressed.Broadcast();
				else OnSwitch4Released.Broadcast();
				break;
			default:
				// Should not happen
				break;
			}
			UE_LOG(LogHenetSwitchControl, Verbose, TEXT("Switch %d event fired (Pressed: %s)"), Event.SwitchNumber, Event.bIsPressed ? TEXT("true") : TEXT("false"));
		}
	}
}