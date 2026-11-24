#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#include "aes.h"

#ifdef _WIN32
#define PROBE_EXPORT __declspec(dllexport)
#else
#define PROBE_EXPORT __attribute__((visibility("default")))
#endif

#ifndef _WIN32
static uint64_t now_ns(void)
{
  struct timespec value;
  clock_gettime(CLOCK_MONOTONIC, &value);
  return (uint64_t)value.tv_sec * 1000000000ULL + (uint64_t)value.tv_nsec;
}
#endif

PROBE_EXPORT int probe_backend(void)
{
  static const uint8_t key[AES_KEYLEN] = {0};
  struct AES_ctx ctx;
  AES_init_ctx(&ctx, key);
  return ctx.CtrUseAesni != 0;
}

PROBE_EXPORT void probe_transform(const uint8_t* key, const uint8_t* iv,
                                  uint8_t* data, size_t length,
                                  const size_t* chunks, size_t chunk_count,
                                  int force_portable)
{
  struct AES_ctx ctx;
  size_t i, offset = 0;
  AES_init_ctx_iv(&ctx, key, iv);
  if (force_portable)
    ctx.CtrUseAesni = 0;
  for (i = 0; i < chunk_count; ++i)
  {
    uint8_t* fragment = data == NULL ? NULL : data + offset;
    AES_CTR_xcrypt_buffer(&ctx, fragment, chunks[i]);
    offset += chunks[i];
  }
  (void)length;
}

PROBE_EXPORT void probe_reset(const uint8_t* key, const uint8_t* first_iv,
                              const uint8_t* second_iv, uint8_t* first_byte,
                              uint8_t* second, size_t second_length)
{
  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key, first_iv);
  AES_CTR_xcrypt_buffer(&ctx, first_byte, 1);
  AES_ctx_set_iv(&ctx, second_iv);
  AES_CTR_xcrypt_buffer(&ctx, second, second_length);
}

PROBE_EXPORT uint64_t probe_bench_ns(const uint8_t* key, const uint8_t* iv,
                                     uint8_t* data, size_t length,
                                     size_t iterations)
{
  struct AES_ctx ctx;
#ifdef _WIN32
  LARGE_INTEGER frequency, start, stop;
#else
  uint64_t start, stop;
#endif
  size_t i;
  AES_init_ctx_iv(&ctx, key, iv);
#ifdef _WIN32
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&start);
#else
  start = now_ns();
#endif
  for (i = 0; i < iterations; ++i)
    AES_CTR_xcrypt_buffer(&ctx, data, length);
#ifdef _WIN32
  QueryPerformanceCounter(&stop);
  return (uint64_t)((stop.QuadPart - start.QuadPart) * 1000000000ULL /
                    frequency.QuadPart);
#else
  stop = now_ns();
  return stop - start;
#endif
}
