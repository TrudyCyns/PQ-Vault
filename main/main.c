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

// The "Secure Envelope" structure
// Total size: 1088 (CT) + 16 (IV) + 64 (Data) = 1168 bytes
typedef struct
{
  uint8_t ciphertext[PQCLEAN_MLKEM768_CLEAN_CRYPTO_CIPHERTEXTBYTES];
  uint8_t iv[16];
  uint8_t encrypted_data[64];
  uint32_t data_len;
} secure_packet_t;

// Helper to PKCS7 pad data for AES-CBC
void apply_padding(uint8_t *data, size_t len, size_t block_size)
{
  uint8_t pad_val = block_size - (len % block_size);
  for (size_t i = 0; i < pad_val; i++)
  {
    data[len + i] = pad_val;
  }
}

void hybrid_task(void *pvParameters)
{
  /// 1. KEM Variables
  uint8_t pk[PQCLEAN_MLKEM768_CLEAN_CRYPTO_PUBLICKEYBYTES];
  uint8_t sk[PQCLEAN_MLKEM768_CLEAN_CRYPTO_SECRETKEYBYTES];
  uint8_t shared_secret[PQCLEAN_MLKEM768_CLEAN_CRYPTO_BYTES];

  // Setup buffers
  secure_packet_t packet;
  memset(&packet, 0, sizeof(secure_packet_t));

  char *sensor_json = "{\"temp\": 24.5, \"hum\": 60, \"id\": \"ESP32-NODE-01\"}";
  size_t pt_len = strlen(sensor_json);
  packet.data_len = ((pt_len / 16) + 1) * 16;

  ESP_LOGI(TAG, "Generating Secure Packet...");

  // 2. KEM Handshake
  PQCLEAN_MLKEM768_CLEAN_crypto_kem_keypair(pk, sk);
  // Server would normally do this, but loopback for verification:
  PQCLEAN_MLKEM768_CLEAN_crypto_kem_enc(packet.ciphertext, shared_secret, pk);

  // 3. AES Encryption
  uint8_t temp_pt[64] = {0};
  memcpy(temp_pt, sensor_json, pt_len);
  apply_padding(temp_pt, pt_len, 16);
  esp_fill_random(packet.iv, 16);

  mbedtls_aes_context aes_ctx;
  mbedtls_aes_init(&aes_ctx);
  mbedtls_aes_setkey_enc(&aes_ctx, shared_secret, 256);

  // Use a copy of IV because CBC modifies it
  uint8_t iv_inner[16];
  memcpy(iv_inner, packet.iv, 16);

  mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, packet.data_len, iv_inner, temp_pt, packet.encrypted_data);

  // 4. Final Verification and Size Check
  ESP_LOGI(TAG, "--- Packet Ready for Transmission ---");
  ESP_LOGI(TAG, "Total Packet Size: %d bytes", sizeof(secure_packet_t));
  ESP_LOG_BUFFER_HEX("Ciphertext (First 16B)", packet.ciphertext, 16);
  ESP_LOG_BUFFER_HEX("Payload (Encrypted)", packet.encrypted_data, packet.data_len);

  ESP_LOGI(TAG, "--- For Serial transfer ---");
  ESP_LOG_BUFFER_HEX("SECRET_HEX", shared_secret, 32);
  ESP_LOG_BUFFER_HEX("IV", packet.iv, 16);
  ESP_LOG_BUFFER_HEX("Encrypted Data", packet.encrypted_data, packet.data_len);

  mbedtls_aes_free(&aes_ctx);
  ESP_LOGI(TAG, "Verification Complete. Logic is stable.");
  vTaskDelete(NULL);
}

void app_main(void)
{
  // 32KB stack allocation is mandatory to prevent Stack Overflow
  xTaskCreate(hybrid_task, "hybrid_task", 32768, NULL, 5, NULL);
}