#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "aes.h"

static void phex(uint8_t* str);
static void test_ECB(void);


int main(int argc, char** argv)
{
    test_ECB();
    return 0;
}


/* Helpers */

static void test_ECB(void)
{
    uint8_t i, buf[64], buf2[64];

    // 128bit key
    uint8_t key[16] =        { (uint8_t) 0x2b, (uint8_t) 0x7e, (uint8_t) 0x15, (uint8_t) 0x16, (uint8_t) 0x28, (uint8_t) 0xae, (uint8_t) 0xd2, (uint8_t) 0xa6, (uint8_t) 0xab, (uint8_t) 0xf7, (uint8_t) 0x15, (uint8_t) 0x88, (uint8_t) 0x09, (uint8_t) 0xcf, (uint8_t) 0x4f, (uint8_t) 0x3c };
    // 512bit text
    uint8_t plain_text[64] = { (uint8_t) 0x6b, (uint8_t) 0xc1, (uint8_t) 0xbe, (uint8_t) 0xe2, (uint8_t) 0x2e, (uint8_t) 0x40, (uint8_t) 0x9f, (uint8_t) 0x96, (uint8_t) 0xe9, (uint8_t) 0x3d, (uint8_t) 0x7e, (uint8_t) 0x11, (uint8_t) 0x73, (uint8_t) 0x93, (uint8_t) 0x17, (uint8_t) 0x2a,
                               (uint8_t) 0xae, (uint8_t) 0x2d, (uint8_t) 0x8a, (uint8_t) 0x57, (uint8_t) 0x1e, (uint8_t) 0x03, (uint8_t) 0xac, (uint8_t) 0x9c, (uint8_t) 0x9e, (uint8_t) 0xb7, (uint8_t) 0x6f, (uint8_t) 0xac, (uint8_t) 0x45, (uint8_t) 0xaf, (uint8_t) 0x8e, (uint8_t) 0x51,
                               (uint8_t) 0x30, (uint8_t) 0xc8, (uint8_t) 0x1c, (uint8_t) 0x46, (uint8_t) 0xa3, (uint8_t) 0x5c, (uint8_t) 0xe4, (uint8_t) 0x11, (uint8_t) 0xe5, (uint8_t) 0xfb, (uint8_t) 0xc1, (uint8_t) 0x19, (uint8_t) 0x1a, (uint8_t) 0x0a, (uint8_t) 0x52, (uint8_t) 0xef,
                               (uint8_t) 0xf6, (uint8_t) 0x9f, (uint8_t) 0x24, (uint8_t) 0x45, (uint8_t) 0xdf, (uint8_t) 0x4f, (uint8_t) 0x9b, (uint8_t) 0x17, (uint8_t) 0xad, (uint8_t) 0x2b, (uint8_t) 0x41, (uint8_t) 0x7b, (uint8_t) 0xe6, (uint8_t) 0x6c, (uint8_t) 0x37, (uint8_t) 0x10 };

    memset(buf, 0, 64);
    memset(buf2, 0, 64);

    // print text to encrypt, key and IV
    printf("plain text:\n");
    for(i = (uint8_t) 0; i < (uint8_t) 4; ++i)
    {
        phex(plain_text + i * (uint8_t) 16);
    }
    printf("\n");

    printf("key:\n");
    phex(key);
    printf("\n");

    // print the resulting cipher as 4 x 16 byte strings
    printf("ciphertext:\n");
    for(i = 0; i < 4; ++i)
    {
        AES128_ECB_encrypt(plain_text + (i*16), key, buf+(i*16));
        phex(buf + (i*16));
    }
    printf("\n");
}

// prints string as hex
void phex(uint8_t* str)
{
    unsigned char i;
    for(i = 0; i < 16; ++i)
        printf("%.2x", str[i]);
    printf("\n");
}


