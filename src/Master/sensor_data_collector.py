import time
from data_store import SensorDataStore
from master import Master


class SensorDataCollector:
    def __init__(self, master: Master, store: SensorDataStore, interval: float = 1):
        self.master = master
        self.store = store
        self.interval = interval
        self.running = False

    def start(self):
        self.running = True
        self._collect_loop()

    def stop(self):
        self.running = False

    def _collect_loop(self):
        while self.running:
            timestamp = time.strftime("%H:%M:%S")  # Format: 24-hour time

            # Dynamically iterate through all sensors registered in the Master
            for sensor_name, sensor in self.master.sensors.items():
                # Determine how many channels this sensor has; default to 1 if not specified
                channels = getattr(sensor, 'channels', 1)
                for option in range(channels):
                    reading = self.master.read_sensor(sensor_name, option)
                    # Construct a key: for single-channel sensors just use sensor_name,
                    # otherwise use sensor_name_option (e.g., "sgp30_0" for channel 0)
                    key = sensor_name if channels == 1 else f"{sensor_name}_{option}"
                    self.store.add(key, timestamp, reading)

            time.sleep(self.interval)
