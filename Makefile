CC=gcc

INST_DIR = /opt/ssp
INC_DIR  = $(INST_DIR)/include
BIN_DIR  = $(INST_DIR)/bin
BUILD_DIR=$(PWD)/build

CFLAGS   = -I$(INC_DIR) -I$(INC_DIR)/php -I$(INC_DIR)/php/main -I$(INC_DIR)/php/Zend -I$(INC_DIR)/php/TSRM -I$(INC_DIR)/php/ext -DZTS `pkg-config --cflags glib-2.0` `pkg-config --cflags libgtop-2.0`
LFLAGS   = -lstdc++ -L$(INST_DIR)/lib -lphp5 -levent -Wl,-rpath,$(INST_DIR)/lib `pkg-config --libs glib-2.0` `pkg-config --libs libgtop-2.0`

all: $(BIN_DIR) $(BUILD_DIR) $(BIN_DIR)/ssp $(BIN_DIR)/daemon

$(BIN_DIR):
	$(PWD)/reflib.sh

$(BUILD_DIR):
	@mkdir $@

$(BIN_DIR)/ssp: $(BUILD_DIR)/php_ext.o $(BUILD_DIR)/php_func.o $(BUILD_DIR)/socket.o $(BUILD_DIR)/queue.o $(BUILD_DIR)/event.o $(BUILD_DIR)/server.o $(BUILD_DIR)/data.o $(BUILD_DIR)/ssp.o $(BUILD_DIR)/api.o
	@echo -e "\E[34mbuild ssp"
	@tput sgr0
	@$(CC) $(LFLAGS) -o $@ $?

$(BIN_DIR)/daemon: $(BUILD_DIR)/daemon.o $(BUILD_DIR)/api.o
	@echo -e "\E[34mbuild daemon"
	@tput sgr0
	@$(CC) $(LFLAGS) -o $@ $?

$(BUILD_DIR)/%.o: %.c
	@echo -e "\E[32m"$?
	@tput sgr0
	@$(CC) $(CFLAGS) -c $? -o $@

test: kill clean $(BUILD_DIR) $(BIN_DIR)/ssp $(BIN_DIR)/daemon
	@echo -e "\E[32m"$@
	@tput sgr0
	@$(BIN_DIR)/ssp --port 8086 --nthreads 20 --max-clients 5000 --user sspuser -b 5000 -f $(PWD)/bin/init.php -s start

retest: kill
	@echo -e "\E[32m"$@
	@tput sgr0
	@$(BIN_DIR)/ssp --port 8086 --nthreads 20 --max-clients 5000 --user sspuser -b 5000 -f $(PWD)/bin/init.php -s start

kill:
	@echo -e "\E[31m"$@
	@tput sgr0
	@-killall -2 -w -q ssp

bench:
	@echo -e "\E[31m"$@
	@tput sgr0
	@$(PWD)/bin/bench 127.0.0.1 8086 150 30 100

clean:
	@echo -e "\E[33m"$@
	@tput sgr0
	@rm -rf $(BUILD_DIR)/*.o
	@rm -rf $(BIN_DIR)/ssp
	@rm -rf $(BIN_DIR)/daemon
