EXEC = mtk-smc-tool
SRC = mtk_smc_tool.c

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC)

all: $(EXEC)

clean:
	rm -f $(EXEC) *.o
