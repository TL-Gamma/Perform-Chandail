import bluetooth
import threading

class SerialSocket:
    def __init__(self, address):
        self.address = address
        self.socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
        self.connected = False

    def connect(self):
        try:
            self.socket.connect((self.address, 1))
            self.connected = True
            return True
        except Exception as e:
            print("Error:", e)
            return False

    def disconnect(self):
        if self.connected:
            self.socket.close()
            self.connected = False

    def write(self, data):
        if self.connected:
            self.socket.send(data)
        else:
            print("Not connected")

    def read(self):
        if self.connected:
            return self.socket.recv(1024)
        else:
            print("Not connected")

class SerialService:
    def __init__(self):
        self.socket = None
        self.connected_address = None
        self.listener = None
        self.lock = threading.Lock()

    def discover_devices(self):
        nearby_devices = bluetooth.discover_devices(lookup_names=True)
        return nearby_devices

    def connect_to_device(self, address):
        if self.socket:
            self.socket.disconnect()
            self.socket = None
        self.socket = SerialSocket(address)
        if self.socket.connect():
            self.connected_address = address
            return True
        else:
            return False

    def disconnect(self):
        if self.socket:
            self.socket.disconnect()
            self.socket = None
            self.connected_address = None

    def write(self, data):
        if self.socket:
            self.socket.write(data)

    def read(self):
        if self.socket:
            return self.socket.read()

    def set_listener(self, listener):
        self.listener = listener

    def notify_listener(self, event_type, data=None):
        if self.listener:
            self.listener.on_serial_event(event_type, data)

class SerialListener:
    def on_serial_event(self, event_type, data=None):
        pass  # Implement event handling

# Exemple d'utilisation :
if __name__ == "__main__":
    service = SerialService()

    def event_handler(event_type, data=None):
        if event_type == "connect":
            print("Connected to device:", service.connected_address)
        elif event_type == "disconnect":
            print("Disconnected")
        elif event_type == "data":
            print("Received:", data)
        elif event_type == "error":
            print("Error:", data)

    service.set_listener(SerialListener())

    # Après avoir découvert les périphériques Bluetooth
    devices = service.discover_devices()
    for i, (addr, name) in enumerate(devices):
        print(f"{i+1}. {name} ({addr})")


    choice = int(input("Enter device number to connect: ")) - 1
    selected_address = devices[choice][0]
    if service.connect_to_device(selected_address):
        print("Connected successfully!")
    else:
        print("Failed to connect.")
