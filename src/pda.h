#ifndef PDA_H
#define PDA_H

#include <stdint.h>

#define PDA_MAX_SEED_LEN 32
#define PDA_MAX_SEEDS    16
#define PDA_PUBKEY_LEN   32
#define PDA_MAX_BUMP     255
#define PDA_MIN_BUMP     1
#define PDA_SIGN_MASK    0x7F

typedef struct {
    const uint8_t *addr;
    uint64_t       len;
} SignerSeed;

typedef struct {
    const SignerSeed *seeds;
    uint64_t          len;
} SignerSeeds;

typedef enum {
    PDA_OK = 0,
    PDA_ON_CURVE = 1,
    PDA_ERR_NULL = -1,
    PDA_ERR_SEED_TOO_LONG = -2,
    PDA_ERR_MAX_SEEDS = -3,
    PDA_ERR_HASH = -4,
    PDA_ERR_CRYPTO = -5,
    PDA_ERR_NO_BUMP = -6,
} pda_status;

pda_status pda_is_on_curve(const uint8_t point[PDA_PUBKEY_LEN]);
pda_status sha256_seeds(const SignerSeeds *seeds,
                        const uint8_t      program_id[PDA_PUBKEY_LEN],
                        uint8_t            out[PDA_PUBKEY_LEN]);
pda_status create_program_address(const SignerSeeds *seeds,
                                  const uint8_t      program_id[PDA_PUBKEY_LEN],
                                  uint8_t            out[PDA_PUBKEY_LEN]);
pda_status find_program_address(const SignerSeeds *seeds,
                                const uint8_t      program_id[PDA_PUBKEY_LEN],
                                uint8_t out[PDA_PUBKEY_LEN], uint8_t *out_bump);

#endif
