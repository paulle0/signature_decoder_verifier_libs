/* main.c */
#include "filehasher.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        return 2;
    }

    if (sodium_init() < 0) {              /* MUST come before any crypto call */
        fprintf(stderr, "libsodium init failed\n");
        return 1;
    }

    unsigned char digest[crypto_hash_sha256_BYTES];
    if (hashSHA256(argv[1], digest) != 0) {
        fprintf(stderr, "hashing failed: %s\n", argv[1]);
        return 1;
    }

    char hex[crypto_hash_sha256_BYTES * 2 + 1];
    sodium_bin2hex(hex, sizeof hex, digest, sizeof digest);
    printf("%s  %s\n", hex, argv[1]);

    return 0;
}
