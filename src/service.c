#include "service.h"
#include "merrno.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char *trim(char *cmd);

int initServer(unsigned short port)
{
	//创建serverSocket
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	//配置地址
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//配置端口可重用
	int opt = 1;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	//绑定端口
	if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		sysErr("bind");
		return;
	}

	//监听，配置最大连接数
	if(listen(serverSocket, SOMAXCONN))
	{
		sysErr("listen");
		return;
	}

	return serverSocket;
}

int getHttpRequest(int sockFd)
{
	char buffer[1024], ch;
	int index = 0;
	while(read(sockFd, &ch, 1) > 0)
	{
		if(ch == '\n')
		{
			buffer[index] = '\0';
			if(!strcmp("\r", trim(buffer)))
			{
				break;
			}
			index = 0;
		}else
		{
			buffer[index++] = ch;
		}
	}
	return 0;
}

int responseHttp(int sockFd)
{
	char buffer[1024];
	sprintf(buffer, "HTTP/1.0 200 OK\r\n"
		"Connection: Keep-Alive\r\n"
		"Server: Network camera\r\n"
		"Cache-Control: no-cache,no-store,must-revalidate,pre-check=0,max-age=0\r\n"
		"Pragma: no-cache\r\n"
		"Content-Type: multipart/x-mixed-replace;boundary=KK\r\n\r\n");
	if(write(sockFd, buffer, strlen(buffer)) != strlen(buffer)){
		sysErr("responseHttp");
		return -1;
	}
	return 0;
}

int sendPictureHeader(int sockFd, size_t size)
{
	char buffer[128];
	sprintf(buffer, "--KK\r\n"
		"Content-Type: image/jpeg\r\n"
		"Content-Length: %lu\r\n\r\n", size);
	if(write(sockFd, buffer, strlen(buffer)) != strlen(buffer)){
		sysErr("responseHttp type");
		return -1;
	}
	return 0;
}

/**
 * 去除命令行两边的空格
 */
char *trim(char *cmd)
{
	while(*cmd == ' ') cmd++;
	char *end = cmd + strlen(cmd) - 1;
	while(*end == ' ') *end-- = '\0';
	return cmd;
}
