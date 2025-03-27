import time
import serial

class Master:
    def __init__(self, port: str, baudrate: int) -> None:
        self.port_name = port
        self.baudrate = baudrate
        self.timeout = 2
        self.serial_port = serial.Serial(
            port=self.port_name,
            baudrate=self.baudrate,
            timeout=self.timeout,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
        )
    
    def close(self) -> None:
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()

    def request_slave(self) -> bytearray:
        hex_bytes = [0x05, 0x04, 0x00, 0x01, 0x00, 0x01, 0x8E, 0x61]
        print(f"{time.strftime('%Y%m%d:%H%M%S')}: request: {hex_bytes}")
        
        if not self.serial_port.is_open:
            self.serial_port.open()
        self.serial_port.write(bytearray(hex_bytes))
        received = self.serial_port.read(7)
        time.sleep(1)  # Optional delay
        self.serial_port.close()
        return received

    @staticmethod
    def compute_crc(msg: bytearray) -> int:
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

while True:
    master = Master("COM5", 9600)
    received_value =  master.request_slave()
    
    print(f"Received {received_value}")
    