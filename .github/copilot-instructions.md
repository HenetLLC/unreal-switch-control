# HenetSwitchControl Unreal Engine Plugin Instructions

This document provides guidance for AI agents working on the HenetSwitchControl codebase.

## Architecture Overview

This project is an Unreal Engine plugin that monitors a serial port for signals from a Henet-protocol switch device. Its primary purpose is to receive hardware events and expose them to the Unreal Engine Blueprint system.

The architecture is composed of three main parts:

1.  **`FHenetSerialPortReader` (`Source/HenetSwitchControl/Private/HenetSerialPortReader.cpp`)**: This is a C++ class implementing `FRunnable` to run on a dedicated background thread. It handles all low-level serial port communication and parses the incoming byte stream according to the proprietary Henet protocol. It is designed to be non-blocking for the main game thread.

2.  **Event Queue**: The `FHenetSerialPortReader` communicates with the game thread via a thread-safe `TQueue<FHenetSwitchEvent>`. This queue passes switch press and heartbeat events from the worker thread to the Blueprint node.

3.  **`UHenetSwitchMonitorNode` (`Source/HenetSwitchControl/Public/HenetSwitchMonitorNode.h`)**: This is a `UBlueprintAsyncActionBase` class that acts as the bridge between the C++ backend and the Blueprint visual scripting environment. It spawns the `FHenetSerialPortReader` thread and uses a timer (`FTimerHandle`) to poll the event queue each frame. When an event is dequeued, it fires the `OnUpdate` delegate, which appears as an output execution pin in the Blueprint editor.

## Key Files

-   `HenetSwitchControl.uplugin`: The plugin manifest.
-   `Source/HenetSwitchControl/HenetSwitchControl.build.cs`: The Unreal Build Tool script. Note the Windows-specific dependencies (`kernel32.lib`, `setupapi.lib`) and the `HENET_WINDOWS_SERIAL=1` preprocessor definition which enables the serial port code.
-   `Source/HenetSwitchControl/Public/HenetSerialPortReader.h`: Defines the `FRunnable` worker and the `FHenetSwitchEvent` data structure.
-   `Source/HenetSwitchControl/Public/HenetSwitchMonitorNode.h`: Defines the Blueprint-visible node.

## Development Patterns

-   **Threading**: All serial port I/O is performed in the `FHenetSerialPortReader` `FRunnable` to avoid stalls. Do not add blocking code to the game thread (e.g., in `UHenetSwitchMonitorNode`).
-   **Platform-Specific Code**: Windows-specific serial port API calls are located in `Source/HenetSwitchControl/Private/HenetSerialPortReader.cpp` and `Source/HenetSwitchControl/Private/Windows/`. This code is wrapped in `#if HENET_WINDOWS_SERIAL` blocks.
-   **Blueprint API**: To expose new functionality to designers, add new `UFUNCTION`s or `UPROPERTY`s to `UHenetSwitchMonitorNode`. For new events, consider adding new delegates or modifying the existing `FHenetSwitchMonitorOutputPin`.
-   **Protocol Implementation**: The Henet protocol logic is implemented as a state machine in `FHenetSerialPortReader::ParseByte`. Any changes to the protocol should be made there.
