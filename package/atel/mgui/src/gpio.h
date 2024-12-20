#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_MAP(chip, index)  ((chip - 1) * 32 + (index))

#define GPIO_DIR_IN     0
#define GPIO_DIR_OUT    1

enum {
    GPIO_EDGE_NONE = 0,
    GPIO_EDGE_RISING,
    GPIO_EDGE_FALLING,
    GPIO_EDGE_BOTH,
};

int gpio_export(unsigned int gpio);
int gpio_unexport(unsigned int gpio);
int gpio_set_dir(unsigned int gpio, unsigned int out_flag);
int gpio_set_value(unsigned int gpio, unsigned int value);
int gpio_get_value(unsigned int gpio, unsigned int *value);
int gpio_set_edge(unsigned int gpio, const unsigned char *edge);
int gpio_value_open(unsigned int gpio);
int gpio_value_close(int fd);

#endif

