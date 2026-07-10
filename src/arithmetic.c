/*
 * fe_add and fe_sub below are not in use.
 * will implement manually later.
 */

#include <stdint.h>

typedef struct {
    uint64_t v[4];
} fe;

static inline fe fe_add(fe a, fe b) {
    fe       r;

    uint64_t carry = 0;

    for (int i = 0; i < 4; i++) {
        uint64_t t = a.v[i] + carry;
        carry = (t < a.v[i]);

        uint64_t s = t + b.v[i];
        carry += (s < t);

        r.v[i] = s;
    }
    return r;
}

static inline fe fe_sub(fe a, fe b) {
    fe       r;

    uint64_t borrow = 0;

    for (int i = 0; i < 4; i++) {
        uint64_t t = a.v[i] - borrow;
        uint64_t b1 = (a.v[i] < borrow);

        uint64_t d = t - b.v[i];
        uint64_t b2 = (t < b.v[i]);

        r.v[i] = d;
        borrow = b1 | b2;
    }
    return r;
}
