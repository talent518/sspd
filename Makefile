CC=gcc

PHP_DIR = /opt/ssp
INC_DIR=$(PHP_DIR)/include
BIN_DIR=$(PWD)/bin

CFLAGS = -I$(INC_DIR) -I$(INC_DIR)/php -I$(INC_DIR)/php/main -I$(INC_DIR)/php/Zend -I$(INC_DIR)/php/TSRM -DZTS
LFLAGS = -lstdc++ -L$(PHP_DIR)/lib -lphp5 -levent -Wl,-rpath,$(PHP_DIR)/lib

all: $(BIN_DIR) $(BIN_DIR)/ssp $(BIN_DIR)/daemon

$(BIN_DIR):
	@mkdir $@

$(BIN_DIR)/ssp: $(BIN_DIR)/php_ext.o $(BIN_DIR)/php_func.o $(BIN_DIR)/server.o $(BIN_DIR)/node.o $(BIN_DIR)/ssp.o $(BIN_DIR)/api.o
	@echo -e "\E[34mbuild ssp"
	@tput sgr0
	@$(CC) $(LFLAGS) -o $@ $?

$(BIN_DIR)/daemon: $(BIN_DIR)/daemon.o $(BIN_DIR)/api.o
	@echo -e "\E[34mbuild daemon"
	@tput sgr0
	@$(CC) $(LFLAGS) -o $@ $?

$(BIN_DIR)/%.o: %.c
	@echo -e "\E[32m"$?
	@tput sgr0
	@$(CC) $(CFLAGS) -c $? -o $@

clean:
	@rm -rf $(BIN_DIR)/*.o
	@rm -rf $(BIN_DIR)/ssp
