#include    <stdio.h>
#include    <stdlib.h>
#include    <fcntl.h>
#include    <string.h>

#define MOTOR_DEV	"/dev/stmotor"

int motor_fd = 0;

int main_menu(void){
	int key;

	printf("\n\n");
	printf("********** MOTOR menu **********\n");
	printf("* 1.     MOTOR Forward         *\n");
	printf("* 2.     MOTOR Reverse         *\n");
	printf("*                              *\n");
	printf("*                              *\n");
	printf("* 0.     Exit Program          *\n");
	printf("********************************\n");
	printf("\n\n");

	printf("select the command number : ");
	scanf("%d",&key);
	return key;
}

void control(int mode, int val){
        ioctl(motor_fd, mode, &val);
}

int MOTORTest(int mode, int val){
	
	motor_fd = open(MOTOR_DEV, O_RDWR|O_NDELAY);
        if(motor_fd <0)
                printf("Cannot Open Device\n");

        control(mode, val);

        close(motor_fd);
}

void main(int ac, char *av[]){
	int key;

	while((key = main_menu()) != 0){
		switch(key){
			case 1: printf("\t MOTOR Forward \n");
					MOTORTest(1, 250);
				   break;
			case 2: printf("\t MOTOR Reverse \n");
					MOTORTest(3, 250);
				   break;
			default:  printf("\t Unknown command... \n");
					  break;
        	}
	}
}      

/* EOF */
