import time
import serial
import math

# Defined constant for ADC conversion
ADC_STEP_SIZE_U = 3.3 / 4095


def modbus_crc(data: bytearray) -> bytearray:
    """
    Calculate the Modbus RTU CRC16 for the provided data.

    Args:
        data (bytearray): Frame bytes to calculate the CRC for.

    Returns:
        bytearray: A two-byte CRC (high byte first then low byte).
    """
    crc = 0xFFFF
    for pos in data:
        crc ^= pos
        for _ in range(8):
            if crc & 0x0001:
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    return bytearray([(crc >> 8) & 0xFF, crc & 0xFF])


def build_modbus_request(address: int, register: int, count: int) -> bytearray:
    """
    Build a dynamic Modbus request frame.

    Args:
        address (int): The Modbus address of the sensor.
        register (int): The register address to start reading from.
        count (int): The number of registers to read.

    Returns:
        bytearray: The complete request frame including the CRC.
    """
    frame = bytearray([address, 0x04,
                       (register >> 8) & 0xFF, register & 0xFF,
                       (count >> 8) & 0xFF, count & 0xFF])
    frame.extend(modbus_crc(frame))
    return frame


class Sensor:
    """
    Base sensor class with a generic method for reading sensor data.
    """

    def __init__(self, address: int, name: str):
        self.name = name
        self.address = address
        self.units = {}  # Dictionary to store units for each channel
        self.channel_names = {}  # Dictionary to store names for each channel

    @staticmethod
    def read_sensor(serial_port: serial.Serial, request_frame: bytearray, convert_method) -> float:
        """
        Send a request frame and convert the received data.

        Args:
            serial_port (serial.Serial): Serial connection for the sensor.
            request_frame (bytearray): Command to request data.
            convert_method (callable): Function to convert raw data into desired units.

        Returns:
            float: Converted sensor reading.

        Raises:
            ValueError: If the raw data frame is not of the expected length.
        """
        if not serial_port.is_open:
            serial_port.open()

        serial_port.write(request_frame)
        # Print the frame with 2 bytes at a time for debugging
        print("Request Frame: ", end='')
        for i in range(0, len(request_frame), 2):
            print(f"{request_frame[i]:02X} {request_frame[i+1]:02X}", end=' ')
        print()

        raw_value = bytearray(serial_port.read(7))
        if len(raw_value) < 5:
            raise ValueError("Received incomplete data frame from sensor.")
        return convert_method(raw_value)


class SGP30(Sensor):
    """Air quality sensor (SGP30) handling CO2 and VOC readings."""

    def __init__(self, address: int, name: str):
        super().__init__(address, name)
        self.channels = 2  # Option 0: CO2, Option 1: VOC
        self.units = {0: "ppm", 1: "ppb"}  # CO2 in ppm, VOC in ppb
        self.channel_names = {0: "CO2 Concentration", 1: "VOC Concentration"}

    def read(self, serial_port: serial.Serial, option: int) -> float:
        """
        Read sensor data.

        Args:
            serial_port (serial.Serial): Serial connection.
            option (int): 0 for CO2 (ppm), 1 for VOC (ppb).

        Returns:
            float: Sensor reading.
        """
        # For SGP30, assume register 0x0001 holds CO2 and register 0x0002 holds VOC
        if option == 0:
            request_frame = build_modbus_request(self.address, 0x01, 1)
        else:
            request_frame = build_modbus_request(self.address, 0x02, 1)

        return self.read_sensor(serial_port, request_frame, self.convert)

    @staticmethod
    def convert(modbus_frame: bytearray) -> int:
        """
        Convert raw frame bytes into a 16-bit integer.

        Args:
            modbus_frame (bytearray): Raw bytes from the sensor.

        Returns:
            int: Converted value.
        """
        msb = modbus_frame[3]
        lsb = modbus_frame[4]
        return (msb << 8) | lsb


class DHT22(Sensor):
    """
    Temperature and humidity sensor DHT22.

    Enforces a minimum delay between consecutive readings.
    """

    def __init__(self, address: int, name: str):
        super().__init__(address, name)
        self.last_read_time: float = 0.0
        self.channels = 2  # Option 0: Humidity, Option 1: Temperature
        self.units = {0: "%", 1: "°C"}  # Humidity in %, Temperature in °C
        self.channel_names = {0: "Relative Humidity", 1: "Temperature"}

    def read(self, serial_port: serial.Serial, option: int) -> float:
        """
        Read temperature or humidity from the DHT22 sensor.

        Args:
            serial_port (serial.Serial): Serial connection.
            option (int): 0 for temperature, 1 for humidity.

        Returns:
            float: Converted sensor reading.
        """
        # Ensure at least a 0.5s pause between consecutive reads
        elapsed_time = time.time() - self.last_read_time
        if elapsed_time < 0.5:
            time.sleep(0.5 - elapsed_time)

        # For DHT22, assume register 0x0001 is for temperature and 0x0002 for humidity.
        if option == 0:
            request_frame = build_modbus_request(self.address, 0x01, 1)
        else:
            request_frame = build_modbus_request(self.address, 0x02, 1)

        value = self.read_sensor(serial_port, request_frame, self.convert)
        self.last_read_time = time.time()
        return value

    @staticmethod
    def convert(modbus_frame: bytearray) -> float:
        """
        Convert raw frame bytes to a value with one decimal place.

        Args:
            modbus_frame (bytearray): Raw sensor bytes.

        Returns:
            float: Read value scaled to one decimal.
        """
        msb = modbus_frame[3]
        lsb = modbus_frame[4]
        raw_value = (msb << 8) | lsb
        return raw_value / 10.0


class NS1L9M51(Sensor):
    """Light sensor NS1L9M51 converting ADC readings to lux."""

    def __init__(self, address: int, name: str):
        super().__init__(address, name)
        self.channels = 1  # Only one reading (lux)
        self.units = {0: "lux"}  # Light intensity in lux
        self.channel_names = {0: "Light Intensity"}

    def read(self, serial_port: serial.Serial, option: int = 0) -> float:
        """
        Read the lux value from the sensor.

        Args:
            serial_port (serial.Serial): Serial connection.
            option (int): Not used for this sensor.

        Returns:
            float: Calculated lux value.
        """
        # Use register 0x0001 for NS1L9M51
        request_frame = build_modbus_request(self.address, 0x01, 1)
        return self.read_sensor(serial_port, request_frame, self.convert)

    @staticmethod
    def convert(modbus_frame: bytearray) -> float:
        """
        Convert ADC reading to lux.

        Args:
            modbus_frame (bytearray): Raw sensor bytes.

        Returns:
            float: Calculated lux.
        """
        msb = modbus_frame[3]
        lsb = modbus_frame[4]
        adc_result = (msb << 8) | lsb
        voltage = ADC_STEP_SIZE_U * adc_result
        lux = 1.9634 * math.exp(2.1281 * voltage)
        return lux


class LMT84LP(Sensor):
    """
    Temperature sensor LMT84LP that converts ADC readings to temperature.
    """

    def __init__(self, address: int, name: str):
        super().__init__(address, name)
        self.channels = 1  # Only one reading (temperature)
        self.units = {0: "°C"}  # Temperature in °C
        self.channel_names = {0: "Temperature"}

    def read(self, serial_port: serial.Serial, option: int = 0) -> float:
        """
        Read the temperature from the sensor.

        Args:
            serial_port (serial.Serial): Serial connection.
            option (int): Not used for this sensor.

        Returns:
            float: Temperature in degrees Celsius.
        """
        # Use register 0x0001 for LMT84LP
        request_frame = build_modbus_request(self.address, 0x01, 1)
        return self.read_sensor(serial_port, request_frame, self.convert)

    @staticmethod
    def convert(modbus_frame: bytearray) -> float:
        """
        Convert raw frame bytes to temperature using calibration constants.

        Args:
            modbus_frame (bytearray): Raw sensor bytes.

        Returns:
            float: Rounded temperature value.
        """
        # Calibration constants: adjust if sensor calibration changes
        t_max = 150.0
        t_min = -50.0
        u_min = 1.299
        u_max = 0.183

        msb = modbus_frame[3]
        lsb = modbus_frame[4]
        adc_result = (msb << 8) | lsb
        voltage = ADC_STEP_SIZE_U * adc_result

        # Map voltage linearly to the temperature range
        temperature = ((voltage - u_min) / (u_max - u_min)) * \
            (t_max - t_min) + t_min
        return round(temperature, 1)
