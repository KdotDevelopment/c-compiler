rcc: src/*.c
	gcc -o $@ -g $^

run:
	@./rasm test.s
	@chmod +rwx test.o
	@./test.o

run-nasm:
	@nasm -f elf64 -o test.o test.s
	@ld test.o -e main -o test
	@chmod +rwx test
	@./test

clean:
	rm -f rcc *.o