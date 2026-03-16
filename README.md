# Linux AQI Driver Project

A real-time Linux Air Quality (AQI) monitoring stack featuring a kernel driver (CCS811), Python WebSocket gateway, and a modern web dashboard.

## 🚀 Quick Start

### 1. VM Environment (Ubuntu 24.04)
Run the setup script to provision the Lima VM with necessary kernel headers and I2C tools:
```bash
./scripts/setup-lima-vm.sh
```

### 2. Build & Load
Compile and load the kernel module inside the VM:
```bash
limactl shell aqi-dev -- make -C kernel
limactl shell aqi-dev -- sudo insmod kernel/aqi_sensor.ko
```

### 3. Run Simulation
Launch the real-time simulation and backend:
```bash
# Start backend gateway (Host)
uv run backend/src/sensor_stream.py

# Open Dashboard
open frontend/index.html
```

## 🔍 Walkthrough & Verification

### The Stack
- **Kernel Module (`aqi_sensor.c`)**: A character device driver that communicates via I2C.
- **C Reader (`aqi_reader.c`)**: Interfaces with `/dev/aqi_sensor` to output JSON telemetry.
- **Python Backend**: Bridges the VM guest telemetry to host WebSockets.
- **Frontend**: Real-time visualization with threshold alerts.

### Monitoring Dashboard
![Live Dashboard](assets/dashboard_live.png)
*Real-time sensor data visualized on the dashboard.*

![Disconnected State](assets/disconnected.png)
*Dashboard showing disconnected status when the backend is offline.*

### End-to-End Test
We verified the system using `i2c-stub` to mock a physical sensor:
1.  **Mock Injection**: `i2cset` used inside the VM to simulate high/low AQI levels.
2.  **Telemetry Flow**: Data flows from `I2C Bus` -> `Kernel Driver` -> `C Reader` -> `Python Gateway` -> `Browser`.
3.  **Result**: Successfully visualized real-time 400ppm eCO2 and 100ppb TVOC levels on the dashboard.

## 🛠 Features
- [x] Character Device Interface with IOCTL support.
- [x] Automated VM isolation for safe kernel development.
- [x] High-frequency real-time dashboard updates.
- [x] PEP 723 inline dependency management via `uv`.
