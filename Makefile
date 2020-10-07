CC=gcc

INST_DIR  := /opt/ssp74
INC_DIR   := $(INST_DIR)/include
BIN_DIR   := $(INST_DIR)/bin
BUILD_DIR := build

CFLAGS    := -O3 -Wno-unused-result -Wno-implicit-function-declaration -I$(INC_DIR) -I$(INC_DIR)/php -I$(INC_DIR)/php/main -I$(INC_DIR)/php/Zend -I$(INC_DIR)/php/TSRM -I$(INC_DIR)/php/ext -DZTS
LFLAGS    := -L$(INST_DIR)/lib -lm -lpthread -lphp7 -levent -Wl,-rpath,$(INST_DIR)/lib -Wl,-rpath,/usr/lib

LANG      := en

all: $(BIN_DIR) $(BUILD_DIR) $(BIN_DIR)/ssp $(BIN_DIR)/monitor

$(BIN_DIR):
	@chmod +x reflib.sh && $(PWD)/reflib.sh

$(BUILD_DIR):
	@mkdir $@

$(BIN_DIR)/ssp: $(BUILD_DIR)/php_ext.o $(BUILD_DIR)/php_func.o $(BUILD_DIR)/socket.o $(BUILD_DIR)/queue.o $(BUILD_DIR)/ssp_event.o $(BUILD_DIR)/server.o $(BUILD_DIR)/data.o $(BUILD_DIR)/ssp.o $(BUILD_DIR)/api.o $(BUILD_DIR)/crypt.o $(BUILD_DIR)/base64.o $(BUILD_DIR)/md5.o $(BUILD_DIR)/top.o $(BUILD_DIR)/hash.o
	@echo LD ssp
	@$(CC) -o $@ $^ $(LFLAGS)

$(BUILD_DIR)/%.o: %.c %.h config.h
	@echo CC $<
	@$(CC) $(CFLAGS) -S $< -o $(@:.o=.s)
	@$(CC) $(CFLAGS) -E $< -o $(@:.o=.e)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/monitor: $(BUILD_DIR)/top.o monitor.c
	@echo LD monitor
	@$(CC) -o $@ $^ -Wno-format -D_GNU_SOURCE -lm

kill:
	@echo $@
	@-$(BIN_DIR)/ssp --pidfile $(PWD)/ssp.pid --user $(USER) -s stop

clean:
	@echo $@
	@rm -vf $(BUILD_DIR)/*.e $(BUILD_DIR)/*.s $(BUILD_DIR)/*.o
	@rm -vf $(BIN_DIR)/ssp $(BIN_DIR)/monitor

rebuild: kill clean all
	@echo $@

retest: kill all
	@echo $@
	@$(BIN_DIR)/ssp --port 8086 --nthreads 8 --max-clients 6000 --timeout 300 --pidfile $(PWD)/ssp.pid --outfile $(PWD)/ssp.out --errfile $(PWD)/ssp.err --user $(USER) -f $(PWD)/bin/init.php -s start

monitor: $(BIN_DIR)/monitor
	@echo $@
	@$(BIN_DIR)/monitor -P ssp | tee cpu.log

bench: all
	@echo $@
	@$(BIN_DIR)/ssp -f $(PWD)/bin/bench.php -s script 127.0.0.1 8086 8 50 60000

bench2: all
	@echo $@
	@$(BIN_DIR)/ssp --host 127.0.0.1 --port 8086 --nthreads 8 --max-clients 2000 --timeout 300 -f $(PWD)/bin/bench2.php -s bench

kill3:
	@echo $@
	@-$(BIN_DIR)/ssp --pidfile $(PWD)/ssp.pid --user $(USER) -s stop
	@-$(BIN_DIR)/ssp --pidfile $(PWD)/ssp2.pid --user $(USER) -s stop
	@-$(BIN_DIR)/ssp --pidfile $(PWD)/ssp3.pid --user $(USER) -s stop

retest3: kill3 all
	@echo $@
	@$(BIN_DIR)/ssp --port 8082 --nthreads 2 --max-clients 6000 --timeout 300 --pidfile $(PWD)/ssp.pid --outfile $(PWD)/ssp.out --errfile $(PWD)/ssp.err --user $(USER) -f $(PWD)/bin/conv.php -s start
	@$(BIN_DIR)/ssp --port 8084 --nthreads 2 --max-clients 6000 --timeout 300 --pidfile $(PWD)/ssp2.pid --outfile $(PWD)/ssp2.out --errfile $(PWD)/ssp2.err --user $(USER) -f $(PWD)/bin/conv.php -s start
	@$(BIN_DIR)/ssp --port 8086 --nthreads 2 --max-clients 6000 --timeout 300 --pidfile $(PWD)/ssp3.pid --outfile $(PWD)/ssp3.out --errfile $(PWD)/ssp3.err --user $(USER) -f $(PWD)/bin/conv.php -s start

bench3: all
	@echo $@
	@$(BIN_DIR)/ssp --nthreads 8 --max-clients 2000 --timeout 300 -f $(PWD)/bin/bench3.php -s bench

