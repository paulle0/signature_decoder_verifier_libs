/* filehasher.c */
#include "sdv/filehasher.h"

#include <sodium.h>

#include <string.h>

/* The public header hardcodes 32 so it does not have to expose <sodium.h>.
   Fail the build here if the two ever disagree. Note this uses the
   crypto_hash_sha256_BYTES *macro*, which is a constant expression; the
   crypto_hash_sha256_bytes() function form would not compile here. */
typedef char sdv_digest_len_check[(SDV_SHA256_LEN == crypto_hash_sha256_BYTES) ? 1 : -1];

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

void sdv_sha256_to_hex(const sdv_sha256_t *digest, char *out)
{
	static const char hex[] = "0123456789abcdef";
	size_t i;

	for (i = 0; i < SDV_SHA256_LEN; ++i) {
		out[i * 2]     = hex[(digest->bytes[i] >> 4) & 0x0f];
		out[i * 2 + 1] = hex[digest->bytes[i] & 0x0f];
	}
	out[SDV_SHA256_LEN * 2] = '\0';
}

