SRC1 = mfs.c
OBJ1 = $(SRC1:.c=.o)
EXE = $(SRC1:.c=.e)

CFLAGS = -g

all : $(EXE)

$(EXE): $(OBJ1)
	gcc $(CFLAGS) $(OBJ1) -o $(EXE) 

$(OBJ1) : $(SRC1)
	gcc -c $(CFLAGS) $(SRC1) -o $(OBJ1) 
clean:
    rm $(EXE)