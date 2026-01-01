#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "kem.h"

static const char *TAG = "PQC_HANDSHAKE";

void pqc_task(void *pvParameters)
{
  // Buffers for keys (sizes defined in params.h via kem.h)
  uint8_t pk[PQCLEAN_MLKEM768_CLEAN_CRYPTO_PUBLICKEYBYTES];
  uint8_t sk[PQCLEAN_MLKEM768_CLEAN_CRYPTO_SECRETKEYBYTES];
  uint8_t ct[PQCLEAN_MLKEM768_CLEAN_CRYPTO_CIPHERTEXTBYTES];
  uint8_t ss_encap[PQCLEAN_MLKEM768_CLEAN_CRYPTO_BYTES];
  uint8_t ss_decap[PQCLEAN_MLKEM768_CLEAN_CRYPTO_BYTES];

  ESP_LOGI(TAG, "Starting Kyber-768 Key Generation...");

  // 1. Key Generation
  int64_t t0 = esp_timer_get_time();
  PQCLEAN_MLKEM768_CLEAN_crypto_kem_keypair(pk, sk);
  int64_t t1 = esp_timer_get_time();
  ESP_LOGI(TAG, "KeyGen: %lld us", t1 - t0);
  // Record start time in microseconds
  int64_t start = esp_timer_get_time();

  // 2. Encapsulation (Simulating Server side)
  int64_t t2 = esp_timer_get_time();
  PQCLEAN_MLKEM768_CLEAN_crypto_kem_enc(ct, ss_encap, pk);
  int64_t t3 = esp_timer_get_time();
  ESP_LOGI(TAG, "Encap:  %lld us", t3 - t2);

  // 3. Decapsulation (Simulating Node side)
  int64_t t4 = esp_timer_get_time();
  PQCLEAN_MLKEM768_CLEAN_crypto_kem_dec(ss_decap, ct, sk);
  int64_t t5 = esp_timer_get_time();
  ESP_LOGI(TAG, "Decap:  %lld us", t5 - t4);

  // 4. Verification
  if (memcmp(ss_encap, ss_decap, PQCLEAN_MLKEM768_CLEAN_CRYPTO_BYTES) == 0)
  {
    ESP_LOGI(TAG, "SUCCESS: Shared secrets match!");
    ESP_LOG_BUFFER_HEX("Shared Secret", ss_decap, 16);
  }
  else
  {
    ESP_LOGE(TAG, "FAILURE: Shared secrets do not match!");
  }

  vTaskDelete(NULL);
}

void app_main(void)
{
  // 32KB stack allocation is mandatory to prevent Stack Overflow
  xTaskCreate(pqc_task, "pqc_task", 32768, NULL, 5, NULL);
}