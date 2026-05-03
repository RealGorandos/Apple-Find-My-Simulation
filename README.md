# Apple Find My Simulation in OMNeT++

A network simulation project that approximates selected features of Apple's **Find My** ecosystem using **OMNeT++**, **INET**, and **Simu5G**. The simulation models Apple devices, AirTags, a backend server, 5G network components, and short-range wireless communication to study device discovery, location lookup, location sharing, and unknown AirTag detection.

## Project Overview

Apple's Find My network helps users locate Apple devices and AirTags, share locations with trusted contacts, and detect unknown AirTags that may be following them. This project recreates a simplified version of those behaviors in a simulated network environment.

The simulation focuses on three core features:

1. **Apple Device and AirTag Location Retrieval**
   - Apple devices and AirTags periodically send location information.
   - A server stores device locations and responds to lookup requests.

2. **Location Sharing Between Friends**
   - Apple devices maintain friend lists for other Apple devices and AirTags.
   - Devices can share retrieved location data with trusted devices.

3. **Unknown AirTag Detection**
   - Apple devices monitor nearby AirTags.
   - If an unknown AirTag remains close for more than a configured simulation duration, the device logs a tracking/following alert.

## Technologies Used

- **OMNeT++**: Discrete-event simulation framework used to build and run the network simulation.
- **INET Framework**: Provides network models such as hosts, routers, UDP, IPv4 configuration, and wireless radio media.
- **Simu5G**: Adds 5G network components such as NR user equipment, gNodeB, and UPF.
- **C++**: Main implementation language for custom simulation applications.
- **NED**: OMNeT++ network description language used to define modules and topology.
- **Python**: Used for the automated processing pipeline.
- **Matplotlib**: Used to generate result plots from exported simulation data.

## Network Model

The simulated system includes:

- Apple device hosts
- AirTags
- A server
- Router and IP configuration modules
- 5G components, including NR UE, gNodeB, and UPF
- A Bluetooth-like short-range wireless medium modeled using `Ieee802154NarrowbandScalarRadioMedium`

AirTags are modeled using INET `SensorNode` behavior, while Apple devices are implemented through a custom `AppleDeviceNode.ned` module that combines 5G connectivity with short-range wireless communication.

## Main Components

### `ClientApp.h/.cc`

Runs on Apple devices and handles:

- Friend list initialization
- iPhone and AirTag lookup requests
- Sending phone location updates to the server
- Receiving AirTag packets
- Forwarding AirTag data to the server
- Sharing locations with friendly devices
- Detecting unknown AirTags that may be following the device

Packet handling is based on packet type values:

| Packet Type | Meaning |
|---|---|
| `0` | AirTag location packet |
| `1` | AirTag lookup request |
| `2` | AirTag lookup response |
| `3` | iPhone location update |
| `4` | iPhone lookup request |
| `5` | iPhone lookup response |
| `6` | Shared friend-location message |

### `SendLocationApp.h/.cc`

Runs on AirTags and handles:

- Searching for the closest Apple device within wireless range
- Sending AirTag location data to that Apple device
- Using packet type `0` for AirTag location messages

### `ServerApp.h/.cc`

Runs on the backend server and handles:

- Receiving Apple device and AirTag location updates
- Storing location data in server-side lists/databases
- Responding to iPhone and AirTag lookup requests
- Returning a not-found response when requested devices are unavailable

### `DataPacket.msg`

Defines the custom packet format used throughout the simulation. OMNeT++ generates `DataPacket_m.h` and `DataPacket_m.cc` from this file.

Main fields include:

- `type`
- `deviceName`
- `ip`
- `isDeviceFound`
- `deviceLocation`
- `textMessage`

### `AppleDeviceNode.ned`

Custom node definition that combines functionality from Simu5G user equipment and INET wireless sensor behavior. This enables Apple devices to communicate with both:

- The server over the 5G network
- AirTags over the Bluetooth-like wireless medium

## Project Structure

```text
Apple-Find-My-Simulation/
├── FindMy/
│   ├── Makefile
│   ├── images/
│   │   └── <Images used in the project>
│   ├── simulations/
│   │   ├── omnetpp.ini
│   │   ├── network.ned
│   │   ├── config.xml
│   │   └── package.ned
│   └── src/
│       ├── ClientApp.cc
│       ├── ClientApp.h
│       ├── ClientApp.ned
│       ├── DataPacket.msg
│       ├── SendLocationApp.cc
│       ├── SendLocationApp.h
│       ├── SendLocationApp.ned
│       ├── ServerApp.cc
│       ├── ServerApp.h
│       ├── ServerApp.ned
│       ├── UdpBasicAppAux.cc
│       ├── UdpBasicAppAux.h
│       └── AppleDeviceNode.ned
├── exported-results/
│   └── <Simulation and script results>
└── processing/
    └── pipeline_run.py
```

## Processing Pipeline

The project includes an automated Python pipeline that can:

- Build the project
- Run simulation configurations
- Run all configurations
- Clean the working directory
- Extract simulation results from `.vec` and `.sca` files
- Convert selected vectors to CSV using `opp_scavetool`
- Plot available results

Before running the pipeline, source the environment setup file:

```bash
. setenv
```

Then run:

```bash
python3 processing/run_pipeline.py --inet /path/to/inet --simu5g /path/to/Simu5G
```

The pipeline provides an interactive CLI with options such as:

- Run a specific configuration
- Run all configurations
- Build the project
- Clean the working directory
- Plot all available configurations
- Quit

## Experiment Configurations

Several configurations were used to evaluate network behavior:

### General Configuration

Baseline scenario with a moderate number of Apple devices and AirTags. Used as the reference point for comparing other configurations.

### Low Device Amount Configuration

Reduces the number of Apple devices and AirTags. This lowers congestion but also reduces the probability that AirTags are detected by nearby Apple devices.

### High Device Amount Configuration

Increases the number of Apple devices and AirTags. This improves the chance of AirTag detection and successful lookup but increases congestion and can cause delay spikes.

### Long Cable Length Configuration

Uses a longer cable length in the network to study latency and throughput effects caused by longer transmission paths.

## Key Results

- **Device density affects AirTag detection**: More Apple devices and AirTags increase the frequency of unknown AirTag detection.
- **High device density improves AirTag lookup success over time**: With more nearby Apple devices, AirTags are discovered and added to the server database faster.
- **High device density increases network delay**: The high-device configuration produced more delay spikes due to congestion and network load.
- **Server throughput increases with more devices**: More devices generate more lookup requests and location updates, increasing server activity.
- **Longer network paths increase latency**: The long-cable configuration showed larger delay spikes compared with the general configuration.
- **Location sharing scales with device count**: More devices resulted in more friendly location-sharing messages.


## Summary

This project demonstrates a proof-of-concept simulation of selected Find My features in OMNeT++. It models location updates, lookup requests, friend-based sharing, and unknown AirTag tracking alerts across different network configurations. The results show how device density, network congestion, and topology choices influence location discovery, throughput, and latency.

