CC=gcc 
CFLAGS= -Wall
LDFLAGS= -shared
OBJS=mld.o
TAREGT_LIB=libmld.so

TAREGT_LIB:${OBJS}
	${CC} ${LDFLAGS} ${OBJS} -o ${TAREGT_LIB}
mld.o:mld.c
	${CC} ${CFLAGS} -c -fpic mld.c -o mld.o

clean:
	rm -f ${TAREGT_LIB}
	rm -f ${OBJS}
	rm -f *.gch
