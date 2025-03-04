# IEC-60870-5-104 Client Control Program

## Overview
This program is a client application that sends control commands using the IEC-60870-5-104 (IEC104) protocol. It allows users to establish a connection to a remote terminal unit (RTU) and send single or double commands to control remote devices. The program ensures proper communication by waiting for a connection acknowledgement (START_DT) and confirmation of command execution before terminating.
## IEC-60870-5-104 Protocol
IEC-60870-5-104 is a standard industrial protocol used in electrical engineering and power system automation. It extends IEC-60870-5-101 by allowing communication over TCP/IP networks. The protocol is widely used for Supervisory Control and Data Acquisition (SCADA) systems, enabling remote monitoring and control of substations, power plants, and industrial automation devices.

### Control Commands in IEC-60870-5-104
IEC104 defines different types of control commands that can be sent to RTUs, including:
- **Single Command (ASDU Type 45)**: Used for switching a device on or off.
- **Double Command (ASDU Type 46)**: Used when two-bit command states are required (e.g., ON, OFF, intermediate states).

Control commands must be confirmed by the RTU before they take effect, ensuring reliable operation.

## Installation
### Prerequisites
- C++17 or later
- [lib60870-5-104 library](https://github.com/mz-automation/lib60870)
- CMake or a compatible build system

### Building the Program
```sh
mkdir build && cd build
cmake ..
make
```

## Usage
The program takes the following command-line arguments:
```sh
iec104_client_control <RTU_IP> <RTU_Port> <IOA> <ASDU_Type> <Value>
```
### Arguments:
- `<RTU_IP>`: IP address of the RTU (e.g., `192.168.1.2`)
- `<RTU_Port>`: Port number of the RTU (default is `2404` for IEC104)
- `<IOA>`: Information Object Address (IOA) of the controlled device
- `<ASDU_Type>`: 45 for Single Command, 46 for Double Command
- `<Value>`: 1 (ON) or 0 (OFF)

### Example Commands:
#### Send a Single Command to turn a device ON:
```sh
iec104_client_control 192.168.1.2 2404 1000 45 1
```
#### Send a Double Command to turn a device OFF:
```sh
iec104_client_control 192.168.1.2 2404 1000 46 0
```

## License
This project is open-source and free to use under the MIT License.

## Contact
For issues or contributions, please contact the developer or submit a pull request on GitHub.

