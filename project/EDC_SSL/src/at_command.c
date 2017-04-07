/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2013-? PAX Technology, Inc. All rights reserved.
 * Description:
 * Revision History:
 * Date                  Author                 Action
 * 2013-02-21 09:39:27   liuchuan                create
 * ============================================================================
 */

#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>

static int fp_uart = -1;

extern int SetOpt(int fd, int nSpeed, int nBits, char nEvent, int nStop);


static int parse_attr(const char *attr, int *speed, int *databits,
    int *parity, int *stopbits)
{
    int i;
    char buf[50];
    const long baud_tab[] = { 600, 1200, 2400, 4800, 9600, 14400, 19200,
        28800, 38400, 57600, 115200, 230400, };
    char key[] = " ,.\r\n";
    char *str;

    if (strlen(attr) > 49)
        return -1;
    strcpy(buf, attr);
    /* attr = buf; */
    str = strtok(buf, key);
    if (str == NULL)
        return -1;
    *speed = atoi(str);
    for (i = 0; i < (int)(sizeof(baud_tab) / sizeof(long)); i++) {
        if (baud_tab[i] == *speed)
            break;
    }
    if (i == sizeof(baud_tab) / sizeof(long))
        return -1;

    str = strtok(NULL, key);
    if (str == NULL)
        return -1;
    *databits = atoi(str);
    if (*databits < 5 || *databits > 8)
        return -1;

    str = strtok(NULL, key);
    if (str == NULL || strlen(str) > 1)
        return -1;
    if (str[0] == 'e' || str[0] == 'n' || str[0] == 'o') {
        str[0] = toupper(str[0]);
    }
    if (str[0] != 'E' && str[0] != 'N' && str[0] != 'O')
        return -1;
    *parity = str[0];

    str = strtok(NULL, key);
    if (str == NULL || strlen(str) > 1)
        return -1;
    if (str[0] != '1' && str[0] != '2')
        return -1;
    *stopbits = atoi(str);
    return 0;
}

static int time_compare(struct timeval tv1, struct timeval tv2)
{
    if (tv1.tv_sec > tv2.tv_sec) {
        return 1;
    }
    else if (tv1.tv_sec < tv2.tv_sec) {
        return -1;
    }
    else if (tv1.tv_usec > tv2.tv_usec) {
        return 1;
    }
    else if (tv1.tv_usec < tv2.tv_usec) {
        return -1;
    }
    else {
        return 0;
    }
}

static void time_add_ms(struct timeval *tv, int ms)
{
    tv->tv_sec += ms / 1000;
    tv->tv_usec += ((ms % 1000) * 1000);
}

int WlPortOpen(const char *Attr)
{
    int speed, databits, stopbits, parity;
    const char *com =  "/dev/mux1";

    if (Attr == NULL)
        return -1;

    if (parse_attr(Attr, &speed, &databits, &parity, &stopbits) < 0)
        return -1;

    if(fp_uart > 0)
        close(fp_uart);

//    if(0 == OsWlCheck())
        fp_uart = open("/dev/mux1", O_RDWR | O_NOCTTY);
//    else
//        fp_uart = open("/dev/ttyAMA1", O_RDWR | O_NOCTTY);
    if(fp_uart < 0){
        printf("Open %s failed, errno: %d\n", com, errno);
        return -1;
    }
    printf("Open %s\n", com);

    if (SetOpt(fp_uart, speed, databits, parity, stopbits) < 0)
        return -1;
    printf("Open succeeded, returned: %d\n", fp_uart);

    tcflush(fp_uart, TCIOFLUSH);

    return 0;
}

void WlPortClose()
{
    if (fp_uart < 0)
        return;
    close(fp_uart);
    fp_uart = -1;
}

void WlPortReset()
{
    tcflush(fp_uart, TCIOFLUSH);
}

int WlPortSend(const void *SendBuf, int SendLen)
{
    int ret = 0;
    const char *buf = SendBuf;

    /*When you don't set open(..O_NDELAY) or fcntl(fd, F_SETFL, FNDELAY),
    The while() and usleep() isn't needed.*/
    while (SendLen > 0) {
        ret = write(fp_uart, buf, SendLen);
        if (ret < 0) {
            break;
        }
        buf += ret;
        SendLen -= ret;
        usleep(10 * 1000);
    }
    return (ret >= 0) ? 0 : -1;
}

static int port_err(int err)
{
    if (err == EFAULT)
        return -1013;
    return -1014;
}

int WlPortRecv(void *RecvBuf, int RecvLen, int TimeoutMs)
{
    char *buf = RecvBuf;
    int ret, err, total;
    struct timeval tv1, tv2;

    /* Flush the input and output pools. */
    if (buf == NULL && RecvLen == 0) {
        WlPortReset();
        return 0;
    }

    if (buf == NULL || RecvLen < 0 || TimeoutMs < 0)
        return -1;

    if (!RecvLen)
        return 0;

    if (TimeoutMs < 100 && TimeoutMs > 0) {
        TimeoutMs = 100;
    }

    if (gettimeofday(&tv1, NULL) < 0) {
        return -3209;
    }
    time_add_ms(&tv1, TimeoutMs);

    total = 0;
    while (RecvLen > 0) {
        ret = read(fp_uart, buf, RecvLen);
        if (ret < 0) {
            err = errno;
            if (err != EAGAIN && err != EINTR)
                return port_err(err);
        }
        else {
            buf += ret;
            RecvLen -= ret;
            total += ret;
            if (RecvLen <= 0)
                break;
            if (gettimeofday(&tv2, NULL) < 0) {
                return -3209;
            }
            if (time_compare(tv1, tv2) <= 0) {
                break;
            }
        }
    }
    return total;
}

int WlPortCheckTx()
{
    int remain, ret;

    ret = ioctl(fp_uart, TIOCOUTQ, &remain);
    if (ret < 0)
        return -1;
    return remain;
}

