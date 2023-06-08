import socket
import struct

class PacketSerializer:
    def __init__(self, addr: str, port: int):
        self.addr = addr
        self.port = port
        self.socket = None

    def start(self) -> int:
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.socket.connect((self.addr, self.port))
            return 0  # Success
        except socket.error as e:
            print(f"Socket error: {e}")
            return -1  # Connection failed

    def write(self, type: int, data: bytes, size: int) -> int:
        chksum1 = 0xABCD
        chksum2 = 0xDCBA

        buf1 = struct.pack('!IIH', type, size, chksum1)
        buf2 = struct.pack('!H', chksum2)

        self.socket.sendall(buf1)
        self.socket.sendall(data)
        self.socket.sendall(buf2)

        return 0  # Success

if __name__ == '__main__':
    packet_serializer = PacketSerializer('localhost', 8000)
    status = packet_serializer.start()
    if status == 0:
        message = "Hello, world!"
        packet_serializer.write(1, message.encode(), len(message))
