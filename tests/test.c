#include "../src/pda.h"

#include <stdio.h>
#include <string.h>

static int total = 0;
static int failures = 0;

#define PDA_TEST(c)                                                            \
    do {                                                                       \
        total++;                                                               \
        if (!(c)) {                                                            \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c);                \
            failures++;                                                        \
        }                                                                      \
    } while (0)

static int bytes_eq(const uint8_t *a, const uint8_t *b, size_t n) {
    return memcmp(a, b, n) == 0;
}

static const uint8_t SYSTEM_PROGRAM[32] = {0};

static const uint8_t METAPLEX[32] = {11, 112, 101, 177, 227, 209, 124, 69,
                                     56, 157, 82,  127, 107, 4,   195, 205,
                                     88, 184, 108, 115, 26,  160, 253, 181,
                                     73, 182, 209, 188, 3,   248, 41,  70};
static const uint8_t METAPLEX_MINT[32] = {
    6,  221, 246, 225, 215, 101, 161, 147, 217, 203, 225,
    70, 206, 235, 121, 172, 28,  180, 133, 237, 95,  91,
    55, 145, 58,  140, 245, 133, 126, 255, 0,   169};

static const uint8_t SYSVAR_CLOCK[32] = {
    6,   167, 213, 23,  24,  199, 116, 201, 40,  86, 99, 152, 105, 29, 94, 182,
    139, 94,  184, 163, 155, 75,  109, 92,  115, 85, 91, 33,  0,   0,  0,  0};

static void test_find_vectors(void) {
    uint8_t       out[32];
    uint8_t       bump;

    const uint8_t exp0[32] = {176, 201, 248, 101, 246, 103, 172, 131,
                              205, 155, 170, 233, 23,  45,  122, 162,
                              37,  191, 135, 70,  151, 239, 138, 79,
                              91,  239, 72,  25,  23,  115, 246, 254};
    SignerSeeds   empty = {.seeds = NULL, .len = 0};
    pda_status    r = find_program_address(&empty, SYSTEM_PROGRAM, out, &bump);
    PDA_TEST(r == PDA_OK && bytes_eq(out, exp0, 32) && bump == 255);

    const uint8_t vault[] = "vault";
    SignerSeed    sv[] = {{vault, 5}};
    SignerSeeds   seeds_vault = {.seeds = sv, .len = 1};
    const uint8_t exp1[32] = {61,  70,  126, 41,  228, 102, 58,  149,
                              108, 58,  168, 81,  133, 125, 136, 143,
                              96,  50,  196, 215, 96,  104, 37,  164,
                              171, 117, 242, 224, 169, 50,  160, 210};
    r = find_program_address(&seeds_vault, SYSTEM_PROGRAM, out, &bump);
    PDA_TEST(r == PDA_OK && bytes_eq(out, exp1, 32) && bump == 254);

    const uint8_t meta[] = "metadata";
    SignerSeed    sm[] = {{meta, 8}, {METAPLEX_MINT, 32}};
    SignerSeeds   seeds_meta = {.seeds = sm, .len = 2};
    const uint8_t exp2[32] = {253, 241, 33,  198, 104, 174, 62,  78,
                              101, 226, 224, 68,  41,  98,  76,  16,
                              31,  60,  178, 81,  194, 161, 239, 127,
                              223, 57,  128, 252, 173, 91,  74,  53};
    r = find_program_address(&seeds_meta, METAPLEX, out, &bump);
    PDA_TEST(r == PDA_OK && bytes_eq(out, exp2, 32) && bump == 255);

    const uint8_t a[] = "a", b[] = "b", c[] = "c";
    SignerSeed    s3[] = {{a, 1}, {b, 1}, {c, 1}};
    SignerSeeds   seeds3 = {.seeds = s3, .len = 3};
    const uint8_t exp3[32] = {214, 127, 229, 155, 150, 132, 108, 108,
                              148, 229, 209, 155, 26,  216, 247, 116,
                              229, 131, 123, 102, 137, 242, 113, 165,
                              225, 240, 27,  51,  61,  187, 194, 139};
    r = find_program_address(&seeds3, SYSVAR_CLOCK, out, &bump);
    PDA_TEST(r == PDA_OK && bytes_eq(out, exp3, 32) && bump == 253);
}

static void test_is_on_curve(void) {
    const uint8_t on[32] = {82,  173, 156, 51,  5,   84,  128, 195,
                            130, 24,  133, 179, 79,  137, 107, 102,
                            53,  209, 163, 160, 215, 225, 34,  59,
                            160, 52,  246, 139, 184, 235, 130, 212};
    PDA_TEST(pda_is_on_curve(on) == PDA_ON_CURVE);

    const uint8_t off[32] = {61,  70,  126, 41,  228, 102, 58,  149,
                             108, 58,  168, 81,  133, 125, 136, 143,
                             96,  50,  196, 215, 96,  104, 37,  164,
                             171, 117, 242, 224, 169, 50,  160, 210};
    PDA_TEST(pda_is_on_curve(off) == PDA_OK);

    uint8_t y1_s0[32] = {0};
    y1_s0[0] = 1;
    PDA_TEST(pda_is_on_curve(y1_s0) == PDA_ON_CURVE);

    uint8_t y1_s1[32] = {0};
    y1_s1[0] = 1;
    y1_s1[31] = 0x80;
    PDA_TEST(pda_is_on_curve(y1_s1) == PDA_ON_CURVE);

    uint8_t yneg1_s0[32] = {0xEC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F};
    PDA_TEST(pda_is_on_curve(yneg1_s0) == PDA_ON_CURVE);

    uint8_t yneg1_s1[32];
    memcpy(yneg1_s1, yneg1_s0, 32);
    yneg1_s1[31] |= 0x80;
    PDA_TEST(pda_is_on_curve(yneg1_s1) == PDA_ON_CURVE);
}

static void test_create_matches_find(void) {
    const uint8_t vault[] = "vault";
    SignerSeed    sv[] = {{vault, 5}};
    SignerSeeds   seeds_vault = {.seeds = sv, .len = 1};

    uint8_t       found[32], bump;
    find_program_address(&seeds_vault, SYSTEM_PROGRAM, found, &bump);

    SignerSeed  with_bump[] = {{vault, 5}, {&bump, 1}};
    SignerSeeds seeds_with_bump = {.seeds = with_bump, .len = 2};

    uint8_t     created[32];
    pda_status  r =
        create_program_address(&seeds_with_bump, SYSTEM_PROGRAM, created);
    PDA_TEST(r == PDA_OK && bytes_eq(created, found, 32));
}

int main(void) {
    test_find_vectors();
    test_is_on_curve();
    test_create_matches_find();

    printf("%d/%d tests passed\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
