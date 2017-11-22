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

#define  DIR_NAME "/home/mud/aota/aoa/backup/"	/* mud home directory
here! */
#define  MAX_STRING_LENGTH 4096
#define  MAX_NAME_LENGTH 13
#define  MAX_SITE_LENGTH 16
#define  PCFLAG_DEADLY    2
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
   char    site [MAX_SITE_LENGTH];
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

void read_pfile (char *dirname, char *filename, char *pfilename )
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

  if ( ( fp = fopen( pfilename, "r" ) ) == NULL )
     return;
  fclose (fp);

  r.last = 10000 * t->tm_year + 100 * (t->tm_mon+1) + t->tm_mday;

  tdiff = (now_time - fst.st_mtime) / 86400;
  if ( (r.level < 3 && tdiff > 7)
  ||   (r.level < 4 && tdiff > 14)
  ||   (r.level < 5 && tdiff > 21)
  ||   (tdiff > 93) )
  {
     if ( unlink(pfilename) == -1 )
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
  char pfilename[100];

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
            sprintf (pfilename,"/home/mud/aota/aoa/player/%c/%s.gz",
   	       tolower(dentry->d_name[0]),dentry->d_name);
            read_pfile (dir_name, dentry->d_name, pfilename );
            cou++;
            }
         dentry = readdir( dp );
      }
      closedir( dp );
      printf ("Count=%d, deleted %d output %d\n\r", cou, deleted, output);
  }
  fclose(ofp);
}
