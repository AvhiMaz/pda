.PHONY: build run format all clean

build:
	clang -Wall -Wpedantic src/main.c -o main

run:
	./main

format:
	clang-format -i src/*.c 

all:
	clang src/main.c -o main && ./main

clean:
	rm main
