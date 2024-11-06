
all: link build

link: link-dice

link-%:
	gcc -c ./$*.c -o ./obj/$*.o

build:
	gcc $(wildcard ./obj/*.o) main.c -o main