// Copyright Henet LLC 2025
// Header for the Blueprint Async Action

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Containers/Queue.h"
#include "TimerManager.h"
#include "HenetSerialPortReader.h" // Includes FHenetSwitchEvent
#include "HenetSwitchMonitorNode.generated.h"

// --- NEW DELEGATE DEFINITIONS ---
// A single, re-usable delegate type for all events that have no parameters.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHenetMonitorNoParams);


/**
 * Blueprint node to monitor Henet Switch Control protocol from a serial port.
 * This node runs a background thread to listen for serial data and fires
 * specific event pins when data is received.
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
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", ExposedAsyncProxy = "AsyncAction"), Category = "Henet Switch Control")
	static UHenetSwitchMonitorNode* ListenForHenetSwitchEvents(UObject* WorldContextObject, const FString& PortName);

	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	// ~UBlueprintAsyncActionBase interface

	/**
	 * Stops listening on the serial port and cleans up the background thread.
	 * This is your "Close Connection" function.
	 */
	UFUNCTION(BlueprintCallable, Category = "Henet Switch Control")
	void StopListening();

	// --- OUTPUT EXECUTION PINS ---

	/** (Catch-all) Fired for *every* event. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnUpdate;

	/** Fired *only* when the serial port successfully connects. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnConnected;

	/** Fired *only* when the serial port disconnects or fails to connect. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnDisconnected;

	/** Fired *only* for a heartbeat event. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnHeartbeat;

	/** Fired *only* when Switch 1 is pressed. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnSwitch1Pressed;

	/** Fired *only* when Switch 1 is released. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnSwitch1Released;

	/** Fired *only* when Switch 2 is pressed. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnSwitch2Pressed;

	/** Fired *only* when Switch 2 is released. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnSwitch2Released;

	/** Fired *only* when Switch 3 is pressed. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnSwitch3Pressed;

	/** Fired *only* when Switch 3 is released. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnSwitch3Released;

	/** Fired *only* when Switch 4 is pressed. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnSwitch4Pressed;

	/** Fired *only* when Switch 4 is released. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnSwitch4Released;


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

	/** Tracks the last known connection state to fire OnConnected/OnDisconnected only when it changes. */
	bool bIsConnected = false;
};