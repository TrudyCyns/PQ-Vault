#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "kem.h"

static const char *TAG = "PQC_VAULT";

void pqc_task(void *pvParameters)
{
  // Buffers for keys (sizes defined in params.h via kem.h)
  uint8_t pk[PQCLEAN_MLKEM768_CLEAN_CRYPTO_PUBLICKEYBYTES];
  uint8_t sk[PQCLEAN_MLKEM768_CLEAN_CRYPTO_SECRETKEYBYTES];

  ESP_LOGI(TAG, "Starting Kyber-768 Key Generation...");

  // This function uses significant stack memory
  int ret = PQCLEAN_MLKEM768_CLEAN_crypto_kem_keypair(pk, sk);

  if (ret == 0)
  {
    ESP_LOGI(TAG, "SUCCESS: Kyber-768 Keypair generated.");
    ESP_LOG_BUFFER_HEX("PUBLIC KEY (First 16B)", pk, 16);
  }
  else
  {
    ESP_LOGE(TAG, "FAILURE: Keypair generation failed code %d", ret);
  }

  ESP_LOGI(TAG, "Memory check: Task complete.");
  vTaskDelete(NULL);
}

void app_main(void)
{
  // 32KB stack allocation is mandatory to prevent Stack Overflow
  xTaskCreate(pqc_task, "pqc_task", 32768, NULL, 5, NULL);
}