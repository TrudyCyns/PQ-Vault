#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "mbedtls/aes.h" // Standard mbedTLS AES (Hardware Accelerated in ESP-IDF)
#include "kem.h"

static const char *TAG = "PQC_HYBRID";

// Helper to PKCS7 pad data for AES-CBC
void apply_padding(uint8_t *data, size_t len, size_t block_size) {
    uint8_t pad_val = block_size - (len % block_size);
    for (size_t i = 0; i < pad_val; i++) {
        data[len + i] = pad_val;
    }
}

void hybrid_task(void *pvParameters)
{
  /// 1. KEM Variables
    uint8_t pk[PQCLEAN_MLKEM768_CLEAN_CRYPTO_PUBLICKEYBYTES];
    uint8_t sk[PQCLEAN_MLKEM768_CLEAN_CRYPTO_SECRETKEYBYTES];
    uint8_t ct[PQCLEAN_MLKEM768_CLEAN_CRYPTO_CIPHERTEXTBYTES];
    uint8_t shared_secret[PQCLEAN_MLKEM768_CLEAN_CRYPTO_BYTES];

    // 2. AES Variables
    char *plaintext = "{\"temp\": 24.5, \"humidity\": 60, \"status\": \"secure\"}";
    size_t pt_len = strlen(plaintext);
    size_t padded_len = ((pt_len / 16) + 1) * 16;
    uint8_t input_buffer[64] = {0}; 
    uint8_t output_buffer[64] = {0};
    uint8_t iv[16];
    uint8_t iv_copy[16]; // AES-CBC modifies the IV buffer during execution

    ESP_LOGI(TAG, "Starting Hybrid Handshake...");

    // Phase A: PQC Handshake (Simulated Loopback)
    PQCLEAN_MLKEM768_CLEAN_crypto_kem_keypair(pk, sk);
    PQCLEAN_MLKEM768_CLEAN_crypto_kem_enc(ct, shared_secret, pk);

    ESP_LOGI(TAG, "Kyber Shared Secret Derived. Bridging to Hardware AES-256...");

    // Phase B: AES-256 Encryption
    memcpy(input_buffer, plaintext, pt_len);
    apply_padding(input_buffer, pt_len, 16);
    esp_fill_random(iv, 16); // Generate random IV using TRNG
    memcpy(iv_copy, iv, 16);

    mbedtls_aes_context aes_ctx;
    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_enc(&aes_ctx, shared_secret, 256); // mbedTLS requires setting the key specifically for ENCRYPT or DECRYPT

    int64_t start = esp_timer_get_time();
    mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, padded_len, iv_copy, input_buffer, output_buffer);
    int64_t end = esp_timer_get_time();

    ESP_LOGI(TAG, "AES-256 Encryption Complete in %lld us", end - start);
    ESP_LOG_BUFFER_HEX("IV", iv, 16);
    ESP_LOG_BUFFER_HEX("Encrypted Data", output_buffer, padded_len);

    // Phase C: Verification (Decryption)
    uint8_t decrypted_buffer[64] = {0};
    memcpy(iv_copy, iv, 16); // Restore IV

    // Set key for DECRYPT
    mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, padded_len, iv_copy, output_buffer, decrypted_buffer);
    
    ESP_LOGI(TAG, "Decrypted Result: %s", (char*)decrypted_buffer);

    mbedtls_aes_free(&aes_ctx);
    vTaskDelete(NULL);
}

void app_main(void)
{
  // 32KB stack allocation is mandatory to prevent Stack Overflow
  xTaskCreate(hybrid_task, "hybrid_task", 32768, NULL, 5, NULL);
}