# PQ-Secure IoT Vault

A project focused on implementing and benchmarking Post-Quantum Cryptography (PQC) on resource-constrained IoT devices.

## The IoT Use Case: Secure Remote Sensing

To implement a Hybrid Cryptosystem on an ESP32, using Kyber-768 for quantum-resistant key encapsulation (KEM) and AES-256 for secure data encryption.

### The Scenario

1. **Identity & Key Exchange**: The ESP32 (Sensor Node) generates a Kyber-768 keypair.
2. **PQ-Handshake**: A remote server encapsulates a session key using the node's public key.
3. **Hybrid Encryption**: The node decapsulates the key and uses it to establish an AES-256 encrypted tunnel.
4. **Secure Transmission**: Sensor data (Temperature, Humidity, etc.) is sent through the tunnel, protected by both PQC (against quantum attacks) and AES (for efficient symmetric encryption).

## Hardware Specifications

- Device: LOLIN32 (ESP32-WROOM-32)
- Processor: Dual-core 32-bit Xtensa LX6 (240MHz)
- SRAM: 520 KB (Optimized via 32KB dedicated PQC task stack)
- TRNG: Integrated hardware True Random Number Generator.
- Security Logic: Hybrid System (Kyber-768 KEM + AES-256 CBC)

## Current Milestone: Functional PQC Core

- [x] Environment setup (ESP-IDF / VS Code).
- [x] Hardware AES vs. Software AES baseline established.
- [x] PQClean Kyber-768 integrated as an ESP-IDF component (`libpq`).
- [x] Hardware TRNG bridged to `randombytes`.
- [x] Successful runtime execution: Kyber-768 KeyPair generation (32KB stack task).
