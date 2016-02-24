CC=gcc

INST_DIR = /opt/ssp
INC_DIR  = $(INST_DIR)/include
BIN_DIR  = $(INST_DIR)/bin
BUILD_DIR=$(PWD)/build

CFLAGS   = -O3 -I$(INC_DIR) -I$(INC_DIR)/php -I$(INC_DIR)/php/main -I$(INC_DIR)/php/Zend -I$(INC_DIR)/php/TSRM -I$(INC_DIR)/php/ext -DZTS -DHAVE_LIBGTOP `pkg-config --cflags libgtop-2.0`
LFLAGS   = -lstdc++ -L$(INST_DIR)/lib -lphp5 -levent -Wl,-rpath,$(INST_DIR)/lib -Wl,-rpath,/opt/lampp/lib -Wl,-rpath,/usr/lib `pkg-config --libs libgtop-2.0`

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
	@echo -e "\E[34mbuild daemon\E[m"
	@tput sgr0
	@$(CC) $(LFLAGS) -o $@ $?

$(BUILD_DIR)/%.o: %.c
	@echo -e "\E[32m"$?"\E[m"
	@tput sgr0
	@$(CC) $(CFLAGS) -S $? -o $(@:.o=.s)
	@$(CC) $(CFLAGS) -E $? -o $(@:.o=.e)
	@$(CC) $(CFLAGS) -c $? -o $@

kill:
	@echo -e "\E[31m"$@"\E[m"
	@tput sgr0
	@-$(BIN_DIR)/ssp -s stop

clean:
	@echo -e "\E[33m"$@"\E[m"
	@tput sgr0
	@rm -rf $(BUILD_DIR)/*.e $(BUILD_DIR)/*.s $(BUILD_DIR)/*.o
	@rm -rf $(BIN_DIR)/ssp
	@rm -rf $(BIN_DIR)/daemon

rebuild: kill clean $(BUILD_DIR) $(BIN_DIR)/ssp $(BIN_DIR)/daemon
	@echo -e "\E[32m"$@"\E[m"
	@tput sgr0

retest: kill
	@echo -e "\E[32m"$@"\E[m"
	@tput sgr0
	@$(BIN_DIR)/ssp --port 8086 --nthreads 200 --max-clients 10000 --timeout 300 -f $(PWD)/bin/init.php -s start

pidstat: retest
	@echo -e "\E[31m"$@"\E[m"
	@tput sgr0
	@pidstat -r -p `cat /var/run/ssp.pid` 1

bench:
	@echo -e "\E[31m"$@"\E[m"
	@tput sgr0
	@$(BIN_DIR)/ssp -f $(PWD)/bin/bench.php -s script 127.0.0.1 8086 200 20 50000
