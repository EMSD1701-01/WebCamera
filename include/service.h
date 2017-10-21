#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <sys/types.h>

/**
 * 初始化服务器
 * @param port 绑定的端口
 * @return 如果成功返回一个serverSocket，失败返回-1
 */
int initServer(unsigned short port);

int getHttpRequest(int sockFd);

int responseHttp(int sockFd);

int sendPictureHeader(int sockFd, size_t size);

#endif //__SERVICE_H__