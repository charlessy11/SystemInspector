#ifndef _UTIL_H_
#define _UTIL_H_

#include <sys/types.h>

void draw_percbar(char *buf, double frac);
void uid_to_uname(char *name_buf, uid_t uid);

#endif
