#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>
#include <stdbool.h>

// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES encryption in CBC-mode of operation.
// CTR enables encryption in counter-mode.
// ECB enables the basic ECB 16-byte block algorithm. All can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef CBC
  #define CBC 1
#endif

#ifndef ECB
  #define ECB 1
#endif

#ifndef CTR
  #define CTR 1
#endif

#ifndef CCM
  #define CCM 1
#endif


#define AES128 1
//#define AES192 1
//#define AES256 1

#define AES_BLOCKLEN 16 // Block length in bytes - AES is 128b block only

#if defined(AES256) && (AES256 == 1)
    #define AES_KEYLEN 32
    #define AES_keyExpSize 240
#elif defined(AES192) && (AES192 == 1)
    #define AES_KEYLEN 24
    #define AES_keyExpSize 208
#else
    #define AES_KEYLEN 16   // Key length in bytes
    #define AES_keyExpSize 176
#endif

struct AES_ctx
{
  uint8_t RoundKey[AES_keyExpSize];
#if (defined(CBC) && (CBC == 1)) || (defined(CTR) && (CTR == 1))
  uint8_t Iv[AES_BLOCKLEN];
#endif
};

void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key);
#if (defined(CBC) && (CBC == 1)) || (defined(CTR) && (CTR == 1))
void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv);
void AES_ctx_set_iv(struct AES_ctx* ctx, const uint8_t* iv);
#endif

#if defined(ECB) && (ECB == 1)
// buffer size is exactly AES_BLOCKLEN bytes; 
// you need only AES_init_ctx as IV is not used in ECB 
// NB: ECB is considered insecure for most uses
void AES_ECB_encrypt(const struct AES_ctx* ctx, uint8_t* buf);
void AES_ECB_decrypt(const struct AES_ctx* ctx, uint8_t* buf);

#endif // #if defined(ECB) && (ECB == !)


#if defined(CBC) && (CBC == 1)
// buffer size MUST be mutile of AES_BLOCKLEN;
// Suggest https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme
// NOTES: you need to set IV in ctx via AES_init_ctx_iv() or AES_ctx_set_iv()
//        no IV should ever be reused with the same key 
void AES_CBC_encrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length);
void AES_CBC_decrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length);

#endif // #if defined(CBC) && (CBC == 1)


#if defined(CTR) && (CTR == 1)

// Same function for encrypting as for decrypting. 
// IV is incremented for every block, and used after encryption as XOR-compliment for output
// Suggesting https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme
// NOTES: you need to set IV in ctx with AES_init_ctx_iv() or AES_ctx_set_iv()
//        no IV should ever be reused with the same key 
void AES_CTR_xcrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length);

#endif // #if defined(CTR) && (CTR == 1)

#if defined(CCM) && (CCM == 1)

typedef struct {
    uint8_t round_key[AES_keyExpSize];
    uint8_t cbc_buf[AES_BLOCKLEN];
    uint8_t counter[AES_BLOCKLEN];
    uint8_t ctr_buf[AES_BLOCKLEN];
    uint8_t byte_pos;
    uint8_t ctr_len;
} AES_CCM_ctx;
// nonce_len should be between 7..13 bytes.
// Adata is not supported yet.
// AES_CCM_Encrypt and AES_CCM_Decrypt could be called multiple times.
// The sum of len MUST be data_len given in AES_CCM_Init.
// The tag_len in AES_CCM_Init and AES_CCM_GenerateTag MUST be the same.
// After calling AES_CCM_GenerateTag or AES_CCM_verify_tag, ctx MUST be initialized before reused.
void AES_CCM_init(AES_CCM_ctx* ctx, const uint8_t key[AES_KEYLEN],
                  const uint8_t nonce[], uint8_t nonce_len,
                  uint32_t data_len, uint8_t tag_len);
void AES_CCM_encrypt(AES_CCM_ctx* ctx, uint8_t buf[], uint32_t len);
void AES_CCM_decrypt(AES_CCM_ctx* ctx, uint8_t buf[], uint32_t len);
void AES_CCM_generate_tag(AES_CCM_ctx* ctx, uint8_t tag[], uint8_t tag_len);
bool AES_CCM_verify_tag(AES_CCM_ctx* ctx, uint8_t tag[], uint8_t tag_len);

#endif // #if defined(CTR) && (CTR == 1)


#endif // _AES_H_
