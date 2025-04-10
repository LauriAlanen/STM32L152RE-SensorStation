from collections import defaultdict
import threading


class SensorDataStore:
    def __init__(self):
        self.data = defaultdict(lambda: {"times": [], "values": []})
        self.lock = threading.Lock()

    def add(self, sensor: str, time: str, value: float):
        with self.lock:
            self.data[sensor]["times"].append(time)
            self.data[sensor]["values"].append(value)

    def dump(self):
        with self.lock:
            return dict(self.data)
