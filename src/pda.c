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

int pda_is_on_curve(const uint8_t point[32]) {

    if (!point) {
        return -1;
    }

    uint8_t be[32];
    for (int i = 0; i < 32; i++) {
        be[i] = point[31 - i];
    }
    int sign = be[0] >> 7;
    be[0] &= 0x7F;

    int     res = -1;

    BN_CTX *ctx = BN_CTX_new();

    BIGNUM *p = NULL, *d = NULL, *x2 = BN_new(), *y2 = BN_new(),
           *num = BN_new(), *den = BN_new(), *den_inv = BN_new(),
           *exp = BN_new(), *leg = BN_new();

    BN_hex2bn(&p, ED25519_P_HEX);
    BN_hex2bn(&d, ED25519_D_HEX);

    BIGNUM *y = BN_bin2bn(be, 32, NULL);

    /*
     * x^2 = (y^2 - 1) / (d * y^2 + 1)
     */
    BN_mod_sqr(y2, y, p, ctx);
    BN_mod_sub(num, y2, BN_value_one(), p, ctx);

    if (BN_is_zero(num)) {
        res = sign ? 0 : 1;
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

        res = BN_is_one(leg) ? 1 : 0;
    }

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

int sha256_seeds(const SignerSeeds *seeds, const uint8_t program_id[32],
                 uint8_t out[32]) {

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();

    if (!ctx) {
        return -1;
    }

    unsigned int out_len = 32;

    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);

    for (int i = 0; i < seeds->len; i++) {
        EVP_DigestUpdate(ctx, seeds->seeds[i].addr, seeds->seeds[i].len);
    }
    EVP_DigestUpdate(ctx, program_id, 32);
    EVP_DigestUpdate(ctx, marker, 21);
    EVP_DigestFinal_ex(ctx, out, &out_len);
    EVP_MD_CTX_free(ctx);

    return 0;
}

int create_program_address(const SignerSeeds *seeds,
                           const uint8_t program_id[32], uint8_t out[32]) {

    sha256_seeds(seeds, program_id, out);

    int on_curve = pda_is_on_curve(out);

    if (on_curve == 1) {
        return -1;
    }

    return on_curve;
}
