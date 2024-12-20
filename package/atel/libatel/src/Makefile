#CFLAGS+=-I$(TOPDIR)/lib/include #valid
include $(TOPDIR)/.config
LIBS+=-lcrypto -lpthread -lnvram

LIBATEL=libatel.a
LIBATEL_SHARED=libatel.so

CFLAGS+=-DUSE_HOSTCC
LDFLAGS+= -shared -fPIC

# all:	$(LIBATEL) $(LIBATEL_SHARED) 

$(LIBATEL_SHARED): $(OBJSC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^

$(LIBATEL) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBATEL) $(OBJS)

clean:
	$(RM) $(LIBATEL_SHARED)*

