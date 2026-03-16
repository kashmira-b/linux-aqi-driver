# AQI Backend

Python WebSocket gateway that bridges the Linux VM guest telemetry to the Host dashboard.

## Overview
- **Gateway**: Uses `limactl shell` to execute the C reader inside the VM.
- **WebSocket**: Broadcasts JSON telemetry on port `8765`.
- **Environment**: Managed via `uv` with PEP 723 inline dependencies.

## Usage
```bash
uv run sensor_stream.py
```

## Dependencies
- `websockets`
- `asyncio`
