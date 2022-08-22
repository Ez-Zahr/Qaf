CC = gcc
CFLAGS = -c -g -std=c99 -Wall -fPIC -O3
LDFLAGS = -g
SRC = ${wildcard src/*.c}
HDR = ${wildcard include/*.h}
OBJ = ${SRC:.c=.o}
EXE = qaf

all: ${SRC} ${OBJ} ${EXE}

${EXE}: ${OBJ}
	${CC} ${LDFLAGS} $^ -o $@

%.o: %.c ${HDR}
	${CC} ${CFLAGS} $< -o $@

vgrind:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./${EXE} test/test.qaf -s

clean:
	rm src/*.o ${EXE} a.*