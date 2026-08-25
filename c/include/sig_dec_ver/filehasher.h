/* filehash.h */
#ifndef FILEHASH_H
#define FILEHASH_H

#include <sodium.h>

/* Hashes the file at `path`, writing 32 raw bytes into `out`.
 * Returns 0 on success, negative on error. */
/* PRECONDITION: sodium_init() must have returned >= 0 before this is called.
 * The caller owns that; this function does not initialize libsodium. */
int hashSHA256(const char *path, unsigned char out[crypto_hash_sha256_BYTES]);

#endif
