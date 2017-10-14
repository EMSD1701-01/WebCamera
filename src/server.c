#include "server.h"
#include "merrno.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

static char *trim(char *cmd);

int initServer(unsigned short port)
{
	//创建serverSocket
	int serverSocket;
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);

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
		return -1;
	}

	//监听，配置最大连接数
	if(listen(serverSocket, SOMAXCONN))
	{
		sysErr("listen");
		return -1;
	}

	int clientSocket;
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	//接收客户端的连接请求
	if((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen)) < 0)
	{
		sysErr("accept");
		return -1;
	}

	printf("client: %s:%u connected!\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	//关闭serverSocket
	close(serverSocket);

	return clientSocket;
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
			}else
			{
				printf("%s\n", buffer);
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
		"Content-Type: multipart/x-mixed-replace;boundary=KK\r\n");
	if(write(sockFd, buffer, strlen(buffer)) != strlen(buffer))
	{
		sysErr("responseHttp head");
		return -1;
	}

	int imgFd = open("a.jpg", O_RDONLY);
	size_t size = 0;
	if(imgFd < 0)
	{
		sysErr("open a.jpg");
		return -1;
	}else
	{
		struct stat st;
		if(fstat(imgFd, &st))
		{
			sysErr("stat");
		}
		size = st.st_size;
	}
	
	sprintf(buffer, "\r\n--KK\r\n"
		"Content-Type: image/jpeg\n"
		"Content-Length: %lu\n\n", size);
	if(write(sockFd, buffer, strlen(buffer)) != strlen(buffer))
	{
		sysErr("responseHttp type");
	}

	while((size = read(imgFd, buffer, 1024)) > 0)
	{
		write(sockFd, buffer, size);
	}

	close(imgFd);

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