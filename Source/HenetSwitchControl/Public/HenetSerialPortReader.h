// Copyright Henet LLC 2025
// Header for the FRunnable serial port reader

#pragma once

#include "CoreMinimal.h" // <-- Reverted back to CoreMinimal.h
#include "HAL/Runnable.h"
#include "Templates/Atomic.h" // <-- This is the correct header for TAtomic
#include "Containers/Queue.h"

// Define a struct to pass event data from the worker thread to the game thread
struct FHenetSwitchEvent
{
    bool bIsHeartbeat = false;
    int32 SwitchNumber = 0; // 1-4
    bool bIsPressed = false;

    FHenetSwitchEvent() {}

    FHenetSwitchEvent(bool bHeartbeat)
        : bIsHeartbeat(bHeartbeat) {}

    FHenetSwitchEvent(int32 InSwitch, bool bPressed)
        : SwitchNumber(InSwitch), bIsPressed(bPressed) {}
};

/**
 * FRunnable class to handle serial port communication on a separate thread.
 */
class HENETSWITCHCONTROL_API FHenetSerialPortReader : public FRunnable
{
public:
    // Constructor
    FHenetSerialPortReader(const FString& InPortName, TQueue<FHenetSwitchEvent, EQueueMode::Mpsc>& InEventQueue);
    
    // Destructor
    virtual ~FHenetSerialPortReader();

    // FRunnable interface
    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;
    virtual void Exit() override;

    /** Signals to the thread to stop */
    void EnsureCompletion();

private:
    /**
     * Parses the incoming byte stream according to the Henet protocol.
     * This is a state machine.
     */
    void ParseByte(uint8 Byte);

    /** Thread handle */
    FRunnableThread* Thread;

    /** Port name (e.g., "COM3") */
    FString PortName;

    /** Thread-safe queue to send events back to the game thread */
    TQueue<FHenetSwitchEvent, EQueueMode::Mpsc>& EventQueue;

    /** Atomic an_d volatile boolean to stop the thread */
    TAtomic<int32> StopTaskCounter; // <-- Replaced deprecated FThreadSafeCounter

    /** Handle to the serial port (Windows-specific) */
    void* hSerial; // Using void* to avoid including Windows.h in header

    // Protocol Constants
    enum EProtocolChars : uint8
    {
        ENQ = 0x05,
        DLE = 0x10,
        STX = 0x02,
        ETX = 0x03,
        Proto_S = 0x53, // <-- Fixed: Changed 'S' to Proto_S
        Proto_H = 0x48, // <-- Fixed: Changed 'H' to Proto_H
        Proto_P = 0x50, // <-- Fixed: Changed 'P' to Proto_P
        Proto_R = 0x52  // <-- Fixed: Changed 'R' to Proto_R
    };

    // Parser state machine
    enum class EParserState
    {
        Find_ENQ,
        Find_DLE1,
        Find_STX,
        Find_Type,
        Find_SwitchNum,
        Find_EventType,
        Find_DLE2,
        Find_ETX
    };

    EParserState ParserState;
    uint8 TempSwitchNum;
    uint8 TempEventType;
};