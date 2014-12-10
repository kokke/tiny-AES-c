#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>

void AES128_ECB_encrypt(uint8_t* input, const uint8_t* key, uint8_t *output);
void AES128_ECB_decrypt(uint8_t* input, const uint8_t* key, uint8_t *output);
void AES128_ECB_encrypt_nc(uint8_t* data, const uint8_t* key);
void AES128_ECB_decrypt_nc(uint8_t* data, const uint8_t* key);

#endif //_AES_H_
