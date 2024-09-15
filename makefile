rcc: src/*.c
	gcc -o $@ -g $^

run:
	@nasm -f elf64 -o test.o test.s
	@ld test.o -e main -o test
	@chmod +rwx test
	@./test

clean:
	rm -f rcc *.o