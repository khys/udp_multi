all: udpsrv udpcli

udpsrv: udpsrv.o
	gcc -o udpsrv udpsrv.o

udpcli: udpcli.o
	gcc -o udpcli udpcli.o

udpsrv.o: udpsrv.c
	gcc -c udpsrv.c

udpcli.o: udpcli.c
	gcc -c udpcli.c

clean:
	\rm udpsrv udpcli *.o
