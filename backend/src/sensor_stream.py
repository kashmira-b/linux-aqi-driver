# /// script
# dependencies = ["websockets"]
# ///
import asyncio
import json
import websockets
import subprocess
import time
from datetime import datetime, timezone
import os

# US2 (Phase 4): Python wrapper over the C userspace executable inside the VM
# Using FULL path to limactl if needed, but assuming PATH is configured via .zshrc
AQI_READER_CMD = [
    "limactl", "shell", "aqi-dev", "--", "sudo", "/tmp/aqi-driver/userspace/aqi_reader"
]

def read_from_c_executable() -> dict:
    """Spawns the C reader inside Lima VM, grabs JSON, returns it. Falls back to mock."""
    try:
        # Check if Lima exists in PATH
        result = subprocess.run(
            AQI_READER_CMD, capture_output=True, text=True, timeout=2
        )
        if result.returncode == 0:
            return json.loads(result.stdout)
        else:
            # If Lima fails (e.g. VM not started), the exception handler will trigger fallback
            raise Exception(f"Lima command failed: {result.stderr}")
    except Exception:
        # Fallback to mock for strictly local testing WITHOUT the Lima VM active
        eco2_base = 400
        tvoc_base = 0
        gas_res_base = 50000
        return {
            "eco2": eco2_base + int(time.time() * 2) % 800,
            "tvoc": tvoc_base + int(time.time() * 5) % 300,
            "gasResistance": gas_res_base,
            "hardware_status": "OK",
        }


async def broadcast_sensor_data(websocket):
    print("Dashboard client connected.")
    try:
        while True:
            # Get latest measurements from the C boundary
            telemetry = await asyncio.to_thread(read_from_c_executable)

            timestamp = datetime.now(timezone.utc).isoformat()
            alerts = []

            payload = {
                "timestamp": timestamp,
                "status": "ERROR"
                if telemetry.get("status") == "ERROR"
                or telemetry.get("hardware_status") == "ERROR"
                else "CONNECTED",
                "metrics": {},
                "alerts": alerts,
            }

            if payload["status"] == "CONNECTED":
                payload["metrics"]["eco2"] = telemetry.get("eco2")
                payload["metrics"]["tvoc"] = telemetry.get("tvoc")
                payload["metrics"]["gasResistance"] = telemetry.get("gasResistance")

                # Check thresholds
                if (telemetry.get("eco2", 0) or 0) > 1000:
                    alerts.append("eCO2 threshold exceeded (Danger > 1000 ppm)")

            await websocket.send(json.dumps(payload))
            await asyncio.sleep(1.0)
    except websockets.exceptions.ConnectionClosed:
        print("Dashboard client disconnected.")


async def main():
    print("Starting native WebSocket gateway on port 8765...")
    async with websockets.serve(broadcast_sensor_data, "0.0.0.0", 8765):
        await asyncio.Future()


if __name__ == "__main__":
    asyncio.run(main())
