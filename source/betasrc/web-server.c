/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *		Wolfpaw Integrated Web Server Protocol Module		    *
 *--------------------------------------------------------------------------*/

/* Code Based On:
 * ROM 2.4 Integrated Web Server - Version 1.0
 * Copyright 1998 -- Defiant -- Rob Siemborski -- mud@towers.crusoe.net
 * His Copyright states to please include him in your mud credits.
 * Modifications from that release Copyright Greywolf (Dale Corse) 1998
 * My copyright says:
 *
 * [ABSOULUTLY NOTHING!]
 *
 * Except this:
 *
 * I will awnser questions, and help you fix bugs.. with MY CODE
 * not you other stuff, and i am not resposible for anything that
 * happens to you as a result of using this code, not even if it
 * makes you lose your girlfriend, mets your ice cream, formats
 * you hard drive, or causes nuclear holocost (sp?). Also, please
 * dont use this code, and say you did it, cause Defiant, Altrag, and I
 * did. So doing that would piss us off.
 *
 * --GW 
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mud.h"

/* Moved the Struct to Mud.h */

/* FUNCTION DEFS */
int send_buf(int fd, char* buf, bool filter );
void handle_web_request(WEB_DESCRIPTOR *wdesc);
void handle_web_who_request(WEB_DESCRIPTOR *wdesc);
void handle_web_wwwwho_request(WEB_DESCRIPTOR *wdesc);
void handle_web_wizlist_request(WEB_DESCRIPTOR *wdesc);
void handle_web_news_request(WEB_DESCRIPTOR *wdesc);
char *color_filter( char *string );
char *text2html(const char *ip);
char *parse_quotes( char *arg );
    
/* The mark of the end of a HTTP/1.x request */
const char ENDREQUEST[5] = { 13, 10, 13, 10, 0 }; /* (CRLFCRLF) */

/* Externs */
extern int top_web_desc;
/* Linked List stuff --GW */
extern WEB_DESCRIPTOR *first_webdesc;
extern WEB_DESCRIPTOR *last_webdesc;

/* Locals */
int sockfd;

void init_web(int port)
{
    struct sockaddr_in my_addr;
    char buf[MSL];
   
// Disabled - AOTAII has the port now..GW
    return;

    sprintf(buf,"Web features starting on port: %d", port);
    log_string(buf);

    WEBSERVER_STATUS = TRUE;

    /* Lets clear these out .. --GW */
    first_webdesc = NULL;
    last_webdesc = NULL;
    top_web_desc = 0;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("web-socket");
	WEBSERVER_STATUS=FALSE;
	return;
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = htons(INADDR_ANY);
    bzero(&(my_addr.sin_zero),8);

    if((bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr))) == -1)
    {
	perror("web-bind");
	log_string("WebServer Disabled.");
        WEBSERVER_STATUS = FALSE;
	return;
    }

    /* Only listen for 5 connects at once, do we really need more? */
    /* Nah .. but thanks for asking! =P *smirk* --GW */
    listen(sockfd, 5);

}

struct timeval ZERO_TIME = { 0, 0 };

void handle_web(void) {
	int max_fd;
	WEB_DESCRIPTOR *current, *next;
	fd_set readfds;

	if ( WEBSERVER_STATUS == FALSE )
	return;

	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);

	/* it *will* be atleast sockfd */
	max_fd = sockfd;

	/* add in all the current web descriptors */
        /* Linked stuff again --GW */
	for(current = first_webdesc; current; current = current->next)
        {
	    FD_SET(current->fd, &readfds);
	    if(max_fd < current->fd)
		max_fd = current->fd;
	}
	
	/* Wait for ONE descriptor to have activity */
	select(max_fd+1, &readfds, NULL, NULL, &ZERO_TIME);

	if(FD_ISSET(sockfd, &readfds)) {
            /* NEW CONNECTION -- INIT & ADD TO LIST */

	    /* Ok .. so we dont really need those memory functions
	       included in the original release, a function for 1 line
	       of code .. no. --GW */
	    CREATE( current, WEB_DESCRIPTOR, 1 );
	    current->sin_size  = sizeof(struct sockaddr_in);
	    current->request[0] = '\0';

	    if((current->fd = accept(sockfd, (struct sockaddr *)&(current->their_addr), &(current->sin_size))) == -1) {
	    	perror("web-accept");
	    	DISPOSE(current);
		return;
	    }

	    /* Ugh .. lets just use LINK here .. --GW */
	    LINK( current, first_webdesc, last_webdesc, next, prev );

	    /* END ADDING NEW DESC */
	}

	/* DATA IN! */
	/* Nother change for Linked List stuff --GW */
	for(current= first_webdesc; current; current = current->next)
        {
	    if (FD_ISSET(current->fd, &readfds)) /* We Got Data! */
	    {
	    	char buf[MAXDATA];
		int numbytes;

		if((numbytes=read(current->fd,buf,sizeof(buf))) == -1) {
		    perror("web-read");
		    return;
		}

		buf[numbytes] = '\0';

		strcat(current->request,buf);
	    }
	} /* DONE WITH DATA IN */

	/* DATA OUT */
	/* Again .... --GW */
	for(current = first_webdesc; current; current = next )
	{
	    next = current->next;

	    if(strstr(current->request, "HTTP/1.") /* 1.x request (vernum on FIRST LINE) */
	    && strstr(current->request, ENDREQUEST))
		handle_web_request(current);
	    else if(!strstr(current->request, "HTTP/1.")
		 &&  strchr(current->request, '\n')) /* HTTP/0.9 (no ver number) */
		handle_web_request(current);		
	    else {
		continue; /* Don't have full request yet! */
	    }

	    close(current->fd);
	    /* Again, no function needed! 
		also moved this up to here, were done with it, so
		nuke away!  --GW */
	    UNLINK(current, first_webdesc,last_webdesc,next,prev);
	    DISPOSE(current);
	  }
	   /* Removed a whack of stuff here, we dont need it. --GW */

	}   /* END DATA-OUT */


/* Generic Utility Function */

int send_buf(int fd, char* buf, bool filter )
{
   if ( filter )
   {
	send(fd, "<CODE>", 6, 0);
	buf = color_filter(buf);
	buf = text2html(buf);
	send(fd, "</CODE>", 7, 0);
   }
	return send(fd, buf, strlen(buf), 0);
}

void handle_web_request(WEB_DESCRIPTOR *wdesc)
{
char buf[MSL];
char *hstbuf;
struct sockaddr_in *sock;

sock = (struct sockaddr_in *)wdesc->their_addr;

buf[0]='\0';
hstbuf=NULL;

hstbuf = STRALLOC("UNKNOWN");
//hstbuf  = inet_ntoa( sock->sin_addr );

	    /* process request */
	    /* are we using HTTP/1.x? If so, write out header stuff.. */
	    if(!strstr(wdesc->request, "GET")) 
	    {
		send_buf(wdesc->fd,"HTTP/1.0 501 Not Implemented", FALSE);
		return;
	    } else if(strstr(wdesc->request, "HTTP/1."))
	    {
		send_buf(wdesc->fd,"HTTP/1.0 200 OK\n", FALSE);
		send_buf(wdesc->fd,"Content-type: text/html\n\n",FALSE);
	    }

	    /* Handle the actual request */
	    if(strstr(wdesc->request, "/wholist"))
	    {
		log_string("Web Hit: WHOLIST");
		handle_web_who_request(wdesc);
	    } 
	    else if(strstr(wdesc->request, "/wwwlist"))
	    {
		sprintf(buf,"Web Hit: WWW-WHOLIST (IP: %s)",hstbuf);
		log_string(buf);
		handle_web_who_request(wdesc);
	    } 
	    else if(strstr(wdesc->request, "/wizlist"))
	    {
		sprintf(buf,"Web Hit: WIZ-LIST (IP: %s)",hstbuf);
		log_string(buf);
		handle_web_wizlist_request(wdesc);
	    } 
	    else if(!str_prefix(wdesc->request, "/skills"))
	    {
		log_string("Web Hit: Skills Listing");
//	        handle_skills_request(wdesc);
	    }
	    else if(strstr(wdesc->request, "/news"))
	    {
		sprintf(buf,"Web Hit: NEWS (IP: %s)",hstbuf);
		log_string(buf);
		handle_web_news_request(wdesc);
	    }
  	    else 
	    {
		sprintf(buf,"Web Hit: INVALID_URL (IP: %s)",hstbuf);
		log_string(buf);
		send_buf(wdesc->fd,"Option not Currently Supported.", FALSE);
	    }
}

void shutdown_web (void)
{
    WEB_DESCRIPTOR *current,*next;

    /* Close All Current Connections */
    /* lets change this around ... blah blah --GW */
    for(current=first_webdesc; current; current = next)
    {
	next = current->next;
	close(current->fd);
        /* Again, no function needed! --GW */
        UNLINK(current, first_webdesc,last_webdesc,next,prev);
        DISPOSE(current);
    }

    /* Stop Listening */
    close(sockfd);
}

void handle_web_who_request(WEB_DESCRIPTOR *wdesc)
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char *buf2;
    int c;
    int num = 0;

  /* Well .. why have 2 copies of your who? Smaug already supports
     webwho, however wont give anyone the cgi for it .. so here we go!
	--GW */

/* Send the Basic Html Config, hard coded background .. sue me. --GW */
send_buf(wdesc->fd,"<HTML><HEAD><TITLE>Age of the Ancients -- Who Listing</TITLE></HEAD>\n\r", FALSE);
buf2 = STRALLOC("<BODY BACKGROUND=*http://www.ancients.org/PAoA1.jpg* BGCOLOR=*#000000* TEXT=*#F8FFFA* LINK=*#9E9295* VLINK=*#B5B5B5*><B>Age of the Ancients Who Listing</B><P>\n\r");
buf2 = parse_quotes(buf2);
send_buf(wdesc->fd,buf2,FALSE);

/* Decided to change this, to handle the Who stuff right here. --GW */
/*not */
/*
send_buf(wdesc->fd,"<BR>|}*-=-=-=-=-=-=-=-[ Current Adventurers of the Ancients ]-=-=-=-=-=-=-=-*{|</CENTER><BR><BR>",FALSE);

send_buf(wdesc->fd,"<TABLE>",FALSE);
for( desc = first_descriptor; desc; desc = next_desc )
{
   next_desc = desc->next;

   if ( desc->connected != CON_PLAYING )
    continue;

   ch = desc->character;

   if ( IS_SET( ch->pcdata->flagstwo, MOREPC_INCOG ) ||
        IS_SET( ch->act, PLR_WIZINVIS ) )
   continue;

if ( IS_IMMORTAL(ch) )
{
        switch ( ch->level )
        {
        default: break;
*59*  case MAX_LEVEL -  0: class = "HYPERION";break;
*58*  case MAX_LEVEL -  1: class = "COUNCIL";break;
*57*  case MAX_LEVEL -  2: class = "HEAD GOD";break;
*56*  case MAX_LEVEL -  3: class = "GOD";     break;
*55*  case MAX_LEVEL -  4: class = "FATE";    break;
*54*  case MAX_LEVEL -  5: class = "TITAN";   break;
*53*  case MAX_LEVEL -  6: class = "LORD";    break;
*52*  case MAX_LEVEL -  7: class = "WIZARD";  break;
*51*  case MAX_LEVEL -  8: class = "EMPATH";  break;
        }
}

if ( ch->pcdata->clan )
{
  sprintf(clanbuf,"%s",ch->pcdata->clan->whoname);
  clan = STRALLOC(clanbuf);
  fclan = TRUE;
}
else
{
  clan = STRALLOC( " " );
  fclan = FALSE;
}

   send_buf(wdesc->fd,"<TR>",FALSE);

   if ( fclan )
   sprintf(buf,"<TD><FONT FACE=*Ariel,Helvetica*><FONT SIZE=-1>%s</FONT></FONT></TD>", !IS_IMMORTAL(ch) ? color_filter(clan) : class );
   else
   sprintf(buf,"<TD><FONT FACE=*Ariel,Helvetica*><FONT SIZE=-1>%s</FONT></FONT></TD>", !IS_IMMORTAL(ch) ? clan : class );

   buf2 = STRALLOC(buf);
   buf2 = parse_quotes(buf2);
   send_buf(wdesc->fd,buf2,FALSE);
  

   sprintf(titlebuf,"%s",ch->pcdata->title);
   title = STRALLOC(titlebuf);
   sprintf(buf,"<TD><FONT FACE=*Ariel,Helvetica*><FONT SIZE=-1>%s</FONT></FONT></TD>",color_filter(title));
   buf2 = STRALLOC(buf);
   buf2 = parse_quotes(buf2);
   send_buf(wdesc->fd,buf2,FALSE);
 
   send_buf(wdesc->fd,"</TR>",FALSE);
   cnt++;
}
send_buf(wdesc->fd,"</TABLE>",FALSE);
sprintf(buf,"Current Players Online: %d<BR>",cnt);    
send_buf(wdesc->fd,buf,FALSE);   
*/   


do_who(NULL,"");

/*
 * Modified version of Show File, used in here --GW
 */

    if ( (fp = fopen( WHO_FILE, "r" )) != NULL )
    {
      while ( !feof(fp) )
      {
        while ((buf[num]=fgetc(fp)) != EOF
        &&      buf[num] != '\n'
        &&      buf[num] != '\r'
        &&      num < (MAX_STRING_LENGTH-2))
          num++;
        c = fgetc(fp);
        if ( (c != '\n' && c != '\r') || c == buf[num] )
          ungetc(c, fp);
        buf[num++] = '\n';
        buf[num++] = '\r';
        buf[num  ] = '\0';

        if ( strlen(buf) > 32000 )
        {
         bug("Strlen Greater then 32000: show_file",0);
         buf[32000] = '\0';
        }
        num = 0;
      send_buf(wdesc->fd,buf,TRUE);
      send_buf(wdesc->fd,"<BR>",FALSE);  /*Equiv to /n/r --GW */
      }
new_fclose(fp);
    }

return; 
}

void handle_web_wwwwho_request(WEB_DESCRIPTOR *wdesc)
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char *buf2;
    int c;
    int num = 0;

  /* Well .. why have 2 copies of your who? Smaug already supports
     webwho, however wont give anyone the cgi for it .. so here we go!
	--GW */

send_buf(wdesc->fd,"<HTML><HEAD><TITLE>Age of the Ancients -- WWW-Who Listing</TITLE></HEAD>\n\r", FALSE);
buf2 = STRALLOC("<BODY BACKGROUND=*http://www.ancients.org/PAoA1.jpg*BGCOLOR=*#000000* TEXT=*#F8FFFA* LINK=*#9E9295* VLINK=*#B5B5B5*><font face=*Arial*><B>Age of the Ancients WWW-Who Listing</B><P>\n\r");
buf2 = parse_quotes(buf2);
send_buf(wdesc->fd,buf2,FALSE);

do_who(NULL,"www");

/*
 * Modified version of Show File, used in here --GW
 */

    if ( (fp = fopen( WEBWHO_FILE, "r" )) != NULL )
    {
      while ( !feof(fp) )
      {
        while ((buf[num]=fgetc(fp)) != EOF
        &&      buf[num] != '\n'
        &&      buf[num] != '\r'
        &&      num < (MAX_STRING_LENGTH-2))
          num++;
        c = fgetc(fp);
        if ( (c != '\n' && c != '\r') || c == buf[num] )
          ungetc(c, fp);
        buf[num++] = '\n';
        buf[num++] = '\r';
        buf[num  ] = '\0';

        if ( strlen(buf) > 32000 )
        {
         bug("Strlen Greater then 32000: show_file",0);
         buf[32000] = '\0';
        }
        num = 0;
      buf2 = STRALLOC("<font face=*Arial*>");
      buf2 = parse_quotes(buf2);
      send_buf(wdesc->fd,buf2,FALSE);
      send_buf(wdesc->fd,buf,TRUE);
      send_buf(wdesc->fd,"</font>",FALSE);
      send_buf(wdesc->fd,"<BR>",FALSE); /* Equiv to /n/r --GW */      
      }
new_fclose(fp);
    }
return; 
}

#define WEBNEWS_FILE SYSTEM_DIR "WEBNEWS"

void handle_web_news_request(WEB_DESCRIPTOR *wdesc)
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char *buf2;
    int c;
    int num = 0;

  /* Well .. why have 2 copies of your who? Smaug already supports
     webwho, however wont give anyone the cgi for it .. so here we go!
	--GW */

send_buf(wdesc->fd,"<HTML><HEAD><TITLE>Age of the Ancients -- WWW-NEWS Listing</TITLE></HEAD>\n\r", FALSE);
buf2 = STRALLOC("<BODY BACKGROUND=*http://www.ancients.org/PAoA1.jpg*BGCOLOR=*#000000* TEXT=*#F8FFFA* LINK=*#9E9295* VLINK=*#B5B5B5*><font face=*Arial*><B>Age of the Ancients WWW-NEWS Listing</B><P>\n\r");
buf2 = parse_quotes(buf2);
send_buf(wdesc->fd,buf2,FALSE);

/*
 * Modified version of Show File, used in here --GW
 */

    if ( (fp = fopen( WEBNEWS_FILE, "r" )) != NULL )
    {
      while ( !feof(fp) )
      {
        while ((buf[num]=fgetc(fp)) != EOF
        &&      buf[num] != '\n'
        &&      buf[num] != '\r'
        &&      num < (MAX_STRING_LENGTH-2))
          num++;
        c = fgetc(fp);
        if ( (c != '\n' && c != '\r') || c == buf[num] )
          ungetc(c, fp);
        buf[num++] = '\n';
        buf[num++] = '\r';
        buf[num  ] = '\0';

        if ( strlen(buf) > 32000 )
        {
         bug("Strlen Greater then 32000: show_file",0);
         buf[32000] = '\0';
        }
        num = 0;
      buf2 = STRALLOC("<font face=*Arial*>");
      buf2 = parse_quotes(buf2);
      send_buf(wdesc->fd,buf2,FALSE);
      send_buf(wdesc->fd,buf,TRUE);
      send_buf(wdesc->fd,"</font>",FALSE);
      send_buf(wdesc->fd,"<BR>",FALSE); /* Equiv to /n/r --GW */      
      }
new_fclose(fp);
    }
return; 
}

#define WEBWIZLIST_FILE SYSTEM_DIR "WEBWIZLIST"

void handle_web_wizlist_request(WEB_DESCRIPTOR *wdesc)
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char *buf2;
    int c;
    int num = 0;

  /* Well .. why have 2 copies of your who? Smaug already supports
     webwho, however wont give anyone the cgi for it .. so here we go!
	--GW */

send_buf(wdesc->fd,"<HTML><HEAD><TITLE>Age of the Ancients -- Wizlist</TITLE></HEAD>\n\r", FALSE);
buf2 = STRALLOC("<BODY BACKGROUND=*http://www.ancients.org/PAoA1.jpg* BGCOLOR=*#000000* TEXT=*#F8FFFA* LINK=*#9E9295* VLINK=*#B5B5B5*><font face=*Arial*><B>Age of the Ancients Wizlist</B><P>\n\r");
buf2 = parse_quotes(buf2);
send_buf(wdesc->fd,buf2,FALSE);
do_who(NULL,"");

/*
 * Modified version of Show File, used in here --GW
 */

    if ( (fp = fopen( WEBWIZLIST_FILE, "r" )) != NULL )
    {
      while ( !feof(fp) )
      {
        while ((buf[num]=fgetc(fp)) != EOF
        &&      buf[num] != '\n'
        &&      buf[num] != '\r'
        &&      num < (MAX_STRING_LENGTH-2))
          num++;
        c = fgetc(fp);
        if ( (c != '\n' && c != '\r') || c == buf[num] )
          ungetc(c, fp);
        buf[num++] = '\n';
        buf[num++] = '\r';
        buf[num  ] = '\0';

        if ( strlen(buf) > 32000 )
        {
         bug("Strlen Greater then 32000: show_file",0);
         buf[32000] = '\0';
        }
        num = 0;
      /* Lets Center the Who list --GW */
/*      send_buf(wdesc->fd,"<CENTER>",FALSE);*/
      send_buf(wdesc->fd,buf,FALSE);
      send_buf(wdesc->fd,"<BR>",FALSE); /* Equiv to /n/r --GW */      
/*      send_buf(wdesc->fd,"</CENTER>",FALSE);*/
      send_buf(wdesc->fd,"</font>",FALSE);
      }
new_fclose(fp);
    }
return; 
}

/* The Mem functions that did reside here, arnt needed anymore,
    so ill save the real estate. =P --GW */

/* Rip out the Smaug Color Sequences --GW */
/* Hey -- I didnt sayit was perfect.. a hack at most*/
char *color_filter( char *string )
{
int c;
char temp[MSL];
char *temp2;

temp2 = string;

for ( c = 0;  temp2[c] != '\0' ; c++ )
{
/* First Scan for color .. then others --GW */
 if ( temp2[c] == '&' || temp2[c] == '^' )
 {
  temp2[c] = '%';
  temp2[c+1] = 's';
  sprintf(temp,temp2,"");
  temp2 = STRALLOC(temp);
  c = -1; /* found 1 .. start again */
 }
}

return temp2;
}

/*
 * Many thanks to Altrag who contributed this function! --GW
 */
char *text2html(const char *ip)
{
  static struct
  {
    const char *text;
    const char *html;
    int tlen, hlen;
  } convert_table[] =
  { { "<", "&lt;" }, { ">", "&gt;" }, { "&", "&amp;" }, { "\"", "&quot;" },
    { " ", "&nbsp;" }, { NULL, NULL } };

  static char buf[MAX_STRING_LENGTH*2];/* Safety here .. --GW */
  char *bp = buf;
  int i;

  if (!convert_table[0].tlen)
  {
    for (i = 0; convert_table[i].text; ++i)
    {
      convert_table[i].tlen = strlen(convert_table[i].text);
      convert_table[i].hlen = strlen(convert_table[i].html);
    }
  }
  while (*ip)
  {
    for (i = 0; convert_table[i].text; ++i)
      if (!strncmp(ip, convert_table[i].text, convert_table[i].tlen))
        break;
    if (convert_table[i].text)
    {
      strcpy(bp, convert_table[i].html);
      bp += convert_table[i].hlen;
      ip += convert_table[i].tlen;
    }
    else
      *bp++ = *ip++;
  }
  *bp = '\0';
  return buf;
}

char *parse_quotes( char *arg)
{
int str;

for ( str = 0; arg[str] != '\0'; str++ )
{
if ( arg[str] == '*' )
arg[str] = '"';
}

return arg;
}

