// Copyright Henet LLC 2025
// NEW FILE
// This object acts as a handle to a serial connection
// It can be passed around in Blueprints.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HenetSerialPortReader.h" // For FHenetSwitchEvent
#include "Containers/Queue.h"
#include "HenetSerialConnection.generated.h"

/**
 * A UObject that holds a reference to an active serial port reader thread.
 * This can be passed between Blueprint nodes.
 */
UCLASS(BlueprintType)
class HENETSWITCHCONTROL_API UHenetSerialConnection : public UObject
{
	GENERATED_BODY()

public:
	UHenetSerialConnection();

	/**
	 * Opens the serial port connection by spawning the worker thread.
	 * @param PortName The name of the serial port (e.g., "COM3").
	 */
	void Open(const FString& PortName);

	/**
	 * Closes the serial port connection and cleans up the worker thread.
	 */
	void Close();

	/** Thread-safe queue for events from the worker thread */
	TQueue<FHenetSwitchEvent, EQueueMode::Mpsc> EventQueue;

protected:
	/** Overridden from UObject to ensure we clean up the thread when this object is destroyed */
	virtual void BeginDestroy() override;

private:
	/** The worker thread object */
	FHenetSerialPortReader* Worker = nullptr;
};