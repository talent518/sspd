CC=gcc

INST_DIR  := /opt/ssp7
INC_DIR   := $(INST_DIR)/include
BIN_DIR   := $(INST_DIR)/bin
BUILD_DIR := $(PWD)/build

CFLAGS    := -g -O3 -Wno-unused-result -Wno-implicit-function-declaration -I$(INC_DIR) -I$(INC_DIR)/php -I$(INC_DIR)/php/main -I$(INC_DIR)/php/Zend -I$(INC_DIR)/php/TSRM -I$(INC_DIR)/php/ext -DZTS -DHAVE_LIBGTOP `pkg-config --cflags libgtop-2.0`
LFLAGS    := -g -lstdc++ -L$(INST_DIR)/lib -lm -lpthread -lphp7 -levent -Wl,-rpath,$(INST_DIR)/lib -Wl,-rpath,/usr/lib `pkg-config --libs libgtop-2.0`

all: $(BIN_DIR) $(BUILD_DIR) $(BIN_DIR)/ssp $(BIN_DIR)/daemon

$(BIN_DIR):
	$(PWD)/reflib.sh

$(BUILD_DIR):
	@mkdir $@

$(BIN_DIR)/ssp: $(BUILD_DIR)/php_ext.o $(BUILD_DIR)/php_func.o $(BUILD_DIR)/socket.o $(BUILD_DIR)/queue.o $(BUILD_DIR)/ssp_event.o $(BUILD_DIR)/server.o $(BUILD_DIR)/data.o $(BUILD_DIR)/ssp.o $(BUILD_DIR)/api.o
	@echo ssp
	@$(CC) -o $@ $? $(LFLAGS)

$(BIN_DIR)/daemon: $(BUILD_DIR)/daemon.o $(BUILD_DIR)/api.o
	@echo daemon
	@$(CC) -o $@ $? $(LFLAGS)

$(BUILD_DIR)/%.o: %.c
	@echo $?
	@$(CC) $(CFLAGS) -S $? -o $(@:.o=.s)
	@$(CC) $(CFLAGS) -E $? -o $(@:.o=.e)
	@$(CC) $(CFLAGS) -c $? -o $@

kill:
	@echo $@
	@-$(BIN_DIR)/ssp --pidfile $(PWD)/ssp.pid --user $(USER) -s stop

clean:
	@echo $@
	@rm -rf $(BUILD_DIR)/*.e $(BUILD_DIR)/*.s $(BUILD_DIR)/*.o
	@rm -rf $(BIN_DIR)/ssp
	@rm -rf $(BIN_DIR)/daemon

rebuild: kill clean $(BUILD_DIR) $(BIN_DIR)/ssp $(BIN_DIR)/daemon
	@echo $@

retest: kill
	@echo $@
	@$(BIN_DIR)/ssp --port 8086 --nthreads 20 --max-clients 2000 --timeout 300 --pidfile $(PWD)/ssp.pid --user $(USER) -f $(PWD)/bin/init.php -s start

pidstat: retest
	@echo $@
	@pidstat -r -p `cat $(PWD)/ssp.pid` 1

bench:
	@echo $@
	@$(BIN_DIR)/ssp -f $(PWD)/bin/bench.php -s script 127.0.0.1 8086 20 50 60000
