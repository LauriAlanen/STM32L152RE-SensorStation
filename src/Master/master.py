import serial
from typing import Dict, Any
import sensors


class Master:
    def __init__(self, port: str, baudrate: int) -> None:
        print("Creating instance of Master...")
        self.port_name = port
        self.baudrate = baudrate
        self.timeout = 0.1
        self.serial_port = None

        # Sensors stored by name
        self.sensors: Dict[str, Any] = {}

    def connect(self) -> None:
        if self.serial_port is None:
            print("Opening serial connection...")
            self.serial_port = serial.Serial(
                port=self.port_name,
                baudrate=self.baudrate,
                timeout=self.timeout,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
            )
        elif not self.serial_port.is_open:
            print("Re-opening serial connection...")
            self.serial_port.open()

    def close(self) -> None:
        if self.serial_port and self.serial_port.is_open:
            print("Closing serial connection...")
            self.serial_port.close()

    def add_sensor(self, name: str, sensor_instance: Any) -> None:
        """
        Add a sensor dynamically.

        :param name: Unique name for the sensor (used in reads).
        :param sensor_instance: Instantiated sensor object (e.g., sensors.LMT84LP(...))
        """
        if name in self.sensors:
            raise ValueError(f"Sensor '{name}' already exists.")
        print(f"Adding sensor '{name}'...")
        self.sensors[name] = sensor_instance

    def read_sensor(self, name: str, option: int) -> Any:
        """
        Generic sensor read.

        :param name: Name of the sensor as added via `add_sensor`.
        :param option: Option index to read (e.g., 0 for temp, 1 for humidity).
        :return: Sensor reading or fallback value on error.
        """
        if name not in self.sensors:
            raise ValueError(f"Sensor '{name}' not found.")
        try:
            return self.sensors[name].read(self.serial_port, option)
        except Exception as e:
            print(f"Error reading {name} (option {option}): {e}")
            return 0 if isinstance(option, int) else None

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()
