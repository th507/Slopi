all: slopi 

debug: slopi.c
	clear
	gcc -pg -Wall -o slopi slopi.c -lm -pthread

clean: slopi 
	rm slopi

slopi: slopi.c
	clear
	gcc -O3 -o slopi slopi.c -lm -pthread

sold: slop-cachesize=10i.c
	gcc -O3 -o sold "slopi-cachesize=10.c" modp_numtoa.c -lm -pthread

run: slopi.c Makefile
	gcc -O3 -o slopi slopi.c -lm -pthread
	clear
	./slopi
