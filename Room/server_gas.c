#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <mcp3004.h>
#include <pthread.h>

#define BACKLOG 10
#define BASE 100
#define SPI_CHAN 0
//#define GAS_PIN 2
#define BASE 100
#define FAN 25 // FAN GPIO 번호
#define RED 27 // RED GPIO 번호
#define GREEN 28 // GREEN GPIO 번호
#define BLUE 29 // BLUE GPIO 번호
#define BUZ 2 // BUZ GPIO 번호
#define MAX_BUF 1000 //Packet Size 지정

int flag = -1; //flag 1 : 메세지 전달, 2 : PEN 작동, 3 : PEN 작동X, 4 : 가스센서 실행
int count = 0; // 센서 이상 감지 count
int panflag = 0; // panflag 1: ON
unsigned int adcValue1 = 0, adcValue2 = 0; // MQ-7B 센서 값 / MQ-4 센서 값

void* sensorThread(void *arg) // Sensor 동작하는 Thread
{
	int k = *((int*)arg);
    char buf[MAX_BUF]; // 버퍼 Message 담는 변수

	while(1) 
	{
		if (flag == -1) // flag 초기값
		{
			if (flag != 5) // flag = 5(Sensor OFF)가 아닐 때만 Sensor 값을 읽는다.
			{
				adcValue1 = analogRead(BASE + 2);
				adcValue2 = analogRead(BASE + 3);
			}
			if (adcValue1 >= 1000 && adcValue2 >= 1000) // Sensor 값이 둘 다 1000 이상일 경우  // 위험 단계
			{
				digitalWrite(RED, HIGH); // RED ON
				digitalWrite(GREEN, LOW); // GREEN OFF
				digitalWrite(BLUE, LOW); // BLUE OFF
				digitalWrite(BUZ, HIGH); // BUZ ON
				digitalWrite(FAN, HIGH); // FAN ON
				panflag = 0;//수동 조작 금지(자동으로 FAN이 돌아가게 만듦)
				printf("RED LED ON : Dangerous!!\n");
				count++; // 센서 이상 감지로 인해 count ++
			}
			else if(adcValue1 >= 500 && adcValue2 >= 500) // 주의 단계
			{
				digitalWrite(RED, LOW); // RED OFF
				digitalWrite(GREEN, HIGH); // GREEN ON
				digitalWrite(BLUE, LOW); // BLUE OFF
				digitalWrite(BUZ, LOW); // BUZ OFF
				if (panflag != 1) // FAN을 수동 조작하고 있다면 그대로 수동 조작하게 놔둔다.
				{
					digitalWrite(FAN, LOW); // 자동 조작일시 FAN 멈춤
				}
				printf("GREEN LED ON : Warning!!\n");
				count = 0; // 아직 센서 이상 값이 아니므로 count 초기화
			}
			else
			{
				digitalWrite(RED, LOW); // RED OFF
				digitalWrite(GREEN, LOW); // GREEN OFF
				digitalWrite(BLUE, HIGH); // BLUE ON
				digitalWrite(BUZ, LOW); // BUZ OFF
				if (panflag != 1) // FAN을 수동 조작하고 있다면 그대로 수동 조작하게 놔둔다.
				{
					digitalWrite(FAN, LOW); // 자동 조작일시 FAN 멈춤
				}
				printf("BLUE LED ON : Safe\n");
				count = 0; // 아직 센서 이상 값이 아니므로 count 초기화
			}

			if(count > 5) // 30초 동안 센서 이상 감지를 할시
			{
				flag = 1; // flag  = 1( Raspberry Pi -> APP : Message 전송)
				count = 0; // count 초기화
			}
			delay(5000); // 메세지 간격 30초(count 횟수 + 지연시간)
		}
		else if (flag == 2) // PAN_ON
		{
			panflag = 1;//PAN 수동 조절 ON
			sleep(3);
			flag = -1; // 다시 Sensor 동작
		}
		else if (flag == 3) // PAN_OFF
		{
			panflag = 0;//PAN 수동 조절 OFF
			digitalWrite(FAN, LOW); // FAN OFF
			sleep(3);
			flag = -1; // 다시 Sensor 동작
		}
		else if (flag == 4) // Sensor ON
		{
			flag = -1; // Sensor ON
		}
		else if (flag == 5) // Sensor OFF // 가정 1 : 신고 접수 후 재차 신고가 접수 되는 것을 막기 위해 사용자가 직접 Sensor OFF 버튼을 눌러 Sensor 들을 초기화한다.
			                              // 하지만 공간 내 여전히 유독 가스가 차있기 때문에 FAN은 계속 켜둬야한다.
		{
			panflag = 0; // PAN 자동 조작 ON
			digitalWrite(RED, LOW); // RED OFF
			digitalWrite(GREEN, LOW); // GREEN OFF
			digitalWrite(BLUE, LOW); // BLUE OFF
			digitalWrite(FAN, HIGH); // FAN HIGH
			digitalWrite(BUZ, LOW); // BUZ OFF
			sleep(3);
			continue;
		}
	}
	return NULL;
}
void* panThread(void* arg) // PAN 돌아가는 Thread
{
	while (1)
	{
		if (panflag == 1) // FAN 수동 조작 ON일 때
		{
			digitalWrite(FAN, HIGH); // FAN HIGH
		}
		usleep(1000); // Delay for a small time period to avoid hogging CPU resources
	}
	return NULL;
}

void *sendThread(void *arg) // Message 송신 Thread
{
	int k = *((int*)arg); // 송신 Socket
	char buf[MAX_BUF]; // Message 담는 Thread
	
	while(1)
	{
		if(flag != 1) // flag = 1(Raspberry Pi -> APP : Message 전송) 단계가 아닐 때
		{
			usleep(1000); //1ms sleep
		}
		else if (flag == 1) // flag = 1이라면 Message 전송
		{
			sprintf(buf, "101: Danger\n"); //buf 변수에 담아준다.
			int iLength = write(k, buf, strlen(buf)); // buf Message를 전달
			if (iLength < 0)
			{
				perror("Error writing to socket");
			}
			flag = -1;
		}
	}
	return NULL;
}

void *recvThread(void *arg) // Message 수신 Thread
{
	int sock = *((int*)arg); // 수신 Socket
	ssize_t recv_len; // 문자열 길이
	char message[MAX_BUF]; // 수신 Message 담은 변수

	while(1)
	{
		memset(message, 0, sizeof(message)); // message 변수 초기화
		recv_len = read(sock, message, sizeof(message)); // 수신 Message를 담아준다.

		if (recv_len < 0)
		{
			printf("error!!!");
		}
		message[recv_len] = '\0'; // 수신 Message 마지막 부분에 개행을 한다.

		printf("Message received: %s\n", message);
		if (strcmp(message, "PAN_ON") == 0) // 수신 Message가 PAN_ON이라면
		{
			flag = 2; // flag -> 2
		}
		else if (strcmp(message, "PAN_OFF") == 0) // 수신 Message가 PAN_OFF이라면
		{
			flag = 3; // flag -> 3
		}
		else if (strcmp(message, "SENSOR_ON") == 0) // 수신 Message가 SENSOR_ON이라면
		{
			if (flag != 5)
			{
				printf("이미 센서가 작동중입니다."); 
			}
			else
			{
				flag = 4; // flag -> 4
			}
		}
		else if (strcmp(message, "SENSOR_OFF") == 0) // 수신 Message가 SENSOR_OFF이라면
		{
			flag = 5; // flag -> 5
		}

	}
	return NULL;
}

int main(void)
{
	int serverSocket, newSocket; // Socket 변수
	struct sockaddr_in serverAddress, clientAddress; // 주소록
	pthread_t sensor_thread, send_thread, recv_thread, pan_thread; // Thread 변수

	printf("TCP network\n");
	fflush(stdout);
	if((serverSocket=socket(AF_INET, SOCK_STREAM, 0)) == -1) // Server Socket Open
	{
		perror("Error open socket");
		exit(1);
	}
	printf("   ..\n");
	fflush(stdout);
	
	memset(&serverAddress, 0, sizeof(serverAddress)); 
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(50000);//Server 포트 주소(Host endian을 network endian으로 변환)
	
	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) // Socket bind
	{
		perror("Error bind");
		exit(1);
	}

	if (listen(serverSocket, BACKLOG == -1)) // Client가 Connect할 때까지 대기
	{
		perror("Error listen");
		exit(1);
	}
	printf("...\n");
	fflush(stdout);

	if(wiringPiSetup() == -1)
	{
		printf("setup fail");
		return -1;
	}

	printf("wiringPiSPISetup return = %d\n", wiringPiSPISetup(0,500000));
	mcp3004Setup(BASE, SPI_CHAN); // PIN 세팅
	pinMode(RED, OUTPUT); 
	pinMode(GREEN, OUTPUT);
	pinMode(BLUE, OUTPUT);
	pinMode(BUZ, OUTPUT);
	pinMode(FAN, OUTPUT);
	// Raspberry Pi PIN들 초기화

	socklen_t clientAddressSize = sizeof(clientAddress);
	newSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize); // Client Socket 연결 요청 수락
	if (newSocket == -1)
	{
		perror("Error accpet");
		exit(1);
	}

	pthread_create(&sensor_thread, NULL, sensorThread, (void*)&newSocket); // SensorThread 생성
	pthread_create(&send_thread, NULL, sendThread, (void*)&newSocket); // sendThread 생성
	pthread_create(&recv_thread, NULL, recvThread, (void*)&newSocket); // recvThread 생성
	pthread_create(&pan_thread, NULL, panThread, (void*)&newSocket); // FANThread 생성

	pthread_join(sensor_thread,NULL); // SensorThread 실행
	pthread_join(send_thread,NULL); // SendThread 실행
	pthread_join(recv_thread,NULL); // RecvThread 실행
	pthread_join(pan_thread, NULL); // PANThread 실행

	/* 소켓 종료*/
	close(newSocket);
	close(serverSocket);

	return 0;
}
