import serial
import sensors


class Master:
    def __init__(self, port: str, baudrate: int) -> None:
        print("Creating instance of Master...")
        self.port_name = port
        self.baudrate = baudrate
        self.timeout = 0.1
        self.serial_port = None  # Initialize as None

        self.sensors = {
            "lmt84": sensors.LMT84LP("0x01"),
            "ns1l9m51": sensors.NS1L9M51("0x04"),
            "sgp30": sensors.SGP30("0x05"),
            "dht22": sensors.DHT22("0x06"),
        }

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

    # --- Sensor read wrappers below ---
    def read_lmt84_temp(self) -> float:
        try:
            return self.sensors["lmt84"].read(self.serial_port, 0)
        except Exception as e:
            print(f"Error reading LMT84 temperature: {e}")
            return 0.0

    def read_ns1l9m51_lux(self) -> float:
        try:
            return self.sensors["ns1l9m51"].read(self.serial_port, 0)
        except Exception as e:
            print(f"Error reading NS1L9M51 lux: {e}")
            return 0.0

    def read_sgp30_co2(self) -> int:
        try:
            return self.sensors["sgp30"].read(self.serial_port, 0)
        except Exception as e:
            print(f"Error reading SGP30 CO2: {e}")
            return 0

    def read_sgp30_voc(self) -> int:
        try:
            return self.sensors["sgp30"].read(self.serial_port, 1)
        except Exception as e:
            print(f"Error reading SGP30 VOC: {e}")
            return 0

    def read_dht22_humidity(self) -> float:
        try:
            return self.sensors["dht22"].read(self.serial_port, 0)
        except Exception as e:
            print(f"Error reading DHT22 humidity: {e}")
            return 0.0

    def read_dht22_temp(self) -> float:
        try:
            return self.sensors["dht22"].read(self.serial_port, 1)
        except Exception as e:
            print(f"Error reading DHT22 temperature: {e}")
            return 0.0

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()
