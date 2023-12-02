CC = gcc
CFLAGS = -c -g -std=c99 -Wall -fPIC -O3
LDFLAGS = -g
SRC = ${wildcard src/*.c}
HDR = ${wildcard include/*.h}
OBJ = ${SRC:.c=.o}
EXEC = qaf

all: ${SRC} ${OBJ} ${EXEC}
	rm src/*.o

${EXEC}: ${OBJ}
	${CC} ${LDFLAGS} $^ -o $@

%.o: %.c ${HDR}
	${CC} ${CFLAGS} $< -o $@

clean:
	rm ${EXEC}.* a.*