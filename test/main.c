
#include <stdio.h>
#include "aes.h"

void hex_dump(uint8_t * h, int len)
{
    while(len--)
        printf("%02hhx",*h++);
    printf("\n");
}

int main(int argc, char * argv[])
{
    //uint8_t state[16] = "\x00\x03\x01\x01\x00\x01\x00\x00\x03\x03\x01\x01\x03\x03\x01\x01";
    uint8_t state[16] = "\x01\x01\x03\x03\x01\x01\x03\x03\x00\x00\x01\x00\x01\x01\x03\x00";

    set_state(state);
    
    printf("initial state: \n");
    hex_dump(state, 16);

    // MixColumns();
    ShiftRows();

    printf("shift rows: \n");
    hex_dump(state, 16);

    return 0;
    
}
