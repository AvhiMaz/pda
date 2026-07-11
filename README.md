## pda

standalone derivation of solana program derived addresses (PDAs) in C, from scratch, no solana sdk. computes sha-256 of the seeds, checks the result against the ed25519 curve using openssl's bignum arithmetic, and searches bump seeds the same way [agave](https://github.com/anza-xyz/agave/) does.

## how it works

a PDA is valid exactly when its address is _not_ a point on the ed25519 curve, so no private key could ever sign for it.

```
address = sha256(seeds || program_id || "ProgramDerivedAddress")
```

on-curve check solves for x² and tests it with euler's criterion:

```
x² = (y² - 1) / (d·y² + 1)  (mod p)
on curve iff x²^((p-1)/2) mod p == 1
```

`p = 2^255 - 19` and `d` are the fixed ed25519 curve constants (rfc 8032). `find_program_address` tries bump seeds `255` down to `1`, first off-curve result wins, matching agave's `Pubkey::find_program_address`.

every function returns a `pda_status` (`PDA_OK`, `PDA_ON_CURVE`, `PDA_ERR_NULL`, `PDA_ERR_SEED_TOO_LONG`, `PDA_ERR_MAX_SEEDS`, `PDA_ERR_HASH`, `PDA_ERR_CRYPTO`, `PDA_ERR_NO_BUMP`) instead of a bare int, so callers can tell exactly what happened instead of guessing what a number means.

## build

```
make build   # build main
make run     # build and run main
make test    # build and run the test suite
```

## credits

- [libbase58](https://github.com/luke-jr/libbase58) by Luke Dashjr, vendored in `thirdparty/`
- [OpenSSL](https://openssl.org) for bignum modular arithmetic (`BN_*`), not vendored, expected on the system
