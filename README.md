### Tiny AES in C

This is a small and portable implementation of the AES ECB and CBC encryption algorithms written in C.

You can override the default block-size of 128 bit with 192 or 256 bit by defining the symbols AES192 or AES256 in `aes.h`.

The API is very simple and looks like this (I am using C99 `<stdint.h>`-style annotated types):

```C
void AES_ECB_encrypt(uint8_t* input, const uint8_t* key, uint8_t* output);
void AES_ECB_decrypt(uint8_t* input, const uint8_t* key, uint8_t* output);
void AES_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
void AES_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
```


You can choose to use one or both of the modes-of-operation, by defining the symbols CBC and ECB. See the header file for clarification.

There is no built-in error checking or protection from out-of-bounds memory access errors as a result of malicious input. The two functions AES_ECB_xxcrypt() do most of the work, and they expect inputs of 128 bit length.

The module uses less than 200 bytes of RAM and 2.3K ROM when compiled for ARM (<2K for Thumb but YMMV).

It is one of the smallest implementation in C I've seen yet, but do contact me if you know of something smaller (or have improvements to the code here). 

I've successfully used the code on 64bit x86, 32bit ARM and 8 bit AVR platforms.

GCC size output when only ECB mode is compiled for ARM (using 128 bit block size):

    $ arm-none-eabi-gcc -Os -c aes.c -DCBC=0
    $ size aes.o
       text    data     bss     dec     hex filename
       2071       0     184    2255     8cf aes.o


.. and when compiling for the THUMB instruction set, we end up just above 1.7K in code size.

    $ arm-none-eabi-gcc -mthumb -Os -c aes.c -DCBC=0
    $ size aes.o
       text    data     bss     dec     hex filename
       1551       0     184    1735     6c7 aes.o




I am using the Free Software Foundation, ARM GCC compiler:

    $ arm-none-eabi-gcc --version
    arm-none-eabi-gcc (4.8.4-1+11-1) 4.8.4 20141219 (release)
    Copyright (C) 2013 Free Software Foundation, Inc.
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.



This implementation is verified against the data in:

[National Institute of Standards and Technology Special Publication 800-38A 2001 ED](http://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-38a.pdf) Appendix F: Example Vectors for Modes of Operation of the AES.


A heartfelt thank-you to all the nice people out there who have contributed to this project.


All material in this repository is in the public domain.
