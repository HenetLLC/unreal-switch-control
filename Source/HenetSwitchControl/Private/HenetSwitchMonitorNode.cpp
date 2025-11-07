// Copyright Henet LLC 2025
// Implementation of the Blueprint Async Action

#include "HenetSwitchMonitorNode.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HenetSwitchControlModule.h" // <-- Added include for log category

// Link to the custom log category
// DECLARE_LOG_CATEGORY_EXTERN(LogHenetSwitchControl, Log, All); // <-- Removed this line

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
    
    UBlueprintAsyncActionBase::SetReadyToDestroy(); // <-- Fixed: Changed Super:: to UBlueprintAsyncActionBase::
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
    bool bHeartbeat = false; // <-- Moved declaration outside the loop

    // Dequeue all events that have accumulated
    while (EventQueue.Dequeue(Event))
    {
        bHeartbeat = false; // <-- Reset to false at the start of each loop

        // NOTE: The line "Example of EParserState::Find_SwitchNum:" was here and has been removed as it was a C++ syntax error.
        // bool bHeartbeat = false; // <-- This line was moved up

        if (Event.bIsHeartbeat)
        {
            bHeartbeat = true;
            UE_LOG(LogHenetSwitchControl, Log, TEXT("Heartbeat received by BP node."));
        }
        else if (Event.SwitchNumber >= 1 && Event.SwitchNumber <= 4)
        {
            // Update the state of the switch
            bSwitchStates[Event.SwitchNumber - 1] = Event.bIsPressed;
            UE_LOG(LogHenetSwitchControl, Log, TEXT("Switch %d state updated to: %s"), 
                Event.SwitchNumber, bSwitchStates[Event.SwitchNumber - 1] ? TEXT("Pressed") : TEXT("Released"));
        }

        // Broadcast the update.
        // The Heartbeat bool will be true for one frame, then false on the next
        // update unless another heartbeat is received.
        OnUpdate.Broadcast(
            bSwitchStates[0],
            bSwitchStates[1],
            bSwitchStates[2],
            bSwitchStates[3],
            bHeartbeat
        );
    }
}