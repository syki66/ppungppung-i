#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <string.h>
#include <asm/ioctls.h>
#include <stdbool.h>
#include<string.h>
#include <time.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/stat.h>

#define clcd "/dev/clcd"
#define led "/dev/led"
#define dot "/dev/dot"
#define fnd_dev	"/dev/fnd"

static char tactswDev[] = "/dev/tactsw";
static int  tactswFd = (-1);

int led_count = 0; // 베팅액을 확인하기 위한 전역변수
int user_money[4] = { 3, 0, 0, 0 };

bool isEnd = false;

// 아마도 택트 스위치 클릭값 얻기 위한것
unsigned char tactsw_get(int tmo) 
{   
    unsigned char b;
	if (tmo) { 
        	if (tmo < 0)
            		tmo = ~tmo * 1000;
        	else
            		tmo *= 1000000;

        	while (tmo > 0) {
            		usleep(10000);
            		read(tactswFd, &b, sizeof(b));
	       		if (b) return(b);
            			tmo -= 10000;
        	}
	        return(-1); 
    	}
    	else {
      		read(tactswFd, &b, sizeof(b));
        	return(b);
    	}
}

// 호출하면 택트 스위치값 반환해주는 함수
int tact_switch_listener(){
	unsigned char c;
	int selected_tact = 0; // false 값 넣기

	if((tactswFd = open( tactswDev, O_RDONLY )) < 0){     	// 예외처리    
		perror("open faile /dev/key");
		exit(-1);
	}

	// 택트스위치는 1,2,3,4,5번 스위치만 사용함.
	// 1,2,3 은 각각 묵, 찌, 빠 이며, 4번은 베팅금액 증가시키는버튼, 5번은 베팅금액을 확정시키는 버튼.
	while(1){
		c = tactsw_get(10);
		switch (c) {
				case 1:  selected_tact = 1 ; break;
				case 2:  selected_tact = 2 ; break;
				case 3:  selected_tact = 3 ; break;
				case 4:  selected_tact = 4 ; break;
				case 5:  selected_tact = 5 ; break;
				default: printf("press other key\n", c); break;
		}
		return selected_tact; // 어떤 스위치가 눌렸는지 int 형으로 반환함
		}
}

// LCD에 문자열을 표시해주는 함수
void clcd_input(char clcd_text[]){
	int clcd_d;
	clcd_d = open(clcd , O_RDWR);

	if (clcd_d < 0) {	// 예외처리
		printf("clcd error\n");
	}

	write(clcd_d , clcd_text , strlen(clcd_text)); // 두번째부터 각각 문자열, 문자열 크기
	close(clcd_d); 
}

// led 제어하는 함수
void led_control(){
	int dev;
	unsigned char data;
	char led_array[] = { 0xFE,0xFC,0xF8,0xF0,0xE0,0xC0,0x80,0x00 }; // 미리 16진수값들 지정해놓음
	
	dev = open(led,O_RDWR);

	if (dev <0) {		// 예외처리
		printf("led error\n");
		exit(0);
	}

	led_count %= 8; // led 카운트가 8회 이상 넘어가게 되면 다시 0부터 카운트
	data = led_array[led_count]; // 이진수로 제어됨

	write(dev , &data , sizeof(unsigned char)); // 출력
	usleep(500000); // led 0.5초동안 꺼지는거 방지
	
	close(dev);
	
	led_count++; // 함수가 실행될때마다 카운트가 올라감
}

// "행" 값과 "sleep"값을 인자로 받아서 도트 매트릭스 제어하는 함수
void DOT_control(int col, int time_sleep){
	int dot_d;
	unsigned char c[3][8] ={
		{ 0xff,0x81,0xff,0x00,0xff,0x18,0xff,0x01 }, // 가위
		{ 0xfd,0x49,0x49,0x49,0xb5,0xb5,0xb5,0xb5 }, // 바위
		{ 0xaa,0xaa,0xaa,0xfb,0xab,0xaa,0xaa,0xfa }  // 보
	};

	dot_d = open(dot , O_RDWR);
	if(dot_d<0)  	// 예외처리
	{
		printf("Error\n");
	}

	write(dot_d , &c[col], sizeof(c)); // 출력
	
	sleep(time_sleep); // 대기시간이 1초보다 작으면 에러남
	close(dot_d); // 필수.
}

// 가위바위보 함수
int rockScissorsPaper(int com_rsp, int user_rsp) {
	int state = 6; // 승(1),패(0),무(-1) 의 여부를 알려주는 변수. 아래 값들과 겹치지 않는 값으로 초기화
	
	if (com_rsp == user_rsp) {
		clcd_input("Draw, Do it again"); // 비겼으니 다시 가위바위보 하라는 메세지 출력
		state = 0;
	}
	
	else if ( ( (com_rsp == 1) && (user_rsp == 3) ) ||  ( (com_rsp == 2) && (user_rsp == 1) ) || ( (com_rsp == 3) && (user_rsp == 2) ) ) {
		clcd_input("You Win!!"); // 승리 문구 출력
		state = 1;
	}
	else if ( ( (com_rsp == 3) && (user_rsp == 1) ) ||  ( (com_rsp == 1) && (user_rsp == 2) ) || ( (com_rsp == 2) && (user_rsp == 3) ) ) {
		clcd_input("You Lose^^"); // 패배 문구 출력
		state = -1;
	}
	else {
		clcd_input("use key 1 or 2 or 3");
	}
	return state; // 경기 결과 리턴
}

// 게임 처음 실행할때 인트로 화면 설정
void intro(){
	
	//묵찌빠 글자 순서대로 출력
	DOT_control(0, 1); // 0번 행을 1초동안 출력
	DOT_control(1, 1);
	DOT_control(2, 1);
}





void calculate_money(rsp_state){
	if (rsp_state == 1) {
		user_money[1] += led_count;
	}
	else if (rsp_state == -1) {
		user_money[1] -= led_count;
	}
	else {
		printf("unknown Error");
	}
}

void adjust_balance(int money[]){
	if (money[0] <= 0 && money[1] <= 0) {
		isEnd = true;
	}
	else if (money[0] >= 10) {
		isEnd = true;
	}
	else if (money[1] >= 10) { 
		money[0]++;
		money[1] %= 10;
	}
	else if (money[1] < 0) {
		money[0]--;
		money[1] = (10 + money[1]);
	}
	else{ 
		//nothing
	}
}

// fnd control
int FND_control(int money[], int time_sleep){
	adjust_balance(money);
	unsigned char FND_DATA_TBL[]={
        	0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x88,
        	0x83,0xC6,0xA1,0x86,0x8E,0xC0,0xF9,0xA4,0xB0,0x99,0x89
	};

	int fnd_fd = 0;

        unsigned char fnd_num[4];

        fnd_num[0] = FND_DATA_TBL[money[0]];
        fnd_num[1] = FND_DATA_TBL[money[1]];
        fnd_num[2] = FND_DATA_TBL[money[2]];
        fnd_num[3] = FND_DATA_TBL[money[3]];

        fnd_fd = open(fnd_dev, O_RDWR);

        if(fnd_fd <0){
		printf("Can't Open Device\n");
	}
        write(fnd_fd, &fnd_num, sizeof(fnd_num));
        sleep(time_sleep);
        close(fnd_fd);
}


int main() {
	while(!isEnd){

		// 시작부
		while(true){
			clcd_input("press any key to start game"); // LCD에 메세지 출력
			intro(); // 인트로 함수 실행
			
			if (tact_switch_listener()){	// 아무키나 누르게되면 while문 탈출
				break;
			}
		}
		
		clcd_input("  Your Balance");
		FND_control(user_money,3); // sleep
		
		// 베팅금액 설정하는 부분
		clcd_input("    Betting");
		while(true){
			if (tact_switch_listener() == 4){ 	// 4번 스위치를 클릭하였을 경우 led 증가시키기
				led_control();
			}
			else if (tact_switch_listener() == 5){	// 5번 스위치 클릭시 while문 빠져나옴
				break;
			}
			else {  // 만약 플레이어가 4번 ,5번을 제외한 나머지 키를 눌렀을경우 아래 메세지 출력
				clcd_input("use right key, 4:money,5:confirm");
			}
	    }
		
		// 가위바위보 하는 부분
		int user_input = 0; // 사용자가 누른 택트 스위치 확인하는 변수 초기화
		int rsp_state = 0;	// 비겼을때 재경기를 하기 위해 while문 조건에 삽입.
		while(rsp_state == 0){ // 비기지 않을때까지 실행
			clcd_input(" Rock  Scissors     Paper!!");
			
			srand(time(NULL)); // 시드값에 시간함수를 넣어주어 매크로 랜덤이 아닌 완전한 램덤시드생성
			int random = rand() % 3 + 1;	// 0,1,2 랜덤하게 생성후 변수에 대입
			
			if (tact_switch_listener() == 1 || tact_switch_listener() == 2 || tact_switch_listener() == 3) { //1,2,3 번중 하나의 택트스위치를 눌렀을때 분기문으로 들어감
				user_input = tact_switch_listener(); // 사용자가 누른 값을 변수에 대입
				rsp_state = rockScissorsPaper(random, user_input); // 사용자의 값과 위에서 랜덤하게 생성된 수를 가위바위보 시킨 후, 결과를 rsp_state에 저장
				DOT_control(random - 1, 3); // 컴퓨터는 무엇을 내었는지 3초동안 보여줌
			}
			else {
				clcd_input("use right key,1:muk,2:zzi,3:ppa"); // 사용자가 1,2,3 번 택트 스위치를 누르지 않으면 조작법을 lcd에 출력해줌
			}
		}
	calculate_money(rsp_state);
	FND_control(user_money,3);
	
	led_count = 0; // 현 게임의 베팅금액이 다음게임에 이어지지 않도록 베팅액 초기화
	}
		
	return 0;
}