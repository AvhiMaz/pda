#ifndef PDA_H
#define PDA_H

#include <stdint.h>

typedef struct {
    const uint8_t *addr;
    uint64_t       len;
} SignerSeed;

typedef struct {
    const SignerSeed *seeds;
    uint64_t          len;
} SignerSeeds;

int pda_is_on_curve(const uint8_t point[32]);
int sha256_seeds(const SignerSeeds *seeds, const uint8_t program_id[32],
                 uint8_t out[32]);

#endif
