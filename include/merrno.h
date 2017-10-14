#ifndef __MERRNO_H__
#define __MERRNO_H__ 

void strErr(const char *msg);

void sysErr(const char *msg);

void suc_err(int code, const char *msg);

#endif //__MERRNO_H__