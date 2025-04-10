import time
import serial
import sensors


class Master:
    def __init__(self, port: str, baudrate: int) -> None:
        self.port_name = port
        self.baudrate = baudrate
        self.timeout = 0.1
        self.serial_port = serial.Serial(
            port=self.port_name,
            baudrate=self.baudrate,
            timeout=self.timeout,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
        )

        self.sensors = {
            "lmt84": sensors.LMT84LP("0x01"),
            "ns1l9m51": sensors.NS1L9M51("0x04"),
            "sgp30": sensors.SGP30("0x05"),
            "dht22": sensors.DHT22("0x06"),
        }

    def close(self) -> None:
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()

    def read_lmt84_temp(self) -> float:
        """Wrapper to read temperature from the LMT84 sensor."""
        return self.sensors["lmt84"].read(self.serial_port, 0)

    def read_ns1l9m51_lux(self) -> float:
        """Wrapper to read lux from the NS1L9M51 sensor."""
        return self.sensors["ns1l9m51"].read(self.serial_port, 0)

    def read_sgp30_co2(self) -> int:
        """Wrapper to read CO2 from the SGP30 sensor (option 0)."""
        return self.sensors["sgp30"].read(self.serial_port, 0)

    def read_sgp30_voc(self) -> int:
        """Wrapper to read VOC from the SGP30 sensor (option 1)."""
        return self.sensors["sgp30"].read(self.serial_port, 1)

    def read_dht22_humidity(self) -> float:
        """Wrapper to read humidity from the DHT22 sensor (option 0)."""
        return self.sensors["dht22"].read(self.serial_port, 0)

    def read_dht22_temp(self) -> float:
        """Wrapper to read temperature from the DHT22 sensor (option 1)."""
        return self.sensors["dht22"].read(self.serial_port, 1)

    def __enter__(self):
        if not self.serial_port.is_open:
            self.serial_port.open()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()


def main() -> None:
    with Master("COM5", 9600) as master:
        try:
            while True:
                temp = master.read_lmt84_temp()
                print(f"LMT84LP Temperature: {temp}")

                lux = master.read_ns1l9m51_lux()
                print(f"NS1L9M51 Lux: {lux}")

                co2 = master.read_sgp30_co2()
                print(f"SGP30 CO2: {co2}")

                voc = master.read_sgp30_voc()
                print(f"SGP30 VOC: {voc}")

                dht_temp = master.read_dht22_temp()
                print(f"DHT22 Temperature: {dht_temp}")

                dht_humidity = master.read_dht22_humidity()
                print(f"DHT22 Humidity: {dht_humidity}\n")

                if input("Close connection (Y to exit): ").strip().upper() == 'Y':
                    break
        finally:
            master.close()


if __name__ == "__main__":
    main()
