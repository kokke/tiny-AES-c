#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wmmintrin.h>

#include "aes.h"

static void increment_counter(uint8_t* counter)
{
  int i;
  for (i = AES_BLOCKLEN - 1; i >= 0; --i)
  {
    if (++counter[i] != 0)
      break;
  }
}

void tiny_aesni_xcrypt_blocks(const uint8_t* round_key, uint8_t* counter,
                              uint8_t* buf, size_t blocks, uint8_t rounds)
{
  while (blocks-- != 0)
  {
    __m128i state, key, input;
    uint8_t round;
    memcpy(&state, counter, sizeof state);
    memcpy(&key, round_key, sizeof key);
    state = _mm_xor_si128(state, key);
    for (round = 1; round < rounds; ++round)
    {
      memcpy(&key, round_key + round * AES_BLOCKLEN, sizeof key);
      state = _mm_aesenc_si128(state, key);
    }
    memcpy(&key, round_key + rounds * AES_BLOCKLEN, sizeof key);
    state = _mm_aesenclast_si128(state, key);
    memcpy(&input, buf, sizeof input);
    input = _mm_xor_si128(input, state);
    memcpy(buf, &input, sizeof input);
    increment_counter(counter);
    buf += AES_BLOCKLEN;
  }
}
