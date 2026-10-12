import asyncio
from bleak import BleakScanner

async def main():
    print("Scanning for BLE advertisements with details (5 seconds)...")
    devices = await BleakScanner.discover(timeout=5.0, return_adv=True)
    print(f"Discovered {len(devices)} devices:")
    for addr, (dev, adv) in devices.items():
        name = adv.local_name or dev.name or "(unnamed)"
        print(f"  [{addr}] RSSI={adv.rssi} Name='{name}'")
        if "RACECAR" in name.upper() or "L073" in name.upper():
            print(f"  >>> MATCH FOUND: {name} at {addr} <<<")

if __name__ == "__main__":
    asyncio.run(main())
