/*
 * Grub Extract - creates an extract file for the GRUB command
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/dir.h>

#define  DIR_NAME "/home/mud/aota/aoa/lockers/"	/* mud home directoryhere!
*/
#define  MAX_STRING_LENGTH 4096
#define  MAX_NAME_LENGTH 13
#define  MAX_SITE_LENGTH 16
#define  PCFLAG_DEADLY    2
#define  LOWER(c)        ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))

time_t	now_time;
int	deleted = 0;
int	output = 0;

char *get_arg( char *argument, char *arg_first )
{
    int count=0;

    while ( isspace(*argument) ) argument++;
    while ( *argument != '\0' && *argument != 10 && ++count <= 255 )
    {
       if ( *argument == ' ' ) {argument++; break;}
       *arg_first = LOWER(*argument);
       arg_first++;
       argument++;
    }
    *arg_first = '\0';
    while ( isspace(*argument) ) argument++;
    return argument;
}

int read_pfile (char *dirname, char *filename )
{
  FILE *fp;
  char fname[MAX_STRING_LENGTH];
  char fname2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];

  sprintf( fname, "%s/%s",dirname, filename );
  sprintf( fname2, "%s/%s.gz",dirname, filename );

  if ( ( fp = fopen( fname, "r" ) ) == NULL )
  {
    if ( ( fp = fopen( fname2, "r" ) ) == NULL )
     return -1;
  }
  output++;
  return 0;
}

void main (void)
{
  DIR *dp;
  struct dirent *dentry;
  char dir_name[100];
  char buf1[100];
  char buf2[100];
  int alpha_loop;
  int cou=0,del;

  now_time = time(0);
  nice(20);

  for (alpha_loop=0; alpha_loop<=25; alpha_loop++)
  {
      sprintf (dir_name, "%s%c", DIR_NAME, 'a' + alpha_loop);
      printf ("dir=%s\n\r", dir_name);
      dp = opendir( dir_name );
      dentry = readdir( dp );
      while ( dentry )
      {
         if ( dentry->d_name[0] != '.' )
            {
	      sprintf(buf1, "/home/mud/aota/aoa/player/%c",'a' + alpha_loop );
	      sprintf(buf2, "%s/%s",dir_name,dentry->d_name);
              if ( ( del=read_pfile(buf1, dentry->d_name ) ) == -1 )
	      {
	        unlink(buf2);
	        deleted++;
	      }
            cou++;
            }
         dentry = readdir( dp );
      }
      closedir( dp );
      printf ("Count=%d, deleted %d output %d\n\r", cou, deleted, output);
  }
}
