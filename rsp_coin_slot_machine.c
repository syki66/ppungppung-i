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
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#define clcd "/dev/clcd"
#define led "/dev/led"
#define dot "/dev/dot"
#define fnd_dev	"/dev/fnd"

static char tactswDev[] = "/dev/tactsw";
static int  tactswFd = (-1);

int led_count = 0; // 베팅액을 확인하기 위한 전역변수
int user_money[4] = { 3, 0, 0, 0 }; // 초기 지급 금액, 3000원
bool isEnd = false; // 경기가 끝났는지 알려주는 전역변수

unsigned char rps[3][8] ={	// 가위바위보 배열
	{ 0xff,0x81,0xff,0x00,0xff,0x18,0xff,0x01 }, // 가위
	{ 0xfd,0x49,0x49,0x49,0xb5,0xb5,0xb5,0xb5 }, // 바위
	{ 0xaa,0xaa,0xaa,0xfb,0xab,0xaa,0xaa,0xfa }  // 보
};

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
		perror("tact error");
		exit(-1);
	}

	// 택트스위치는 1,2,3,4,5,7,8번 스위치만 사용함
	while(1){
		c = tactsw_get(10);
		switch (c) {
				case 1:  selected_tact = 1 ; break; // 묵
				case 2:  selected_tact = 2 ; break; // 찌
				case 3:  selected_tact = 3 ; break; // 빠
				case 4:  selected_tact = 4 ; break; // 배팅금액 증가
				case 5:  selected_tact = 5 ; break; // 배팅금액 확정
				case 7:  selected_tact = 7 ; break; // 게임 계속하기
				case 8:  selected_tact = 8 ; break; // 게임 나가기
				default: printf("press other key\n", c); break; // 기본값 메세지
		}
		return selected_tact; // 어떤 스위치가 눌렸는지 int 형으로 반환함
		}
}

// LCD에 문자열을 표시해주는 함수
void clcd_input(char clcd_text[]){
	int clcd_d;

	clcd_d = open(clcd , O_RDWR);
	if (clcd_d < 0) { printf("clcd error\n"); }// 예외처리

	write(clcd_d , clcd_text , strlen(clcd_text)); // 두번째부터 각각 문자열, 문자열 크기
	close(clcd_d); 
}

// led 제어하는 함수
void led_control(){
	int dev;
	unsigned char data;
	char led_array[] = { 0xFE,0xFC,0xF8,0xF0,0xE0,0xC0,0x80,0x00 }; // 미리 16진수값들 지정해놓음
	
	dev = open(led,O_RDWR);
	if (dev <0) { printf("led error\n"); exit(0); } // 예외처리

	led_count %= 8; // led 카운트가 8회 이상 넘어가게 되면 다시 0부터 카운트
	
	data = led_array[led_count];
	write(dev , &data , sizeof(unsigned char)); // 출력
	usleep(500000); // led가 0.5초동안 점등되도록 하기
	
	close(dev);
	
	led_count++; // 함수 실행 횟수 카운트
}

// 가위바위보 배열의 "행" 값과 "sleep" 값을 인자로 받아서 도트 매트릭스를 제어하는 함수
void DOT_control(int rps_col, int time_sleep){
	int dot_d;

	dot_d = open(dot , O_RDWR);
	if (dot_d < 0) { printf("dot Error\n"); } // 예외처리

	write(dot_d , &rps[rps_col], sizeof(rps)); // 출력
	sleep(time_sleep); // 몇초동안 점등할지

	close(dot_d);
}

// 가위바위보 함수 (컴퓨터와 유저의 가위가위보 값 비교해서 경기 결과 리턴)
int rockPaperScissors(int com_rps, int user_rps) {
	int state = 6; // 승(1), 무(0), 패(-1) 의 여부를 알려주는 변수. 아래 값들과 겹치지 않는 값으로 초기화
	
	if (com_rps == user_rps) {
		clcd_input("     Draw..       Do it again!  "); // 비겼을때의 메세지 출력
		state = 0;
	}
	else if ( ( (com_rps == 1) && (user_rps == 3) ) ||  ( (com_rps == 2) && (user_rps == 1) ) || ( (com_rps == 3) && (user_rps == 2) ) ) {
		clcd_input("   You Win!!!   "); // 승리 문구 출력
		state = 1;
	}
	else if ( ( (com_rps == 3) && (user_rps == 1) ) ||  ( (com_rps == 1) && (user_rps == 2) ) || ( (com_rps == 2) && (user_rps == 3) ) ) {
		clcd_input("   You Lose^^   "); // 패배 문구 출력
		state = -1;
	}
	else {
		clcd_input("use key 1 or 2 or 3"); // 키를 잘못 입력했을 경우 메세지 출력
	}
	return state; // 경기 결과 리턴
}

// 게임 인트로 띄어주는 함수
void intro(){
	int dot_d;
	int dev;
	unsigned char data;
	
	int rps_length = sizeof(rps) / sizeof(rps[0]); // 배열의 길이
	int intro_led_count, intro_dot_count = 0;
	while( intro_dot_count < rps_length ) { // 묵찌빠 배열의 길이만큼 실행
		dot_d = open(dot , O_RDWR);
		dev = open(led,O_RDWR);

		if(dot_d < 0 || dev < 0) { printf("Dot or Led error\n"); exit(0); } // 예외처리

		// dot 제어
		write(dot_d , &rps[intro_dot_count], sizeof(rps)); // 도트 매트릭스 출력
		usleep(50000); // 도트매트릭스 0.05초 점등
		close(dot_d);
		
		// led 제어
		data = rand() % 256; // 0 ~ 255 중 랜덤값 부여 (led는 16진수로 인식함)
		write(dev , &data , sizeof(unsigned char)); // LED 출력
		usleep(50000); // LED 0.05초 점등
		close(dev);
		
		// LED가 20번 바뀔때마다 도트 1번 바뀜	
		intro_led_count++;
		if (intro_led_count % 20 == 0) {
			intro_dot_count++;
		}
	}
}

// 승패 여부에 따라서 잔고의 100의 자리수를 조절해주는 함수
void calculate_user_money(rps_state){
	if (rps_state == 1) {	// 승리했을경우 배팅금액 더해주기
		user_money[1] += led_count;
	}
	else if (rps_state == -1) {	// 패배했을경우 배팅금액 빼주기
		user_money[1] -= led_count;
	}
	else { printf("unknown Error");	}
}

// fnd에 한 자리수의 양수로 출력할 수 있도록, user_money 배열 조정해주는 함수
void adjust_user_money(int money[]){
	if (money[1] >= 10) {  // 100의 자리수가 10보다 커질때
		money[0]++; // 1000의 자리수 올려주기
		money[1] %= 10; // 나머지값을 100의 자릿수에 대입
	}
	else if (money[1] < 0) { // 100의 자리수가 0보다 작아질때
		money[0]--; // 1000의 자릿수 내려주기
		money[1] = (10 + money[1]); // 100의 자릿수 조정
	}
	else{ /*pass*/ }
}

// 세그먼트 제어 함수
int FND_control(int money[], int time_sleep){
	unsigned char FND_DATA_TBL[]={
        	0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x88,
        	0x83,0xC6,0xA1,0x86,0x8E,0xC0,0xF9,0xA4,0xB0,0x99,0x89
	};

	int fnd_fd = 0;
    unsigned char fnd_num[4];

	// money 배열의 원소들을 순서에 맞게 넣어주기
    fnd_num[0] = FND_DATA_TBL[money[0]];
    fnd_num[1] = FND_DATA_TBL[money[1]];
    fnd_num[2] = FND_DATA_TBL[money[2]];
    fnd_num[3] = FND_DATA_TBL[money[3]];

    fnd_fd = open(fnd_dev, O_RDWR);
	if(fnd_fd <0){ printf("fnd error\n"); } // 예외처리

    write(fnd_fd, &fnd_num, sizeof(fnd_num)); // 출력
    sleep(time_sleep); // 점등시간 조절

    close(fnd_fd);
}


int main() {
	// 시작부
	while(true){
		clcd_input(" press any key   to start game  "); // LCD에 메세지 출력
		intro(); // 인트로 함수 실행
		
		if (tact_switch_listener()){	// 아무키나 누르게되면 while문 탈출
			break;
		}
	}
	
	clcd_input("  Your Balance");
	FND_control(user_money,3); // 플레이어의 잔고를 3초동안 fnd에 출력해주기
	
	// 반복부
	while(!isEnd){	
		// 베팅금액 설정
		clcd_input("    Betting     4.bet  5.confirm");
		while(true){
			if (tact_switch_listener() == 4){ 	// 4번 스위치를 클릭하였을 경우 led 증가시키기
				led_control();
			}
			else if (tact_switch_listener() == 5){	// 5번 스위치 클릭시 while문 빠져나옴
				break;
			}
			else {  // 만약 사용자가 잘못된 키를 입력했을 경우
				clcd_input("use right key,  4:bet, 5:confirm");
			}
	    }
		
		// 가위바위보 하는 부분
		int user_input = 0; // 사용자가 누른 택트 스위치 확인하는 변수 초기화
		int rps_state = 0;	// 비겼을때 재경기를 하기 위해 while문 조건에 삽입.
		while(rps_state == 0){ // 비기지 않을때까지 실행
			clcd_input(" Rock  Scissors     Paper!!");
			
			srand(time(NULL)); // 시드값에 시간함수를 넣어주어 매크로 랜덤이 아닌 완전한 램덤시드생성
			int random = rand() % 3 + 1;	// 0 ~ 2 랜덤값을 변수에 대입
			
			if (tact_switch_listener() == 1 || tact_switch_listener() == 2 || tact_switch_listener() == 3) { // 1,2,3 번중 하나의 택트스위치를 눌렀을때 분기문으로 들어감
				user_input = tact_switch_listener(); // 사용자가 누른 값을 변수에 대입
				rps_state = rockPaperScissors(random, user_input); // 사용자의 값과 위에서 랜덤하게 생성된 수를 가위바위보 시킨 후, 결과를 rps_state에 저장
				DOT_control(random - 1, 3); // 컴퓨터는 무엇을 내었는지 3초동안 보여줌
			}
			else {
				clcd_input("use right key,1:muk,2:zzi,3:ppa"); // 사용자가 스위치를 잘못누르면 조작법을 lcd에 출력해줌
			}
		}
		
		calculate_user_money(rps_state); // 승패 여부에 따라 돈 계산하기
		adjust_user_money(user_money); // money 배열에서 10 이상 또는 음수값을 알맞게 조정

		// 게임을 계속할수 있는 상태인지 확인
		if ( (user_money[0] < 0) || (user_money[0] == 0 && user_money[1] == 0) ) { // 현재 잔액이 0원 이하이면 게임을 끝냄
			isEnd = true;
			clcd_input("You run out of money, Cant play.");
			break;
		}
		else if (user_money[0] >= 10) { // 현재 잔액이 9900원 초과이면 게임을 끝냄
			isEnd = true;
			clcd_input("It's END, Cash in your chips");		
			break;
		}
		else { 
			clcd_input("  Your Balance");
			FND_control(user_money,3);
		}
	
		// 게임을 계속할건지 종료할건지 사용자의 입력 받기
		while(!isEnd){
			clcd_input("   Continue??     7.Yes   8.No  ");
			if (tact_switch_listener() == 7){	// 7번 스위치 클릭시 while문 빠져나오면서 첫 부분으로 돌아감
				break;
			}
			else if (tact_switch_listener() == 8){ // 8번 스위치를 누르면 게임이 끝나게됨
				clcd_input("Cash in your chips,  Good Bye!!");
				FND_control(user_money,3); // 잔액을 3초동안 보여줌
				isEnd = true;
				break;
			}
			else { /*pass*/ }
		}
		led_count = 0; // 현 게임의 베팅금액이 다음게임에 이어지지 않도록 베팅액 초기화
	}
	
	return 0;
}