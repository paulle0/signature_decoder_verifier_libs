/* filehasher.c */
#include "sdv/filehasher.h"

#include <sodium.h>

#include <string.h>

/* The public header hardcodes 32 so it does not have to expose <sodium.h>.
   Fail the build here if the two ever disagree. Note this uses the
   crypto_hash_sha256_BYTES *macro*, which is a constant expression; the
   crypto_hash_sha256_bytes() function form would not compile here. */
typedef char sdv_digest_len_check[(SDV_SHA256_LEN == crypto_hash_sha256_BYTES) ? 1 : -1];

/* BLAKE2b's output length is chosen by the caller rather than fixed by the
   algorithm, so there is no single _BYTES macro to compare against. What
   must hold is that both lengths sit inside the range libsodium accepts;
   crypto_generichash_blake2b_init() would reject anything else at runtime. */
typedef char sdv_blake2b_len_check[
	(SDV_BLAKE2B256_LEN >= crypto_generichash_blake2b_BYTES_MIN &&
	 SDV_BLAKE2B256_LEN <= crypto_generichash_blake2b_BYTES_MAX &&
	 SDV_BLAKE2B512_LEN >= crypto_generichash_blake2b_BYTES_MIN &&
	 SDV_BLAKE2B512_LEN <= crypto_generichash_blake2b_BYTES_MAX) ? 1 : -1];

/* Read size. Any value is correct; 32 KiB is a good trade-off between
   syscall count and stack usage. libsodium's update() takes an
   unsigned long long length, so the chunk size is purely about I/O
   buffering and imposes no limit of its own. */
#define SDV_CHUNK 32768

int sdv_init(void)
{
	return (sodium_init() < 0) ? 0 : 1;
}

sdv_sha256_t sdv_hash_sha256(FILE *fp)
{
	sdv_sha256_t digest;
	crypto_hash_sha256_state ctx;
	unsigned char buf[SDV_CHUNK];
	size_t n;

	memset(&digest, 0, sizeof digest);

	if (fp == NULL)
		return digest;              /* digest.ok stays 0 */

	/* Idempotent once it has succeeded. Racing the very first call from
	   several threads is undefined, which is what sdv_init() is for. */
	if (!sdv_init())
		return digest;              /* digest.ok stays 0 */

	crypto_hash_sha256_init(&ctx);

	while ((n = fread(buf, 1, sizeof buf, fp)) > 0)
		crypto_hash_sha256_update(&ctx, buf, (unsigned long long)n);

	if (ferror(fp)) {               /* short read caused by an error */
		sodium_memzero(&ctx, sizeof ctx);
		return digest;              /* digest.ok stays 0 */
	}

	crypto_hash_sha256_final(&ctx, digest.bytes);
	digest.ok = 1;

	sodium_memzero(&ctx, sizeof ctx);   /* do not leave state on the stack */
	return digest;
}

/* Shared body for both BLAKE2b sizes: the only thing that differs between
   them is `outlen`, which libsodium takes as an ordinary argument. Writes
   `outlen` bytes to `out` and returns 1 on success; on failure `out` is
   left untouched and 0 is returned, so callers can keep the all-zero
   digest they started with.

   Unkeyed, i.e. a plain hash rather than a MAC: passing a NULL key of
   length 0 is how libsodium spells that. */
static int sdv_blake2b_stream(FILE *fp, unsigned char *out, size_t outlen)
{
	crypto_generichash_blake2b_state ctx;
	unsigned char buf[SDV_CHUNK];
	size_t n;
	int rc = 0;

	if (fp == NULL)
		return 0;

	/* Idempotent once it has succeeded. Racing the very first call from
	   several threads is undefined, which is what sdv_init() is for. */
	if (!sdv_init())
		return 0;

	/* Unlike the SHA-256 init, this one can fail -- it validates outlen. */
	if (crypto_generichash_blake2b_init(&ctx, NULL, 0, outlen) != 0)
		return 0;

	while ((n = fread(buf, 1, sizeof buf, fp)) > 0) {
		if (crypto_generichash_blake2b_update(&ctx, buf,
		                                      (unsigned long long)n) != 0)
			goto out;           /* rc stays 0 */
	}

	if (ferror(fp))                 /* short read caused by an error */
		goto out;                   /* rc stays 0 */

	if (crypto_generichash_blake2b_final(&ctx, out, outlen) == 0)
		rc = 1;

out:
	sodium_memzero(&ctx, sizeof ctx);   /* do not leave state on the stack */
	return rc;
}

sdv_blake2b256_t sdv_hash_blake2b256(FILE *fp)
{
	sdv_blake2b256_t digest;

	memset(&digest, 0, sizeof digest);
	digest.ok = sdv_blake2b_stream(fp, digest.bytes, SDV_BLAKE2B256_LEN);

	return digest;
}

sdv_blake2b512_t sdv_hash_blake2b512(FILE *fp)
{
	sdv_blake2b512_t digest;

	memset(&digest, 0, sizeof digest);
	digest.ok = sdv_blake2b_stream(fp, digest.bytes, SDV_BLAKE2B512_LEN);

	return digest;
}

/* Shared body for the three *_to_hex() wrappers. */
static void sdv_bytes_to_hex(const unsigned char *bytes, size_t len, char *out)
{
	static const char hex[] = "0123456789abcdef";
	size_t i;

	for (i = 0; i < len; ++i) {
		out[i * 2]     = hex[(bytes[i] >> 4) & 0x0f];
		out[i * 2 + 1] = hex[bytes[i] & 0x0f];
	}
	out[len * 2] = '\0';
}

void sdv_sha256_to_hex(const sdv_sha256_t *digest, char *out)
{
	sdv_bytes_to_hex(digest->bytes, SDV_SHA256_LEN, out);
}

void sdv_blake2b256_to_hex(const sdv_blake2b256_t *digest, char *out)
{
	sdv_bytes_to_hex(digest->bytes, SDV_BLAKE2B256_LEN, out);
}

void sdv_blake2b512_to_hex(const sdv_blake2b512_t *digest, char *out)
{
	sdv_bytes_to_hex(digest->bytes, SDV_BLAKE2B512_LEN, out);
}
