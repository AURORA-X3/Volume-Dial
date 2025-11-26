# Volume-Dial

Volume Dial – Technical Documentation

Volume Dial is a wireless hardware system designed to control a computer’s audio volume using a magnetic rotary encoder.
The system is built around multiple ESP32-WROOM-32 modules that communicate through Wi-Fi, with a dedicated PC application handling volume adjustments on the host computer.

This document provides a technical overview of the architecture, communication flows, hardware interfaces, installation procedure, and operational details.

1. System Overview

Volume Dial operates as a distributed system composed of three primary components:

1.1 ESP32 Client

Interfaces with a magnetic rotary encoder through the I²C bus.
Continuously reads the encoder’s angular position.
Converts angular variations into volume increment or decrement commands.
Transmits volume control commands to the ESP32 Server over a Wi-Fi network.

1.2 ESP32 Server

Receives commands from the Client via Wi-Fi.
Connects to the host PC using a USB serial link.
Forwards volume commands to a dedicated PC application.
Optionally sends current volume information to a display module.

1.3 Optional Display Module (Tab5M5)

Displays the real-time volume level.
Receives data from the ESP32 Server.

2. Functional Description
2.1 Magnetic Encoder Interface

Communication protocol: I²C
Typical encoder supported: AS5600 or similar magnetic angle sensors
Resolution dependent on encoder characteristics (typically 12-bit, 4096 steps per revolution)

2.2 Client-Side Processing

Sampling of the encoder angle at a defined frequency
Detection of rotation direction
Calculation of delta angle between samples
Filtering and debouncing to avoid noise-induced fluctuations
Conversion of angular displacement into volume step commands
Wi-Fi-based transmission to the Server

2.3 Server-Side Processing

Wi-Fi server mode (TCP/UDP/MQTT depending on implementation)
Parsing of incoming commands from the Client
Serial communication with the PC via USB
Forwarding commands to the host software, following a defined message protocol
Optional transmission of volume feedback to a display

2.4 PC Application

Receives commands from the Server via serial link
Reads current system volume
Applies volume changes (increment, decrement or absolute value)
Can return updated volume to the Server for display

3. System Architecture

[Magnetic Encoder]
        │ I²C
        ▼
[ESP32 Client] ─── Wi-Fi ───► [ESP32 Server] ─── USB Serial ───► [PC Volume Application]
                                    │
                                    └────► [Optional Display]

4. Communication Protocols
4.1 Client → Server

Medium: Wi-Fi
Transport: configurable (default: TCP or UDP)
Message format (example):
VOL:+1 (increase volume one step)
VOL:-2 (decrease volume two steps)

4.2 Server → PC

Medium: USB serial (CDC)
Baud rate: defined by configuration (typically 115200 bps)
Message protocol identical or adapted from Client format

4.3 Server → Display (optional)

Medium: serial or Wi-Fi
Payload: absolute volume value or percentage

5. Hardware Requirements

Two ESP32-WROOM-32 modules
One magnetic rotary encoder (AS5600, MLX90333 or equivalent)
3.3 V power supply for ESP32 boards
Wi-Fi network for communication
Host PC with USB port
Optional: Tab5M5 display module

6. Firmware Structure
6.1 Client Firmware

I²C driver configuration
Encoder reading task
Wi-Fi client initialization
Command generation logic
Communication handler

6.2 Server Firmware

Wi-Fi server initialization
Socket listener
Serial interface driver
Command forwarding logic
Optional display interface handler

7. Installation Procedure

Flash the Client firmware onto the ESP32 connected to the magnetic encoder.
Flash the Server firmware onto the ESP32 connected to the PC.
Install and start the PC application responsible for reading and adjusting the system volume.
Ensure both ESP32 devices are configured for the same Wi-Fi network.
Power the system. The Client automatically connects to the Server.
Rotate the magnetic encoder to verify volume control operation.
(Optional) Connect and configure the display module (M5 Tab5).

8. Future Enhancements

Configurable sensitivity and acceleration curves for volume control
Enhanced error handling for network instability
Enclosure design and mechanical integration
Support for multiple Clients or multi-channel volume control

9. License

This project is distributed under the MIT License unless otherwise specified.
