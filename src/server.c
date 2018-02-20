#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "dev.h"
#include "merrno.h"
#include "service.h"
#include "print.h"

#define MAX_CLIENTS_COUNT 128

static int clients[MAX_CLIENTS_COUNT];
static size_t clientCount = 0;

/**
 * 获取有效用户数量
 * @return 返回有效用户个数
 */
size_t getValidClient();

//初始化线程锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * 接收客户端线程
 */
void *tAcceptClient(void *arg)
{
	int serverSocket = initServer(*(short*)arg);
	if(serverSocket == -1){
		return (void*)NULL;
	}
	strErr("服务端初始化完毕...");

	//接收客户端的连接请求
	int clientSocket;
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	while(1){
		if((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen)) < 0)
		{
			sysErr("accept");
		}else{
			fprintf(stderr, "客户端: %s:%u 连接成功!\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			getHttpRequest(clientSocket);
			responseHttp(clientSocket);
			
			pthread_mutex_lock(&mutex); //加锁
			int i;
			for(i = 0; i < MAX_CLIENTS_COUNT; i++){
				if(clients[i] == 0){
					clients[i] = clientSocket;
					if(i + 1 > clientCount){
						clientCount = i + 1;
					}
					break;
				}
			}
			fprintf(stderr, "客户端数量: %lu\n", getValidClient());
			pthread_mutex_unlock(&mutex); //解锁
		}
	}
	
	//关闭serverSocket
	close(serverSocket);
	strErr("服务端关闭...");
}

int main(int argc, char **argv)
{
	//默认端口号
	short int port = 10000;
	if(argc < 2){
		fprintf(stderr, "-Usage: %s <dev> [port]\n", argv[0]);
		return -1;
	}else if(argc >= 3){
		port = atoi(argv[2]);
	}

//	int logFd = open("log/log", O_WRONLY|O_APPEND|O_CREAT, 0644);
//	if(logFd < 0){
//		sysErr("log日志打开失败！");
//	}else{
//		dup2(logFd, 1);
//		dup2(logFd, 2);
//	}

	//忽略管道信号，当客户端结束时不要退出程序
	signal(SIGPIPE, SIG_IGN);
	
	//开启线程监听客户端
	pthread_t acceptClientThread;
	pthread_create(&acceptClientThread, NULL, tAcceptClient, &port);

	//初始化摄像头设备
	init_dev(argv[1]);
	//打开摄像头
	cam_on();
	get_dev_info();

	int i;
	while(1){
		get_frame();

		pthread_mutex_lock(&mutex);
		for(i = 0; i < clientCount; i++){
			if(clients[i] != 0){
				if(sendPictureHeader(clients[i], buffer[okindex].length)){
					struct sockaddr_in addr;
					socklen_t len = sizeof(addr);
					fprintf(stderr, "一个客户端断开连接！\n");
					clients[i] = 0;
					if(i == clientCount - 1){
						clientCount--;
					}
					fprintf(stderr, "客户端数量: %lu\n", getValidClient());
					continue;
				}
				print_picture(clients[i], buffer[okindex].start, buffer[okindex].length);
			}
		}
		pthread_mutex_unlock(&mutex);
	}

	//关闭设备
	cam_off();
	uninit_dev();

	return 0;
}

/**
 * 获取有效用户数量
 * @return 返回有效用户个数
 */
size_t getValidClient()
{
	int i;
	size_t ret = 0;
	for(i = 0; i < clientCount; i++){
		if(clients[i] != 0){
			ret++;
		}
	}
	return ret;
}

