rcc: src/*.c
	gcc -o $@ -g $^

clean:
	rm -f rcc *.o