# Builds libsdv.a (the file hasher) and its tests.
#
#   make            build the static library
#   make test       build and run the tests
#   make clean      remove everything under build/
#
# SHA-256 and BLAKE2b both come from libsodium, linked dynamically. Install the
# development package before configuring:
#
#   Debian/Ubuntu   apt-get install libsodium-dev
#   Fedora/RHEL     dnf install libsodium-devel
#   macOS           brew install libsodium
#
# NOTE FOR CONSUMERS: libsdv.a is a static archive and therefore does not
# record its dependency on libsodium. Anything linking against it must add
# $(SODIUM_LIBS) (i.e. -lsodium) to its own link line, exactly as the test
# target below does.

CC      ?= cc
AR      ?= ar
ARFLAGS  = rcs
CFLAGS  ?= -std=c99 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS += -Iinclude

# Fall back to bare -lsodium when pkg-config is unavailable (common on
# macOS/Homebrew setups without PKG_CONFIG_PATH set).
PKG_CONFIG ?= pkg-config
SODIUM_CFLAGS := $(shell $(PKG_CONFIG) --cflags libsodium 2>/dev/null)
SODIUM_LIBS   := $(shell $(PKG_CONFIG) --libs libsodium 2>/dev/null || echo -lsodium)

BUILD   := build
LIB     := $(BUILD)/libsdv.a

LIB_SRCS := src/filehasher.c
LIB_OBJS := $(LIB_SRCS:%.c=$(BUILD)/%.o)

TEST_SRC := tests/sdv/test_filehasher.c
TEST_BIN := $(BUILD)/test_filehasher
TEST_DATA := tests/data/test.txt

.PHONY: all test clean

all: $(LIB)

$(LIB): $(LIB_OBJS)
	@mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $^

# Objects mirror the source tree under build/.
$(BUILD)/%.o: %.c | sodium-check
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(SODIUM_CFLAGS) $(CFLAGS) -c $< -o $@

# Link the test against the archive, exactly as a consumer of the library
# would -- including having to name -lsodium itself.
$(TEST_BIN): $(TEST_SRC) $(LIB)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIB) $(SODIUM_LIBS) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN) $(TEST_DATA)

# Fail with a useful message instead of a confusing "sodium.h: No such file".
.PHONY: sodium-check
sodium-check:
	@printf '#include <sodium.h>\nint main(void){return 0;}\n' \
		| $(CC) $(SODIUM_CFLAGS) -x c - $(SODIUM_LIBS) -o /dev/null 2>/dev/null \
		|| { \
		echo "error: libsodium development files not found."; \
		echo "       install libsodium-dev / libsodium-devel / brew install libsodium"; \
		exit 1; }

clean:
	rm -rf $(BUILD)
