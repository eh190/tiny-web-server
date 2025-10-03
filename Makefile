CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
BIN_DIR = bin
SRC_DIR = src
OBJ_DIR = build
HEADER_DIR = headers
TARGET = tiny-server

default: dirs $(TARGET)

$(TARGET): ./$(OBJ_DIR)/main.o ./$(OBJ_DIR)/rio.o ./$(OBJ_DIR)/utils.o
	$(CC) $(CFLAGS) ./$(OBJ_DIR)/main.o ./$(OBJ_DIR)/rio.o ./$(OBJ_DIR)/utils.o -o ./$(BIN_DIR)/$(TARGET) 

./$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -c -I ./$(HEADER_DIR) -o ./$(OBJ_DIR)/main.o $(SRC_DIR)/main.c

./$(OBJ_DIR)/rio.o: $(SRC_DIR)/rio.c $(HEADER_DIR)/rio.h
	$(CC) $(CFLAGS) -c -I ./$(HEADER_DIR) -o ./$(OBJ_DIR)/rio.o $(SRC_DIR)/rio.c

./$(OBJ_DIR)/utils.o: $(SRC_DIR)/utils.c $(HEADER_DIR)/utils.h
	$(CC) $(CFLAGS) -c -I ./$(HEADER_DIR) -o ./$(OBJ_DIR)/utils.o $(SRC_DIR)/utils.c

# Ensure the output directories exist before we try to put files there
dirs:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

clean: clean-bin clean-o

clean-bin: 
	rm $(BIN_DIR)/$(TARGET)
clean-o:
	rm ./$(OBJ_DIR)/*.o
