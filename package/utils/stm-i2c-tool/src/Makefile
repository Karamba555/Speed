EXEC = stm-i2c-tool
SRC = stm_i2c_main.c _stm_i2cnl_ll.c _stm_i2cnl_cmds.c _stm_i2cnl_upgrade.c

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC)

all: $(EXEC)

clean:
	rm -f $(EXEC) *.o
