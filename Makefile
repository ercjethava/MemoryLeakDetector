CC=gcc 
CFLAGS=-g -Wall
EXTERNAL_LIBS= -lmld
TAREGT_APP= test_app
LDPATH = $(shell pwd)/mld_src

TAREGT_APP:test_app.o
	${CC} ${CFLAGS} -L${LDPATH} test_app.o -o ${TAREGT_APP} ${EXTERNAL_LIBS}
test_app.o:test_app.c
	${CC} ${CFLAGS} -c test_app.c -o test_app.o

clean:
	rm -f testapp.o
	rm -f *.gch
