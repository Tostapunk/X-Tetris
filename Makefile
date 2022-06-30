CC := gcc
CFLAGS := -ansi -Wall -Wextra -Wpedantic -O3

INCLUDE := -I include
SRC_DIR := src
EXE_DIR := build
OBJ_DIR := $(EXE_DIR)/obj
EXE := X_Tetris


$(EXE): main.o utils.o
	mkdir -p $(EXE_DIR)
	$(CC) -o $(EXE_DIR)/$(EXE) $(OBJ_DIR)/main.o $(OBJ_DIR)/utils.o $(CFLAGS) -lm

main.o:
	mkdir -p $(OBJ_DIR)
	$(CC) -o $(OBJ_DIR)/main.o -c $(SRC_DIR)/main.c $(INCLUDE) $(CFLAGS) 

utils.o:
	$(CC) -o $(OBJ_DIR)/utils.o -c $(SRC_DIR)/utils.c $(INCLUDE) $(CFLAGS) 

.PHONY: clean
clean:
	rm -r $(EXE_DIR)

run:
	./$(EXE_DIR)/$(EXE)
