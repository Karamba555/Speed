
include $(TOPDIR)/.config


LIB_STATIC=libusercrypto.a
LIB_SHARED=libusercrypto.so


LDFLAGS+= -shared -fPIC



OBJS	=  aes.o rsa.o rwsig.o sha256.o rsa_verify.o
OBJSC	=  aes.c rsa.c rwsig.c sha256.c rsa_verify.c



#all:	$(LIB_STATIC) $(LIB_SHARED) 

$(LIB_SHARED): $(OBJSC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^

$(LIB_STATIC) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIB_STATIC) $(OBJS)

clean:
	$(RM) $(LIB_SHARED)*

