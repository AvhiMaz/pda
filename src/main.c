/*
 * ref: https://www.rfc-editor.org/info/rfc8032/
 */

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <stdint.h>
#include <stdio.h>

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

    BN_CTX *ctx = BN_CTX_new();

    BIGNUM *p = NULL, *d = NULL;

    BN_hex2bn(&p, ED25519_P_HEX);
    BN_hex2bn(&d, ED25519_D_HEX);

    BIGNUM *y = BN_bin2bn(be, 32, NULL);

    /*
     * x^2 = (y^2 - 1) / (d * y^2 + 1)
     */
    BIGNUM *x2 = BN_new(), *y2 = BN_new(), *num = BN_new(), *den = BN_new(),
           *den_inv = BN_new();

    BN_mod_sqr(y2, y, p, ctx);
    BN_mod_sub(num, y2, BN_value_one(), p, ctx);
    BN_mod_mul(den, d, y2, p, ctx);
    BN_mod_add(den, den, BN_value_one(), p, ctx);
    BN_mod_inverse(den_inv, den, p, ctx);
    BN_mod_mul(x2, num, den_inv, p, ctx);

    return 0;
}

int main(void) {
    printf("hello world\n");
    return 0;
}
