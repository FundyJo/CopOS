#ifndef MYOS_SERIAL_H
#define MYOS_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

void serial_init(void);
void serial_write_char(char c);
void serial_write(const char *s);

#ifdef __cplusplus
}
#endif

#endif
