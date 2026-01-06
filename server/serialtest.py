import sys
import binascii
import re
import time

# --- DEPENDENCY CHECK ---
try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("\n[!] Error: 'pyserial' module not found.")
    print("Please run: python -m pip install pyserial")
    sys.exit(1)

try:
    from Crypto.Cipher import AES
    from Crypto.Util.Padding import unpad
except ImportError:
    print("\n[!] Error: 'pycryptodome' module not found.")
    print("Please run: python -m pip install pycryptodome")
    sys.exit(1)

# --- CONFIGURATION ---
# Set to None to attempt auto-discovery
SERIAL_PORT = None 
BAUD_RATE = 115200

def decrypt_payload(ss_hex, iv_hex, data_hex):
    """
    Cleans hex strings and decrypts the AES-256-CBC payload.
    """
    try:
        # Remove any non-hex characters (like spaces or prefixes)
        def clean_hex(h): return re.sub(r'[^0-9a-fA-F]', '', h)
        
        key_bytes = binascii.unhexlify(clean_hex(ss_hex))
        iv_bytes = binascii.unhexlify(clean_hex(iv_hex))
        ciphertext_bytes = binascii.unhexlify(clean_hex(data_hex))

        # We only need the first 32 bytes of the shared secret for AES-256
        cipher = AES.new(key_bytes[:32], AES.MODE_CBC, iv=iv_bytes)
        decrypted = cipher.decrypt(ciphertext_bytes)
        
        # Unpad PKCS7 and decode
        plaintext = unpad(decrypted, AES.block_size)
        print(f"\n[⚡] DECRYPTED JSON: {plaintext.decode('utf-8')}")
        print("-" * 50)
        
    except Exception as e:
        print(f"\n[!] Decryption Error: {e}")
        # Debugging: print lengths if it fails
        try:
            print(f"    (Debug: Key Len={len(binascii.unhexlify(clean_hex(ss_hex)))}, "
                  f"IV Len={len(binascii.unhexlify(clean_hex(iv_hex)))}, "
                  f"Data Len={len(binascii.unhexlify(clean_hex(data_hex)))})")
        except: pass

def find_esp32_port():
    """Attempts to find the likely ESP32 serial port."""
    ports = list_ports.comports()
    for port in ports:
        if any(x in port.description for x in ["CP210", "CH340", "USB", "UART"]):
            return port.device
    return None

def run_bridge():
    port = SERIAL_PORT or find_esp32_port()
    
    if not port:
        print("[!] No serial port specified or detected.")
        print("Please set SERIAL_PORT manually in the script (e.g., 'COM3' or '/dev/ttyUSB0').")
        return

    print(f"--- PQC-AES Automated Serial Decryptor ---")
    print(f"Monitoring {port} at {BAUD_RATE} baud...")
    print(f"Waiting for ESP32 logs... (Press Reset on board if nothing happens)")
    
    ss, iv, data = None, None, None
    last_received_time = time.time()
    
    try:
        with serial.Serial(port, BAUD_RATE, timeout=0.1) as ser:
            ser.reset_input_buffer()
            
            while True:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if not line:
                    if time.time() - last_received_time > 5:
                        print("... (Still listening, no data received) ...")
                        last_received_time = time.time()
                    continue
                
                last_received_time = time.time()
                print(f" > {line}")

                # Accumulate hex buffers because ESP_LOG_BUFFER_HEX splits them across lines
                if "SECRET_HEX" in line:
                    val = line.split(":")[-1].strip()
                    ss = (ss or "") + " " + val
                elif "IV:" in line and "SECRET_HEX" not in line:
                    val = line.split(":")[-1].strip()
                    iv = (iv or "") + " " + val
                elif "Encrypted Data" in line:
                    val = line.split(":")[-1].strip()
                    data = (data or "") + " " + val
                
                # Trigger decryption when the final verification message is seen
                if "Verification Complete" in line:
                    if ss and iv and data:
                        decrypt_payload(ss, iv, data)
                    else:
                        print("[!] Missing components for decryption.")
                    
                    # Reset buffers for the next cycle
                    ss = iv = data = None

    except KeyboardInterrupt:
        print("\nStopping bridge...")
    except serial.SerialException as e:
        print(f"\n[!] Serial Error: {e}")

if __name__ == "__main__":
    run_bridge()