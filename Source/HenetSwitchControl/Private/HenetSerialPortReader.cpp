// Copyright Henet LLC 2025
// Implementation of the FRunnable serial port reader

#include "HenetSerialPortReader.h"
#include "HAL/PlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "Logging/LogMacros.h"
#include "HenetSwitchControlModule.h"

// Conditionally include Windows headers only on Windows
#if PLATFORM_WINDOWS && HENET_WINDOWS_SERIAL
#include "Windows/WindowsMinimal.h"
#else
// Define placeholder types if not on Windows to allow compilation
typedef void* HANDLE;
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
typedef struct _DCB { uint32 DCBlength; } DCB;
typedef struct _COMMTIMEOUTS { uint32 ReadIntervalTimeout; } COMMTIMEOUTS;
#endif

FHenetSerialPortReader::FHenetSerialPortReader(const FString& InPortName, TQueue<FHenetSwitchEvent, EQueueMode::Mpsc>& InEventQueue)
    : PortName(InPortName)
    , EventQueue(InEventQueue)
    , StopTaskCounter(0)
    , hSerial(INVALID_HANDLE_VALUE)
    , ParserState(EParserState::Find_ENQ)
    , TempSwitchNum(0)
    , TempEventType(0)
{
    // Create the thread
    Thread = FRunnableThread::Create(this, TEXT("HenetSerialPortReaderThread"), 0, TPri_BelowNormal);
}

FHenetSerialPortReader::~FHenetSerialPortReader()
{
    // Ensure the thread is stopped
    if (Thread)
    {
        EnsureCompletion();
        delete Thread;
        Thread = nullptr;
    }
}

bool FHenetSerialPortReader::Init()
{
    UE_LOG(LogHenetSwitchControl, Log, TEXT("Serial reader thread initializing..."));
    
#if PLATFORM_WINDOWS && HENET_WINDOWS_SERIAL
    // Try to open the serial port
    // Format port name for CreateFile (e.g., "\\\\.\\COM3")
    FString FullPortName = FString(TEXT("\\\\.\\")) + PortName;
    
    hSerial = CreateFile(
        *FullPortName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        DWORD LastError = GetLastError();
        UE_LOG(LogHenetSwitchControl, Error, TEXT("Failed to open serial port %s. Error code: %d"), *PortName, LastError);
        EventQueue.Enqueue(FHenetSwitchEvent::MakeConnectionStatus(false)); // <-- NEW
        return false;
    }

    // Configure the serial port (9600, 8N1)
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        UE_LOG(LogHenetSwitchControl, Error, TEXT("Failed to get serial port state."));
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        EventQueue.Enqueue(FHenetSwitchEvent::MakeConnectionStatus(false)); // <-- NEW
        return false;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.StopBits = ONESTOPBIT;

    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        UE_LOG(LogHenetSwitchControl, Error, TEXT("Failed to set serial port state."));
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        EventQueue.Enqueue(FHenetSwitchEvent::MakeConnectionStatus(false)); // <-- NEW
        return false;
    }

    // Set timeouts
    // We'll use a short read timeout to make the ReadFile call non-blocking
    // and allow the thread loop to check for the Stop signal.
    COMMTIMEOUTS timeouts = {0};

    // --- MODIFIED TIMEOUTS ---
    // This changes ReadFile to a blocking call that waits up to 100ms for data.
    // This is much more reliable than a non-blocking (0ms) read.
    timeouts.ReadIntervalTimeout = MAXDWORD; 
    timeouts.ReadTotalTimeoutConstant = 100; // Wait 100ms for *any* data
    timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
    // --- End of modification ---

    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        UE_LOG(LogHenetSwitchControl, Error, TEXT("Failed to set serial port timeouts."));
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        EventQueue.Enqueue(FHenetSwitchEvent::MakeConnectionStatus(false)); // <-- NEW
        return false;
    }
    else
    {
        // <-- NEW: Added log to confirm timeouts were set -->
        UE_LOG(LogHenetSwitchControl, Log, TEXT("Successfully set serial port timeouts."));
    }

    UE_LOG(LogHenetSwitchControl, Log, TEXT("Successfully opened and configured serial port %s."), *PortName);
    EventQueue.Enqueue(FHenetSwitchEvent::MakeConnectionStatus(true)); // <-- NEW
    return true;

#else
    UE_LOG(LogHenetSwitchControl, Warning, TEXT("Serial communication is only supported on Windows."));
    EventQueue.Enqueue(FHenetSwitchEvent::MakeConnectionStatus(false)); // <-- NEW
    return false;
#endif
}

uint32 FHenetSerialPortReader::Run()
{
    // Check if initialization failed
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        return 1; // Return error
    }

    UE_LOG(LogHenetSwitchControl, Log, TEXT("Serial reader thread running..."));

    // Buffer to read data into
    uint8 ReadBuffer[256];
    DWORD BytesRead = 0;

    // Main thread loop
    while (StopTaskCounter.Load() == 0)
    {
        // <-- NEW: Added log to see if the loop is spinning -->
        UE_LOG(LogHenetSwitchControl, VeryVerbose, TEXT("Run() loop spinning..."));

#if PLATFORM_WINDOWS && HENET_WINDOWS_SERIAL
        // Try to read data from the port
        // <-- NEW: Added log before ReadFile -->
        UE_LOG(LogHenetSwitchControl, VeryVerbose, TEXT("Calling ReadFile..."));
        if (ReadFile(hSerial, ReadBuffer, sizeof(ReadBuffer), &BytesRead, NULL))
        {
            if (BytesRead > 0)
            {
                // <-- Log level changed to Verbose -->
                FString HexString = FString::FromHexBlob(ReadBuffer, BytesRead);
                UE_LOG(LogHenetSwitchControl, Verbose, TEXT("Serial Data Received (%d bytes): %s"), BytesRead, *HexString);
                // <-- End of change -->

                // Process every byte read
                for (DWORD i = 0; i < BytesRead; ++i)
                {
                    ParseByte(ReadBuffer[i]);
                }
            }
            else
            {
                // <-- NEW: Added log for ReadFile returning 0 bytes -->
                UE_LOG(LogHenetSwitchControl, VeryVerbose, TEXT("ReadFile successful, but BytesRead = 0."));
            }
        }
        else
        {
            // ReadFile failed, likely a disconnect
            UE_LOG(LogHenetSwitchControl, Error, TEXT("ReadFile failed. Error code: %d. Stopping thread."), GetLastError());
            EventQueue.Enqueue(FHenetSwitchEvent::MakeConnectionStatus(false)); // <-- NEW
            StopTaskCounter.Store(1);
        }
#else
        // Sleep if not on Windows, as this loop will just spin
        FPlatformProcess::Sleep(0.1f);
        StopTaskCounter.Store(1); // <-- FIXED: Removed garbled text from this line
#endif
    }

    UE_LOG(LogHenetSwitchControl, Log, TEXT("Serial reader thread stopping."));
    return 0;
}

void FHenetSerialPortReader::Stop()
{
    // This is called by the FRunnable interface, signals the thread to stop
    StopTaskCounter.Store(1);
}

void FHenetSerialPortReader::Exit()
{
    // Called after Run() completes
#if PLATFORM_WINDOWS && HENET_WINDOWS_SERIAL
    if (hSerial != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        UE_LOG(LogHenetSwitchControl, Log, TEXT("Serial port %s closed."), *PortName);
    }
#endif
}

void FHenetSerialPortReader::EnsureCompletion()
{
    Stop();
    if (Thread)
    {
        Thread->WaitForCompletion();
    }
}

void FHenetSerialPortReader::ParseByte(uint8 Byte)
{
    // <-- Log level changed to Verbose -->
    UE_LOG(LogHenetSwitchControl, Verbose, TEXT("ParseByte: 0x%02X, State: %d"), Byte, static_cast<int32>(ParserState));

    // --- REFACTORED STATE MACHINE ---
    // ENQ (0x05) is treated as a "reset" signal at any point.

    // Check for ENQ first, as it can reset the state at any time.
    if (Byte == EProtocolChars::ENQ)
    {
        UE_LOG(LogHenetSwitchControl, Verbose, TEXT("Sender is sending data (ENQ received)."));
        ParserState = EParserState::Find_DLE1;
        // Reset message data on ENQ
        TempSwitchNum = 0;
        TempEventType = 0;
        return; // Byte processed, move to next
    }

    // Process byte based on current state
    switch (ParserState)
    {
    case EParserState::Find_ENQ:
        // We are just waiting for an ENQ, which is handled above.
        // Any other byte is ignored.
        break;

    case EParserState::Find_DLE1:
        if (Byte == EProtocolChars::DLE)
        {
            ParserState = EParserState::Find_STX;
        }
        else
        {
            UE_LOG(LogHenetSwitchControl, Warning, TEXT("Parse Error: No DLE was received as expected. Resetting."));
            ParserState = EParserState::Find_ENQ;
        }
        break;

    case EParserState::Find_STX:
        if (Byte == EProtocolChars::STX)
        {
            UE_LOG(LogHenetSwitchControl, Verbose, TEXT("Transmission started (STX received)."));
            ParserState = EParserState::Find_Type;
        }
        else
        {
            UE_LOG(LogHenetSwitchControl, Warning, TEXT("Parse Error: No STX was received as expected. Resetting."));
            ParserState = EParserState::Find_ENQ;
        }
        break;

    case EParserState::Find_Type:
        if (Byte == EProtocolChars::Proto_H) // Heartbeat
        {
            TempSwitchNum = 0; // Ensure TempSwitchNum is 0 for heartbeat
            ParserState = EParserState::Find_DLE2;
        }
        else if (Byte == EProtocolChars::Proto_S) // Switch Event
        {
            ParserState = EParserState::Find_SwitchNum;
        }
        else
        {
            UE_LOG(LogHenetSwitchControl, Warning, TEXT("Parse Error: Invalid message type (0x%02X). Resetting."), Byte);
            ParserState = EParserState::Find_ENQ;
        }
        break;

    case EParserState::Find_SwitchNum:
        if (Byte >= '1' && Byte <= '4')
        {
            TempSwitchNum = Byte;
            ParserState = EParserState::Find_EventType;
        }
        else
        {
            UE_LOG(LogHenetSwitchControl, Warning, TEXT("Parse Error: Invalid switch number (0x%02X). Resetting."), Byte);
            ParserState = EParserState::Find_ENQ;
        }
        break;

    case EParserState::Find_EventType:
        if (Byte == EProtocolChars::Proto_P || Byte == EProtocolChars::Proto_R)
        {
            TempEventType = Byte;
            ParserState = EParserState::Find_DLE2;
        }
        else
        {
            UE_LOG(LogHenetSwitchControl, Warning, TEXT("Parse Error: Invalid event type (0x%02X). Resetting."), Byte);
            ParserState = EParserState::Find_ENQ;
        }
        break;

    case EParserState::Find_DLE2:
        if (Byte == EProtocolChars::DLE)
        {
            ParserState = EParserState::Find_ETX;
        }
        else
        {
            UE_LOG(LogHenetSwitchControl, Warning, TEXT("Parse Error: No DLE (2) was received as expected. Resetting."));
            ParserState = EParserState::Find_ENQ;
        }
        break;

    case EParserState::Find_ETX:
        if (Byte == EProtocolChars::ETX)
        {
            UE_LOG(LogHenetSwitchControl, Verbose, TEXT("End of data was received (ETX)."));
            
            // --- Valid Message Received ---
            if (TempSwitchNum == 0) // This means it was a heartbeat
            {
                UE_LOG(LogHenetSwitchControl, Verbose, TEXT("Heartbeat Message Parsed."));
                EventQueue.Enqueue(FHenetSwitchEvent(true));
            }
            else
            {
                int32 SwitchNum = TempSwitchNum - '0'; // Convert '1' -> 1
                bool bPressed = (TempEventType == EProtocolChars::Proto_P);
                UE_LOG(LogHenetSwitchControl, Verbose, TEXT("Switch Message Parsed: Switch %d, %s"), 
                    SwitchNum, bPressed ? TEXT("Pressed") : TEXT("Released"));
                EventQueue.Enqueue(FHenetSwitchEvent(SwitchNum, bPressed));
            }
        }
        else
        {
            UE_LOG(LogHenetSwitchControl, Warning, TEXT("Parse Error: ETX was expected but not received (0x%02X). Resetting."), Byte);
        }
        
        // Always reset after processing or error at this stage
        ParserState = EParserState::Find_ENQ;
        TempSwitchNum = 0;
        TempEventType = 0;
        break;

    default:
        // Should never happen
        ParserState = EParserState::Find_ENQ;
        break;
    }
}