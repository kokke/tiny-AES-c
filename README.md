### Tiny AES128 in C

This is a small and portable implementation of the AES128 ECB encryption algorithm implemented in C.

The API is very simple and looks like this (I am using `<stdint.h>`-style annotated types):

```C
void AES128_ECB_encrypt(uint8_t* input, uint8_t* key, uint8_t *output);
void AES128_ECB_decrypt(uint8_t* input, uint8_t* key, uint8_t *output);
```

The module uses just a bit more than 200 bytes of RAM and 2.5K ROM when compiled for ARM (YMMV).

It is the smallest implementation in C I've seen yet, but do contact me if you know of something smaller (or have improvements to the code here).


I've successfully used the code on 64bit x86, 32bit ARM and 8 bit AVR platforms.


GCC size output when compiled for ARM:



    $ arm-none-eabi-gcc -Os -c aes.c
    $ size aes.o
       text    data     bss     dec     hex filename
       2515       0     204    2719     a9f aes.o


I am using Mentor Graphics free ARM toolchain:


    $ arm-none-eabi-gcc --version
    arm-none-eabi-gcc (GNU Tools for ARM Embedded Processors) 4.8.3 20131129 (release) [ARM/embedded-4_8-branch revision 205641]
    Copyright (C) 2013 Free Software Foundation, Inc.
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.




The implementation is verified against the data in:

[National Institute of Standards and Technology Special Publication 800-38A 2001 ED](http://csrc.nist.gov/publications/nistpubs/800-38a/sp800-38a.pdf) Appendix F: Example Vectors for Modes of Operation of the AES.



