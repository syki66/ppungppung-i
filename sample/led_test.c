#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#define led_dev	"/dev/led"

#define UP_	1
#define DOWN_	2
#define ALL_ 	3
#define EXIT_	0

static int key;

int print_menu(void){

	int key;
	
	printf("\n");
	printf("*********** Choice Menu **********\n");
	printf("* 1.     LED UP    Shift         *\n");
	printf("* 2.     LED DOWN  Shift         *\n");
	printf("* 3.     ALL                     *\n");
	printf("* 0.           EXIT              *\n");
	printf("**********************************\n");
	printf("\n");
	
	printf("Select th menu number :");
	scanf("%d", &key);
	return key;
}
	
void led_down_shift(int *dev){
		
	unsigned char data, data2;
	int led_device, count;

        for ( count = 0; count <16 ; count++){       
		data2 = (~(data >> 7)) & 0x01;
                data  = (data << 1 ) | data2;
		usleep(50000);
		write (*dev, &data, sizeof(unsigned char));
        }
}

void led_up_shift(int *dev){

	unsigned char data, data2;
       	int led_device, count;

	for( count =0 ; count < 16 ; count++) {
                data2 = (~(data << 7)) & 0x80;
                data  = (data >> 1 ) | data2;
		usleep(50000);
		write (*dev, &data, sizeof(unsigned char));
        }
}

void led_all( int *dev){
	unsigned char data;
	int led_device, count;

	for( count =0 ; count <16; count ++) {
		if( count%2){
			data = 0xff;
			write(*dev, &data, sizeof(unsigned char));
		}
		else{ 	
			data = 0x00;
			write(*dev, &data, sizeof(unsigned char));
		}
		usleep(100000);
	}
}


int main(){

	int dev;
	
	dev = open(led_dev, O_RDWR);
	if (dev < 0){
		fprintf(stderr, "cannot open LED Device (%d)", dev);
		exit(2);
	}
	led_up_shift(&dev);
	led_all(&dev);
	led_down_shift(&dev);	

	while(( key = print_menu()) != 0){
		if(key == UP_)	led_up_shift( &dev);
		else if (key== DOWN_)	led_down_shift(&dev);
		else if (key == ALL_)	led_all(&dev);
		else if (key == EXIT_)	{printf("\n\t\t\t EXIT Program \n"); break; return 0;}
		else { printf(" unknow command ~ ^^\n"); break; }
	}//end while
	close(dev);
	return 0;
}


