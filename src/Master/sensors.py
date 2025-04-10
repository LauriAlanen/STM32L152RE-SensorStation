import time
from time import sleep
import serial
import math

ADC_STEP_SIZE_U = 3.3 / 4095


class Sensor:
    """
    A base class for sensors that provides a generic read method.
    """
    @staticmethod
    def read_sensor(serial_port: serial.Serial, request_frame: bytearray, convert_method) -> int:
        """
        Generic method to read sensor data.

        Args:
            serial_port (serial.Serial): The serial port object used for communication.
            request_frame (bytearray): The request frame to send to the sensor.
            convert_method (function): The conversion method to process the raw data.

        Returns:
            int: The converted sensor data value.
        """
        if not serial_port.is_open:
            serial_port.open()

        serial_port.write(request_frame)

        raw_value = bytearray(serial_port.read(7))

        converted_value = convert_method(raw_value)

        return converted_value


class SGP30(Sensor):
    """Air quality sensor (SGP30)"""

    def __init__(self, name):
        self.name = name
        self.co2_request_frame = bytearray(
            [0x05, 0x04, 0x00, 0x01, 0x00, 0x01, 0x8E, 0x61])
        self.voc_request_frame = bytearray(
            [0x05, 0x04, 0x00, 0x02, 0x00, 0x01, 0x8E, 0x91])

    def read(self, serial_port: serial.Serial, option: int) -> int:
        """Reads CO2 data from the SGP30 sensor"""

        if option == 0:
            return self.read_sensor(serial_port, self.co2_request_frame, self.convert)

        return self.read_sensor(serial_port, self.voc_request_frame, self.convert)

    @staticmethod
    def convert(modbus_frame: bytearray) -> int:
        """Converts raw data to CO2 (ppm) and VOC (ppb)"""
        msb = modbus_frame[3]
        lsb = modbus_frame[4]

        # Reconstruct the uint16_t value
        raw_value = (msb << 8) | lsb

        return raw_value


class DHT22(Sensor):
    def __init__(self, name):
        self.name = name
        self.temperature_request_frame = bytearray(
            [0x06, 0x04, 0x00, 0x01, 0x00, 0x01, 0xBD, 0x61])
        self.humidity_request_frame = bytearray(
            [0x06, 0x04, 0x00, 0x02, 0x00, 0x01, 0xBD, 0x91])
        self.last_read_time = None  # Track last read timestamp

    def read(self, serial_port: serial.Serial, option: int) -> int:
        # Enforce cooldown if this is not the first read
        if self.last_read_time is not None:
            elapsed_time = time.time() - self.last_read_time
            if elapsed_time < 0.5:
                # Wait for remaining cooldown time
                time.sleep(0.5 - elapsed_time)

        if option == 0:
            value = self.read_sensor(
                serial_port, self.temperature_request_frame, self.convert)
        else:
            value = self.read_sensor(
                serial_port, self.humidity_request_frame, self.convert)

        # Update last read timestamp
        self.last_read_time = time.time()
        return value

    @staticmethod
    def convert(modbus_frame: bytearray) -> float:
        """Converts raw data to uint16_t value"""
        msb = modbus_frame[3]
        lsb = modbus_frame[4]

        # Combine MSB and LSB into a single 16-bit value
        raw_value = (msb << 8) | lsb

        # Interpret it as a value with one decimal place
        return raw_value / 10.0


class NS1L9M51(Sensor):
    def __init__(self, name):
        self.name = name
        self.request_frame = bytearray(
            [0x04, 0x04, 0x00, 0x01, 0x00, 0x01, 0x5F, 0x60])

    def read(self, serial_port: serial.Serial, option: int) -> int:
        return self.read_sensor(serial_port, self.request_frame, self.convert)

    @staticmethod
    def convert(modbus_frame: bytearray) -> float:
        msb = modbus_frame[3]
        lsb = modbus_frame[4]

        # Reconstruct the uint16_t value
        adc_result = (msb << 8) | lsb

        # Apply the voltage and lux calculation
        voltage = ADC_STEP_SIZE_U * adc_result
        lux = 1.9634 * math.exp(2.1281 * voltage)

        return lux


class LMT84LP(Sensor):
    def __init__(self, name):
        self.name = name
        self.request_frame = bytearray(
            [0x01, 0x04, 0x00, 0x01, 0x00, 0x01, 0x0A, 0x60])

    def read(self, serial_port: serial.Serial, option: int) -> int:
        return self.read_sensor(serial_port, self.request_frame, self.convert)

    @staticmethod
    def convert(modbus_frame: bytearray) -> float:
        t_max = 150.0
        t_min = -50.0
        u_min = 1.299
        u_max = 0.183

        # Extract ADC result from modbus_frame
        msb = modbus_frame[3]
        lsb = modbus_frame[4]

        # Combine MSB and LSB to get the ADC result (16-bit value)
        adc_result = (msb << 8) | lsb

        # Calculate the voltage (assuming ADC_STEP_SIZE_U is known)
        voltage = ADC_STEP_SIZE_U * adc_result

        # Calculate the temperature using the given formula
        temperature = ((voltage - u_min) / (u_max - u_min)) * \
            (t_max - t_min) + t_min

        return round(temperature, 1)
