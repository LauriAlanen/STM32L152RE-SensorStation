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

            # Collect data from each sensor and add it to the store
            lmt84_temp = self.master.read_lmt84_temp()
            self.store.add("lmt84", timestamp, lmt84_temp)

            ns1l9m51_lux = self.master.read_ns1l9m51_lux()
            self.store.add("ns1l9m51", timestamp, ns1l9m51_lux)

            sgp30_co2 = self.master.read_sgp30_co2()
            self.store.add("sgp30_co2", timestamp, sgp30_co2)

            sgp30_voc = self.master.read_sgp30_voc()
            self.store.add("sgp30_voc", timestamp, sgp30_voc)

            dht22_humidity = self.master.read_dht22_humidity()
            self.store.add("dht22_humidity", timestamp, dht22_humidity)

            dht22_temp = self.master.read_dht22_temp()
            self.store.add("dht22_temp", timestamp, dht22_temp)

            time.sleep(self.interval)
