#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

void main ( void )
{
FILE *fp;
struct timeval now_time;
time_t current_time;
char *strtime;

gettimeofday( &now_time, NULL );
current_time = (time_t) now_time.tv_sec;
strtime=ctime(&current_time);

fp=fopen("../system/boot_time","w");
fprintf(fp,"%s",strtime);
fclose(fp);
exit(0);
}

