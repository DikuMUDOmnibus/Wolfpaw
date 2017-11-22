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

/* mud home directory here! */
#define  DIR_NAME "/usr/users/mud/greywolf/aoa/player/"
#define  MAX_STRING_LENGTH 4096
#define  MAX_NAME_LENGTH 100
#define  MAX_SITE_LENGTH 16
#define  PCFLAG_DEADLY    2
#define  LOWER(c)        ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))

time_t	now_time;
int	deleted = 0;
int	output = 0;

struct rec_struct
{
   char    email [MAX_NAME_LENGTH];
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

  if ( ( fp = fopen( fname, "r" ) ) == NULL )
     return;
  memset( &r, 0, sizeof r);
  fgets( s, 2048, fp);
  while (!feof(fp))
  {
     ps = s;
     if (ferror(fp)) {printf("file error\n"); break;}

     if ( s[0]=='E' && s[1]=='m' && s[2]=='a' && s[3]=='i' && s[4]=='i' )
     {
        ps = get_arg (ps, b); ps = get_arg (ps, b);
        if ( b[ strlen(b) - 1] == '~' ) b[ strlen(b) - 1] = '\0';
        strcpy (r.email, b);
     }

     strcat(r.email,"\n");
     fgets(s, 2048, fp);
  }
  fclose (ofp);
     fprintf( fp, r.email );

/*  if (fwrite(&r, sizeof r.email, 1, ofp) < 1)
  {
     printf("write error");
     exit(1);
  }*/
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

  if ( ( ofp = fopen( "/usr/users/mud/greywolf/aoa/system/email.dat", "w")) == NULL )
  {
     printf("Open error on output file.\n\r");
     exit(1);
  }

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
	    printf("Reading: %s\n\r",dentry->d_name);
            read_pfile (dir_name, dentry->d_name, ofp);
            cou++;
            }
         dentry = readdir( dp );
      }
      closedir( dp );
      printf ("Count=%d, deleted %d output %d\n\r", cou, deleted, output);
  }
  fclose(ofp);
}
