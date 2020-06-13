#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#define clcd "/dev/clcd"

void clcd_input(char clcd_text[]);

int main() {
	clcd_input("press any key to start game");
	
	return 0;
}


void clcd_input(char clcd_text[]){
	int clcd_d;
	clcd_d = open(clcd , O_RDWR);

	if (clcd_d < 0) {
		printf("디바이스 드라이버가 없습니다.\n");
	}
	
	write(clcd_d , clcd_text , strlen(clcd_text)); // 1.디스크립터  2.문자들 3.문자들의 크기
	close(clcd_d);
}
