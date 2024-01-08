#include "unix_serial.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>

int setInterfaceAttribs(int fd, speed_t speed)
{
    struct termios tty;

    if (tcgetattr (fd, &tty) != 0)
        return -1;

    cfmakeraw(&tty);

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    /* ignore modem controls, enable reading */
    tty.c_cflag |= (CLOCAL | CREAD); 
    tty.c_cflag &= ~(PARENB | PARODD); /* shut off parity */
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
        return -1;

    tcflush(fd, TCIOFLUSH);

    return 0;
}

Serial *serialOpenPort(const char *pPort, speed_t rate)
{
    Serial *pSerial = calloc(1, sizeof *pSerial);

    if (pSerial == NULL)
        return NULL;
    
    pSerial->fd = open(pPort, O_RDWR | O_NOCTTY | O_SYNC);
    pSerial->rate = rate;

    if (pSerial->fd == -1) {
        free(pSerial);
        return NULL;
    }

    if (setInterfaceAttribs(pSerial->fd, rate) == -1) {
        close(pSerial->fd);
        free(pSerial);
        return NULL;
    }

    return pSerial;

error:
    free(pSerial);
    return NULL;
}

void serialWrite(Serial *pSerial, const uint8_t *pBytes, size_t count)
{
    if (pSerial != NULL && pBytes != NULL && count > 0)
        write(pSerial->fd, pBytes, count);
}

size_t serialRead(Serial *pSerial, uint8_t *pBuffer, size_t count)
{
    fd_set read_fds;
    struct timeval timeout = { 1, 0 }; /* 1 sec */
    int ret;

    if (pSerial == NULL || pBuffer == NULL || count == 0)
        return 0;

    /* initialize the file descriptor set */
    FD_ZERO(&read_fds);
    FD_SET(pSerial->fd, &read_fds);

    /* wait for data to be available */
    ret = select(pSerial->fd + 1, &read_fds, NULL, NULL, &timeout);

    if (ret > 0)
        return read(pSerial->fd, pBuffer, count);
    else
        return -1;
}

void serialClose(Serial *pSerial)
{
    if (pSerial == NULL)
        return;
    
    close(pSerial->fd);
    free(pSerial);
}