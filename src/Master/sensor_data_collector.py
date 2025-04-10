import time
# your custom class for encapsulating data
from data_store import SensorDataStore
from master import Master


class SensorDataCollector:
    def __init__(self, master: Master, store: SensorDataStore, interval: float = 0.5):
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
        start_time = time.time()
        while self.running:
            timestamp = time.time() - start_time
            temp = self.master.read_lmt84_temp()
            self.store.add("lmt84", timestamp, temp)
            time.sleep(self.interval)
