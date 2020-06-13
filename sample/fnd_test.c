#include	<stdio.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<string.h>

#define fnd_dev	"/dev/fnd"

int FNDTest(char a, char b, char c, char d);

int FNDTest(char a, char b, char c, char d){

	unsigned char FND_DATA_TBL[]={
        	0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x88,
        	0x83,0xC6,0xA1,0x86,0x8E,0xC0,0xF9,0xA4,0xB0,0x99,0x89
	};

	int fnd_fd = 0;

        unsigned char fnd_num[4];

        fnd_num[0] = FND_DATA_TBL[a];
        fnd_num[1] = FND_DATA_TBL[b];
        fnd_num[2] = FND_DATA_TBL[c];
        fnd_num[3] = FND_DATA_TBL[d];

        fnd_fd = open(fnd_dev, O_RDWR);

        if(fnd_fd <0){
		printf("Can't Open Device\n");
	}
        write(fnd_fd, &fnd_num, sizeof(fnd_num));
}

void main()
{

	int i;

	for(i=0; i<16; i++){
		FNDTest(i,i,i,i);
		sleep(1);
	}		
}
/*	EOF	*/
