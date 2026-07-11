/*
 * ref: https://www.rfc-editor.org/info/rfc8032/
 */

#include "pda.h"

#include <openssl/bn.h>
#include <openssl/evp.h>

static const char marker[] = "ProgramDerivedAddress";

/*
 * p = 2^255 - 19, converted to hex
 */
static const char ED25519_P_HEX[] =
    "7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED";

/*
 * d = -121665/121666 mod p, converted to hex
 */
static const char ED25519_D_HEX[] =
    "52036CEE2B6FFE738CC740797779E89800700A4D4141D8AB75EB4DCA135978A3";

pda_status pda_is_on_curve(const uint8_t point[PDA_PUBKEY_LEN]) {

    if (!point) {
        return PDA_ERR_NULL;
    }

    uint8_t be[PDA_PUBKEY_LEN];
    for (int i = 0; i < PDA_PUBKEY_LEN; i++) {
        be[i] = point[PDA_PUBKEY_LEN - 1 - i];
    }
    be[0] &= PDA_SIGN_MASK;

    pda_status res = PDA_ERR_CRYPTO;

    BN_CTX    *ctx = BN_CTX_new();

    BIGNUM    *p = NULL, *d = NULL, *y = NULL, *x2 = BN_new(), *y2 = BN_new(),
           *num = BN_new(), *den = BN_new(), *den_inv = BN_new(),
           *exp = BN_new(), *leg = BN_new();

    BN_hex2bn(&p, ED25519_P_HEX);
    BN_hex2bn(&d, ED25519_D_HEX);
    y = BN_bin2bn(be, PDA_PUBKEY_LEN, NULL);

    if (!ctx || !p || !d || !x2 || !y2 || !num || !den || !den_inv || !exp ||
        !leg || !y) {
        res = PDA_ERR_CRYPTO;
        goto out;
    }

    /*
     * x^2 = (y^2 - 1) / (d * y^2 + 1)
     */
    BN_mod_sqr(y2, y, p, ctx);
    BN_mod_sub(num, y2, BN_value_one(), p, ctx);

    if (BN_is_zero(num)) {
        res = PDA_ON_CURVE;
    } else {
        BN_mod_mul(den, d, y2, p, ctx);
        BN_mod_add(den, den, BN_value_one(), p, ctx);
        BN_mod_inverse(den_inv, den, p, ctx);
        BN_mod_mul(x2, num, den_inv, p, ctx);

        /*
         * Euler's criterion: x2^((p-1)/2) mod p == 1 iff x2 is a real square.
         */
        BN_sub(exp, p, BN_value_one());
        BN_rshift1(exp, exp);
        BN_mod_exp(leg, x2, exp, p, ctx);

        res = BN_is_one(leg) ? PDA_ON_CURVE : PDA_OK;
    }

out:
    BN_free(y);
    BN_free(p);
    BN_free(d);
    BN_free(x2);
    BN_free(y2);
    BN_free(num);
    BN_free(den);
    BN_free(den_inv);
    BN_free(exp);
    BN_free(leg);
    BN_CTX_free(ctx);

    return res;
}

pda_status sha256_seeds(const SignerSeeds *seeds,
                        const uint8_t      program_id[PDA_PUBKEY_LEN],
                        uint8_t            out[PDA_PUBKEY_LEN]) {

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();

    if (!ctx) {
        return PDA_ERR_HASH;
    }

    unsigned int out_len = PDA_PUBKEY_LEN;

    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);

    for (uint64_t i = 0; i < seeds->len; i++) {
        EVP_DigestUpdate(ctx, seeds->seeds[i].addr, seeds->seeds[i].len);
    }
    EVP_DigestUpdate(ctx, program_id, PDA_PUBKEY_LEN);
    EVP_DigestUpdate(ctx, marker, sizeof(marker) - 1);
    EVP_DigestFinal_ex(ctx, out, &out_len);
    EVP_MD_CTX_free(ctx);

    return PDA_OK;
}

pda_status create_program_address(const SignerSeeds *seeds,
                                  const uint8_t      program_id[PDA_PUBKEY_LEN],
                                  uint8_t            out[PDA_PUBKEY_LEN]) {

    for (uint64_t i = 0; i < seeds->len; i++) {
        if (seeds->seeds[i].len > PDA_MAX_SEED_LEN) {
            return PDA_ERR_SEED_TOO_LONG;
        }
    }

    if (seeds->len > PDA_MAX_SEEDS) {
        return PDA_ERR_MAX_SEEDS;
    }

    pda_status hash_status = sha256_seeds(seeds, program_id, out);
    if (hash_status != PDA_OK) {
        return hash_status;
    }

    return pda_is_on_curve(out);
}

pda_status find_program_address(const SignerSeeds *seeds,
                                const uint8_t      program_id[PDA_PUBKEY_LEN],
                                uint8_t            out[PDA_PUBKEY_LEN],
                                uint8_t           *out_bump) {

    SignerSeed extended[seeds->len + 1];

    for (uint64_t i = 0; i < seeds->len; i++) {
        extended[i] = seeds->seeds[i];
    }

    uint8_t bump;

    extended[seeds->len].addr = &bump;
    extended[seeds->len].len = 1;

    SignerSeeds extended_seeds = {.seeds = extended, .len = seeds->len + 1};

    for (int b = PDA_MAX_BUMP; b >= PDA_MIN_BUMP; b--) {
        bump = (uint8_t)b;
        pda_status r = create_program_address(&extended_seeds, program_id, out);

        if (r == PDA_OK) {
            *out_bump = bump;
            return PDA_OK;
        } else if (r != PDA_ON_CURVE) {
            return r;
        }
    }

    return PDA_ERR_NO_BUMP;
}
