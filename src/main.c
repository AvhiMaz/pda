#include "pda.h"

#include <libbase58.h>
#include <stdio.h>

int main(void) {
    const uint8_t system_program[32] = {0};
    uint8_t       vault[] = "vault";

    SignerSeed    seed[1] = {{.addr = vault, .len = 5}};
    SignerSeeds   seeds = {.seeds = seed, .len = 1};

    uint8_t       address[32], bump;
    int r = find_program_address(&seeds, system_program, address, &bump);

    if (r != 0) {
        printf("find_program_address failed: %d\n", r);
        return 1;
    }

    char   b58[50];
    size_t b58_len = sizeof(b58);
    b58enc(b58, &b58_len, address, 32);

    printf("bump = %d\n", bump);
    printf("address = %s\n", b58);

    return 0;
}
