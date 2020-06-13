#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <string.h>
#include <asm/ioctls.h>

static char dipswDev[] = "/dev/dipsw";
static int  dipswFd = (-1);

unsigned char dipsw_get(int tmo)
{
    unsigned char b;

        if (tmo) {
                if (tmo < 0)
                        tmo = ~tmo * 1000;
                else
                        tmo *= 1000000;

                while (tmo > 0) {
                        usleep(10000);
                        read(dipswFd, &b, sizeof(b));
                        if (b) return(b);
                                tmo -= 10000;
                }
                return(-1);
        }
        else {

                read(dipswFd, &b, sizeof(b));
                return(b);
        }
}
int main()
{
    unsigned char c;

    if((dipswFd = open( dipswDev, O_RDONLY )) < 0){         // KEY open
		perror("open faile /dev/dipsw");
		exit(-1);
	}

	while(1){
		c = dipsw_get(10);
		printf("DIP --> %d\n", c);
        }
	return 0;
}

/* EOF */
