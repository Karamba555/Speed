QL_CM_SRC=QmiWwanCM.c GobiNetCM.c main.c MPQMUX.c QMIThread.c util.c qmap_bridge_mode.c mbim-cm.c device.c myserial.c
QL_CM_DHCP=udhcpc.c

CFLAGS := -Wall -W -Werror -O3 -Wno-format-truncation -fPIE -fstack-protector-strong
LDFLAGS := -lpthread -ldl -Wl,-znoexecstack -pie

release: clean qmi-proxy
	$(CC) -Wall ${CFLAGS} -s ${QL_CM_SRC} ${QL_CM_DHCP} -o quectel-CM ${LDFLAGS} -luci

all: clean
	$(CC) -Wall ${CFLAGS} -g -DCM_DEBUG ${QL_CM_SRC} ${QL_CM_DHCP} -o quectel-CM ${LDFLAGS}

qmi-proxy:
	$(CC) -Wall ${CFLAGS} -s quectel-qmi-proxy.c myserial.c -o quectel-qmi-proxy ${LDFLAGS}

clean:
	rm -rf quectel-CM *~
	rm -rf quectel-qmi-proxy
