#include "dev.h"
#include "merrno.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

static void init_fmt();
static void init_mmap();

//摄像头文件描述符
int camera_fd;

//存储映射数组
Videobuf *buffer;
//记录缓存的数量
int bufs_num;
//记录捕获了多少张图像
int counter;
//记录已经刷新好的缓存的索引号
int okindex;
char *tmp_buf;
int on_off;

int init_dev()
{
	//初始化视频格式
	init_fmt();
	//初始化内存映射
	init_mmap();

	return 0;
}

int uninit_dev()
{
	int i;
	for (i = 0; i < bufs_num; ++i)
	{
		int res = munmap(buffer[i].start, buffer[i].length);
		suc_err(res, "munmap");
	}
	free(buffer);
	free(tmp_buf);
	close(camera_fd);
	return 0;
}

int get_dev_info()
{
	//获取当前设备的属性
	struct v4l2_capability cap;
	int res = ioctl(camera_fd,VIDIOC_QUERYCAP,&cap);

	//获取当前设备的输出格式
	struct v4l2_format fmt;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	res = ioctl(camera_fd,VIDIOC_G_FMT,&fmt);

	//获取当前设备的帧率
	struct v4l2_streamparm parm;
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	res = ioctl(camera_fd,VIDIOC_G_PARM,&parm);

	//打印输出设备信息：
	printf("----------------dev_info---------------\n");
	printf("driver:	%s\n",cap.driver);
	printf("card:	%s\n",cap.card);//摄像头的设备名
	printf("bus:	%s\n",cap.bus_info);
	printf("width:	%d\n",fmt.fmt.pix.width);//当前的图像输出宽度
	printf("height:	%d\n",fmt.fmt.pix.height);//当前的图像输出高度
	printf("FPS:	%d\n",parm.parm.capture.timeperframe.denominator);
	printf("------------------end------------------\n");

	return 0;
}

int cam_on()
{
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int res = ioctl(camera_fd,VIDIOC_STREAMON,&type);
	suc_err(res, "camera on");

	//进行一次缓存刷新
	int i;
	struct v4l2_buffer buf;
	for(i = 0;i < bufs_num;i++)
	{
	    memset(&buf,0,sizeof(buf));
	    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	    buf.memory = V4L2_MEMORY_MMAP;
	    buf.index = i;
	    res = ioctl(camera_fd,VIDIOC_QBUF,&buf);
	    suc_err(res,"Q_buf_init");
	}

	return 0;
}

int cam_off()
{
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	int res = ioctl(camera_fd, VIDIOC_STREAMOFF, &type);
	suc_err(res, "close stream");
	
	on_off = 0;
	
	return 0;
}

int get_frame()
{
	struct v4l2_buffer buf;
	int i = 0,res;
	counter++;
	memset(&buf,0,sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	fd_set	readfds;
	FD_ZERO(&readfds);
	FD_SET(camera_fd,&readfds);
	struct timeval tv;//设置设备响应时间
	tv.tv_sec = 1;//秒
	tv.tv_usec = 0;//微秒
	while(select(camera_fd + 1,&readfds,NULL,NULL,&tv) <= 0)
	{
		fprintf(stderr,"camera busy,Dq_buf time out\n");
		FD_ZERO(&readfds);
		FD_SET(camera_fd,&readfds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
	}
	res = ioctl(camera_fd,VIDIOC_DQBUF,&buf);
	suc_err(res,"Dq_buf");

	//buf.index表示已经刷新好的可用的缓存索引号
	okindex = buf.index;
	//更新缓存已用大小
	buffer[okindex].length = buf.bytesused;
	//第n次捕获图片:(第n回刷新整个缓存队列-第n个缓存被刷新)
	//printf("Image_%03d:(%d-%d)\n",counter,counter / bufs_num,okindex);

	//把一张图片放入缓存
	int fd = open("a.jpg", O_RDWR|O_CREAT|O_TRUNC);
	suc_err(fd, "open");
	print_picture(fd, buffer[okindex].start, buffer[okindex].length);
	close(fd);

	//把图像放入缓存队列中(入列)
	res = ioctl(camera_fd,VIDIOC_QBUF,&buf);
	suc_err(res,"Q_buf");

	return 0;
}

//初始化视频格式
void init_fmt()
{
	struct v4l2_format fmt;
    memset(&fmt,0,sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//数据流的类型
    fmt.fmt.pix.width = 320;//图像的宽度
    fmt.fmt.pix.height = 240;//图像的高度
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;//彩色空间
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    int res = ioctl(camera_fd,VIDIOC_S_FMT,&fmt);
    suc_err(res, "format");
}

//初始化内存映射
void init_mmap()
{
	int res;
	//请求缓存
	struct v4l2_requestbuffers req; 
	memset(&req,0,sizeof(req));
	req.count = 4;//缓存数量
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	res = ioctl(camera_fd,VIDIOC_REQBUFS,&req);
	suc_err(res,"Req_bufs");

	buffer = calloc(req.count,sizeof(Videobuf));
	struct v4l2_buffer buf;
	for(bufs_num = 0;bufs_num < req.count;bufs_num++)
	{
		memset(&buf,0,sizeof(buf));
		buf.index = bufs_num;//设置缓存索引号
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.field = V4L2_FIELD_INTERLACED;
		buf.memory = V4L2_MEMORY_MMAP;
		//读取缓存信息
		res = ioctl(camera_fd,VIDIOC_QUERYBUF,&buf);
		suc_err(res,"Query_buf");
		//设置缓存大小
		buffer[bufs_num].length = buf.length;
		//在堆空间中动态分配二级缓存空间
		tmp_buf = (char*)calloc(buffer[okindex].length,sizeof(char));
		//将设备文件的地址映射到用户空间的物理地址
		buffer[bufs_num].start = mmap(NULL,
							buf.length,
							PROT_READ|PROT_WRITE,
							MAP_SHARED,
							camera_fd,
							buf.m.offset);
		if(buffer[bufs_num].start == MAP_FAILED)
			suc_err(-1,"Mmap");
		else
			suc_err(0,"Mmap");
	}
}