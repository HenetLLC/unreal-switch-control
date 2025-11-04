# Henet Switch Control Plugin for Unreal Engine

This plugin provides a simple way to monitor a serial port for switch press events from a Henet-protocol compatible hardware device and expose them to the Unreal Engine Blueprint system.

## Features

-   Listens for switch press and release events on a specified serial port.
-   Communicates with hardware using the Henet protocol.
-   Exposes events through a Blueprint Async Action node for easy integration into game logic.
-   Runs serial communication on a background thread to prevent any impact on game performance.

## Compatibility

This plugin is designed for **Unreal Engine 5.6**.

## Installation

1.  Download the latest release from the [releases page](https://github.com/your-repo/HenetSwitchControl/releases).
2.  In your Unreal Engine project, create a `Plugins` folder at the root if it doesn't already exist.
3.  Extract the downloaded archive and copy the `HenetSwitchControl` folder into your project's `Plugins` folder.
4.  Restart the Unreal Editor. The plugin should be automatically detected.
5.  Open the Plugins window (`Edit > Plugins`) and verify that "Henet Switch Control" is enabled under the "Project" or "Installed" category.

## How to Use

1.  Enable the "Henet Switch Control" plugin in your project's plugin settings.
2.  In any Blueprint, right-click to bring up the context menu.
3.  Search for and add the **Listen For Henet Switch Events** node.
4.  Specify the **Port Name** (e.g., "COM3" on Windows) that your device is connected to.
5.  The **OnUpdate** execution pin will fire every time a valid message is received from the device. The output pins will provide the current state of each switch (pressed or not) and a heartbeat signal.

![Blueprint Node Example](httpsd://example.com/blueprint_node.png) *(Note: This is a placeholder image link. You can replace it with a screenshot of the actual Blueprint node.)*

## How It Works

The plugin consists of two main parts:

-   A C++ worker (`FHenetSerialPortReader`) that runs on a dedicated thread to handle low-level serial port I/O and parse the incoming data stream.
-   A Blueprint node (`UHenetSwitchMonitorNode`) that spawns the worker thread and provides a simple interface for Blueprints to receive events from the hardware.

This design ensures that all blocking I/O operations are kept off the main game thread, guaranteeing smooth performance.
