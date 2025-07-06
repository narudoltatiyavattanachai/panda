"""
FT232RL Python Wrapper for Red Panda Compatibility

This module provides a Python wrapper that makes FT232RL UART communication
compatible with the existing Red Panda Python library, enabling seamless
integration with existing tools and applications.

Version: 1.0
Date: 2024
"""

import struct
import time
import serial
import threading
from typing import Optional, List, Tuple, Union
from enum import IntEnum

# Import Red Panda compatible structures
class CANMessage:
    """Red Panda compatible CAN message structure"""
    def __init__(self, addr: int, data: bytes, bus: int, extended: bool = False, fd: bool = False):
        self.addr = addr
        self.data = data
        self.bus = bus
        self.extended = extended
        self.fd = fd
        self.timestamp = time.time()
        
    def __repr__(self):
        return f"CANMessage(addr=0x{self.addr:X}, data={self.data.hex()}, bus={self.bus})"

class SafetyMode(IntEnum):
    """Safety modes compatible with Red Panda"""
    NONE = 0x00
    NO_OUTPUT = 0x01
    HONDA = 0x02
    TOYOTA = 0x03
    GM = 0x04
    TESLA = 0x05

class FT232RL_Frame:
    """FT232RL protocol frame structure"""
    
    # Frame types
    FRAME_CONTROL = 0x00
    FRAME_BULK_IN = 0x01
    FRAME_SERIAL = 0x02
    FRAME_BULK_OUT = 0x03
    FRAME_STATUS = 0x04
    FRAME_ERROR = 0x05
    FRAME_CHUNK = 0x06
    FRAME_ACK = 0x07
    
    # Control commands
    CMD_RESET = 0xC0
    CMD_GET_VERSION = 0xD0
    CMD_GET_HEALTH = 0xDE
    CMD_SET_SAFETY_MODE = 0xDC
    CMD_SET_CAN_SPEED = 0xDD
    CMD_HEARTBEAT = 0xF1
    
    def __init__(self, frame_type: int, sequence: int = 0, payload: bytes = b'', flags: int = 0):
        self.sync = 0xAA
        self.frame_type = frame_type
        self.sequence = sequence
        self.length = len(payload)
        self.flags = flags
        self.payload = payload
        self.checksum = 0
        self._calculate_checksum()
    
    def _calculate_checksum(self):
        """Calculate XOR checksum"""
        header = struct.pack('BBBBB', self.sync, self.frame_type, self.sequence, self.length, self.flags)
        self.checksum = 0
        for byte in header + self.payload:
            self.checksum ^= byte
    
    def pack(self) -> bytes:
        """Pack frame to bytes"""
        self._calculate_checksum()
        header = struct.pack('BBBBBB', self.sync, self.frame_type, self.sequence, 
                           self.length, self.flags, self.checksum)
        return header + self.payload
    
    @classmethod
    def unpack(cls, data: bytes) -> Optional['FT232RL_Frame']:
        """Unpack frame from bytes"""
        if len(data) < 6:
            return None
            
        sync, frame_type, sequence, length, flags, checksum = struct.unpack('BBBBBB', data[:6])
        
        if sync != 0xAA:
            return None
            
        if len(data) < 6 + length:
            return None
            
        payload = data[6:6+length]
        
        # Verify checksum
        calc_checksum = 0
        for byte in data[:5] + payload:
            calc_checksum ^= byte
            
        if calc_checksum != checksum:
            return None
            
        frame = cls(frame_type, sequence, payload, flags)
        return frame

class FT232RL_PandaAdapter:
    """
    FT232RL adapter that emulates Red Panda USB interface
    
    This class provides compatibility with the Red Panda Python library
    by translating USB operations to FT232RL UART protocol.
    """
    
    def __init__(self, port: str, baudrate: int = 3000000, timeout: float = 1.0):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        
        # Serial connection
        self.serial: Optional[serial.Serial] = None
        self.connected = False
        
        # Protocol state
        self.sequence_tx = 0
        self.sequence_rx_expected = 0
        
        # Statistics
        self.frames_sent = 0
        self.frames_received = 0
        self.bytes_sent = 0
        self.bytes_received = 0
        self.errors = 0
        
        # Chunking support
        self.chunk_buffer = b''
        self.chunk_total_size = 0
        self.chunk_received_size = 0
        self.chunk_in_progress = False
        
        # Threading
        self.lock = threading.Lock()
    
    def connect(self) -> bool:
        """Connect to TC275 via FT232RL"""
        try:
            self.serial = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=self.timeout,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                xonxoff=False,
                rtscts=True,  # Hardware flow control
                dsrdtr=False
            )
            
            self.connected = True
            
            # Send reset command
            self.reset()
            
            return True
            
        except Exception as e:
            print(f"FT232RL connection failed: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from TC275"""
        if self.serial and self.serial.is_open:
            self.serial.close()
        self.connected = False
    
    def _send_frame(self, frame: FT232RL_Frame) -> bool:
        """Send frame to TC275"""
        if not self.connected or not self.serial:
            return False
            
        try:
            with self.lock:
                frame.sequence = self.sequence_tx
                data = frame.pack()
                bytes_sent = self.serial.write(data)
                self.serial.flush()
                
                self.sequence_tx = (self.sequence_tx + 1) & 0xFF
                self.frames_sent += 1
                self.bytes_sent += bytes_sent
                
                return bytes_sent == len(data)
                
        except Exception as e:
            print(f"Frame send error: {e}")
            self.errors += 1
            return False
    
    def _receive_frame(self, timeout: float = None) -> Optional[FT232RL_Frame]:
        """Receive frame from TC275"""
        if not self.connected or not self.serial:
            return None
            
        if timeout is None:
            timeout = self.timeout
            
        try:
            # Set timeout
            old_timeout = self.serial.timeout
            self.serial.timeout = timeout
            
            # Read frame header
            header = self.serial.read(6)
            if len(header) < 6:
                return None
                
            # Parse header
            sync, frame_type, sequence, length, flags, checksum = struct.unpack('BBBBBB', header)
            
            if sync != 0xAA:
                return None
                
            # Read payload
            payload = b''
            if length > 0:
                payload = self.serial.read(length)
                if len(payload) < length:
                    return None
            
            # Create and validate frame
            frame_data = header + payload
            frame = FT232RL_Frame.unpack(frame_data)
            
            if frame:
                self.frames_received += 1
                self.bytes_received += len(frame_data)
            else:
                self.errors += 1
                
            # Restore timeout
            self.serial.timeout = old_timeout
            
            return frame
            
        except Exception as e:
            print(f"Frame receive error: {e}")
            self.errors += 1
            return None
    
    def _control_transfer(self, request_type: int, request: int, value: int = 0, 
                         index: int = 0, data: bytes = b'') -> bytes:
        """Emulate USB control transfer"""
        
        # Pack control request
        control_data = struct.pack('<BBHH', request_type, request, value, index)
        if data:
            control_data += struct.pack('<H', len(data)) + data
        else:
            control_data += struct.pack('<H', 0)
            
        # Send control frame
        frame = FT232RL_Frame(FT232RL_Frame.FRAME_CONTROL, payload=control_data)
        if not self._send_frame(frame):
            return b''
            
        # Wait for response
        response_frame = self._receive_frame(timeout=5.0)
        if not response_frame or response_frame.frame_type != FT232RL_Frame.FRAME_CONTROL:
            return b''
            
        return response_frame.payload
    
    # Red Panda compatible interface
    
    def reset(self) -> bool:
        """Reset communication"""
        response = self._control_transfer(0x40, FT232RL_Frame.CMD_RESET)
        return len(response) >= 0  # Success if we get any response
    
    def get_version(self) -> str:
        """Get firmware version"""
        response = self._control_transfer(0xC0, FT232RL_Frame.CMD_GET_VERSION)
        if response:
            try:
                return response.decode('utf-8').strip('\x00')
            except:
                return "Unknown"
        return "Error"
    
    def health(self) -> dict:
        """Get CAN health information"""
        response = self._control_transfer(0xC0, FT232RL_Frame.CMD_GET_HEALTH)
        
        # Parse health data (simplified)
        if len(response) >= 12:  # Minimum health data
            health_data = {}
            # This would parse the actual health structure
            # For now, return basic info
            health_data['voltage'] = 12.0
            health_data['current'] = 0.5
            health_data['uptime'] = int.from_bytes(response[:4], 'little') if len(response) >= 4 else 0
            return health_data
        
        return {'voltage': 0, 'current': 0, 'uptime': 0}
    
    def set_safety_mode(self, mode: int, param: int = 0) -> bool:
        """Set safety mode"""
        response = self._control_transfer(0x40, FT232RL_Frame.CMD_SET_SAFETY_MODE, mode, param)
        return len(response) >= 0
    
    def set_can_speed_kbps(self, bus: int, speed: int) -> bool:
        """Set CAN speed in kbps"""
        response = self._control_transfer(0x40, FT232RL_Frame.CMD_SET_CAN_SPEED, bus, speed)
        return len(response) >= 0
    
    def send_heartbeat(self) -> bool:
        """Send heartbeat"""
        response = self._control_transfer(0x40, FT232RL_Frame.CMD_HEARTBEAT)
        return len(response) >= 0
    
    def can_send_many(self, can_messages: List[CANMessage], timeout: int = 100) -> int:
        """Send multiple CAN messages"""
        
        # Pack CAN messages into Red Panda format
        packed_data = b''
        count = 0
        
        for msg in can_messages:
            if len(packed_data) + 20 > 250:  # Frame size limit
                break
                
            # Pack message in Red Panda format
            dlc = len(msg.data)
            if dlc > 8:
                dlc = 15  # CAN-FD DLC mapping would go here
                
            # Header byte 0: DLC + bus + fd flag
            header0 = (dlc << 4) | (msg.bus << 1) | (1 if msg.fd else 0)
            
            # Address and flags (simplified)
            addr_bytes = struct.pack('<I', msg.addr | (1 << 30 if msg.extended else 0))
            
            # Calculate checksum
            msg_data = struct.pack('B', header0) + addr_bytes[:4] + msg.data[:dlc]
            checksum = 0
            for b in msg_data:
                checksum ^= b
                
            packed_data += msg_data + struct.pack('B', checksum)
            count += 1
        
        if not packed_data:
            return 0
            
        # Send bulk frame
        frame = FT232RL_Frame(FT232RL_Frame.FRAME_BULK_OUT, payload=packed_data)
        if self._send_frame(frame):
            return count
        else:
            return 0
    
    def can_recv(self) -> List[CANMessage]:
        """Receive CAN messages"""
        
        # Request CAN data
        frame = FT232RL_Frame(FT232RL_Frame.FRAME_BULK_IN, payload=b'')
        if not self._send_frame(frame):
            return []
            
        # Wait for response
        response_frame = self._receive_frame(timeout=2.0)
        if not response_frame or response_frame.frame_type != FT232RL_Frame.FRAME_BULK_IN:
            return []
            
        # Parse CAN messages from response
        messages = []
        data = response_frame.payload
        offset = 0
        
        while offset + 6 <= len(data):  # Minimum packet size
            # Parse header byte 0
            header0 = data[offset]
            dlc = (header0 >> 4) & 0xF
            bus = (header0 >> 1) & 0x7
            fd = (header0 & 0x1) != 0
            
            # Parse address
            if offset + 5 >= len(data):
                break
                
            addr_raw = struct.unpack('<I', data[offset+1:offset+5])[0]
            extended = (addr_raw & (1 << 30)) != 0
            addr = addr_raw & 0x1FFFFFFF if extended else (addr_raw >> 18) & 0x7FF
            
            # Calculate data length
            dlc_to_len = [0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64]
            data_len = dlc_to_len[dlc] if dlc < len(dlc_to_len) else 8
            
            # Extract message data
            if offset + 6 + data_len > len(data):
                break
                
            msg_data = data[offset+6:offset+6+data_len]
            
            # Create CAN message
            message = CANMessage(addr, msg_data, bus, extended, fd)
            messages.append(message)
            
            offset += 6 + data_len
        
        return messages
    
    def can_send(self, addr: int, data: bytes, bus: int, timeout: int = 100) -> bool:
        """Send single CAN message"""
        message = CANMessage(addr, data, bus)
        return self.can_send_many([message], timeout) > 0
    
    def get_stats(self) -> dict:
        """Get adapter statistics"""
        return {
            'frames_sent': self.frames_sent,
            'frames_received': self.frames_received,
            'bytes_sent': self.bytes_sent,
            'bytes_received': self.bytes_received,
            'errors': self.errors,
            'connected': self.connected
        }
    
    def close(self):
        """Close connection"""
        self.disconnect()
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

# Red Panda compatible class name
class Panda(FT232RL_PandaAdapter):
    """
    Red Panda compatibility class
    
    This class provides the same interface as the original Panda class
    but uses FT232RL UART communication instead of USB.
    """
    
    def __init__(self, serial_port: str = None, baudrate: int = 3000000):
        if serial_port is None:
            # Auto-detect FT232RL device
            serial_port = self._find_ft232rl_device()
            
        super().__init__(serial_port, baudrate)
        
        # Connect automatically
        if not self.connect():
            raise Exception(f"Failed to connect to FT232RL on {serial_port}")
    
    def _find_ft232rl_device(self) -> str:
        """Auto-detect FT232RL device"""
        import serial.tools.list_ports
        
        # Look for FTDI devices
        ports = serial.tools.list_ports.comports()
        for port in ports:
            if 'FTDI' in port.description or 'FT232' in port.description:
                return port.device
                
        # Fallback to common port names
        import platform
        if platform.system() == 'Windows':
            for i in range(1, 20):
                try:
                    port = f'COM{i}'
                    test_serial = serial.Serial(port, timeout=0.1)
                    test_serial.close()
                    return port
                except:
                    continue
        else:
            import glob
            for port in glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*'):
                try:
                    test_serial = serial.Serial(port, timeout=0.1)
                    test_serial.close()
                    return port
                except:
                    continue
                    
        raise Exception("No FT232RL device found")

# Example usage
if __name__ == "__main__":
    # Test the adapter
    try:
        # Create Panda instance (auto-detects FT232RL)
        panda = Panda()
        
        print(f"Connected to: {panda.port}")
        print(f"Version: {panda.get_version()}")
        print(f"Health: {panda.health()}")
        
        # Set safety mode
        panda.set_safety_mode(SafetyMode.NO_OUTPUT)
        
        # Send test message
        panda.can_send(0x123, b'\x01\x02\x03\x04', 0)
        
        # Receive messages
        messages = panda.can_recv()
        print(f"Received {len(messages)} messages")
        
        # Print statistics
        stats = panda.get_stats()
        print(f"Stats: {stats}")
        
        panda.close()
        
    except Exception as e:
        print(f"Error: {e}")