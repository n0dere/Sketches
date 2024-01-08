/***********************************************************************\
 * This header file defines the Serial structure and the functions for *
 * serial communication on Unix systems.                               *
\***********************************************************************/

#ifndef _UNIX_SERIAL_H
#define _UNIX_SERIAL_H

#include <stdint.h>
#include <stdlib.h>
#include <termios.h>

typedef struct _Serial Serial;

struct _Serial {
    int fd;             /* The file descriptor of the serial port. */
    speed_t rate;       /* The baud rate of the serial port. */
};

Serial *serialOpenPort(const char *pPort, speed_t rate);

void serialWrite(Serial *pSerial, const uint8_t *pBytes, size_t count);

size_t serialRead(Serial *pSerial, uint8_t *pBuffer, size_t count);

void serialClose(Serial *pSerial);

#endif /* _UNIX_SERIAL_H */