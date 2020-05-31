CC=gcc
SRC_FILE=$(wildcard *.c)
OBJ_FILE=$(patsubst %.c, %.o ,$(SRC_FILE))
target=Multithread_sorting

all : $(target)
	
$(target) : $(OBJ_FILE)
	$(CC) -o $@ $^ -lpthread

$(OBJ_FILE) : $(SRC_FILE)
	$(CC) -o $@ -c $^ 

clean:
	$(RM) $(target) $(OBJ_FILE)
