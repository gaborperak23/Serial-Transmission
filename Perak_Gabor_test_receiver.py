import serial
import struct

# Configuration
SERIAL_PORT = "/dev/serial0"    # Default port
BAUD_RATE = 9600                # Baud rate

def calc_checksum(data):        #calculate checksum
    checksum = 0
    for byte in data:
        checksum ^= byte
    return checksum

def receive_packet(ser):
# Read header
    header_size = struct.calcsize("<BbH")
    header = ser.read(header_size)
    if len(header) != header_size:
        print("Error: Invalid header size received.")
        return None

    type_, rssi, length = struct.unpack("<BbH", header)
# Read recieved data
    remaining_size = length + 4 + 1 # 4 for timestamp and 1 for checksum
    remaining_data = ser.read(remaining_size)
    if len(remaining_data) != remaining_size:   # Error handling
        print("Error")
        return None

# Reverse engineer checksum
    received_data = header + remaining_data
    received_checksum = received_data[-1]
    payload = received_data[:-1]

# Recalculate checksum
    calculated_checksum = calc_checksum(payload)

# Comparision of original and recalculated checksum
    if received_checksum != calculated_checksum:
        print("Checksum ERROR")             # Error handling
        return None

    print("Checksum OK")
    return payload

def extract_values(payload):
# Mine data from payload

    packet_format = "<BbH"  # type (uint8_t), rssi (int8_t), length (uint16_t)
    type_, rssi, length = struct.unpack_from(packet_format, payload)
    offset = struct.calcsize(packet_format)

    wifi_data = payload[offset:offset + length]
    offset += length

    timestamp_format = "<I"
    timestamp = struct.unpack_from(timestamp_format, payload, offset)[0]

    return type_, rssi, length, wifi_data, timestamp

def main():
# Initialization of serial port
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"Checking on {SERIAL_PORT} at {BAUD_RATE} baud rate...")  # not necessary feedack, but good for debugging

    while True:
# Recieve packets from server
        payload = receive_packet(ser)
        if payload:
# Mine values
            type_, rssi, length, timestamp = extract_values(payload)

# Print values to terminal
            print(f"Received Packet:")
            print(f"  Type: {type_}")
            print(f"  RSSI: {rssi}")
            print(f"  Length: {length}")
            print(f"  Timestamp: {timestamp}")
            print("-" * 30)

if __name__ == "__main__":
    main()