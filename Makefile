build:
	clang -std=c99 -Wall -pedantic *.c -o application

build-asan:
	clang -std=c99 -Wall -pedantic -fsanitize=address *.c -o application
