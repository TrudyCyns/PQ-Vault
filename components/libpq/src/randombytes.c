#include "esp_random.h"
#include <stddef.h>
#include <stdint.h>

// This matches the signature expected by PQClean
void randombytes(uint8_t *out, size_t n) {
    esp_fill_random(out, n);
}