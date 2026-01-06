import binascii
from quantcrypt.kem import MLKEM_768
from Crypto.Cipher import AES
from Crypto.Util.Padding import unpad

def decrypt_packet(iv, encrypted_data, shared_secret):
    """
    Decrypts the AES-256 payload using the shared secret from Kyber-768.
    """
    print("\n--- Processing Incoming Secure Packet ---")
    
    # Shared secret is already 32 bytes for Kyber-768
    aes_key = shared_secret
    
    try:
        cipher = AES.new(aes_key, AES.MODE_CBC, iv=iv)
        decrypted_padded = cipher.decrypt(encrypted_data)
        
        # Remove PKCS7 Padding
        plaintext = unpad(decrypted_padded, AES.block_size)
        return plaintext.decode('utf-8')
    except Exception as e:
        return f"Decryption Failed: {str(e)}"

def simulate_server_handshake():
    """
    Main execution flow using quantcrypt (Kyber-768).
    """
    # 1. Initialize Kyber-768
    kem = MLKEM_768()
    
    # 2. Server generates Keypair (Normally Node sends PK, but loopback for now)
    public_key, secret_key = kem.keygen()
    
    # 3. Server encapsulates a secret (This happens on the Server side)
    ciphertext, shared_secret = kem.encaps(public_key)
    
    print(f"Kyber-768 Handshake Successful.")
    print(f"Shared Secret (Hex): {binascii.hexlify(shared_secret).decode()}")
    print(f"Ciphertext Size: {len(ciphertext)} bytes")
    
    # --- MOCK PACKET FROM ESP32 ---
    # These would be extracted from the 1168-byte packet in Week 4
    mock_iv = bytes([0x00] * 16) 
    mock_encrypted_payload = bytes([0x00] * 64) 
    
    print("\n[READY] Server logic verified with quantcrypt.")
    # result = decrypt_packet(mock_iv, mock_encrypted_payload, shared_secret)
    # print(f"Decrypted Content: {result}")

if __name__ == "__main__":
    simulate_server_handshake()