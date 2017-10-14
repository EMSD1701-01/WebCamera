#include "merrno.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>

void strErr(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
}

void sysErr(const char *msg)
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}

void suc_err(int code, const char *msg)
{
	if(code < 0){
		sysErr(msg);
	}else{
		strErr(msg);
	}
}