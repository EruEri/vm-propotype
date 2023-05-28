FLAGS = -Wall -Werror

main: main.o stack.o util.o vm.o
	cc $(FLAGS) -o $@ $^

%.o: %.c
	cc $(FLAGS) -c -o $@ $<

clean:
	rm -r *.o