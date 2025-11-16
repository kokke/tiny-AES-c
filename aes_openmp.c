/*

This is an OpenMP parallelized implementation of the AES algorithm, specifically CTR mode.
Block size can be chosen in aes.h - available choices are AES128, AES192, AES256.

The parallelization strategy leverages the fact that CTR mode encrypts each block
independently, making it ideal for multi-threaded processing. Each thread computes
its own counter value and encrypts it independently.

*/

#include <string.h>
#include <omp.h>
#include "aes.h"
#include "aes_openmp.h"
// #include <stdio.h>
/*
 * Increment IV by a specified number of blocks
 * Directly adds the blocks count to the IV without looping for each block
 * Treats the 16-byte IV as a big-endian 128-bit counter
 */
static void IncrementIvBy(uint8_t* Iv, size_t blocks)
{
  // Add 'blocks' directly to the IV counter (treating it as big-endian)
  // Process from right to left, adding byte by byte with carry
  size_t carry = blocks;
  for (int i = (AES_BLOCKLEN - 1); i >= 0 && carry > 0; --i)
  {
    size_t sum = Iv[i] + (carry & 0xFF);
    Iv[i] = (uint8_t)(sum & 0xFF);
    carry = (carry >> 8) + (sum >> 8);
  }
}

/*
 * OpenMP parallel version of AES CTR mode - same interface as AES_CTR_xcrypt_buffer
 *
 * Parallelization approach:
 * - Save initial IV before parallel region (all threads need same starting point)
 * - Each thread calculates IV for its assigned block using IncrementIvBy
 * - Threads encrypt their counters independently with AES_ECB_encrypt
 * - XOR encrypted counter with plaintext/ciphertext
 * - Update main context IV to next counter value after all threads complete
 * - Handle remaining bytes sequentially
 */
void AES_CTR_xcrypt_buffer_openmp(struct AES_ctx* ctx, uint8_t* buf, size_t length)
{
  uint8_t buffer[AES_BLOCKLEN];

  size_t i;
  size_t num_blocks = length / AES_BLOCKLEN;
  // printf("Number of blocks to process: %zu\n", num_blocks);
  // Save initial IV so all threads reference the same starting point
  uint8_t initial_iv[AES_BLOCKLEN];
  memcpy(initial_iv, ctx->Iv, AES_BLOCKLEN);

  // Parallel block encryption
  #pragma omp parallel private(buffer, i)
  {
    uint8_t thread_iv[AES_BLOCKLEN];
    struct AES_ctx thread_local_ctx;
    memcpy(thread_local_ctx.RoundKey, ctx->RoundKey, AES_keyExpSize);

    #pragma omp for schedule(static)
    for (size_t block_idx = 0; block_idx < num_blocks; ++block_idx)
    {
      // Each thread computes the IV for its block index
      memcpy(thread_iv, initial_iv, AES_BLOCKLEN);

      // Increment this thread's IV copy by the block index
      IncrementIvBy(thread_iv, block_idx);

      // Encrypt the counter value
      memcpy(buffer, thread_iv, AES_BLOCKLEN);
      AES_ECB_encrypt(&thread_local_ctx, buffer);

      // XOR encrypted counter with plaintext/ciphertext
      size_t buf_idx = block_idx * AES_BLOCKLEN;
      for (i = 0; i < AES_BLOCKLEN; ++i)
      {
        buf[buf_idx + i] ^= buffer[i];
      }
    }
  }  // End parallel region - all threads synchronize here

  // Update main context IV to next counter value for any future operations
  // This maintains the same behavior as the sequential AES_CTR_xcrypt_buffer
  memcpy(ctx->Iv, initial_iv, AES_BLOCKLEN);
  IncrementIvBy(ctx->Iv, num_blocks);

  // Handle remaining bytes (less than one full block) sequentially
  // This follows the same pattern as the original function
  if (length % AES_BLOCKLEN)
  {
    struct AES_ctx remainder_ctx;
    memcpy(remainder_ctx.RoundKey, ctx->RoundKey, AES_keyExpSize);

    memcpy(buffer, ctx->Iv, AES_BLOCKLEN);
    AES_ECB_encrypt(&remainder_ctx, buffer);

    uint8_t bi = 0;
    for (i = (num_blocks * AES_BLOCKLEN); i < length; ++i, ++bi)
    {
      buf[i] = (buf[i] ^ buffer[bi]);
    }

    // Increment IV one more time
    IncrementIvBy(ctx->Iv, 1);
  }
}

/*
 * OpenMP parallel version of AES CTR mode WITHOUT RoundKey copying
 *
 * This version directly accesses ctx->RoundKey without thread-local copying.
 * This demonstrates the performance impact of cache contention when multiple
 * threads access the same shared RoundKey data.
 *
 * EXPECTED RESULT: Significantly slower than the copy version due to:
 * - Cache line contention on RoundKey arrays
 * - False sharing when threads access different parts of RoundKey
 * - Loss of cache locality optimization
 */
void AES_CTR_xcrypt_buffer_openmp_no_copy(struct AES_ctx* ctx, uint8_t* buf, size_t length)
{
  uint8_t buffer[AES_BLOCKLEN];

  size_t i;
  size_t num_blocks = length / AES_BLOCKLEN;
  // printf("Number of blocks to process (no-copy): %zu\n", num_blocks);
  // Save initial IV so all threads reference the same starting point
  uint8_t initial_iv[AES_BLOCKLEN];
  memcpy(initial_iv, ctx->Iv, AES_BLOCKLEN);

  // Parallel block encryption - WITHOUT RoundKey copying
  #pragma omp parallel private(buffer, i)
  {
    uint8_t thread_iv[AES_BLOCKLEN];
    // Note: NOT copying RoundKey to thread-local storage
    // All threads will access ctx->RoundKey directly

    #pragma omp for schedule(static)
    for (size_t block_idx = 0; block_idx < num_blocks; ++block_idx)
    {
      // Each thread computes the IV for its block index
      memcpy(thread_iv, initial_iv, AES_BLOCKLEN);

      // Increment this thread's IV copy by the block index
      IncrementIvBy(thread_iv, block_idx);

      // Encrypt the counter value - directly access ctx->RoundKey
      memcpy(buffer, thread_iv, AES_BLOCKLEN);
      AES_ECB_encrypt((struct AES_ctx*)ctx, buffer);

      // XOR encrypted counter with plaintext/ciphertext
      size_t buf_idx = block_idx * AES_BLOCKLEN;
      for (i = 0; i < AES_BLOCKLEN; ++i)
      {
        buf[buf_idx + i] ^= buffer[i];
      }
    }
  }  // End parallel region

  // Update main context IV to next counter value
  memcpy(ctx->Iv, initial_iv, AES_BLOCKLEN);
  IncrementIvBy(ctx->Iv, num_blocks);

  // Handle remaining bytes (less than one full block) sequentially
  if (length % AES_BLOCKLEN)
  {
    memcpy(buffer, ctx->Iv, AES_BLOCKLEN);
    // Still use ctx directly for remainder since it's sequential
    AES_ECB_encrypt(ctx, buffer);

    uint8_t bi = 0;
    for (i = (num_blocks * AES_BLOCKLEN); i < length; ++i, ++bi)
    {
      buf[i] = (buf[i] ^ buffer[bi]);
    }

    // Increment IV one more time
    IncrementIvBy(ctx->Iv, 1);
  }
}
