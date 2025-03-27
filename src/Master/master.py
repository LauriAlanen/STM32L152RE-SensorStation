"""
Modbus Master Serial Communication Module

This module provides a `Master` class for communicating with a Modbus slave
device over a serial connection. It supports sending requests, receiving
responses, and computing CRC checksums.

Usage Example:
    from master import Master

    modbus_master = Master(port='COM5', baudrate=9600)
    request_frame = bytearray([0x01, 0x04, 0x00, 0x01, 0x00, 0x01, 0x60, 0x0A])
    response = modbus_master.request_slave(request_frame)
    modbus_master.close()

Functions:
    - Master: Handles serial communication with the Modbus slave.
    - compute_crc: Computes a CRC-16 (Modbus) checksum for error checking.

Author: Lauri Alanen
Date: 2025-03-27
"""

import serial

import sensors


class Master:
    """
    A class to communicate with a Modbus slave over a serial connection.

    Attributes:
        port_name (str): The name of the serial port (e.g., 'COM5').
        baudrate (int): The baud rate for serial communication.
        timeout (int): The timeout value for the serial connection.
        serial_port (serial.Serial): The serial port instance.
    """

    def __init__(self, port: str, baudrate: int) -> None:
        """
        Initializes the Master device with a serial connection.

        Args:
            port (str): The name of the serial port.
            baudrate (int): The baud rate for communication.
        """
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

        self.sensors_list = [
            sensors.SGP30("0x05"),
            sensors.DHT22("0x06")
        ]

    def close(self) -> None:
        """
        Closes the serial port if it is open.
        """
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()

    def read_slave(self, sensor_index: int, option: int) -> int:
        return self.sensors_list[sensor_index].read(self.serial_port, option)

    @staticmethod
    def compute_crc(msg: bytearray) -> int:
        """
        Computes the CRC-16 (Modbus) checksum for a given message.

        Args:
            msg (bytearray): The message for which the CRC is to be calculated.

        Returns:
            int: The computed CRC value.
        """
        crc = 0xFFFF
        for byte in msg:
            crc ^= byte
            for _ in range(8):
                if crc & 1:
                    crc >>= 1
                    crc ^= 0xA001
                else:
                    crc >>= 1
        return crc

    def __enter__(self):
        """Context manager entry: Open the serial port."""
        if not self.serial_port.is_open:
            self.serial_port.open()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        """Context manager exit: Close the serial port when done."""
        self.close()


if __name__ == "__main__":
    with Master("COM5", 9600) as master:
        while True:
            # sensor_value = master.read_slave(0, 0)
            # print(f"SGP30 Co2 : {sensor_value}")

            # sensor_value = master.read_slave(0, 1)
            # print(f"SGP30 VoC : {sensor_value}\n")

            sensor_value = master.read_slave(1, 0)
            print(f"DHT22 Temperature : {sensor_value}\n")

            sensor_value = master.read_slave(1, 1)
            print(f"DHT22 Humidity : {sensor_value}\n")

            if input("Close connection : ") == 'Y':
                break

        master.close()
