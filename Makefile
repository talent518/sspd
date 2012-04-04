#
# $Header$
#
# See http://phi.lv?p=240
#

CC=gcc

PHP_DIR = /usr
INC_DIR=$(PHP_DIR)/include/php
BIN_DIR=$(PWD)/bin

CFLAGS = -I$(INC_DIR) -I$(INC_DIR)/main -I$(INC_DIR)/Zend -I$(INC_DIR)/TSRM 
LFLAGS = -lstdc++ -L$(PHP_DIR)/lib -lphp5

all: .c.o ssp
	@test -d $(BIN_DIR) || mkdir $(BIN_DIR)
	@mv -t $(BIN_DIR) *.o ssp

.c.o:
	$(CC) $(CFLAGS) -c *.c

ssp: *.o
	$(CC) $(LFLAGS) *.o -o $@

clean:
	@rm -rf $(BIN_DIR)/*.o
	@rm -rf $(BIN_DIR)/ssp

#
# $Log$
#
