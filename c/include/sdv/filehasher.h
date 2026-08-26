/* sdv/filehasher.h */
#ifndef SDV_FILEHASHER_H
#define SDV_FILEHASHER_H

/* Hashes the file at `path`, writing 32 raw bytes into `out`.
 * Returns 0 on success, negative on error. */
int sdv_hash_sha256(const char *path, unsigned char out[SDV_OUT_ARRAY(SDV_SHA256_BYTES)]);

#endif
