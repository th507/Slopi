all: slopi 

debug: slopi.c
	clear
	gcc -pg -Wall -o slopi slopi.c -lm -pthread

clean: slopi 
	rm slopi

slopi: slopi.c
	gcc -O3 -Wall -o slopi slopi.c -lm -pthread
