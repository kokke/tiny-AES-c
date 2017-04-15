#include <string.h>
#include <stdint.h>

#include "aes128cbc.h"

uint32_t aes128cbc_calbufsize(uint32_t dat_len){
	return (dat_len+16-(dat_len%16));
}

uint32_t aes128cbc_datpad(uint8_t* out, uint8_t* dat, uint32_t dat_len){
	uint32_t new_len = aes128cbc_calbufsize(dat_len);
	uint32_t idx = 0;
	//memset(out, 0, new_len);
	//memcpy(out, dat, dat_len);
	while(idx<dat_len){
		out[idx] = dat[idx];
		idx++;
	}
	while(idx<new_len){
		out[idx] = 0;
		idx++;
	}
	return new_len;
}

void aes128cbc_encrypt(uint8_t* out, uint8_t* in, uint32_t len, const uint8_t* key, const uint8_t* iv ){
	AES128_CBC_encrypt_buffer(out, in, len, key, iv);
}

void aes128cbc_decrypt(uint8_t* out, uint8_t* in, uint32_t len, const uint8_t* key, const uint8_t* iv ){
	AES128_CBC_decrypt_buffer(out+0, in+0,  len, key, iv);
}

// same as aes128cbc_decrypt()
void aes128cbc_decrypt_block(uint8_t* out, uint8_t* in, uint32_t len, const uint8_t* key, const uint8_t* iv ){
	uint32_t bias = 0;
	uint32_t round = (len/16)+((len%16==0)?0:1);
	AES128_CBC_decrypt_buffer(out+0, in+0,  16, key, iv);
	while(--round){
		bias = bias + 16;
		AES128_CBC_decrypt_buffer(out+bias, in+bias, 16, 0, 0);
	}
}
