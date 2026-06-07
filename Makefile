CC ?= cc
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L
AR ?= ar
PREFIX ?= /usr/local
BUILD_DIR := build
SRC := src/qihash.c
OBJ := $(BUILD_DIR)/qihash.o
LIB := $(BUILD_DIR)/libqihash.a
CLI := $(BUILD_DIR)/qihash
TEST := $(BUILD_DIR)/test_qihash
NEG_TEST := $(BUILD_DIR)/test_parser_negative
EXAMPLE := $(BUILD_DIR)/c_example
FUZZ_PARSE := $(BUILD_DIR)/fuzz_parse_encoded
FUZZ_CODEC := $(BUILD_DIR)/fuzz_codecs

.PHONY: all clean test install uninstall example dist sanitize fuzz

all: $(LIB) $(CLI)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ): $(SRC) include/qihash.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Iinclude -c $(SRC) -o $(OBJ)

$(LIB): $(OBJ)
	$(AR) rcs $(LIB) $(OBJ)

$(CLI): cli/qihash.c $(LIB)
	$(CC) $(CFLAGS) -Iinclude cli/qihash.c $(LIB) -o $(CLI)

$(TEST): tests/test_qihash.c $(LIB)
	$(CC) $(CFLAGS) -Iinclude tests/test_qihash.c $(LIB) -o $(TEST)

$(NEG_TEST): tests/test_parser_negative.c $(LIB)
	$(CC) $(CFLAGS) -Iinclude tests/test_parser_negative.c $(LIB) -o $(NEG_TEST)

$(EXAMPLE): examples/c_example.c $(LIB)
	$(CC) $(CFLAGS) -Iinclude examples/c_example.c $(LIB) -o $(EXAMPLE)

test: $(TEST) $(NEG_TEST) $(CLI)
	./$(TEST)
	./$(NEG_TEST)
	./$(CLI) selftest

example: $(EXAMPLE)
	./$(EXAMPLE)

sanitize:
	$(MAKE) clean
	$(MAKE) CC=$(CC) CFLAGS="-std=c99 -O1 -g -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer" test

$(FUZZ_PARSE): fuzz/fuzz_parse_encoded.c $(SRC) include/qihash.h | $(BUILD_DIR)
	clang -std=c99 -O1 -g -Iinclude -fsanitize=fuzzer,address,undefined fuzz/fuzz_parse_encoded.c $(SRC) -o $(FUZZ_PARSE)

$(FUZZ_CODEC): fuzz/fuzz_codecs.c $(SRC) include/qihash.h | $(BUILD_DIR)
	clang -std=c99 -O1 -g -Iinclude -fsanitize=fuzzer,address,undefined fuzz/fuzz_codecs.c $(SRC) -o $(FUZZ_CODEC)

fuzz: $(FUZZ_PARSE) $(FUZZ_CODEC)
	@echo "Run examples:"
	@echo "  ./$(FUZZ_PARSE) -max_total_time=60"
	@echo "  ./$(FUZZ_CODEC) -max_total_time=60"

install: all
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/include
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 755 $(CLI) $(DESTDIR)$(PREFIX)/bin/qihash
	install -m 644 include/qihash.h $(DESTDIR)$(PREFIX)/include/qihash.h
	install -m 644 $(LIB) $(DESTDIR)$(PREFIX)/lib/libqihash.a

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/qihash
	rm -f $(DESTDIR)$(PREFIX)/include/qihash.h
	rm -f $(DESTDIR)$(PREFIX)/lib/libqihash.a

clean:
	rm -rf $(BUILD_DIR)

dist: clean
	cd .. && zip -r qihash.zip qihash -x 'qihash/.git/*'
