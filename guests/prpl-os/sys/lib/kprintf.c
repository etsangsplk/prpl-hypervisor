/**
 * @file kprintf.c
 * @author Sergio Johann Filho
 * @date February 2016
 * 
 * @section DESCRIPTION
 * 
 * Kernel printing / conversion auxiliary functions.
 * 
 */

#include <os.h>

static int8_t *kitoa(int32_t val, int32_t base)
{
	static int8_t buf[32] = {0};
	int32_t i = 30, n = 0;
	uint32_t val2;
	
	if (val == 0) return "0";
	if (base == 16){
		val2 = val;
		for(; val2 && i ; --i, val2 /= base)
			buf[i] = "0123456789abcdef"[val2 % base];
	}else{
		if (val < 0){
			n = 1;
			val = -val;
		}
		
		for(; val && i ; --i, val /= base)
			buf[i] = "0123456789abcdef"[val % base];
			
		if (n) buf[i--] = '-';
	}
	return &buf[i+1];
	
}

static int32_t kprint(const int8_t *fmt, va_list args)
{
	int8_t *s;
	int32_t i;

	while (*fmt){
		if (*fmt != '%'){
			putchar(*fmt++);
		}else{
			i = 0;
			switch (*++fmt){
				case 'd':
					s = kitoa(va_arg(args, int32_t), 10);
					while (s[i]) putchar(s[i++]);
					break;
				case 'x':
					s = kitoa(va_arg(args, int32_t), 16);
					while (s[i]) putchar(s[i++]);
					break;
				case 'c':
					putchar((int8_t)va_arg(args, int32_t));
					break;
				case 's':
					s = (int8_t *)va_arg(args, int32_t);
					if (!s) s = "(null)";
					while (*s) putchar(*s++);
					break;
				case '%':
					putchar('%');
					break;
				default:
					putchar('?');
					break;
			}
			fmt++;
		}
	}

	return 0;
}

/**
 * @internal
 * @brief Kernel short version of printf().
 * 
 * @param fmt is a pointer to formatted data to be print.
 * 
 * @return 0.
 */
int kprintf(const int8_t *fmt, ...)
{
        va_list args;
        
        va_start(args, fmt);
        return kprint(fmt, args);
}