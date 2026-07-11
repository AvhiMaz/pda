#ifndef PDA_H
#define PDA_H

#include <stdint.h>

#define PDA_MAX_SEED_LEN 32
#define PDA_MAX_SEEDS    16

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
int create_program_address(const SignerSeeds *seeds,
                           const uint8_t program_id[32], uint8_t out[32]);
int find_program_address(const SignerSeeds *seeds, const uint8_t program_id[32],
                         uint8_t out[32], uint8_t *out_bump);

#endif
