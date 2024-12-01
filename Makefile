
all: build-c

build-java:
	javac ./java/DiceNotation.java
	java ./java/DiceNotation

build-c: link build

link: link-dice

link-%:
	gcc -c ./$*.c -o ./obj/$*.o

build:
	gcc $(wildcard ./obj/*.o) main.c -o main