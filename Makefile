CC=gcc

PHP_DIR = /usr
INC_DIR=$(PHP_DIR)/include/php
BIN_DIR=$(PWD)/bin

CFLAGS = -I$(INC_DIR) -I$(INC_DIR)/main -I$(INC_DIR)/Zend -I$(INC_DIR)/TSRM 
LFLAGS = -lstdc++ -L$(PHP_DIR)/lib -lphp5

all: $(BIN_DIR) $(BIN_DIR)/ssp

$(BIN_DIR):
	mkdir $@

$(BIN_DIR)/ssp: $(BIN_DIR)/php_ext.o $(BIN_DIR)/php_func.o $(BIN_DIR)/server.o $(BIN_DIR)/ssp.o
	$(CC) $(LFLAGS) -o $@ $?

$(BIN_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $? -o $@

clean:
	@rm -rf $(BIN_DIR)/*.o
	@rm -rf $(BIN_DIR)/ssp
