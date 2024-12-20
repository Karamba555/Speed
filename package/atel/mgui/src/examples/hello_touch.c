#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/input.h>
#define TS_DEV "/dev/input/event3"
static int ts_fd = -1;

static int init_device()
{
        if((ts_fd = open(TS_DEV, O_RDONLY)) < 0)
        {
                printf("Error open %s\n", TS_DEV);
                return -1;
        }
        return ts_fd;
}
 
int main()
{
        struct input_event data;
        if(init_device() < 0)
                return -1;
        while(1)
        {
                read(ts_fd, &data, sizeof(data));
                printf("data.type = %d, data.code = %d, data.value = %d\n",data.type,data.code,data.value);
                if (data.type == EV_KEY)
                {
                        printf("type: EV_KEY, event = %s, value = %d\n\n",
                                data.code == BTN_TOUCH ? "BTN_TOUCH" : "Unkown", data.value);
                }
                else if(data.type == EV_ABS)
                {
                        printf("type: EV_ABS, event = %s, value = %d\n\n",
                                data.code == ABS_X ? "ABS_X" :
                                data.code == ABS_Y ? "ABS_Y" :
                                data.code == ABS_PRESSURE ? "ABS_PRESSURE" :
                                "Unkown", data.value);
                }
                else if (data.type == EV_SYN)
                {
                        printf("type: EV_SYN, event = %s, value = %d\n\n",
                                data.code == SYN_REPORT ? "SYN_REPORT" : "Unkown", data.value);
                }
                else
                {
                        printf("type: 0x%x, event = 0x%x, value = %d\n\n", data.type, data.code, data.value);
                }
        }
        return 0;
}
