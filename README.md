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

1.  Download the latest `HenetSwitchControl-vX.X.X.zip` file from the [GitHub Releases page](https://github.com/HenetLLC/unreal-switch-control/releases).
2.  In your Unreal Engine project, create a `Plugins` folder at the root if it doesn't already exist.
3.  Extract the downloaded zip archive into your project's `Plugins` folder. The structure should be `YourProject/Plugins/HenetSwitchControl/`.
4.  Restart the Unreal Editor.
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

## Adjusting log levels

Console:  
or 
DefaultEngine.ini: log LogHenetSwitchControl VeryVerbose
```
[Core.Log]
LogHenetSwitchControl=VeryVerbose
```


# Notes

Paikallinen buildi: "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="C:\VirtualWorlds\Oulu2026HenetPlugin\Plugins\HenetSwitchControl\HenetSwitchControl.uplugin" -Package="C:\VirtualWorlds\Oulu2026HenetPlugin\Plugins\HenetSwitchControl\Packaged" -TargetPlatforms=Win64 -CreateSubFolder

Projektin sln ei onnistu ennen:

        Open Your Project in the Unreal Editor (it will open fine now).

        Add a New C++ Class:

        In the editor, go to the top menu and click Tools -> New C++ Class....

        A "Choose Parent Class" window will appear. Select Actor (it's a common, simple class).

        Click Next.

        Name the Class:

        It will ask for a Name. You can just leave the default (MyActor) or call it EmptyCppClass.

        Click Create Class.

        Wait for Unreal to Work:

        Unreal will now create the new class, automatically generate the Source folder for your project, and (usually) open Visual Studio for you.

        You'll see a "Compiling..." or "Adding code..." popup. This is the magic step.

        Close the Editor and Visual Studio.
