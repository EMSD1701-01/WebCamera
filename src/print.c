#include "print.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/types.h>
#include <string.h>
#include <fcntl.h>
#include <wait.h>
#include <time.h>
#include <limits.h>
#include "huffman.h"

static int
is_huffman(unsigned char *buf)
{
  unsigned char *ptbuf;
  int i = 0;
  ptbuf = buf;
  while (((ptbuf[0] << 8) | ptbuf[1]) != 0xffda) {
    if (i++ > 2048)
      return 0;
    if (((ptbuf[0] << 8) | ptbuf[1]) == 0xffc4)
      return 1;
    ptbuf++;
  }
  return 0;
}

extern int
print_picture(int fd, unsigned char *buf, int size)
{
    unsigned char *ptdeb, *ptcur = buf;
    int sizein;


    if (!is_huffman(buf)) {
        ptdeb = ptcur = buf;
        while (((ptcur[0] << 8) | ptcur[1]) != 0xffc0)
            ptcur++;
        sizein = ptcur - ptdeb;
        if( write(fd, buf, sizein) <= 0) return -1;
        if( write(fd, dht_data, DHT_SIZE) <= 0) return -1;
        if( write(fd, ptcur, size - sizein) <= 0) return -1;
    } else {
        if( write(fd, ptcur, size) <= 0) return -1;
    }
    return 0;
}
