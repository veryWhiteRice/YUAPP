햐#include <stdlib.h>
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
#define FAN 25
#define RED 27
#define GREEN 28
#define BLUE 29
#define BUZ 2
#define MAX_BUF 1000 //Packet Size 지정

int flag = -1; //flag 1 : 메세지 전달, 2 : PEN 작동, 3 : PEN 작동X, 4 : 가스센서 실행
int count = 0;
int panflag = 0; // panflag 1: ON
unsigned int adcValue1 = 0, adcValue2 = 0;
//pthread_mutex_t sensor_value_mutex = PTHREAD_MUTEX_INITIALIZER;

void* sensorThread(void *arg)
{
	int k = *((int*)arg);
    char buf[MAX_BUF];

	while(1)
	{
		if (flag == -1)
		{
			if (flag != 5)
			{
				adcValue1 = analogRead(BASE + 2);
				adcValue2 = analogRead(BASE + 3);
			}
			if (adcValue1 >= 1000 && adcValue2 >= 1000)
			{
				digitalWrite(RED, HIGH);
				digitalWrite(GREEN, LOW);
				digitalWrite(BLUE, LOW);
				digitalWrite(BUZ, HIGH);
				digitalWrite(FAN, HIGH);
				panflag = 0;//수동 조작 금지
				printf("RED LED ON : Dangerous!!\n");
				count++;
			}
			else if(adcValue1 >= 500 && adcValue2 >= 500)
			{
				digitalWrite(RED, LOW);
				digitalWrite(GREEN, HIGH);
				digitalWrite(BLUE, LOW);
				digitalWrite(BUZ, LOW);
				if (panflag != 1)
				{
					digitalWrite(FAN, LOW);
				}
				printf("GREEN LED ON : Warning!!\n");
				count = 0;
			}
			else
			{
				digitalWrite(RED, LOW);
				digitalWrite(GREEN, LOW);
				digitalWrite(BLUE, HIGH);
				digitalWrite(BUZ, LOW);
				if (panflag != 1)
				{
					digitalWrite(FAN, LOW);
				}
				printf("BLUE LED ON : Safe\n");
				count = 0;
			}

			if(count > 5)
			{
				flag = 1;
				count = 0;
			}
			delay(5000); // 메세지 간격 30초(count 횟수 + 지연시간)
		}
		else if (flag == 2)
		{
			panflag = 1;//PAN 수동 조절 ON
			sleep(3);
			flag = -1;
		}
		else if (flag == 3)
		{
			panflag = 0;//PAN 수동 조절 OFF
			digitalWrite(FAN, LOW);
			sleep(3);
			flag = -1;
		}
		else if (flag == 4)
		{
			flag = -1;
		}
		else if (flag == 5)
		{
			panflag = 0;
			digitalWrite(RED, LOW);
			digitalWrite(GREEN, LOW);
			digitalWrite(BLUE, LOW);
			digitalWrite(FAN, HIGH);
			digitalWrite(BUZ, LOW);
			sleep(3);
			continue;
		}
	}
	return NULL;
}
void* panThread(void* arg)
{
	while (1)
	{
		if (panflag == 1)
		{
			digitalWrite(FAN, HIGH);
		}
		usleep(1000); // Delay for a small time period to avoid hogging CPU resources
	}
	return NULL;
}


void *sendThread(void *arg)
{
	int k = *((int*)arg);
	char buf[MAX_BUF];
	
	while(1)
	{
		if(flag != 1)
		{
			usleep(1000); //1ms sleep
		}
		else if (flag == 1)
		{
			//pthread_mutex_lock(&sensor_value_mutex);
			sprintf(buf, "101: Danger\n");
			//pthread_mutex_unlock(&sensor_value_mutex);
			int iLength = write(k, buf, strlen(buf));
			if (iLength < 0)
			{
				perror("Error writing to socket");
			}
			flag = -1;
		}
		//sleep(5000);
	}
	return NULL;
}

void *recvThread(void *arg)
{
	int sock = *((int*)arg);
	ssize_t recv_len;
	char message[MAX_BUF];

	while(1)
	{
		memset(message, 0, sizeof(message));
		recv_len = read(sock, message, sizeof(message));

		if (recv_len < 0)
		{
			printf("error!!!");
		}
		message[recv_len] = '\0';

		printf("Message received: %s\n", message);
		if (strcmp(message, "PAN_ON") == 0)
		{
			flag = 2;
		}
		else if (strcmp(message, "PAN_OFF") == 0)
		{
			flag = 3;
		}
		else if (strcmp(message, "SENSOR_ON") == 0)
		{
			if (flag != 5)
			{
				printf("이미 센서가 작동중입니다.");
			}
			else
			{
				flag = 4;
			}
		}
		else if (strcmp(message, "SENSOR_OFF") == 0)
		{
			flag = 5;
		}

	}
	return NULL;
}

int main(void)
{
	int serverSocket, newSocket;
	struct sockaddr_in serverAddress, clientAddress;
	pthread_t sensor_thread, send_thread, recv_thread, pan_thread;

	printf("TCP network\n");
	fflush(stdout);
	if((serverSocket=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Error open socket");
		exit(1);
	}
	printf("   ..\n");
	fflush(stdout);
	
	//SOCKADDR_IN serverAddress;//Server 주소(모텔/호텔 프론트)
	//bzero(&serverAddress, sizeof(serverAddress));
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(50000);//Server 포트 주소(Host endian을 network endian으로 변환)
	
	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
	{
		perror("Error bind");
		exit(1);
	}

	if (listen(serverSocket, BACKLOG == -1))
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
	mcp3004Setup(BASE, SPI_CHAN);
	pinMode(RED, OUTPUT);
	pinMode(GREEN, OUTPUT);
	pinMode(BLUE, OUTPUT);
	pinMode(BUZ, OUTPUT);
	pinMode(FAN, OUTPUT);

	socklen_t clientAddressSize = sizeof(clientAddress);
	newSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);
	if (newSocket == -1)
	{
		perror("Error accpet");
		exit(1);
	}

	pthread_create(&sensor_thread, NULL, sensorThread, (void*)&newSocket);
	pthread_create(&send_thread, NULL, sendThread, (void*)&newSocket);
	pthread_create(&recv_thread, NULL, recvThread, (void*)&newSocket);
	pthread_create(&pan_thread, NULL, panThread, (void*)&newSocket);

	pthread_join(sensor_thread,NULL);
	pthread_join(send_thread,NULL);
	pthread_join(recv_thread,NULL);
	pthread_join(pan_thread, NULL)

	/* 소켓 종료*/
	close(newSocket);
	close(serverSocket);

	return 0;
}
