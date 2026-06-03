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
EXAMPLE := $(BUILD_DIR)/c_example

.PHONY: all clean test install uninstall example dist

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

$(EXAMPLE): examples/c_example.c $(LIB)
	$(CC) $(CFLAGS) -Iinclude examples/c_example.c $(LIB) -o $(EXAMPLE)

test: $(TEST) $(CLI)
	./$(TEST)
	./$(CLI) selftest

example: $(EXAMPLE)
	./$(EXAMPLE)

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
