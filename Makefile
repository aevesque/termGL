NAME=termGL.a

CC=cc
CFLAGS= -O -lm

RM=rm -rf

SRC=termGL.c

OBJ=${addprefix ${OBJ_DIR}/, ${SRC:.c=.o}}
OBJ_DIR=objs

all: ${NAME}

${NAME}: ${OBJ}
	ar rc ${NAME} $^

${OBJ_DIR}/%.o: %.c | ${OBJ_DIR}
	${CC} ${CFLAGS} -c $< -o $@

${OBJ_DIR}:
	mkdir ${OBJ_DIR}

clean:
	${RM} ${OBJ_DIR}

fclean: clean
	${RM} ${NAME}

re: fclean all
