#include <stdio.h>
#include <stdarg.h>
#include "mgui_utils.h"

int __mgui_log_print(const char *fmt, ...)
{
	va_list ap;
	char buf[LOG_BUF_SIZE];
	int write_length = 0;

	va_start(ap, fmt);
	write_length = vsnprintf(&buf[write_length], LOG_BUF_SIZE, fmt, ap);
	va_end(ap);
	buf[write_length++] = '\n';
	buf[write_length++] = '\0';
	printf("%s", buf);

	return write_length;
}
