/* sdv/filehasher.h */
#ifndef SDV_FILEHASHER_H
#define SDV_FILEHASHER_H

#include <stddef.h>

#define SDV_OUT_ARRAY(n) n

/*Size of a SHA-256 digest, in bytes. Mirrors crypto_hash_sha256_BYTES; the
 * implementation static-asserts that the two agree. */
#define SDV_SHA256_BYTES 32

/* Return codes. */
#define SDV_OK          0
#define SDV_ERR_ARGS   -1   /* path or out was NULL */
#define SDV_ERR_OPEN   -2   /* file could not be opened */
#define SDV_ERR_READ   -3   /* I/O error while reading */
#define SDV_ERR_CRYPTO -4   /* the crypto backend reported a failure */
#define SDV_ERR_INIT   -5   /* the crypto backend could not be initialised */

/* Initialises the library. Idempotent and thread-safe. Calling it is optional
 * because every entry point initialises on demand, but calling it once at
 * start-up surfaces initialisation failure early. Returns SDV_OK/SDV_ERR_INIT. */
int sdv_init(void);

/* Hashes the file at `path`, writing 32 raw bytes into `out`.
 * Returns 0 on success, negative on error. */
int sdv_hash_sha256(const char *path, unsigned char out[SDV_OUT_ARRAY(SDV_SHA256_BYTES)]);

#endif
