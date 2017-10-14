#ifndef __SERVER_H__
#define __SERVER_H__

/**
 * 初始化服务器
 * @param port 绑定的端口
 * @return 如果成功返回一个文件描述符，失败返回-1
 */
int initServer(unsigned short port);

/**
 * responseHttp
 */
int responseHttp(int sockFd);

int getHttpRequest(int sockFd);

#endif //__SERVER_H__