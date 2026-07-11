CC       = clang
CFLAGS   = -Wall -Wextra -Wpedantic
CPPFLAGS = -I/opt/homebrew/opt/openssl@3/include
LDFLAGS  = -L/opt/homebrew/opt/openssl@3/lib
LDLIBS   = -lcrypto

SRC       = src/pda.c
MAIN_SRC  = src/main.c thirdparty/base58.c
TEST_SRC  = tests/test.c
TEST_BIN  = tests/test

.PHONY: build run test format all clean

build: main

main: $(MAIN_SRC) $(SRC)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

run: build
	./main

test: $(TEST_SRC) $(SRC)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $^ -o $(TEST_BIN) $(LDLIBS)
	./$(TEST_BIN)

format:
	clang-format -i src/*.c src/*.h tests/*.c thirdparty/*.c thirdparty/*.h

all: run

clean:
	rm -f main $(TEST_BIN)
