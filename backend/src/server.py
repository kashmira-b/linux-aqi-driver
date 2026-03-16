import asyncio
import json
import websockets
import time
from datetime import datetime, timezone

# For User Story 1 (Phase 3 MVP), this server generates mock JSON data.
# In Phase 4 (US2), it will read from the userspace C process.


async def broadcast_sensor_data(websocket):
    print("Dashboard client connected.")
    eco2_base = 400
    tvoc_base = 0
    gas_res_base = 50000

    try:
        while True:
            # Simulate a continuous 1Hz stream with some jitter
            timestamp = datetime.now(timezone.utc).isoformat()

            # Add stochastic drift
            eco2 = eco2_base + int(time.time() * 2) % 800
            tvoc = tvoc_base + int(time.time() * 5) % 300

            alerts = []
            if eco2 > 1000:
                alerts.append("eCO2 threshold exceeded (Danger > 1000 ppm)")

            payload = {
                "timestamp": timestamp,
                "status": "CONNECTED",
                "metrics": {"eco2": eco2, "tvoc": tvoc, "gasResistance": gas_res_base},
                "alerts": alerts,
            }

            await websocket.send(json.dumps(payload))
            await asyncio.sleep(1.0)
    except websockets.exceptions.ConnectionClosed:
        print("Dashboard client disconnected.")


async def main():
    print("Starting minimal WebSocket server on port 8765...")
    async with websockets.serve(broadcast_sensor_data, "localhost", 8765):
        await asyncio.Future()  # run forever


if __name__ == "__main__":
    asyncio.run(main())
