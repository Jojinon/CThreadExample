main : main.c
	gcc -g main.c -o main -lpthread

clean :
	rm main
