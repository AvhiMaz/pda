#include <stdint.h>
#include <stdio.h>

int pda_is_on_curve(const uint8_t p[32]) {
    if (!p) {
        return -1;
    }

    uint8_t be[32];
    for (int i = 0; i < 32; i++) {
        be[i] = p[31 - i];
    }
    int sign = be[0] >> 7;
    be[0] &= 0x7F;

    return 0;
}

int main(void) {
    printf("hello world\n");
    return 0;
}
