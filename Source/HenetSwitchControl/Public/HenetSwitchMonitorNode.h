// Copyright Henet LLC 2025
// Header for the Blueprint Async Action

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Containers/Queue.h"
#include "TimerManager.h" // <-- Added this include for FTimerHandle
#include "HenetSerialPortReader.h" // Includes FHenetSwitchEvent
#include "HenetSwitchMonitorNode.generated.h"

// Delegate for the "On Update" output pin
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FHenetSwitchMonitorOutputPin, bool, Switch1Pressed, bool, Switch2Pressed, bool, Switch3Pressed, bool, Switch4Pressed, bool, Heartbeat);

/**
 * Blueprint node to monitor Henet Switch Control protocol from a serial port.
 * This node runs a background thread to listen for serial data and fires
 * an event pin when data is received.
 */
UCLASS()
class HENETSWITCHCONTROL_API UHenetSwitchMonitorNode : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    /**
     * Starts listening for switch and heartbeat events from the specified serial port.
     * @param PortName The name of the serial port (e.g., "COM3").
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Henet Switch Control")
    static UHenetSwitchMonitorNode* ListenForHenetSwitchEvents(UObject* WorldContextObject, const FString& PortName);

    // UBlueprintAsyncActionBase interface
    virtual void Activate() override;
    virtual void SetReadyToDestroy() override;
    // ~UBlueprintAsyncActionBase interface

    /** Output pin: Fired every time a valid message is received, updating all states. */
    UPROPERTY(BlueprintAssignable)
    FHenetSwitchMonitorOutputPin OnUpdate;

private:
    /** Polls the event queue from the worker thread */
    void CheckForUpdates();

    /** Callback for the timer */
    UFUNCTION()
    void TimerCallback();

    /** The serial port to listen on (e.g., "COM3") */
    FString PortName;

    /** Handle to the world's timer manager */
    UPROPERTY()
    TObjectPtr<UObject> WorldContextObject;

    /** Timer handle for polling */
    FTimerHandle TimerHandle;
    
    /** The worker thread object */
    FHenetSerialPortReader* Worker = nullptr;
    
    /** Thread-safe queue for events from the worker thread */
    TQueue<FHenetSwitchEvent, EQueueMode::Mpsc> EventQueue;

    // Current state of all switches
    bool bSwitchStates[4] = { false, false, false, false };
};