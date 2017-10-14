#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include "dev.h"
#include "merrno.h"
#include "server.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "-Usage: %s <dev> <port>\n", argv[0]);
		return -1;
	}

	int fd = initServer(10000);
	if(fd >= 0){
		getHttpRequest(fd);
		responseHttp(fd);
	}

	close(fd);

	// camera_fd = open(argv[1], O_RDWR | O_NONBLOCK);
	// init_dev();
	// cam_on();

	// get_dev_info();

	// get_frame();

	// cam_off();

	// close(camera_fd);

	return 0;
}