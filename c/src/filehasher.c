/* filehasher.c */
#include "sdv/filehasher.h"

#include <sha256.h>

#define FH_CHUNK 16384

int sdv_hash_sha256(const char *path, unsigned char out[crypto_hash_sha256_BYTES])
{
    FILE *f = fopen(path, "rb");          /* "b" matters on Windows */
    if (!f)
        return -1;                        /* cannot open */

    crypto_hash_sha256_state state;
    if (crypto_hash_sha256_init(&state) != 0) {
        fclose(f);
        return -3;                        /* crypto failure */
    }

    unsigned char buf[FH_CHUNK];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, f)) > 0) {
        if (crypto_hash_sha256_update(&state, buf, n) != 0) {
            fclose(f);
            return -3;
        }
    }

    if (ferror(f)) {                      /* short read == EOF or error */
        fclose(f);
        return -2;                        /* read error */
    }
    fclose(f);

    if (crypto_hash_sha256_final(&state, out) != 0)
        return -3;

    return 0;
}
