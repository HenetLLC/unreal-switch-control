// Copyright Henet LLC 2025
// MODIFIED FILE
// This is now Node 2, the Event Listener.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Containers/Queue.h"
#include "TimerManager.h"
// #include "HenetSerialPortReader.h" // No longer need this, HenetSerialConnection.h includes it
#include "HenetSerialConnection.h" // <-- NEW: Include the connection object
#include "HenetSwitchMonitorNode.generated.h"

// A single, re-usable delegate type for all events that have no parameters.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHenetMonitorNoParams);


/**
 * (NODE 2)
 * Blueprint node to monitor events from an *existing* Henet Serial Connection.
 */
UCLASS()
class HENETSWITCHCONTROL_API UHenetSwitchMonitorNode : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Starts listening for switch and heartbeat events from the specified serial connection.
	 * @param Connection The connection object from "OpenHenetSerialConnection".
	 */
	 // <-- MODIFIED: Function signature changed -->
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", ExposedAsyncProxy = "AsyncAction"), Category = "Henet Switch Control")
	static UHenetSwitchMonitorNode* ListenForHenetSwitchEvents(UObject* WorldContextObject, UHenetSerialConnection* Connection);

	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	// ~UBlueprintAsyncActionBase interface

	/**
	 * Stops listening for events *on this node*.
	 * This does NOT close the serial port. Use "CloseHenetSerialConnection" for that.
	 */
	UFUNCTION(BlueprintCallable, Category = "Henet Switch Control")
	void StopListening();

	// --- OUTPUT EXECUTION PINS ---
	// (These are all unchanged from before)

	/** (Catch-all) Fired for *every* event. */
	UPROPERTY(BlueprintAssignable)
	FHenetMonitorNoParams OnUpdate;
// ... (rest of the delegates are identical) ...
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
	
	/** Handle to the world's timer manager */
	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	/** Timer handle for polling */
	FTimerHandle TimerHandle;

	// --- MODIFIED: Replaced Worker and EventQueue with the Connection object ---
	
	/** The connection object we are listening to. */
	UPROPERTY()
	TObjectPtr<UHenetSerialConnection> TargetConnection;

	/** Tracks the last known connection state to fire OnConnected/OnDisconnected only when it changes. */
	bool bIsConnected = false;
};