#!/usr/bin/env python3
"""
Simple CAN Test Example for FT232RL Red Panda Implementation

This example demonstrates basic CAN communication using the FT232RL
adapter with Red Panda compatibility.

Usage:
    python simple_can_test.py [COM_PORT]

Example:
    python simple_can_test.py COM3        # Windows
    python simple_can_test.py /dev/ttyUSB0 # Linux
"""

import sys
import time
import signal
from typing import Optional

# Import our FT232RL wrapper
sys.path.append('../PC')
from ft232rl_python_wrapper import Panda, SafetyMode, CANMessage

class CANTester:
    """Simple CAN testing class"""
    
    def __init__(self, port: Optional[str] = None):
        self.panda: Optional[Panda] = None
        self.running = False
        self.port = port
        
        # Statistics
        self.messages_sent = 0
        self.messages_received = 0
        self.start_time = time.time()
        
    def connect(self) -> bool:
        """Connect to TC275"""
        try:
            if self.port:
                self.panda = Panda(serial_port=self.port)
            else:
                self.panda = Panda()  # Auto-detect
                
            print(f"✅ Connected to FT232RL on {self.panda.port}")
            return True
            
        except Exception as e:
            print(f"❌ Connection failed: {e}")
            return False
    
    def setup(self) -> bool:
        """Setup CAN interface"""
        if not self.panda:
            return False
            
        try:
            # Get version info
            version = self.panda.get_version()
            print(f"📋 Firmware version: {version}")
            
            # Get health info
            health = self.panda.health()
            print(f"💓 Health: {health}")
            
            # Set safety mode to allow output
            self.panda.set_safety_mode(SafetyMode.NO_OUTPUT)
            print("🔒 Safety mode: NO_OUTPUT")
            
            # Set CAN speeds
            for bus in range(3):
                self.panda.set_can_speed_kbps(bus, 500)  # 500 kbps
                print(f"🚌 CAN Bus {bus}: 500 kbps")
            
            # Send heartbeat
            self.panda.send_heartbeat()
            print("💗 Heartbeat sent")
            
            return True
            
        except Exception as e:
            print(f"❌ Setup failed: {e}")
            return False
    
    def send_test_messages(self):
        """Send test CAN messages"""
        if not self.panda:
            return
            
        try:
            # Test messages
            test_messages = [
                CANMessage(0x123, b'\x01\x02\x03\x04', 0),
                CANMessage(0x456, b'\x05\x06\x07\x08', 1),
                CANMessage(0x789, b'\x09\x0A\x0B\x0C', 2),
                CANMessage(0x7FF, b'\xAA\xBB\xCC\xDD\xEE\xFF', 0, extended=False),
                CANMessage(0x1FFFFFFF, b'\x11\x22\x33\x44', 1, extended=True),
            ]
            
            print(f"\n📤 Sending {len(test_messages)} test messages...")
            
            sent_count = self.panda.can_send_many(test_messages, timeout=1000)
            self.messages_sent += sent_count
            
            print(f"✅ Sent {sent_count}/{len(test_messages)} messages")
            
        except Exception as e:
            print(f"❌ Send failed: {e}")
    
    def receive_messages(self):
        """Receive and display CAN messages"""
        if not self.panda:
            return
            
        try:
            messages = self.panda.can_recv()
            
            if messages:
                print(f"\n📥 Received {len(messages)} messages:")
                for i, msg in enumerate(messages):
                    addr_str = f"0x{msg.addr:08X}" if msg.extended else f"0x{msg.addr:03X}"
                    data_str = msg.data.hex().upper()
                    flags = []
                    if msg.extended:
                        flags.append("EXT")
                    if msg.fd:
                        flags.append("FD")
                    flag_str = f"[{','.join(flags)}]" if flags else ""
                    
                    print(f"  {i+1:2d}: Bus{msg.bus} {addr_str} {data_str} {flag_str}")
                    
                self.messages_received += len(messages)
            
        except Exception as e:
            print(f"❌ Receive failed: {e}")
    
    def print_statistics(self):
        """Print current statistics"""
        if not self.panda:
            return
            
        try:
            # Get adapter stats
            stats = self.panda.get_stats()
            
            # Calculate runtime
            runtime = time.time() - self.start_time
            
            print(f"\n📊 Statistics (Runtime: {runtime:.1f}s)")
            print(f"   CAN Messages - Sent: {self.messages_sent}, Received: {self.messages_received}")
            print(f"   Protocol Frames - Sent: {stats['frames_sent']}, Received: {stats['frames_received']}")
            print(f"   Bytes - Sent: {stats['bytes_sent']}, Received: {stats['bytes_received']}")
            print(f"   Errors: {stats['errors']}")
            
            # Calculate rates
            if runtime > 0:
                msg_rate = (self.messages_sent + self.messages_received) / runtime
                byte_rate = (stats['bytes_sent'] + stats['bytes_received']) / runtime
                print(f"   Rates - Messages: {msg_rate:.1f}/s, Bytes: {byte_rate:.1f}/s")
            
        except Exception as e:
            print(f"❌ Stats failed: {e}")
    
    def run_interactive_test(self):
        """Run interactive test mode"""
        self.running = True
        print("\n🎮 Interactive Test Mode")
        print("Commands:")
        print("  's' - Send test messages")
        print("  'r' - Receive messages")
        print("  'h' - Send heartbeat")
        print("  'i' - Show statistics")
        print("  'q' - Quit")
        
        while self.running:
            try:
                # Auto-receive messages
                self.receive_messages()
                
                # Check for user input (non-blocking)
                import select
                import sys
                
                if select.select([sys.stdin], [], [], 0.1)[0]:
                    cmd = input("\n> ").strip().lower()
                    
                    if cmd == 's':
                        self.send_test_messages()
                    elif cmd == 'r':
                        self.receive_messages()
                    elif cmd == 'h':
                        if self.panda:
                            self.panda.send_heartbeat()
                            print("💗 Heartbeat sent")
                    elif cmd == 'i':
                        self.print_statistics()
                    elif cmd == 'q':
                        self.running = False
                        break
                    else:
                        print("❓ Unknown command")
                
                time.sleep(0.1)
                
            except KeyboardInterrupt:
                self.running = False
                break
            except Exception as e:
                print(f"❌ Error: {e}")
                time.sleep(1)
    
    def run_automatic_test(self, duration: int = 30):
        """Run automatic test for specified duration"""
        print(f"\n🤖 Automatic Test Mode ({duration}s)")
        
        self.running = True
        end_time = time.time() + duration
        last_send_time = 0
        last_stats_time = 0
        
        while self.running and time.time() < end_time:
            try:
                current_time = time.time()
                
                # Send messages every 2 seconds
                if current_time - last_send_time >= 2.0:
                    self.send_test_messages()
                    last_send_time = current_time
                
                # Receive messages
                self.receive_messages()
                
                # Print stats every 10 seconds
                if current_time - last_stats_time >= 10.0:
                    self.print_statistics()
                    last_stats_time = current_time
                
                # Send heartbeat
                if self.panda and int(current_time) % 5 == 0:
                    self.panda.send_heartbeat()
                
                time.sleep(0.5)
                
            except KeyboardInterrupt:
                self.running = False
                break
            except Exception as e:
                print(f"❌ Error: {e}")
                time.sleep(1)
    
    def cleanup(self):
        """Cleanup resources"""
        if self.panda:
            try:
                self.print_statistics()
                self.panda.close()
                print("🔌 Disconnected")
            except:
                pass

def signal_handler(sig, frame):
    """Handle Ctrl+C gracefully"""
    print("\n\n⏹️  Stopping test...")
    global tester
    if tester:
        tester.running = False

def main():
    """Main function"""
    global tester
    
    # Parse command line arguments
    port = None
    if len(sys.argv) > 1:
        port = sys.argv[1]
    
    # Create tester instance
    tester = CANTester(port)
    
    # Setup signal handler
    signal.signal(signal.SIGINT, signal_handler)
    
    try:
        # Connect and setup
        if not tester.connect():
            return 1
            
        if not tester.setup():
            return 1
        
        # Choose test mode
        print("\n🎯 Test Modes:")
        print("  1 - Interactive mode")
        print("  2 - Automatic mode (30s)")
        print("  3 - Quick test")
        
        try:
            choice = input("\nSelect mode (1-3): ").strip()
        except:
            choice = "3"  # Default to quick test
            
        if choice == "1":
            tester.run_interactive_test()
        elif choice == "2":
            tester.run_automatic_test(30)
        else:
            # Quick test
            print("\n⚡ Quick Test")
            tester.send_test_messages()
            time.sleep(1)
            tester.receive_messages()
            tester.print_statistics()
        
        return 0
        
    except Exception as e:
        print(f"❌ Test failed: {e}")
        return 1
        
    finally:
        tester.cleanup()

if __name__ == "__main__":
    exit(main())