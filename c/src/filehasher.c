/* filehasher.c */
#include "sdv/filehasher.h"
#include "sha256.h"

#include <string.h>

/* The public header hardcodes 32 so it does not have to expose sha256.h.
   Fail the build here if the two ever disagree. */
typedef char sdv_digest_len_check[(SDV_SHA256_LEN == SHA256_BLOCK_SIZE) ? 1 : -1];

/* Read size. Any value is correct; 32 KiB is a good trade-off between
   syscall count and stack usage. It also keeps every single call to
   sha256_update() far below UINT_MAX, which matters because that
   function counts with a 32-bit WORD internally. */
#define SDV_CHUNK 32768
 
sdv_sha256_t sdv_hash_sha256(FILE *fp)
{
	sdv_sha256_t digest;
	SHA256_CTX ctx;
	BYTE buf[SDV_CHUNK];
	size_t n;
 
	memset(&digest, 0, sizeof digest);
 
	if (fp == NULL)
		return digest;              /* digest.ok stays 0 */
 
	sha256_init(&ctx);
 
	while ((n = fread(buf, 1, sizeof buf, fp)) > 0)
		sha256_update(&ctx, buf, n);
 
	if (ferror(fp)) {               /* short read caused by an error */
		memset(&ctx, 0, sizeof ctx);
		return digest;              /* digest.ok stays 0 */
	}
 
	sha256_final(&ctx, digest.bytes);
	digest.ok = 1;
 
	memset(&ctx, 0, sizeof ctx);    /* do not leave state on the stack */
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


