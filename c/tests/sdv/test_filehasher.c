/* test_filehasher.c -- minimal checks for the sdv_hash_*() functions. */
#include "sdv/filehasher.h"

#include <stdio.h>
#include <string.h>

/* Digests of "Hello world!" -- 12 bytes, no trailing newline. */
#define EXPECTED_HELLO \
	"c0535e4be2b79ffd93291305436bf889314e4a3faec05ecffcbb7df31ad9e51a"
#define EXPECTED_HELLO_B2_256 \
	"3fbc092db9350757e2ab4f7ee9792bfcd2f5220ada5a4bc684487f60c6034369"
#define EXPECTED_HELLO_B2_512 \
	"0389abc5ab1e8e170e95aff19d341ecbf88b83a12dd657291ec1254108ea9735" \
	"2c2ff5116902b9fe4021bfe5a6a4372b0f7c9fc2d7dd810c29f85511d1e04c59"

/* Digests of the same text with a trailing newline; only used to give a
   better message when the fixture got mangled on checkout. */
#define EXPECTED_HELLO_NL \
	"0ba904eae8773b70c75333db4de2f3ac45a8ad4ddba1b242f0b3cfc199391dd8"
#define EXPECTED_HELLO_NL_B2_256 \
	"ca57d5b2364b0e3660f8dd44eafed7455b7ba59e3652309b45475edd9aaa1eeb"
#define EXPECTED_HELLO_NL_B2_512 \
	"5c5ce11923c07698f54cf30196efb3b038b44a77046fbabbdf3f2c2a924e1008" \
	"1e5f79cb4cd562f5590dc917c67fc0cdfaec0a992469e5cd0b3f7d1c249f9015"

/* Digests of the empty stream. */
#define EXPECTED_EMPTY \
	"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
#define EXPECTED_EMPTY_B2_256 \
	"0e5751c026e543b2e8ab2eb06099daa1d1e5df47778f7787faab45cdf12fe3a8"
#define EXPECTED_EMPTY_B2_512 \
	"786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419" \
	"d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce"

/* Longest hex string any of the three digests needs, plus the NUL. */
#define MAX_HEX (SDV_BLAKE2B512_LEN * 2 + 1)

static int failures;

static void check_hex(const char *what, const char *got, const char *want)
{
	if (strcmp(got, want) == 0) {
		printf("PASS  %s\n", what);
	} else {
		printf("FAIL  %s\n        got  %s\n        want %s\n", what, got, want);
		++failures;
	}
}

/* Report an already-detected failure in the usual format. */
static void fail(const char *what, const char *why)
{
	printf("FAIL  %s: %s\n", what, why);
	++failures;
}

/* Hash the fixture file with every algorithm and compare against the known
   digests. Each algorithm re-opens the file: sdv_hash_*() consumes the
   stream from wherever it happens to be positioned. */
static void test_file(const char *path)
{
	static const struct {
		const char *what;
		const char *want;
		const char *want_nl;
	} cases[] = {
		{ "sha256 of test.txt",       EXPECTED_HELLO,        EXPECTED_HELLO_NL        },
		{ "blake2b-256 of test.txt",  EXPECTED_HELLO_B2_256, EXPECTED_HELLO_NL_B2_256 },
		{ "blake2b-512 of test.txt",  EXPECTED_HELLO_B2_512, EXPECTED_HELLO_NL_B2_512 },
	};
	size_t i;

	for (i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
		char hex[MAX_HEX];
		FILE *fp = fopen(path, "rb");
		int ok;

		if (fp == NULL) {
			fail(cases[i].what, "cannot open file");
			continue;
		}

		if (i == 0) {
			sdv_sha256_t d = sdv_hash_sha256(fp);
			if ((ok = d.ok) != 0)
				sdv_sha256_to_hex(&d, hex);
		} else if (i == 1) {
			sdv_blake2b256_t d = sdv_hash_blake2b256(fp);
			if ((ok = d.ok) != 0)
				sdv_blake2b256_to_hex(&d, hex);
		} else {
			sdv_blake2b512_t d = sdv_hash_blake2b512(fp);
			if ((ok = d.ok) != 0)
				sdv_blake2b512_to_hex(&d, hex);
		}

		fclose(fp);

		if (!ok) {
			fail(cases[i].what, "read error");
			continue;
		}

		check_hex(cases[i].what, hex, cases[i].want);

		if (strcmp(hex, cases[i].want_nl) == 0)
			printf("        hint: %s has a trailing newline; it must hold\n"
			       "        exactly the 12 bytes \"Hello world!\"\n", path);
	}
}

/* An empty stream must still produce the well-known empty digests. */
static void test_empty_stream(void)
{
	char hex[MAX_HEX];
	FILE *fp;

	fp = tmpfile();
	if (fp == NULL) {
		fail("hashes of empty stream", "tmpfile() failed");
		return;
	}

	{
		sdv_sha256_t d = sdv_hash_sha256(fp);
		if (!d.ok) {
			fail("sha256 of empty stream", "read error");
		} else {
			sdv_sha256_to_hex(&d, hex);
			check_hex("sha256 of empty stream", hex, EXPECTED_EMPTY);
		}
	}

	/* Each hash consumed the stream to EOF, so rewind before the next. */
	rewind(fp);

	{
		sdv_blake2b256_t d = sdv_hash_blake2b256(fp);
		if (!d.ok) {
			fail("blake2b-256 of empty stream", "read error");
		} else {
			sdv_blake2b256_to_hex(&d, hex);
			check_hex("blake2b-256 of empty stream", hex, EXPECTED_EMPTY_B2_256);
		}
	}

	rewind(fp);

	{
		sdv_blake2b512_t d = sdv_hash_blake2b512(fp);
		if (!d.ok) {
			fail("blake2b-512 of empty stream", "read error");
		} else {
			sdv_blake2b512_to_hex(&d, hex);
			check_hex("blake2b-512 of empty stream", hex, EXPECTED_EMPTY_B2_512);
		}
	}

	fclose(fp);
}

/* A NULL stream must be reported through `ok`, not crash. */
static void test_null_stream(void)
{
	sdv_sha256_t     s  = sdv_hash_sha256(NULL);
	sdv_blake2b256_t b2 = sdv_hash_blake2b256(NULL);
	sdv_blake2b512_t b5 = sdv_hash_blake2b512(NULL);

	if (s.ok == 0 && b2.ok == 0 && b5.ok == 0) {
		printf("PASS  NULL stream reports failure\n");
	} else {
		printf("FAIL  NULL stream reported success\n");
		++failures;
	}
}

/* A failed digest must be all zeroes, so a caller that ignores `ok` cannot
   mistake stack leftovers for a real hash. */
static void test_failed_digest_is_zeroed(void)
{
	sdv_blake2b512_t d = sdv_hash_blake2b512(NULL);
	unsigned char zero[SDV_BLAKE2B512_LEN];

	memset(zero, 0, sizeof zero);

	if (memcmp(d.bytes, zero, sizeof zero) == 0) {
		printf("PASS  failed digest is zeroed\n");
	} else {
		printf("FAIL  failed digest is not zeroed\n");
		++failures;
	}
}

/* Hashing must not depend on the data arriving in one read(), so a file
   larger than the internal chunk size has to match the one-shot digest of
   the same bytes. */
static void test_multi_chunk_stream(void)
{
	/* Bigger than filehasher.c's 32 KiB read buffer, and deliberately not
	   a multiple of it, so the last update() is a partial chunk. */
	static const size_t total = 70000;
	char hex[MAX_HEX];
	FILE *fp;
	size_t i;

	fp = tmpfile();
	if (fp == NULL) {
		fail("blake2b-512 of multi-chunk stream", "tmpfile() failed");
		return;
	}

	for (i = 0; i < total; ++i) {
		if (fputc((int)(i % 251), fp) == EOF) {
			fail("blake2b-512 of multi-chunk stream", "write failed");
			fclose(fp);
			return;
		}
	}
	rewind(fp);

	{
		sdv_blake2b512_t d = sdv_hash_blake2b512(fp);

		fclose(fp);

		if (!d.ok) {
			fail("blake2b-512 of multi-chunk stream", "read error");
			return;
		}

		sdv_blake2b512_to_hex(&d, hex);
	}

	/* Independently produced digest of the same 70000 bytes:
	   python3 -c "import hashlib,sys; \
	     print(hashlib.blake2b(bytes(i%251 for i in range(70000)), \
	                           digest_size=64).hexdigest())" */
	check_hex("blake2b-512 of multi-chunk stream", hex,
/* test_filehasher.c -- minimal checks for the sdv_hash_*() functions. */
#include "sdv/filehasher.h"

#include <stdio.h>
#include <string.h>

/* Digests of "Hello world!" -- 12 bytes, no trailing newline. */
#define EXPECTED_HELLO \
	"c0535e4be2b79ffd93291305436bf889314e4a3faec05ecffcbb7df31ad9e51a"
#define EXPECTED_HELLO_B2_256 \
	"3fbc092db9350757e2ab4f7ee9792bfcd2f5220ada5a4bc684487f60c6034369"
#define EXPECTED_HELLO_B2_512 \
	"0389abc5ab1e8e170e95aff19d341ecbf88b83a12dd657291ec1254108ea9735" \
	"2c2ff5116902b9fe4021bfe5a6a4372b0f7c9fc2d7dd810c29f85511d1e04c59"

/* Digests of the same text with a trailing newline; only used to give a
   better message when the fixture got mangled on checkout. */
#define EXPECTED_HELLO_NL \
	"0ba904eae8773b70c75333db4de2f3ac45a8ad4ddba1b242f0b3cfc199391dd8"
#define EXPECTED_HELLO_NL_B2_256 \
	"ca57d5b2364b0e3660f8dd44eafed7455b7ba59e3652309b45475edd9aaa1eeb"
#define EXPECTED_HELLO_NL_B2_512 \
	"5c5ce11923c07698f54cf30196efb3b038b44a77046fbabbdf3f2c2a924e1008" \
	"1e5f79cb4cd562f5590dc917c67fc0cdfaec0a992469e5cd0b3f7d1c249f9015"

/* Digests of the empty stream. */
#define EXPECTED_EMPTY \
	"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
#define EXPECTED_EMPTY_B2_256 \
	"0e5751c026e543b2e8ab2eb06099daa1d1e5df47778f7787faab45cdf12fe3a8"
#define EXPECTED_EMPTY_B2_512 \
	"786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419" \
	"d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce"

/* Longest hex string any of the three digests needs, plus the NUL. */
#define MAX_HEX (SDV_BLAKE2B512_LEN * 2 + 1)

static int failures;

static void check_hex(const char *what, const char *got, const char *want)
{
	if (strcmp(got, want) == 0) {
		printf("PASS  %s\n", what);
	} else {
		printf("FAIL  %s\n        got  %s\n        want %s\n", what, got, want);
		++failures;
	}
}

/* Report an already-detected failure in the usual format. */
static void fail(const char *what, const char *why)
{
	printf("FAIL  %s: %s\n", what, why);
	++failures;
}

/* Hash the fixture file with every algorithm and compare against the known
   digests. Each algorithm re-opens the file: sdv_hash_*() consumes the
   stream from wherever it happens to be positioned. */
static void test_file(const char *path)
{
	static const struct {
		const char *what;
		const char *want;
		const char *want_nl;
	} cases[] = {
		{ "sha256 of test.txt",       EXPECTED_HELLO,        EXPECTED_HELLO_NL        },
		{ "blake2b-256 of test.txt",  EXPECTED_HELLO_B2_256, EXPECTED_HELLO_NL_B2_256 },
		{ "blake2b-512 of test.txt",  EXPECTED_HELLO_B2_512, EXPECTED_HELLO_NL_B2_512 },
	};
	size_t i;

	for (i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
		char hex[MAX_HEX];
		FILE *fp = fopen(path, "rb");
		int ok;

		if (fp == NULL) {
			fail(cases[i].what, "cannot open file");
			continue;
		}

		if (i == 0) {
			sdv_sha256_t d = sdv_hash_sha256(fp);
			if ((ok = d.ok) != 0)
				sdv_sha256_to_hex(&d, hex);
		} else if (i == 1) {
			sdv_blake2b256_t d = sdv_hash_blake2b256(fp);
			if ((ok = d.ok) != 0)
				sdv_blake2b256_to_hex(&d, hex);
		} else {
			sdv_blake2b512_t d = sdv_hash_blake2b512(fp);
			if ((ok = d.ok) != 0)
				sdv_blake2b512_to_hex(&d, hex);
		}

		fclose(fp);

		if (!ok) {
			fail(cases[i].what, "read error");
			continue;
		}

		check_hex(cases[i].what, hex, cases[i].want);

		if (strcmp(hex, cases[i].want_nl) == 0)
			printf("        hint: %s has a trailing newline; it must hold\n"
			       "        exactly the 12 bytes \"Hello world!\"\n", path);
	}
}

/* An empty stream must still produce the well-known empty digests. */
static void test_empty_stream(void)
{
	char hex[MAX_HEX];
	FILE *fp;

	fp = tmpfile();
	if (fp == NULL) {
		fail("hashes of empty stream", "tmpfile() failed");
		return;
	}

	{
		sdv_sha256_t d = sdv_hash_sha256(fp);
		if (!d.ok) {
			fail("sha256 of empty stream", "read error");
		} else {
			sdv_sha256_to_hex(&d, hex);
			check_hex("sha256 of empty stream", hex, EXPECTED_EMPTY);
		}
	}

	/* Each hash consumed the stream to EOF, so rewind before the next. */
	rewind(fp);

	{
		sdv_blake2b256_t d = sdv_hash_blake2b256(fp);
		if (!d.ok) {
			fail("blake2b-256 of empty stream", "read error");
		} else {
			sdv_blake2b256_to_hex(&d, hex);
			check_hex("blake2b-256 of empty stream", hex, EXPECTED_EMPTY_B2_256);
		}
	}

	rewind(fp);

	{
		sdv_blake2b512_t d = sdv_hash_blake2b512(fp);
		if (!d.ok) {
			fail("blake2b-512 of empty stream", "read error");
		} else {
			sdv_blake2b512_to_hex(&d, hex);
			check_hex("blake2b-512 of empty stream", hex, EXPECTED_EMPTY_B2_512);
		}
	}

	fclose(fp);
}

/* A NULL stream must be reported through `ok`, not crash. */
static void test_null_stream(void)
{
	sdv_sha256_t     s  = sdv_hash_sha256(NULL);
	sdv_blake2b256_t b2 = sdv_hash_blake2b256(NULL);
	sdv_blake2b512_t b5 = sdv_hash_blake2b512(NULL);

	if (s.ok == 0 && b2.ok == 0 && b5.ok == 0) {
		printf("PASS  NULL stream reports failure\n");
	} else {
		printf("FAIL  NULL stream reported success\n");
		++failures;
	}
}

/* A failed digest must be all zeroes, so a caller that ignores `ok` cannot
   mistake stack leftovers for a real hash. */
static void test_failed_digest_is_zeroed(void)
{
	sdv_blake2b512_t d = sdv_hash_blake2b512(NULL);
	unsigned char zero[SDV_BLAKE2B512_LEN];

	memset(zero, 0, sizeof zero);

	if (memcmp(d.bytes, zero, sizeof zero) == 0) {
		printf("PASS  failed digest is zeroed\n");
	} else {
		printf("FAIL  failed digest is not zeroed\n");
		++failures;
	}
}

/* Hashing must not depend on the data arriving in one read(), so a file
   larger than the internal chunk size has to match the one-shot digest of
   the same bytes. */
static void test_multi_chunk_stream(void)
{
	/* Bigger than filehasher.c's 32 KiB read buffer, and deliberately not
	   a multiple of it, so the last update() is a partial chunk. */
	static const size_t total = 70000;
	char hex[MAX_HEX];
	FILE *fp;
	size_t i;

	fp = tmpfile();
	if (fp == NULL) {
		fail("blake2b-512 of multi-chunk stream", "tmpfile() failed");
		return;
	}

	for (i = 0; i < total; ++i) {
		if (fputc((int)(i % 251), fp) == EOF) {
			fail("blake2b-512 of multi-chunk stream", "write failed");
			fclose(fp);
			return;
		}
	}
	rewind(fp);

	{
		sdv_blake2b512_t d = sdv_hash_blake2b512(fp);

		fclose(fp);

		if (!d.ok) {
			fail("blake2b-512 of multi-chunk stream", "read error");
			return;
		}

		sdv_blake2b512_to_hex(&d, hex);
	}

	/* Independently produced digest of the same 70000 bytes:
	   python3 -c "import hashlib,sys; \
	     print(hashlib.blake2b(bytes(i%251 for i in range(70000)), \
	                           digest_size=64).hexdigest())" */
	check_hex("blake2b-512 of multi-chunk stream", hex,
	          "a09aa84d38f17f9c7c3c9e307732384f894aa41136abf106fa6107531adcd6cc"
	          "900fb716349e9d61a0cac3606f9d8f2f7993a5bf963b809324209fe5a8268e38");
}

int main(int argc, char **argv)
{
	const char *path = (argc > 1) ? argv[1] : "test.txt";

	test_file(path);
	test_empty_stream();
	test_null_stream();
	test_failed_digest_is_zeroed();
	test_multi_chunk_stream();

	if (failures == 0) {
		printf("all tests passed\n");
		return 0;
	}

	printf("%d test(s) failed\n", failures);
	return 1;
}	          "a09aa84d38f17f9c7c3c9e307732384f894aa41136abf106fa6107531adcd6cc"
	          "900fb716349e9d61a0cac3606f9d8f2f7993a5bf963b809324209fe5a8268e38");
}

int main(int argc, char **argv)
{
	const char *path = (argc > 1) ? argv[1] : "test.txt";

	test_file(path);
	test_empty_stream();
	test_null_stream();
	test_failed_digest_is_zeroed();
	test_multi_chunk_stream();

	if (failures == 0) {
		printf("all tests passed\n");
		return 0;
	}

	printf("%d test(s) failed\n", failures);
	return 1;
}
