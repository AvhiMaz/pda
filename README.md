## pda

standalone derivation of solana program derived addresses (pdas) in c, from scratch, no solana sdk. computes sha-256 of the seeds, checks the result against the ed25519 curve using openssl's bignum arithmetic, and searches bump seeds the same way agave does.

## how it works

a pda is valid exactly when its address is _not_ a point on the ed25519 curve, so no private key could ever sign for it.

```
address = sha256(seeds || program_id || "ProgramDerivedAddress")
```

on-curve check solves for x² and tests it with euler's criterion:

```
x² = (y² - 1) / (d·y² + 1)  (mod p)
on curve iff x²^((p-1)/2) mod p == 1
```

`p = 2^255 - 19` and `d` are the fixed ed25519 curve constants (rfc 8032). `find_program_address` tries bump seeds `255` down to `1`, first off-curve result wins, matching agave's `Pubkey::find_program_address`.

## credits

- [libbase58](https://github.com/luke-jr/libbase58) by Luke Dashjr, vendored in `thirdparty/`
- [OpenSSL](https://openssl.org) for bignum modular arithmetic (`BN_*`), not vendored, expected on the system
