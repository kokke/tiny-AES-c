#ifndef _AES_OPENMP_H_
#define _AES_OPENMP_H_

#include "aes.h"

// OpenMP parallel version of AES CTR mode (with RoundKey copying for cache efficiency)
// This function parallelizes the CTR mode encryption/decryption across multiple threads
void AES_CTR_xcrypt_buffer_openmp(struct AES_ctx* ctx, uint8_t* buf, size_t length);

// OpenMP parallel version WITHOUT RoundKey copying (for performance comparison)
// This version directly accesses ctx->RoundKey without thread-local copying
void AES_CTR_xcrypt_buffer_openmp_no_copy(struct AES_ctx* ctx, uint8_t* buf, size_t length);

#endif // _AES_OPENMP_H_
