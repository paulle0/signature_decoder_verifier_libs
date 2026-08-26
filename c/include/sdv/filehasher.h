/* sdv/filehasher.h */
#ifndef SDV_FILEHASHER_H
#define SDV_FILEHASHER_H

#include <stdio.h>
#include "sha256.h"
 
/* SHA256_BLOCK_SIZE from sha256.h is 32 = the digest length in bytes.
   Aliased here under a clearer name. */
#define SDV_SHA256_LEN SHA256_BLOCK_SIZE
 
/* Digest returned by value, so there is nothing to free and no output
   parameter. `ok` is 0 if the stream could not be read completely, in
   which case `bytes` is all zeroes and must not be used. */
typedef struct {
	BYTE bytes[SDV_SHA256_LEN];
	int  ok;
} sdv_sha256_t;
 
/* Hash everything from the current position of `fp` to end of stream.
   `fp` must be open for reading, ideally in binary mode ("rb").
   The caller keeps ownership of `fp` and is responsible for closing it. */
sdv_sha256_t sdv_hash_sha256(FILE *fp);
 
/* Convenience: write the digest as 64 lowercase hex chars + NUL.
   `out` must have room for SDV_SHA256_LEN * 2 + 1 = 65 bytes. */
void sdv_sha256_to_hex(const sdv_sha256_t *digest, char *out);

#endif
