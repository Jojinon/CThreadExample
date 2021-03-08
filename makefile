main : main.o task.o
	gcc -g -o main *.o -lpthread

main.o :
	gcc -g -c main.c

task.o :
	gcc -g -c task.c

clean :
	rm main main.o task.o
