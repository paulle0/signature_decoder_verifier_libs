/* test_filehasher.c -- minimal checks for sdv_hash_sha256(). */
#include "sdv/filehasher.h"

#include <stdio.h>
#include <string.h>

/* sha256("Hello world!") -- 12 bytes, no trailing newline. */
#define EXPECTED_HELLO "c0535e4be2b79ffd93291305436bf889314e4a3faec05ecffcbb7df31ad9e51a"

/* sha256 of the same text with a trailing newline; only used to give a
   better message when the fixture got mangled on checkout. */
#define EXPECTED_HELLO_NL "0ba904eae8773b70c75333db4de2f3ac45a8ad4ddba1b242f0b3cfc199391dd8"

/* sha256("") -- the empty stream. */
#define EXPECTED_EMPTY "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"

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

/* Hash the fixture file and compare against the known digest. */
static void test_file(const char *path)
{
	sdv_sha256_t digest;
	char hex[SDV_SHA256_LEN * 2 + 1];
	FILE *fp;

	fp = fopen(path, "rb");
	if (fp == NULL) {
		printf("FAIL  hash of %s: cannot open file\n", path);
		++failures;
		return;
	}

	digest = sdv_hash_sha256(fp);
	fclose(fp);

	if (!digest.ok) {
		printf("FAIL  hash of %s: read error\n", path);
		++failures;
		return;
	}

	sdv_sha256_to_hex(&digest, hex);
	check_hex("sha256 of test.txt", hex, EXPECTED_HELLO);

	if (strcmp(hex, EXPECTED_HELLO_NL) == 0)
		printf("        hint: %s has a trailing newline; it must hold\n"
		       "        exactly the 12 bytes \"Hello world!\"\n", path);
}

/* An empty stream must still produce the well-known empty digest. */
static void test_empty_stream(void)
{
	sdv_sha256_t digest;
	char hex[SDV_SHA256_LEN * 2 + 1];
	FILE *fp;

	fp = tmpfile();
	if (fp == NULL) {
		printf("FAIL  sha256 of empty stream: tmpfile() failed\n");
		++failures;
		return;
	}

	digest = sdv_hash_sha256(fp);
	fclose(fp);

	if (!digest.ok) {
		printf("FAIL  sha256 of empty stream: read error\n");
		++failures;
		return;
	}

	sdv_sha256_to_hex(&digest, hex);
	check_hex("sha256 of empty stream", hex, EXPECTED_EMPTY);
}

/* A NULL stream must be reported through `ok`, not crash. */
static void test_null_stream(void)
{
	sdv_sha256_t digest = sdv_hash_sha256(NULL);

	if (digest.ok == 0) {
		printf("PASS  NULL stream reports failure\n");
	} else {
		printf("FAIL  NULL stream reported success\n");
		++failures;
	}
}

int main(int argc, char **argv)
{
	const char *path = (argc > 1) ? argv[1] : "test.txt";

	test_file(path);
	test_empty_stream();
	test_null_stream();

	if (failures == 0) {
		printf("all tests passed\n");
		return 0;
	}

	printf("%d test(s) failed\n", failures);
	return 1;
}
