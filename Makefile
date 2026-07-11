.PHONY: build run format all clean

build:
	clang -Wall -Wpedantic -Wextra -Ilibbase58 -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib src/main.c src/pda.c thirdparty/base58.c -o main -lcrypto

run:
	./main

format:
	clang-format -i src/*.c src/*.h

all:
	clang -Wall -Wpedantic -Wextra -Ilibbase58 -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib src/main.c src/pda.c thirdparty/base58.c -o main -lcrypto && ./main

clean:
	rm main
