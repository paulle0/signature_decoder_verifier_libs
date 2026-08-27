/* sdv/filehasher.h */
#ifndef SDV_FILEHASHER_H
#define SDV_FILEHASHER_H

#include <stdio.h>
 
/* SHA-256 digest length in bytes. filehasher.c static-asserts that this
   still matches libsodium's crypto_hash_sha256_BYTES. */
#define SDV_SHA256_LEN 32
 
/* Digest returned by value, so there is nothing to free and no output
   parameter. `ok` is 0 if the stream could not be read completely, in
   which case `bytes` is all zeroes and must not be used. */
typedef struct {
	unsigned char bytes[SDV_SHA256_LEN];
	int  ok;
} sdv_sha256_t;

/* Initialise the backing crypto library. Returns 1 on success, 0 on
   failure; safe and cheap to call repeatedly once it has succeeded.

   sdv_hash_sha256() calls this itself, so single-threaded callers can
   ignore it. Callers that hash from several threads MUST call it once,
   successfully, from a single thread first: racing the very first
   initialisation is undefined. */
int sdv_init(void);
 
/* Hash everything from the current position of `fp` to end of stream.
   `fp` must be open for reading, ideally in binary mode ("rb").
   The caller keeps ownership of `fp` and is responsible for closing it. 
   `ok` is also 0 if the crypto library failed to initialise. */
sdv_sha256_t sdv_hash_sha256(FILE *fp);
 
/* Convenience: write the digest as 64 lowercase hex chars + NUL.
   `out` must have room for SDV_SHA256_LEN * 2 + 1 = 65 bytes. */
void sdv_sha256_to_hex(const sdv_sha256_t *digest, char *out);

#endif
