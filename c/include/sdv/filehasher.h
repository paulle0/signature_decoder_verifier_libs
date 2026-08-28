/* sdv/filehasher.h */
#ifndef SDV_FILEHASHER_H
#define SDV_FILEHASHER_H

#include <stdio.h>
 
/* SHA-256 digest length in bytes. filehasher.c static-asserts that this
   still matches libsodium's crypto_hash_sha256_BYTES. */
#define SDV_SHA256_LEN 32

/* BLAKE2b digest lengths in bytes. BLAKE2b has a selectable output size;
   these are the two this library exposes. filehasher.c static-asserts that
   both stay within libsodium's supported range. */
#define SDV_BLAKE2B256_LEN 32
#define SDV_BLAKE2B512_LEN 64
 
/* Digest returned by value, so there is nothing to free and no output
   parameter. `ok` is 0 if the stream could not be read completely, in
   which case `bytes` is all zeroes and must not be used. */
typedef struct {
	unsigned char bytes[SDV_SHA256_LEN];
	int  ok;
} sdv_sha256_t;

/* Same contract as sdv_sha256_t, for the two BLAKE2b sizes. These are
   distinct types on purpose: a 256-bit digest cannot be passed to the
   512-bit hex printer by accident. */
typedef struct {
	unsigned char bytes[SDV_BLAKE2B256_LEN];
	int  ok;
} sdv_blake2b256_t;

typedef struct {
	unsigned char bytes[SDV_BLAKE2B512_LEN];
	int  ok;
} sdv_blake2b512_t;

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

/* Unkeyed BLAKE2b with a 256-bit output. Same stream contract as
   sdv_hash_sha256(); this is the plain hash, not the keyed MAC, so the
   digest matches `b2sum -l 256` and libsodium's crypto_generichash()
   at its default output length. */
sdv_blake2b256_t sdv_hash_blake2b256(FILE *fp);

/* Unkeyed BLAKE2b with a 512-bit output -- BLAKE2b's full width, and what
   `b2sum` prints by default. Same stream contract as above. */
sdv_blake2b512_t sdv_hash_blake2b512(FILE *fp);
 
/* Convenience: write the digest as 64 lowercase hex chars + NUL.
   `out` must have room for SDV_SHA256_LEN * 2 + 1 = 65 bytes. */
void sdv_sha256_to_hex(const sdv_sha256_t *digest, char *out);

/* As above: 64 lowercase hex chars + NUL, so `out` needs
   SDV_BLAKE2B256_LEN * 2 + 1 = 65 bytes. */
void sdv_blake2b256_to_hex(const sdv_blake2b256_t *digest, char *out);

/* As above: 128 lowercase hex chars + NUL, so `out` needs
   SDV_BLAKE2B512_LEN * 2 + 1 = 129 bytes. */
void sdv_blake2b512_to_hex(const sdv_blake2b512_t *digest, char *out);

#endif
