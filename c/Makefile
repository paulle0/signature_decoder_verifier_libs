# Builds libsdv.a (the file hasher plus the vendored SHA-256) and its tests.
#
#   make            build the static library
#   make test       build and run the tests
#   make clean      remove everything under build/
#
# The SHA-256 implementation lives in a git submodule; after a fresh clone run
#   git submodule update --init --recursive

CC      ?= cc
AR      ?= ar
ARFLAGS  = rcs
CFLAGS  ?= -std=c99 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS += -Iinclude 
LIB_CPPFLAGS := -Ilib/crypto-algorithms

BUILD   := build
LIB     := $(BUILD)/libsdv.a

LIB_SRCS := src/filehasher.c lib/crypto-algorithms/sha256.c
LIB_OBJS := $(LIB_SRCS:%.c=$(BUILD)/%.o)

TEST_SRC := tests/sdv/test_filehasher.c
TEST_BIN := $(BUILD)/test_filehasher
TEST_DATA := tests/data/test.txt

.PHONY: all test clean

all: $(LIB)

$(LIB): $(LIB_OBJS)
	@mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $^

# Objects mirror the source tree under build/, so the two sha256.o names
# (ours and the submodule's) can never collide.
$(BUILD)/%.o: %.c | submodule-check
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(LIB_CPPFLAGS) $(CFLAGS) -c $< -o $@

# Link the test against the archive, exactly as a consumer of the library would.
$(TEST_BIN): $(TEST_SRC) $(LIB)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIB) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN) $(TEST_DATA)

# Fail with a useful message instead of a confusing "sha256.h: No such file".
.PHONY: submodule-check
submodule-check:
	@test -f lib/crypto-algorithms/sha256.h || { \
		echo "error: lib/crypto-algorithms is empty."; \
		echo "       run: git submodule update --init --recursive"; \
		exit 1; }

clean:
	rm -rf $(BUILD)
