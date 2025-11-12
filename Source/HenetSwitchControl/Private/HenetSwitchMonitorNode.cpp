// Copyright Henet LLC 2025
// Implementation of the Blueprint Async Action

#include "HenetSwitchMonitorNode.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HenetSwitchControlModule.h"

UHenetSwitchMonitorNode* UHenetSwitchMonitorNode::ListenForHenetSwitchEvents(UObject* InWorldContextObject, const FString& InPortName)
{
	UHenetSwitchMonitorNode* Node = NewObject<UHenetSwitchMonitorNode>();
	Node->WorldContextObject = InWorldContextObject;
	Node->PortName = InPortName;
	return Node;
}

void UHenetSwitchMonitorNode::Activate()
{
	if (!WorldContextObject)
	{
		UE_LOG(LogHenetSwitchControl, Error, TEXT("HenetSwitchMonitorNode: WorldContextObject is null."));
		SetReadyToDestroy();
		return;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogHenetSwitchControl, Error, TEXT("HenetSwitchMonitorNode: Failed to get World."));
		SetReadyToDestroy();
		return;
	}

	// Set the initial connection state to false.
	// We will fire OnConnected if the worker's Init() succeeds.
	bIsConnected = false;

	// Start the worker thread
	Worker = new FHenetSerialPortReader(PortName, EventQueue);
	if (!Worker)
	{
		UE_LOG(LogHenetSwitchControl, Error, TEXT("HenetSwitchMonitorNode: Failed to create serial worker thread."));
		SetReadyToDestroy();
		return;
	}

	// Start a timer on the game thread to poll the queue
	World->GetTimerManager().SetTimer(TimerHandle, this, &UHenetSwitchMonitorNode::TimerCallback, 0.01f, true);

	UE_LOG(LogHenetSwitchControl, Log, TEXT("HenetSwitchMonitorNode activated. Listening on port %s."), *PortName);
}

void UHenetSwitchMonitorNode::SetReadyToDestroy()
{
	// Clean up the worker thread
	if (Worker)
	{
		Worker->EnsureCompletion();
		delete Worker;
		Worker = nullptr;
	}

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
	UE_LOG(LogHenetSwitchControl, Log, TEXT("StopListening called on HenetSwitchMonitorNode. Cleaning up..."));
	SetReadyToDestroy();
}

void UHenetSwitchMonitorNode::TimerCallback()
{
	// This function runs on the Game Thread
	CheckForUpdates();
}

void UHenetSwitchMonitorNode::CheckForUpdates()
{
	if (!Worker) return;

	FHenetSwitchEvent Event;

	// Dequeue all events that have accumulated
	while (EventQueue.Dequeue(Event))
	{
		// --- Fire the "OnUpdate" (catch-all) Pin ---
		// This fires for *every* event, regardless of type.
		OnUpdate.Broadcast();

		// --- Fire Specific Event Pins ---

		if (Event.bIsConnectionStatus)
		{
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