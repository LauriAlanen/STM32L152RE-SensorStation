import serial


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

        # Send the request frame to the sensor
        serial_port.write(request_frame)

        # Read the raw data from the sensor (adjust length as needed)
        raw_value = bytearray(serial_port.read(7))

        # Use the passed conversion method to process the raw data
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
        # Received b'\x05\x04\x02\x01\xa4'
        msb = modbus_frame[3]
        lsb = modbus_frame[4]

        # Reconstruct the uint16_t value
        raw_value = (msb << 8) | lsb

        return raw_value
