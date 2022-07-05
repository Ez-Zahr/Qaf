CC = gcc
CFLAGS = -c -g -std=c99
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

clean:
	rm src/*.o ${EXE}