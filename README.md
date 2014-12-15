### Tiny AES128 in C

This is a small and portable implementation of the AES128 ECB and CBC encryption algorithms written in C.

The API is very simple and looks like this (I am using C99 `<stdint.h>`-style annotated types):

```C
void AES128_ECB_encrypt(uint8_t* input, const uint8_t* key, uint8_t* output);
void AES128_ECB_decrypt(uint8_t* input, const uint8_t* key, uint8_t* output);
void AES128_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
void AES128_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
```

You can choose to use one or both of the modes-of-operation, by defining the symbols CBC and ECB. See the header file for clarification.

The module uses around 200 bytes of RAM and 2.5K ROM when compiled for ARM (~2K for Thumb but YMMV).

It is the smallest implementation in C I've seen yet, but do contact me if you know of something smaller (or have improvements to the code here).


I've successfully used the code on 64bit x86, 32bit ARM and 8 bit AVR platforms.


GCC size output when ECB mode is compiled for ARM:



    $ arm-none-eabi-gcc -Os -c aes.c -DCBC=0
    $ size aes.o
       text    data     bss     dec     hex filename
       2323       0     184    2507     9cb aes.o




.. and when compiling for the THUMB instruction set, we end up around 2K in code size.

    $ arm-none-eabi-gcc -mthumb -Os -c aes.c -DCBC=0
    $ size aes.o
       text    data     bss     dec     hex filename
       1775       0     184    1959     7a7 aes.o



I am using Mentor Graphics free ARM toolchain:


    $ arm-none-eabi-gcc --version
    arm-none-eabi-gcc (GNU Tools for ARM Embedded Processors) 4.8.4 20140526 (release) [ARM/embedded-4_8-branch revision 211358]
    Copyright (C) 2013 Free Software Foundation, Inc.
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.




This implementation is verified against the data in:

[National Institute of Standards and Technology Special Publication 800-38A 2001 ED](http://csrc.nist.gov/publications/nistpubs/800-38a/sp800-38a.pdf) Appendix F: Example Vectors for Modes of Operation of the AES.


All material in this repository is in the public domain.

