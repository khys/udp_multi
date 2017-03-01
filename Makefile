udpsrv: udpsrv.o
	gcc -o udpsrv udpsrv.o

udpsrv.o: udpsrv.c
	gcc -c udpsrv.c

clean:
	\rm udpsrv *.o
