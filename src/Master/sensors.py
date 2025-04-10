import time
import serial
import math

# Defined constant for ADC conversion
ADC_STEP_SIZE_U = 3.3 / 4095


class Sensor:
    """
    Base sensor class with a generic method for reading sensor data.
    """
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
        raw_value = bytearray(serial_port.read(7))

        if len(raw_value) < 5:
            raise ValueError("Received incomplete data frame from sensor.")

        return convert_method(raw_value)


class SGP30(Sensor):
    """Air quality sensor (SGP30) handling CO2 and VOC readings."""

    def __init__(self, name: str):
        self.name = name
        # Precomputed request frames for each type
        self.co2_request_frame = bytearray(
            [0x05, 0x04, 0x00, 0x01, 0x00, 0x01, 0x8E, 0x61])
        self.voc_request_frame = bytearray(
            [0x05, 0x04, 0x00, 0x02, 0x00, 0x01, 0x8E, 0x91])

    def read(self, serial_port: serial.Serial, option: int) -> float:
        """
        Read sensor data.

        Args:
            serial_port (serial.Serial): Serial connection.
            option (int): 0 for CO2 (ppm), any other integer for VOC (ppb).

        Returns:
            int: Sensor reading.
        """
        if option == 0:
            return self.read_sensor(serial_port, self.co2_request_frame, self.convert)
        else:
            return self.read_sensor(serial_port, self.voc_request_frame, self.convert)

    @staticmethod
    def convert(modbus_frame: bytearray) -> int:
        """
        Convert raw frame bytes into a 16-bit integer.

        Args:
            modbus_frame (bytearray): Raw bytes from the sensor.

        Returns:
            int: Converted value.
        """
        # Using byte indices 3 and 4 for MSB and LSB respectively
        msb = modbus_frame[3]
        lsb = modbus_frame[4]
        return (msb << 8) | lsb


class DHT22(Sensor):
    """
    Temperature and humidity sensor DHT22.

    Enforces a minimum delay between consecutive readings.
    """

    def __init__(self, name: str):
        self.name = name
        self.temperature_request_frame = bytearray(
            [0x06, 0x04, 0x00, 0x01, 0x00, 0x01, 0xBD, 0x61])
        self.humidity_request_frame = bytearray(
            [0x06, 0x04, 0x00, 0x02, 0x00, 0x01, 0xBD, 0x91])
        self.last_read_time: float = 0.0  # Initialized to epoch zero

    def read(self, serial_port: serial.Serial, option: int) -> float:
        """
        Read temperature or humidity from the DHT22 sensor.

        Args:
            serial_port (serial.Serial): Serial connection.
            option (int): 0 for temperature, any other integer for humidity.

        Returns:
            float: Converted sensor reading.
        """
        # Enforce at least 0.5s cooldown between reads
        elapsed_time = time.time() - self.last_read_time
        if elapsed_time < 0.5:
            time.sleep(0.5 - elapsed_time)

        if option == 0:
            value = self.read_sensor(
                serial_port, self.temperature_request_frame, self.convert)
        else:
            value = self.read_sensor(
                serial_port, self.humidity_request_frame, self.convert)

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

    def __init__(self, name: str):
        self.name = name
        self.request_frame = bytearray(
            [0x04, 0x04, 0x00, 0x01, 0x00, 0x01, 0x5F, 0x60])

    def read(self, serial_port: serial.Serial, option: int = 0) -> float:
        """
        Read the lux value from the sensor.

        Args:
            serial_port (serial.Serial): Serial connection.
            option (int): Not used for this sensor.

        Returns:
            float: Calculated lux value.
        """
        return self.read_sensor(serial_port, self.request_frame, self.convert)

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

    def __init__(self, name: str):
        self.name = name
        self.request_frame = bytearray(
            [0x01, 0x04, 0x00, 0x01, 0x00, 0x01, 0x0A, 0x60])

    def read(self, serial_port: serial.Serial, option: int = 0) -> float:
        """
        Read the temperature from the sensor.

        Args:
            serial_port (serial.Serial): Serial connection.
            option (int): Not used for this sensor.

        Returns:
            float: Temperature in degrees Celsius.
        """
        return self.read_sensor(serial_port, self.request_frame, self.convert)

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

        # Linear mapping from voltage to temperature based on calibration
        temperature = ((voltage - u_min) / (u_max - u_min)) * \
            (t_max - t_min) + t_min
        return round(temperature, 1)
