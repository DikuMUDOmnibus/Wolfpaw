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

#define  DIR_NAME "../corpses/"	/* mud home directory here! */
#define  MAX_STRING_LENGTH 4096
#define  MAX_NAME_LENGTH 20
#define  LOWER(c)        ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))

time_t	now_time;
int	deleted = 0;
int	output = 0;

struct rec_struct
{
   char    name [MAX_NAME_LENGTH];
   char    sex;
   char    class;
   char    race;
   char    level;
   short   room;
   long    gold;
   char    clan;
   char    council;
   long    last;
   char    pkill;
};

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

void read_pfile (char *dirname, char *filename, FILE *ofp)
{
  FILE *fp;
  struct rec_struct r;
  char b[MAX_STRING_LENGTH], s[MAX_STRING_LENGTH], *ps;
  char fname[MAX_STRING_LENGTH];
  struct stat fst;
  struct tm *t;
  time_t tdiff;
  int    flags;

  sprintf( fname, "%s/%s", dirname, filename );

  if ( stat( fname, &fst ) != -1 )
  {
     t = localtime( &fst.st_mtime );
  }
  else
  {
     t = NULL;
  }

  printf("Checking: %s\n",fname);
  r.last = 10000 * t->tm_year + 100 * (t->tm_mon+1) + t->tm_mday;
  tdiff = (now_time - fst.st_mtime) / 86400;

  if (tdiff > 93) 
  {
     printf("Unlinking: %s\n",fname);
     if ( unlink(fname) == -1 )
	perror( "Unlink" );
     else
     {
	++deleted;
        return;
     }
  }

  output++;
}

void main (void)
{
  FILE *ofp;
  DIR *dp;
  struct dirent *dentry;
  char dir_name[100];
  int alpha_loop;
  int cou=0;

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
            read_pfile (dir_name, dentry->d_name, ofp);
            cou++;
            }
         dentry = readdir( dp );
      }
      closedir( dp );
      printf ("Count=%d, deleted %d output %d\n\r", cou, deleted, output);
  }
}
