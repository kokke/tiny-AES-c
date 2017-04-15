#ifndef _AES128CBC_H_
#define _AES128CBC_H_

// Enable both ECB and CBC mode. Note this can be done before including aes.h or at compile-time.
// E.g. with GCC by using the -D flag: gcc -c aes.c -DCBC=0 -DECB=1
//#define CBC 1
//#define ECB 0
#include "aes.h"

/*
CipherText = PlainText + 16 - (PlainText MOD 16)
http://www.obviex.com/articles/CiphertextSize.pdf
*/
uint32_t aes128cbc_calbufsize(uint32_t dat_len);
uint32_t aes128cbc_datpad(uint8_t* out, uint8_t* dat, uint32_t dat_len);
void aes128cbc_encrypt(uint8_t* out, uint8_t* in, uint32_t len, const uint8_t* key, const uint8_t* iv);
void aes128cbc_decrypt(uint8_t* out, uint8_t* in, uint32_t len, const uint8_t* key, const uint8_t* iv);
void aes128cbc_decrypt_block(uint8_t* out, uint8_t* in, uint32_t len, const uint8_t* key, const uint8_t* iv);

#endif /* _AES128CBC_H_ */
