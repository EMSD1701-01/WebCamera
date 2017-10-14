#ifndef __DEV_H__
#define __DEV_H__ 

#include <sys/types.h>

typedef struct
{
	void *start; //映射后的物理首地址
	size_t length; //存储映射的地址长度
}Videobuf;

//摄像头文件描述符
extern int camera_fd;
//存储映射数组
extern Videobuf *buffer;
//记录缓存的数量
extern int bufs_num;
//记录捕获了多少张图像
extern int counter;
//记录已经刷新好的缓存的索引号
extern int okindex;
//临时缓冲区
extern char *tmp_buf;
//记录摄像头打开状态
extern int on_off;

int init_dev();

int uninit_dev();

int get_dev_info();

int cam_on();

int cam_off();

int get_frame();


#endif //__DEV_H__