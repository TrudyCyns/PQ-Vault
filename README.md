# PQ-Secure IoT Vault

A project focused on implementing and benchmarking Post-Quantum Cryptography (PQC) on resource-constrained IoT devices.

## Project Objective

To implement a Hybrid Cryptosystem on an ESP32, using Kyber-768 for quantum-resistant key encapsulation (KEM) and AES-256 for secure data encryption.

## Hardware Specifications

- Device: LOLIN32 (ESP32-WROOM-32)
- Processor: Dual-core 32-bit Xtensa LX6 (240MHz)
- SRAM: 520 KB
- TRNG: Integrated hardware True Random Number Generator.

## Current Milestone: Component Integration

- [x] Environment setup (ESP-IDF / VS Code).
- [x] Hardware AES vs. Software AES baseline established.
- [x] PQClean Kyber-768 integrated as an ESP-IDF component (libpq).
- [x] Hardware TRNG bridged to randombytes.
- [x] Successful project-wide compilation.

## How to Build

1. Setup the ESP-IDF environment.
2. Clone this repo.
3. Build he project:

```bash
idf.py build
```
