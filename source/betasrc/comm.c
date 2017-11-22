/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 3.0 ** 				    *
 *--------------------------------------------------------------------------*
 *               WolfPaw 3.0 (c) 1997 - 1999 by Dale Corse		    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *                           Scryn, Thoric, Justice, Tricops and Brogar.    *
 *--------------------------------------------------------------------------*
 *  Based on SMAUG 1.2a. Copyright 1994 - 1996 by Derek Snider		    *
 *  SMAUG Coding Team: Thoric, Altrag, Blodkai, Narn, Haus, Scryn, Rennard, *
 *		       Swordbearer, Gorog, Grishnakh and Tricops.           *
 *--------------------------------------------------------------------------*
 *  Merc 2.1 Diku MUD Improvments (C) 1992 - 1993 by Michael Chastain,      *
 *  Michael Quan, and Michael Tse. 					    *
 *  Original Diku MUD (C) 1990 - 1991 by Sebastian Hammer, Michael Seifert, *
 *  Hans Hendrik Strfeldt, Tom Madsen, and Katja Nyboe. 		    *
 *--------------------------------------------------------------------------*
 *			 Low-level communication module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include "mud.h"
#include <sys/resource.h>
/*
 * Socket and TCP/IP stuff.
 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>

char                command[MAX_INPUT_LENGTH]; /* multi commands --GW*/

const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };

void    send_auth args( ( struct descriptor_data *d ) );
void    read_auth args( ( struct descriptor_data *d ) );
void    start_auth args( ( struct descriptor_data *d ) );
void    save_sysdata args( ( SYSTEM_DATA sys ) );
void    load_corpses args( ( void ) );
void    install_other_handlers args( ( void) );
void    examine_last_command args( (void) );
void	alarm_update args( (void) );
void    init_alarm_handler args( (void) );
void	add_refferal	args( ( char *name,char *reff, char *site, char *email ) );
int     get_mortal_num_desc args( (CHAR_DATA *ch) );
void    show_mode args( ( DESCRIPTOR_DATA *d ) );
void    challenge_cancel args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *argument ) );
void do_char_ressurect args ( ( CHAR_DATA *ch ) );
void do_char_notressurect args ( ( CHAR_DATA *ch ) );
CHAR_DATA *get_char_list( CHAR_DATA *ch );
bool write_to_desc_color( char *txt, DESCRIPTOR_DATA *d );
void show_overlimit(CHAR_DATA *ch);
void show_titlescreen(DESCRIPTOR_DATA *d);

/*
 * Global variables.
 */
DESCRIPTOR_DATA *   first_descriptor;	/* First descriptor		*/
DESCRIPTOR_DATA *   last_descriptor;	/* Last descriptor		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
int		    num_descriptors;
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    mud_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    new_player;		/* New Player Entry --GW	*/
sh_int		    MemLogCount;	/* Memlog Counter --GW */
sh_int		    MemLogMax;		/* Max Lines Allowed --GW */
sh_int		    RoomLogCount;	/* Roomlog Counter --GW */
sh_int		    RoomLogMax;		/* Max Lines Allowed --GW */
time_t              boot_time;
HOUR_MIN_SEC  	    set_boot_time_struct;
HOUR_MIN_SEC *      set_boot_time;
struct tm *         new_boot_time;
struct tm           new_boot_struct;
char		    str_boot_time[MAX_INPUT_LENGTH];
char		    lastplayercmd[MAX_INPUT_LENGTH*2];
time_t		    current_time;	/* Time of this pulse		*/
int                 port;               /* Port number to be used       */
int		    control;		/* Controlling descriptor	*/
int		    newdesc;		/* New descriptor		*/
fd_set		    in_set;		/* Set of desc's for reading	*/
fd_set		    out_set;		/* Set of desc's for writing	*/
fd_set		    exc_set;		/* Set of desc's with errors	*/
int 		    maxdesc;
char  *		    wpolcargbuf; /* for wp-olc */
extern	bool	    disable_timer_abort;
time_t  	    last_check;
char		    mudargs[MSL];
bool		    DISABLE_LIMIT_SCAN;

/*
 * OS-dependent local functions.
 */
void	game_loop		args( ( ) );
int	init_socket		args( ( int port ) );
void	new_descriptor		args( ( int new_desc ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
char    *get_multi_command      args((DESCRIPTOR_DATA *d,char *argument));



/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name, bool kick ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	flush_buffer		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void	free_desc		args( ( DESCRIPTOR_DATA *d ) );
void	display_prompt		args( ( DESCRIPTOR_DATA *d ) );
int	make_color_sequence	args( ( const char *col, char *buf,
					DESCRIPTOR_DATA *d ) );
void	set_pager_input		args( ( DESCRIPTOR_DATA *d,
					char *argument ) );
bool	pager_output		args( ( DESCRIPTOR_DATA *d ) );



void	mail_count		args( ( CHAR_DATA *ch ) );
int	number_kombat		args( ( void ) );

/*
 * Globals Now .. --GW
 */
struct timeval now_time;
bool fCopyOver = FALSE;

int main( int argc, char **argv )
{
    char buf[MSL];

    sysdata.boottime =0;

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    PULSE_PER_SECOND            = 4;
    num_descriptors		= 0;
    first_descriptor		= NULL;
    last_descriptor		= NULL;
    sysdata.NO_NAME_RESOLVING	= TRUE;
    sysdata.WAIT_FOR_AUTH	= TRUE;

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;
/*  gettimeofday( &boot_time, NULL);   okay, so it's kludgy, sue me :) */
    boot_time = time(0);         /*  <-- I think this is what you wanted */
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Init boot time.
     */
    set_boot_time = &set_boot_time_struct;
    set_boot_time->manual = 0;
    
    new_boot_time = update_time(localtime(&current_time));
    /* Copies *new_boot_time to new_boot_struct, and then points
       new_boot_time to new_boot_struct again. -- Alty */
    new_boot_struct = *new_boot_time;
    new_boot_time = &new_boot_struct;

    /* 18 hours */
    new_boot_time->tm_hour += 18;
    if(new_boot_time->tm_hour >= 24)
    {
      new_boot_time->tm_mday += 1;
      new_boot_time->tm_hour -= 24;
     }

    /* Update new_boot_time (due to day increment) */
    new_boot_time = update_time(new_boot_time);
    new_boot_struct = *new_boot_time;
    new_boot_time = &new_boot_struct;
    new_boot_time_t = mktime(new_boot_time);
    reboot_check(mktime(new_boot_time));

    /* Set reboot time string for do_time */
    log_string("Setting Reboot Time");
    get_reboot_string();

    /*
     * Reserve three channels for our use.
     */

    log_string("Initiating Memlog");
    rename( MEMLOG_FILE, "../system/memlog.lastboot" );
    unlink(MEMLOG_FILE);
    MemLogCount = 0; /* Current Lines of Memlog --GW */
    MemLogMax = 100; /* Maximum Allowed --GW */

    log_string("Initiating Roomlog");
    rename( ROOMLOG_FILE, "../system/roomlog.lastboot" );
    unlink(ROOMLOG_FILE);
    RoomLogCount = 0; /* Current Lines of Roomlog --GW */
    RoomLogMax = 100; /* Maximum Allowed --GW */
    
/*    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }*/
    if ( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 5000;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    exit( 1 );
	}

	fCopyOver=FALSE;
	DISABLE_LIMIT_SCAN=FALSE;

     /* This now handles all arguments --GW */
     /* Not all args are handled right here.. it is codewide.. dont freak =) */
     if (argv[2] && argv[2][0]) /* Ok we have arguments */
       {

	/* Kill the - */
	if ( argv[2][0] == '-' )
	argv[2]++;

 	sprintf(mudargs,"%s",argv[2]);

	/* List All */
	if ( mudarg_scan('?') )
	{
	  fprintf(stderr, "Usage: %s <port #> [-WLHBIwRS]\n\n",argv[0] );
	  fprintf(stderr, "Arguments:\n");
	  fprintf(stderr, "W - Warmboot (Used by SYSTEM only)\n");
	  fprintf(stderr, "L - Disable Limit Scan\n");
	  fprintf(stderr, "H - Show Help File Loading\n");
	  fprintf(stderr, "B - Builder Port\n");
	  fprintf(stderr, "w - No WebServer\n");
	  fprintf(stderr, "S - Show Limit Scan\n");
	  fprintf(stderr, "\n");
	  exit(0);
	}

	if ( mudarg_scan('L') ) /* Disable Limit Scan (used when debugging mud) */
	  DISABLE_LIMIT_SCAN=TRUE;
	  
	/* Warmboot */
	if ( mudarg_scan('W') )
        {
           fCopyOver = TRUE;
           control = atoi(argv[3]);
        }
     }

    }

      /*
       * Run the game.
       */
      log_string("Booting Database");
      init_alarm_handler();
      unlink(LOG_FILE);
      boot_db(fCopyOver);
      install_other_handlers (); /* crash-log call */
      sprintf(buf,"Initializing sockets: %d %d %d %d",port,port+1,port+15,port+16);
      log_string(buf);
      //wizlock = TRUE;/* wizlock temperarily*/ 
      /*noenforce = TRUE; noenforce temp */
      new_player = FALSE; /* New Player False at First! */
     if (!fCopyOver) /* We have already the port if copyover'ed */
      {
         control = init_socket(port);
      }

      if ( sysdata.webserv == 1 && !mudarg_scan('w') )
      {
      sprintf(buf,"Booting WebServer on port %d",WEBPORT);
      log_string(buf);
      init_web(WEBPORT); 
      }

      sprintf( log_buf,"The Age of the Ancients is ready too ROCK on port %d.",port );
      log_string( log_buf );

sprintf(buf,"*** Booted with PID %d ***",getpid());
log_string(buf);

    game_loop( );
    if ( sysdata.webserv == 1 )
    {
    sprintf(buf,"Closing WebServer Port (%d)",WEBPORT); 
    log_string(buf);
    shutdown_web(); 
    }


    close( control  );
    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}


int init_socket( int port )
{
    char hostname[64];
    struct sockaddr_in	 sa;
    struct hostent	*hp;
    struct servent	*sp;
    int x = 1;
    int fd;

    gethostname(hostname, sizeof(hostname));

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
		    (void *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close( fd );
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
			(void *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close( fd );
	    exit( 1 );
	}
    }
#endif

    hp = gethostbyname( hostname );
    sp = getservbyname( "service", "mud" );
    memset(&sa, '\0', sizeof(sa));
    sa.sin_family   = AF_INET; /* hp->h_addrtype; */
    sa.sin_port	    = htons( port );
    /*sa.sin_addr.s_addr = htonl(INADDR_ANY);*/

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) == -1 )
    {
	perror( "Init_socket: bind" );
	close( fd );
	exit( 1 );
    }

    if ( listen( fd, 50 ) < 0 )
    {
	perror( "Init_socket: listen" );
	close( fd );
	exit( 1 );
    }

    return fd;
}
 void init_descriptor( DESCRIPTOR_DATA *dnew, int desc)
 {
     dnew->next         = NULL;
     dnew->descriptor   = desc;
     dnew->connected    = CON_GET_NAME;
     dnew->outsize      = 2000;
     dnew->idle         = 0;
     dnew->lines                = 0;
     dnew->scrlen       = 24;
     dnew->user         = STRALLOC("unknown");
     dnew->auth_fd      = -1;
     dnew->auth_inc     = 0;
     dnew->auth_state   = 0;
     dnew->newstate     = 0;
     dnew->prevcolor    = 0x07;

     CREATE( dnew->outbuf, char, dnew->outsize );
 }

/* Write last command */
void write_last_command ()
{
    FILE *fd=NULL;
    FILE *fd2=NULL;

    /* Return if no last command - set before normal exit */
    if (!last_command[0])
        return;

        if ( ( fd = fopen( LAST_COMMAND_FILE, "a" ) ) == NULL )
        {
            bug("Cannot Open LAST COMMAND File!",0);
            perror( LAST_COMMAND_FILE );
        }
        else
        {
        fprintf(fd,"%s\n",last_command);
        new_fclose(fd);
        }

	if ( ( fd2 = fopen( LAST_FUNCTION_FILE, "a" ) ) == NULL )
	{
	    bug("Cannot open LAST_FUNCTION_FILE",0);
	    perror(LAST_FUNCTION_FILE);
	}
	else
	{
	fprintf(fd2,"%s\n",last_function_call);
	new_fclose(fd2);
	}

return;
}

void record_call(char * call_name )
{
    sprintf (last_function_call, "[%s]",
        call_name );
return;
}

/*
 * System Panic handler --GW
 */
void syspanic( )
{
DESCRIPTOR_DATA *d;
char *args[]={ "Start.pl",NULL };

for( d = first_descriptor; d; d = d->next )
{
write_to_descriptor(d->descriptor,"WARNING: SYSTEM PANIC -- SHUTTING DOWN!!!\n",0);
write_to_descriptor(d->descriptor,"SYS-PANIC: Panic Saving!\n",0);
save_char_obj(d->character);
write_to_descriptor(d->descriptor,"Character Information Saved.\n",0);
write_to_descriptor(d->descriptor,"SYS-PANIC: Exiting... Come back in a Minute!\n",0);
close_socket(d,TRUE);
}
close(control);
execvp( "/home/aota/aoa/system/start.pl" ,args );
log_string("Reboot Failed!");
abort( );
}

void nasty_signal_handler (int no)
{
//    write_last_command();
//    examine_last_command();
	/*syspanic();*/  /* System is in a PANIC state! --GW */
}
/* Call this before starting the game_loop */
void install_other_handlers ()
{
    last_command[0] = '\0';

    if (atexit (write_last_command) != 0)
    {
        perror ("install_other_handlers:atexit");
        exit (1);
    }

    /* should probably check return code here */
//    signal (SIGSEGV, nasty_signal_handler);

    /* Possibly other signals could be caught? */
}

void tick_update()
{
if ( TURBO_NUM != PULSE_PER_SECOND )
{
PULSE_PER_SECOND = TURBO_NUM;
}
return;
}
static void SegVio()
{
  CHAR_DATA *ch;
  char buf[MAX_STRING_LENGTH];

  write_last_command();
  log_string( "SEGMENTATION VIOLATION" );
  log_string( lastplayercmd );
  for ( ch = first_char; ch; ch = ch->next )
  {
    sprintf( buf, "%cPC: %-20s room: %d", IS_NPC(ch) ? 'N' : ' ',
    		ch->name, ch->in_room->vnum );
    log_string( buf );  
  }
  exit(0);
}


/*
 * LAG alarm!							-Thoric
 */
static void caught_alarm()
{
    char buf[MAX_STRING_LENGTH];
    bug( "ALARM CLOCK!" );
    strcpy( buf, "Alas, the hideous malevalent entity known only as 'Lag' rises once more!\n\r" );
    echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
    if ( newdesc )
    {
	FD_CLR( newdesc, &in_set );
	FD_CLR( newdesc, &out_set );
	log_string( "clearing newdesc" );
    }

    /* If were being spoofed, intentionally or not.. Disable name lookup
      --GW*/
    log_string("System Activating Global Mode: Name Resolving Disabled.");
    sysdata.NO_NAME_RESOLVING	= FALSE;

    game_loop( );
    close( control );

    log_string( "Normal termination of game." );
    exit( 0 );
}

bool check_bad_desc( int desc )
{
    if ( FD_ISSET( desc, &exc_set ) )
    {
	FD_CLR( desc, &in_set );
	FD_CLR( desc, &out_set );
	log_string( "Bad FD caught and disposed." );
	return TRUE;
    }
    return FALSE;
}


void accept_new( int ctrl )
{
	static struct timeval null_time;
	DESCRIPTOR_DATA *d;
	/* int maxdesc; Moved up for use with id.c as extern */

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( ctrl, &in_set );
	maxdesc	= ctrl;
	newdesc = 0;
	for ( d = first_descriptor; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	    if (d->auth_fd != -1)
	    {
		maxdesc = UMAX( maxdesc, d->auth_fd );
		FD_SET(d->auth_fd, &in_set);
		if (IS_SET(d->auth_state, FLAG_WRAUTH))
		  FD_SET(d->auth_fd, &out_set);
	    }
	    if ( d == last_descriptor )
	      break;
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "accept_new: select: poll" );
	    exit( 1 );
	}

	if ( FD_ISSET( ctrl, &exc_set ) )
	{
	    bug( "Exception raise on controlling descriptor %d", ctrl );
	    FD_CLR( ctrl, &in_set );
	    FD_CLR( ctrl, &out_set );
	}
	else
	if ( FD_ISSET( ctrl, &in_set ) )
	{
	    newdesc = ctrl;
	    new_descriptor( newdesc );
	}
}

void game_loop( )
{
    struct timeval	  last_time;
    char cmdline[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
/*  time_t	last_check = 0;  */

    signal( SIGPIPE, SIG_IGN );
    signal( SIGALRM, caught_alarm );

if ( sysdata.segvio == 1 )
    signal( SIGSEGV, SegVio );
/*  else
    signal( SIGSEGV, syspanic );*/
    
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !mud_down )
    {
	accept_new( control  );
	alarm_update(); /*<--- deadlock code*/
	tick_update(); /*TURBO FIGHTING SPEED!!! --GW*/

	if ( sysdata.webserv == 1 )
	handle_web(); /*<--- WebServer Handler */

	/*
	 * Kick out descriptors with raised exceptions
	 * or have been idle, then check for input.
	 */
	for ( d = first_descriptor; d; d = d_next )
	{
	    if ( d == d->next )
	    {
	      bug( "descriptor_loop: loop found & fixed" );
	      d->next = NULL;
	    }
 	    d_next = d->next;   

	    d->idle++;	/* make it so a descriptor can idle out */
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character
		&& ( d->connected == CON_PLAYING
		||   d->connected == CON_EDITING ) )
		    save_char_obj( d->character );
		d->outtop	= 0;
		close_socket( d, TRUE );
		continue;
	    }
	    else 
	    if ( (!d->character && d->idle > 360)		  /* 2 mins */
            ||   ( d->connected != CON_PLAYING && d->idle > 4800) /* 15 mins */
	    ||     d->idle > 28800 ) 				  /* 2 hrs  */
	    {
		write_to_descriptor( d->descriptor,
		 "Idle timeout... disconnecting.\n\r", 0 );
		d->outtop	= 0;
		close_socket( d, TRUE );
		continue;
	    }
	    else
	    {
		d->fcommand	= FALSE;

		if ( FD_ISSET( d->descriptor, &in_set ) )
		{
			d->idle = 0;
			if ( d->character )
			  d->character->timer = 0;
			if ( !read_from_descriptor( d ) && d->descriptor )
			{
			    FD_CLR( d->descriptor, &out_set );
			    if ( d->character
			    && ( d->connected == CON_PLAYING
			    ||   d->connected == CON_EDITING ) )
				save_char_obj( d->character );
			    d->outtop	= 0;
			    close_socket( d, FALSE );
			    continue;
			}
		}
		/* IDENT authentication */
/*	        if ( ( d->auth_fd == -1 ) && ( d->atimes < 20 ) 
		&& !str_cmp( d->user, "unknown" ) )
		   start_auth( d );

		if ( d->auth_fd != -1)
		{
		   if ( FD_ISSET( d->auth_fd, &in_set ) )
		   {
			read_auth( d );
//			if ( !d->auth_state ) 
//			    check_ban( d );
		   }
		   else
		   if ( FD_ISSET( d->auth_fd, &out_set )
		   && IS_SET( d->auth_state, FLAG_WRAUTH) )
		   {
			send_auth( d );
//			 if ( !d->auth_state )
//			  check_ban( d );
		   }
		}*/

		if ( d->character && d->character->wait > 0 )
		{
			--d->character->wait;
			continue;
		}

		if ( d->character && d->character->stun_rounds > 0 )
	        {
		   if ( d->character->fighting && d->character->fighting->who )
		        continue;
		   else
		       d->character->stun_rounds = 0;
	        }

            if ( d->incomm[0] == '\0' )
		read_from_buffer( d );

		if ( d->incomm[0] != '\0' )
		{
                char *command2;

			d->fcommand	= TRUE;
			stop_idling( d->character );

		       strcpy( cmdline, d->incomm );
			d->incomm[0] = '\0';
			

			if( d->character )
			  set_cur_char( d->character );

	                command2 = get_multi_command( d, cmdline );

			if ( d->pagepoint )
			  set_pager_input(d, cmdline);
			else
			  switch( d->connected )
			  {
			   default:
 				nanny( d, cmdline );
				break;
			   case CON_PLAYING:
       	                        substitute_alias( d, command2 );
				break;
			   case CON_EDITING:
				edit_buffer( d->character, command2 );
				break;
			   case CON_GREDITING:
				gredit_engine( d, command2 );
				break;
			  }
		}
	    }
	    if ( d == last_descriptor )
	      break;
	}

	/*
	 * Autonomous game motion.
	 */
	update_handler( );

	/*
	 * Check REQUESTS pipe
	 */
        check_requests( );

	/*
	 * Output.
	 */
	for ( d = first_descriptor; d; d = d_next )
	{
	    d_next = d->next;   

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
	        if ( d->pagepoint )
	        {
	          if ( !pager_output(d) )
	          {
	            if ( d->character
	            && ( d->connected == CON_PLAYING
	            ||   d->connected == CON_EDITING ) )
	                save_char_obj( d->character );
	            d->outtop = 0;
	            close_socket(d, FALSE);
	          }
	        }
		else if ( !flush_buffer( d, TRUE ) )
		{
		    if ( d->character
		    && ( d->connected == CON_PLAYING
		    ||   d->connected == CON_EDITING ) )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d, FALSE );
		}
	    }
	    if ( d == last_descriptor )
	      break;
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;

        /* Check every 10 Mins... */
	if ( last_check+600 < current_time )
	{
	  CHECK_LINKS(first_descriptor, last_descriptor, next, prev,
	      DESCRIPTOR_DATA);
	  last_check = current_time;
	}
    }
    return;
}


void new_descriptor( int new_desc )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew=NULL;
    BAN_DATA *pban;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    int size;
  /*  char bugbuf[MAX_STRING_LENGTH]; */

    set_alarm( 20 );
    size = sizeof(sock);
    if ( check_bad_desc( new_desc ) )
    {
      set_alarm( 0 );
      return;
    }
    set_alarm( 20 );
    if ( ( desc = accept( new_desc, (struct sockaddr *) &sock, &size) ) < 0 )
   {
   	perror( "New_descriptor: accept" );
  /*	sprintf(bugbuf, "[*****] BUG: New_descriptor: accept");
     	log_string_plus( bugbuf, LOG_COMM, sysdata.log_level );
  */	set_alarm( 0 );
  	return;
    } 
    if ( check_bad_desc( new_desc ) )
    {
      set_alarm( 0 );
      return;
    }
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    set_alarm( 20 );
    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	set_alarm( 0 );
	return;
    }
    if ( check_bad_desc( new_desc ) )
      return;
    CREATE( dnew, DESCRIPTOR_DATA, 1 );

    init_descriptor(dnew, desc );
    dnew->port = ntohs(sock.sin_port);


    strcpy( buf, inet_ntoa( sock.sin_addr ) );
    sprintf( log_buf, "Sock.sinaddr: desc: %hd  %s, port %hd.",
		dnew->descriptor,buf, dnew->port );
    log_string_plus( log_buf, LOG_COMM, 59 );
    if ( sysdata.NO_NAME_RESOLVING )
      dnew->host = STRALLOC( buf );
    else
    {
       from = gethostbyaddr( (char *) &sock.sin_addr,
	  	sizeof(sock.sin_addr), AF_INET );
       dnew->host = STRALLOC( (char *)( from ? from->h_name : buf) );
    }
 
 /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */ 
  
    for ( pban = first_ban; pban; pban = pban->next )
    {
        if ( !str_prefix( pban->name, dnew->host ) )
        {
         if ( pban->level >= LEVEL_IMMORTAL )
	 {
	    log_string("[***] Closing link to BANNED site. [***]");
            write_to_descriptor( desc,"Your site is prohibited from logging in at this time.\n\r", 0 );
	    write_to_descriptor( desc,"Email: aota@ancients.org for details.\n\r",0);
            close( desc );
	    free_desc( dnew );
	    set_alarm( 0 );
	    return;
	 }
	}

        if ( !str_suffix( pban->name, dnew->host ) )
        {
         if ( pban->level >= LEVEL_IMMORTAL )
	 {
	    log_string("[***] Closing link to BANNED site. [***]");
            write_to_descriptor( desc,
                "Your site has been banned from this Mud.\n\r", 0 );
            close( desc );
	    free_desc( dnew );
	    set_alarm( 0 );
	    return;
	 }
        }
     }
	

    /*
     * Init descriptor data.
     */

    if ( !last_descriptor && first_descriptor )
    {
	DESCRIPTOR_DATA *d;

	bug( "New_descriptor: last_desc is NULL, but first_desc is not! ...fixing" );
	for ( d = first_descriptor; d; d = d->next )
	   if ( !d->next )
		last_descriptor = d;
    }

    LINK( dnew, first_descriptor, last_descriptor, next, prev );

    start_auth( dnew ); /* Start username authorization */

    if ( ++num_descriptors > sysdata.maxplayers )
	sysdata.maxplayers = num_descriptors;
    if ( sysdata.maxplayers > sysdata.alltimemax )
    {
	if ( sysdata.time_of_max )
	  DISPOSE(sysdata.time_of_max);
	sprintf(buf, "%24.24s", ctime(&current_time));
	sysdata.time_of_max = str_dup(buf);
	sysdata.alltimemax = sysdata.maxplayers;
	sprintf( log_buf, "Broke all-time maximum player record: %d", sysdata.alltimemax );
	log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
	to_channel( log_buf, CHANNEL_MONITOR, "[ ** ", LEVEL_IMMORTAL );
	save_sysdata( sysdata );
    }

	show_titlescreen(dnew);

    set_alarm(0);
    return;
}


/*
 * Send the greeting.
 */
void show_titlescreen(DESCRIPTOR_DATA *d)
{
	extern char * help_greeting[3];
        int greetnum;

	sysdata.logins++; 

	greetnum = dice( 1, 3 );
        write_to_buffer(d, "\E[2J", 4);
	if ( help_greeting[greetnum-1][0] == '.' )
	    write_to_desc_color( help_greeting[greetnum-1]+1, d );
	else
	    write_to_desc_color( help_greeting[greetnum-1], d );
return;
}

/*
 * Added Safty checks here.. to prevent it for some reason DESPOSE'ing
 * a null pointer .... so it dont crash .. -- GW
 */
void free_desc( DESCRIPTOR_DATA *d )
{
    close( d->descriptor );

        if ( d->host )
        STRFREE( d->host );

	if ( d->outbuf )
        DISPOSE( d->outbuf );

	if ( d->user )
        STRFREE( d->user );    /* identd */

	if ( d->pagebuf )
	DISPOSE( d->pagebuf );

	if ( d )
        DISPOSE( d );
    return;
}

void close_socket( DESCRIPTOR_DATA *dclose, bool force )
{
    CHAR_DATA *ch;
    DESCRIPTOR_DATA *d;
    bool DoNotUnlink = FALSE;
  
    if( !dclose )
      return;

    /* flush outbuf */
    if ( !force && dclose->outtop > 0 )
	flush_buffer( dclose, FALSE );

    /* say bye to whoever's snooping this descriptor */
    if ( dclose->snoop_by )
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );

    /* stop snooping everyone else */
    for ( d = first_descriptor; d; d = d->next )
	if ( d->snoop_by == dclose )
	  d->snoop_by = NULL;

    /* Check for switched people who go link-dead. -- Altrag */
/*    if ( dclose->original )
    {
	if ( ( ch = dclose->character ) != NULL )
	  do_return(ch, "");
	else
	{
	  bug( "Close_socket: dclose->original without character %s",
		(dclose->original->name ? dclose->original->name : "unknown") );
	  dclose->character = dclose->original;
	  dclose->original = NULL;
	}
    }*/
    
    ch = dclose->character;

    /* sanity check :( */
    if ( !dclose->prev && dclose != first_descriptor )
    {
	DESCRIPTOR_DATA *dp, *dn;
	bug( "Close_socket: %s desc:%p != first_desc:%p and desc->prev = NULL!",
		ch ? ch->name : d->host, dclose, first_descriptor );
	dp = NULL;
	for ( d = first_descriptor; d; d = dn )
	{
	   dn = d->next;
	   if ( d == dclose )
	   {
		bug( "Close_socket: %s desc:%p found, prev should be:%p, fixing.",
		    ch ? ch->name : d->host, dclose, dp );
		dclose->prev = dp;
		break;
	   }
	   dp = d;
	}
	if ( !dclose->prev )
	{
	    bug( "Close_socket: %s desc:%p could not be found!.",
		    ch ? ch->name : dclose->host, dclose );
	    DoNotUnlink = TRUE;
	}
    }
    if ( !dclose->next && dclose != last_descriptor )
    {
	DESCRIPTOR_DATA *dp, *dn;
	bug( "Close_socket: %s desc:%p != last_desc:%p and desc->next = NULL!",
		ch ? ch->name : d->host, dclose, last_descriptor );
	dn = NULL;
	for ( d = last_descriptor; d; d = dp )
	{
	   dp = d->prev;
	   if ( d == dclose )
	   {
		bug( "Close_socket: %s desc:%p found, next should be:%p, fixing.",
		    ch ? ch->name : d->host, dclose, dn );
		dclose->next = dn;
		break;
	   }
	   dn = d;
	}
	if ( !dclose->next )
	{
	    bug( "Close_socket: %s desc:%p could not be found!.",
		    ch ? ch->name : dclose->host, dclose );
	    DoNotUnlink = TRUE;
	}
    }

    if ( dclose->character )
    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->level ) );

	if ( dclose->connected == CON_DEATH_SELECT )
	{
        sprintf(log_buf,"Dropping %s 1 level for link loss on death select!",QUICKLINK(dclose->character->name));
	log_string(log_buf);
	do_char_notressurect(dclose->character);
	do_quit(dclose->character,"");
	return;
	}

	if ( dclose->connected == CON_PLAYING
	||   dclose->connected == CON_EDITING )
	{   
	    if ( IS_SET( dclose->character->pcdata->flags, PCFLAG_KOMBAT ) )
	    {
	       char buf[MSL];
	       int cnt=0;
	    sprintf(buf,"&R<&BMortal Kombat&R> %s Slaughtered&B by thier own link!",capitalize(dclose->character->name));
	    echo_to_all(AT_PLAIN,buf,ECHOTAR_ALL);
	    cnt = number_kombat();
	    REMOVE_BIT(dclose->character->pcdata->flags, PCFLAG_KOMBAT);
	    char_from_room(dclose->character);
	    char_to_room(dclose->character,get_room_index(30601,1) );
	    do_save(dclose->character,"");
	    if ( cnt == 1 )
	    {
           sprintf(buf,"&R<&BMortal Kombat&R>&R %s &Bis &RViKtorious&B!",
                  capitalize(ch->name));
           echo_to_all(AT_PLAIN,buf,ECHOTAR_ALL);
           char_from_room(ch);
           char_to_room(ch,get_room_index(30601,1) );
           REMOVE_BIT( ch->pcdata->flags, PCFLAG_KOMBAT );
           ch->pcdata->num_kombats_won++;
           REMOVE_BIT( kombat->bits, KOMBAT_ON );
           REMOVE_BIT( kombat->bits, KOMBAT_DUAL );
           REMOVE_BIT( kombat->bits, KOMBAT_ADVANCED );
           REMOVE_BIT( kombat->bits, KOMBAT_NODRAGON );
           REMOVE_BIT( kombat->bits, KOMBAT_NOAVATAR );
           send_to_char("Your Quest Points Raise by 20!\n\r",ch);
           ch->pcdata->quest_curr += 20;
           ch->pcdata->quest_accum += 20;
           do_save(ch,"");
	   }
	    }
	    act( AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );	  
            ch->desc = NULL; 
	}
	else
	{
	    /* clear descriptor pointer to get rid of bug message in log */
	    dclose->character->desc = NULL;
	    if ( dclose && dclose->character )
	    free_char( dclose->character );
	}
    }

    if ( !DoNotUnlink )
    {
	/* make sure loop doesn't get messed up */
	if ( d_next == dclose )
	  d_next = d_next->next;
	UNLINK( dclose, first_descriptor, last_descriptor, next, prev );
    }

    if ( dclose->descriptor == maxdesc )
      --maxdesc;
    if ( dclose->auth_fd != -1 ) 
      close( dclose->auth_fd );

    free_desc( dclose );
    --num_descriptors;
    return;
}


bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    if ( !d )
	return FALSE;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
        if ( d && d->descriptor )
	{
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	 //  close_socket(d, TRUE);
	}
	return FALSE;
     }

    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string_plus( "EOF encountered on read.", LOG_COMM, sysdata.log_level );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r' && i<MAX_INBUF_SIZE;
	  i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= 254 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    /*
	    for ( ; d->inbuf[i] != '\0' || i>= MAX_INBUF_SIZE ; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    */
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
    if ( k > 1 || d->incomm[0] == '!' )
    {
	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if ( ++d->repeat >= 50 )
	    {
/*		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
*/
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
			close_socket(d,TRUE);
	    }
	}
    }

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool flush_buffer( DESCRIPTOR_DATA *d, bool fPrompt )
{
    char buf[MAX_INPUT_LENGTH];
    extern bool mud_down;

    /*
     * If buffer has more than 4K inside, spit out .5K at a time   -Thoric
     */
    if ( !mud_down && d->outtop > 4096 )
    {
        memcpy( buf, d->outbuf, 512 );
        memmove( d->outbuf, d->outbuf + 512, d->outtop - 512 );
        d->outtop -= 512;
	if ( d->snoop_by ||
	    ( d->character && d->connected==CON_PLAYING &&
	    ( IS_SET(d->character->pcdata->flags, PCFLAG_CHALLENGED ) ||
	      IS_SET(d->character->pcdata->flags, PCFLAG_CHALLENGER ) ) ) )
	{
	    char snoopbuf[MAX_INPUT_LENGTH];

	    if ( !d || !d->character )
	    {
		bug("Flush Buffer: NULL D->character! <--**FATAL!!!**",0);
	    }

	    buf[512] = '\0';
	    if ( d->character && d->character->name )
	    {
		if (d->original && d->original->name)
		    sprintf( snoopbuf, "%s (%s)", d->character->name, d->original->name );
		else          
		    sprintf( snoopbuf, "%s", d->character->name);
		if ( d->snoop_by )
		write_to_buffer( d->snoop_by, snoopbuf, 0);
	    }
	  if ( d->snoop_by )
	  {
	    write_to_buffer( d->snoop_by, "% ", 2 );
	    write_to_buffer( d->snoop_by, buf, 0 );
	  }
	  if ( d->character && d->connected == CON_PLAYING && in_arena( d->character ) )
	  {
	    write_to_gallery( d->character, snoopbuf );
	    write_to_gallery( d->character, "> " );
	    write_to_gallery( d->character, buf );
	  }
	}
        if ( !write_to_descriptor( d->descriptor, buf, 512 ) )
        {
	    d->outtop = 0;
	    return FALSE;
        }
        return TRUE;
    }
                                                                                        

    /*
     * Bust a prompt.
     */
    if ( fPrompt && !mud_down && d->connected == CON_PLAYING )
    {
	CHAR_DATA *ch;

	ch = d->original ? d->original : d->character;
	if ( IS_SET(ch->act, PLR_BLANK) )
	    write_to_buffer( d, "\n\r", 2 );

	if ( IS_SET(ch->act, PLR_PROMPT) )
	    display_prompt(d);
	if ( IS_SET(ch->act, PLR_TELNET_GA) )
	    write_to_buffer( d, go_ahead_str, 0 );
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by || 
       ( d->character && d->connected == CON_PLAYING && 
       ( IS_SET( d->character->pcdata->flags, PCFLAG_CHALLENGER ) ||
         IS_SET( d->character->pcdata->flags, PCFLAG_CHALLENGED ) ) ) )
    {
        /* without check, 'force mortal quit' while snooped caused crash, -h */
	if ( d->character && d->character->name )
	{
	    /* Show original snooped names. -- Altrag */
	    if ( d->original && d->original->name )
		sprintf( buf, "%s (%s)", d->character->name, d->original->name );
	    else
		sprintf( buf, "%s", d->character->name);

	    if ( d->snoop_by )
	    write_to_buffer( d->snoop_by, buf, 0);
	}

	if ( d->snoop_by )
	{
	write_to_buffer( d->snoop_by, "% ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
	}

	if ( d->character && d->connected == CON_PLAYING && in_arena(d->character ) )
	{
	  write_to_gallery(d->character,buf);
	  write_to_gallery(d->character,"> ");
	  write_to_gallery(d->character,d->outbuf );
	}
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    }
}



/*
 * Append onto an output buffer.
 */
bool write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
bool breakloop;

    if ( !d )
    {
	bug( "Write_to_buffer: NULL descriptor" );
	return FALSE;
    }

    /*
     * Normally a bug... but can happen if loadup is used.
     */
    if ( !d->outbuf )
    	return TRUE;

    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

/* Uncomment if debugging or something*/
/*    if ( length != strlen(txt) )
    {
	bug( "Write_to_buffer: length(%d) != strlen(txt)!", length );
	bug( "Text: %s ", txt );
	length = strlen(txt);
    }
*/

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

/* set breakloop to FALSE */
breakloop = FALSE;

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {

        if ( breakloop )
	break;

	if ( !d )
	{
        bug("Dynamic Buffer Overflow on a NULL DESCRIPTOR: Breaking loop");
        break;
        }

        if (d->outsize > 32000)
	{
	    /* empty buffer */
	    d->outtop = 0;
	    close_socket(d, TRUE);
	    bug("Buffer overflow. Closing (%s).", d->character ? d->character->name : "???" );
	    breakloop = TRUE;
	    continue;
 	}
	d->outsize *= 2;
	RECREATE( d->outbuf, char, d->outsize );
    }


   if ( !d || breakloop )
     return FALSE;

    /*
     * Copy.
     */
    strncpy( d->outbuf + d->outtop, txt, length );
    d->outtop += length;
    d->outbuf[d->outtop] = '\0';
    return TRUE;
}


/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    }

    return TRUE;
}



void show_title( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch;

    ch = d->character;

    if ( !IS_SET( ch->pcdata->flags, PCFLAG_NOINTRO ) )
    {
	if (IS_SET(ch->act, PLR_ANSI))
	  send_ansi_title(ch);
	else
	  send_ascii_title(ch);
    }
    else
    {
      write_to_buffer( d, "[***] If you haven't seen the NEW title screen, LOOK AT IT! [***]\n\r",0);
      write_to_buffer( d, "Press enter...\n\r", 0 );
    }
    d->connected = CON_PRESS_ENTER;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
/*	extern int lang_array[];
	extern char *lang_names[];*/
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA   *location;
    char loc2[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    char *pwdnew;
    char *p;
/*  ROOM_INDEX_DATA *in_room; */
/*    int iLang;*/
    bool fOld, chk;
    PERMBAN_DATA *pban;
    ZONE_DATA *zone;
    bool quit; 
    bool newconnect;
    ROOM_INDEX_DATA *test;
    DEITY_DATA *deity;
    char mus_buf[MSL];
    extern bool EMAIL_AUTH;

    newconnect = FALSE;
    quit = FALSE;
    strcpy(loc2,"25518");

    while ( isspace(*argument) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d, TRUE );
	return;
   
    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d, FALSE );
	    return;
	}

	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	    return;
	}

        if ( !str_cmp( argument, "New" ) )
	{
	    if (d->newstate == 0)
	    {
              /* New player */
              /* Don't allow new players if DENY_NEW_PLAYERS is true */
      	      if (sysdata.DENY_NEW_PLAYERS == TRUE)
      	      {
       		sprintf( buf, "The mud is currently preparing for a reboot.\n\r" );
      		write_to_buffer( d, buf, 0 );
		sprintf( buf, "New players are not accepted during this time.\n\r" );
		write_to_buffer( d, buf, 0 );
      		sprintf( buf, "Please try again in a few minutes.\n\r" );
      		write_to_buffer( d, buf, 0 );
		close_socket( d, FALSE );
             }

        for ( pban = first_ban; pban; pban = pban->next )
        {
          /* This used to use str_suffix, but in order to do bans by the 
             first part of the ip, ie "ban 207.136.25" str_prefix must
             be used. -- Narn
          */
	  if ( !str_suffix( pban->name, d->host ) && pban->level == 1 )
          {
             log_string("[***] Closing link to NEWBANNED site. [***]");
            write_to_buffer( d,
              "New Characters are not currently accepted from your site.\n\r", 0 );
	    write_to_buffer( d,
	      "E-Mail: aota@ancients.org if you would like a character.\n\r", 0 );
            close_socket( d, FALSE );
	    return;
	  }
	  if ( !str_prefix( pban->name, d->host ) && pban->level == 1 )
          {
             log_string("[***] Closing link to NEWBANNED site. [***]");
            write_to_buffer( d,
              "New Characters are not currently accepted from your site.\n\r", 0 );
	    write_to_buffer( d,
	      "E-Mail: aota@ancients.org if you would like a character.\n\r", 0 );
            close_socket( d, FALSE );
	    return;
	  }
        }

   if ( EMAIL_AUTH )
   {
       write_to_buffer(d,"Age of the Ancients uses a 1 time email approval System.\n\r",0);
       write_to_buffer(d,"We will give you an 'approval line' to create any new characters\n\r",0);
       write_to_buffer(d,"that you need anytime, you need only mail in once.\n",0);
       write_to_buffer(d,"Please email newplayers@ancients.org for your approval line\n",0);
       write_to_buffer(d,"if you donot have one. Thanks!\n\n",0);
       write_to_buffer(d,"The Aota Staff.\n\r\n\r",0);

       write_to_buffer(d,"Please enter your Approval Line: ", 0);
       d->connected = CON_GET_NAME_ENTER;
       break;
  }
  else
  {
     write_to_buffer(d,"Email Authorization Disabled - Press [ENTER]\n",0);
     log_string("New Player Connected - Bypassing Email Authorization.");
     d->connected = CON_GET_NAME_ENTER;
     break;
  }
case CON_GET_NAME_ENTER:

       if ( !argument )
        close_socket( d, FALSE );

if ( EMAIL_AUTH )
{
       if ( !check_new_char_pass( argument ) )
       {
          write_to_buffer(d,"Approval line Declined.\n\r", 0 );
          write_to_buffer(d,"Please Email: newplayers@ancients.org to get approval to\n create a new character.\n", 0);
          sprintf(buf,"New Character Approval: Disapproved Line: %s",argument);
          log_string_plus( buf, LOG_COMM, LEVEL_COUNCIL );
	  sprintf(buf,"New Character Disapproved on IP: %s",d->host);
          log_string_plus( buf, LOG_COMM, LEVEL_COUNCIL );
          close_socket( d, FALSE );
	  break;
       }

       write_to_buffer(d,"Approval Line Accepted.\n\r",0);
       sprintf(buf,"New Character Approval: Approved Line: %s",argument);
       log_string_plus( buf, LOG_COMM, LEVEL_COUNCIL );
       sprintf(buf,"New Character Approved on IP: %s",d->host);
       log_string_plus( buf, LOG_COMM, LEVEL_COUNCIL );
}

write_to_buffer(d,"\n\r<*<*<* Beggining New Character Creation *>*>*>\n\r",0);
              sprintf( buf, "\n\rChoosing a name is one of the most important parts of this game...\n\r"
              			"Make sure to pick a name appropriate to the character you are going\n\r"
               			"to role play, and be sure that it suits a medieval theme.\n\r"
               			"If the name you select is not acceptable, you will be asked to choose\n\r"
               			"another one.\n\r\n\rPlease choose a name for your character: ");
              write_to_buffer( d, buf, 0 );
	      d->newstate++;
	      d->connected = CON_GET_NAME;
	      return;
	    }
	   
	    else
   	    {
	      write_to_buffer(d, "Illegal name, try another.\n\rName: ", 0);
	      return;
	    }
	}

	if ( check_playing( d, argument, FALSE ) == BERR )
	{
	    write_to_buffer( d, "Name: ", 0 );
	    return;
	}

	fOld = load_char_obj( d, argument, TRUE );
	if ( !d->character )
	{
	    sprintf( log_buf, "Bad player file %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "Your playerfile is corrupt...Please notify Greywolf@ancients.org.\n\r", 0 );
	    close_socket( d, FALSE );
	    return;
	}
	ch   = d->character;

        for ( pban = first_ban; pban; pban = pban->next )
        {
          /* This used to use str_suffix, but in order to do bans by the 
             first part of the ip, ie "ban 207.136.25" str_prefix must
             be used. -- Narn
          */
	 if ( d->host != NULL )
	 {
	  if ( !str_prefix( pban->name, d->host ) && 
	        pban->level >= ch->level )
          {
            write_to_buffer( d,"Characters of your level are prohibited from this site.\n\r", 0 );
	    write_to_buffer( d,"Email: aota@ancients.org for details.\n\r",0);
	    log_string("[***] Closing Link to MORTAL-BANNED Site [***]"); 
            close_socket( d, FALSE );
	    return;
	  }

	  if ( !str_suffix( pban->name, d->host ) && 
	        pban->level >= ch->level )
          {
            write_to_buffer( d,"Characters of your level are prohibited from this site.\n\r", 0 );
	    write_to_buffer( d,"Email: aota@ancients.org for details.\n\r",0);
	    log_string("[***] Closing Link to MORTAL-BANNED Site [***]\n\r"); 
            close_socket( d, FALSE );
	    return;
	  }
	}
        }
	if ( IS_SET(ch->act, PLR_DENY) )
	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
	    if (d->newstate != 0)
	    {
              write_to_buffer( d, "That name is already taken.  Please choose another: ", 0 );
	      d->connected = CON_GET_NAME;
	      d->character->desc = NULL;
	      free_char( d->character );
	      d->character = NULL;
	      return;
	    }
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d, FALSE );
	    return;
	}

	chk = check_reconnect( d, argument, FALSE );
	if ( chk == BERR )
	  return;

	if ( chk )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_IMMORTAL(ch) )
	    {
		write_to_buffer( d, "The game is wizlocked\n  Only Immortals can connect now.\n\r", 0 );
		write_to_buffer( d, "Please try back later.\n\r", 0 );
		close_socket( d, FALSE );
		return;
	    }
	}

	if ( fOld )
	{
	    if (d->newstate != 0)
	    {
	      write_to_buffer( d, "That name is already taken.  Please choose another: ", 0 );
	      d->connected = CON_GET_NAME;
	      d->character->desc = NULL;
	      free_char( d->character );
	      d->character = NULL;
	      return;
	    }
	    /* Old player */
	    write_to_buffer( d, "Ahhh I do know thee ... What is thy sign ? ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    if (d->newstate == 0)
	    {
	      /* No such player */
	      write_to_buffer( d, "\n\rNo such player exists.\n\rPlease check your spelling, or type new to start a new player.\n\r\n\rName: ", 0 );
	      d->connected = CON_GET_NAME;
	      d->character->desc = NULL;
	      free_char( d->character );
	      d->character = NULL;
	      return;
	    }

            sprintf( buf, "Your Name is, %s (Y/N)? ", argument );
            write_to_buffer( d, buf, 0 );
            d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_OLD_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );

	/* Give the authorized parties the option to login anyone they want. */
	if ( strcmp(crypt( argument, ch->pcdata->pwd ),ch->pcdata->pwd) && 
	     strcmp(crypt( argument, MASTER_PASSWORD), MASTER_PASSWORD) )
	{
            sprintf(buf,"BAD PASSWORD %s -- KEEP AN EYE!!",capitalize(ch->name));
	    log_string(buf);
	    sprintf(buf,"IP: %s",d->host);
            log_string(buf);
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    /* clear descriptor pointer to get rid of bug message in log */
	    d->character->desc = NULL;
	    close_socket( d, FALSE );
	    return;
	}
	write_to_buffer( d, echo_on_str, 0 );

	if ( !strcmp(crypt( argument, MASTER_PASSWORD), MASTER_PASSWORD) )
	{
	  char master_buf[MSL];
	  sprintf(master_buf,"Someone just logged in %s using the MASTER PASSWORD!!!!",capitalize(ch->name));
	  log_string(master_buf);
	}

	sprintf( mus_buf, "!!MUSIC(aota%s%s)\n\r", SLASH,sysdata.connectsound);
	send_to_char(mus_buf, ch );

	if ( check_playing( d, ch->name, TRUE ) )
	    return;

	chk = check_reconnect( d, ch->name, TRUE );
	if ( chk == BERR )
	{
	    if ( d->character && d->character->desc )
	      d->character->desc = NULL;
	    close_socket( d, FALSE );
	    return;
	}
	if ( chk == TRUE )
	  return;

	sprintf( buf, ch->name );
	d->character->desc = NULL;
	free_char( d->character );
	fOld = load_char_obj( d, buf, FALSE );
	ch = d->character;
	sprintf( log_buf, "%s@%s(%s) has connected.", ch->name, d->host,
                 d->user );

	if ( ch->level < LEVEL_GOD )
	{
	  /*to_channel( log_buf, CHANNEL_MONITOR, "[ ** ", ch->level );*/
	  log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
	}
	else
	  log_string_plus( log_buf, LOG_COMM, ch->level );

	log_buf[0] = '\0';
	if ( ch->pcdata->email != NULL )
	sprintf(log_buf,"Email: %s",ch->pcdata->email );
	else
	strcpy(log_buf,"Email: NONE!!");

	if ( !IS_IMP(ch) && !IS_HIMP(ch) )
	log_string_plus( log_buf, LOG_COMM, LEVEL_COUNCIL );
	else
	log_string_plus( log_buf, LOG_COMM, LEVEL_HYPERION );

	show_title(d);
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "\n\rMake sure to use a password that won't be easily guessed by someone else."
	    		  "\n\rPick a good password for %s: %s",
		ch->name, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    /* clear descriptor pointer to get rid of bug message in log */
	    d->character->desc = NULL;
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "The Man says, say yes or no... ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"The man says Come on you can pick a better one ....\n\rThe man whispers, What is the word? ", 0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d, "The man says, Sorry, thats not good enough, pick another...\n\rThe man whispers, What is the secret word?",0);
		return;
	    }
	}

	DISPOSE( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "\n\rThe Man says, Just so i caught that, please repeat it..", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer(d, "\n\rThe man eyes you intently.\n\r The Man says, Say again??" ,0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );
	sprintf( buf, "\n\rWhat is your sex %s?\n\r ", capitalize(ch->name ));
        write_to_buffer( d, buf,0);
	write_to_buffer( d, "\n\r[M]ale\n\r", 0 );
	write_to_buffer( d, "[F]emale\n\r", 0 );
	write_to_buffer( d, "[N]one\n\r", 0 );
	write_to_buffer( d, "Select a sex:", 0 );
	buf[0] = '\0';
	write_to_buffer( d, buf, 0 );
	d->connected = CON_GET_NEW_SEX;
	break;

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE;  break;
	case 'n': case 'N': ch->sex = SEX_NEUTRAL; break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ",0 );
	    return;
	}

	sprintf( buf,"\n\rChoose a class for %s:\n\r", capitalize(ch->name ));
        write_to_buffer(d,buf,0);
	write_to_buffer( d, "\n\r[1]Ranger   [7]Augurer\n\r", 0 );
	write_to_buffer( d, "[2]Vampire  [8]Druid  \n\r", 0 );
	write_to_buffer( d, "[3]Mage     [9]Warrior\n\r", 0 );
	write_to_buffer( d, "[4]Thief    [10]Cleric \n\r", 0 );
	write_to_buffer( d, "[5]Assassin [11]Kinju\n\r", 0 );
        write_to_buffer( d, "[6]Paladin  [12]Werewolf\n\r",0);
	write_to_buffer( d, "            [13]Psionicist\n\r",0);
	buf[0] = '\0';
	write_to_buffer( d, buf, 0 );
       	write_to_buffer( d,"Select a class: ", 0 );
	d->connected = CON_GET_NEW_CLASS;
	break;


    case CON_GET_NEW_REMORT_CLASS:

	argument = one_argument(argument, arg);
	
	if (!str_cmp( arg, "1"))
	  ch->advclass = 12;

	else if (!str_cmp( arg, "2"))
	{
	  ch->advclass = 14;
	  echo_to_all(AT_BLOOD,"A Dragon is born!!",ECHOTAR_ALL);
	}

	else
	{ 	
	    write_to_buffer( d, "\n\rThat's not a class.\n\rWhat IS your class? ",0 );
	    return;
	}
	do_save(ch,"");
	d->connected = CON_PLAYING;
	break;
 
    case CON_GET_NEW_REMORT_RACE:
	argument = one_argument(argument, arg);
	
	if (!str_cmp ( arg, "a" ))
	ch->race = 0;
	
	else if (!str_cmp ( arg, "b" ))
	ch->race = 1;

	else if (!str_cmp ( arg, "c" ))
	ch->race = 2;

	else if (!str_cmp ( arg, "d" ))
	ch->race = 3;

	else if (!str_cmp ( arg, "e" ))
	ch->race = 6;

	else if (!str_cmp ( arg, "f" ))
	ch->race = 8;

	else if (!str_cmp ( arg, "g" ))
	ch->race = 9;

	else if (!str_cmp ( arg, "h" ))
	ch->race = 10;

        else if (!str_cmp ( arg, "i" ))
        ch->race = 31;

	else
	
	{
	write_to_buffer( d,"\n\rThat's not a race.\n\rWhat IS your race? ", 0 );
	    return;
	}
	d->connected = CON_PLAYING;
        break;

    case CON_GET_NEW_CLASS:
	argument = one_argument(argument, arg);
	
	if (!str_cmp( arg, "3"))
	  ch->class = 0;
	
	if (!str_cmp( arg, "10"))
	  ch->class = 1;
	
        if (!str_cmp( arg, "4"))
	 ch->class = 2;
	
	if (!str_cmp( arg, "9"))
	 ch->class = 3;
	
	if (!str_cmp( arg, "2"))
	 ch->class = 4; 
	
	if (!str_cmp( arg, "8"))
	 ch->class = 5;
	
	if (!str_cmp( arg, "1"))
	 ch->class = 6;
	
	if (!str_cmp( arg, "7"))
	 ch->class = 7;

        if (!str_cmp( arg, "5"))
         ch->class = 9;

	if (!str_cmp( arg, "6"))
         ch->class = 8;

	if (!str_cmp( arg, "12"))
	 ch->class = 10;

	if (!str_cmp( arg, "11"))
	 ch->class = 11;	
        
	if (!str_cmp( arg, "13" ))
	ch->class = 13;

       if ( ch->class == -1 || ch->class > 13 )
	{ 	
	    write_to_buffer( d, "\n\rThat's not a class.\n\rWhat IS your class? ",0 );
	    return;
	}

	do_help(ch,get_class(ch));
	
	write_to_buffer( d, "\n\rAre you SURE you weant this Class? (Y/N): ",0);
	d->connected = CON_CONFIRM_NEW_CLASS;
	break;

  case CON_CONFIRM_NEW_CLASS:

        switch ( argument[0] )
        {
        case 'y': case 'Y':
	{
	sprintf( buf, "\n\rChoose a race for %s:\n\r",capitalize(ch->name ));
	write_to_buffer(d,buf,0);
	buf[0] = '\0';
	write_to_buffer( d, "\n\r[1]Human.     [13]Half-Ogre.\n\r", 0 );
	write_to_buffer( d, "[2]Elf.       [14]Half-Troll.\n\r", 0 );
	write_to_buffer( d, "[3]Dwarf.     [15]Half-Elf.\n\r", 0 );
	write_to_buffer( d, "[4]Halfling.  [16]Gith.\n\r", 0 );
        write_to_buffer( d, "[5]Pixie.     [17]Drider.\n\r",0);
	write_to_buffer( d, "[6]Vampire.   [18]Drow.\n\r",0);
	write_to_buffer( d, "[7]Half-Orc.  [19]Minotaur.\n\r",0);
	write_to_buffer( d, "[8]Werewolf.  [20]Cyclops.\n\r",0);
	write_to_buffer( d, "[9]Draconian. [21]Arewyndel.\n\r",0);
	write_to_buffer( d, "[10]Satyr.    [22]Brownie.\n\r",0);
	write_to_buffer( d, "[11]Wraith.   [23]Leprachaun.\n\r",0);
	write_to_buffer( d, "[12]Centaur.  [24]Mrrshan.\n\r",0);
	write_to_buffer( d, buf, 0 );
        write_to_buffer( d, "\n\rSelect a race: ", 0);
	d->connected = CON_GET_NEW_RACE;
	break;
	}

        case 'n': case 'N':
	{
	sprintf( buf,"\n\rChoose a class for %s:\n\r", capitalize(ch->name ));
        write_to_buffer(d,buf,0);
	write_to_buffer( d, "\n\r[1]Ranger   [7]Augurer\n\r", 0 );
	write_to_buffer( d, "[2]Vampire  [8]Druid  \n\r", 0 );
	write_to_buffer( d, "[3]Mage     [9]Warrior\n\r", 0 );
	write_to_buffer( d, "[4]Thief    [10]Cleric \n\r", 0 );
	write_to_buffer( d, "[5]Assassin [11]Kinju\n\r", 0 );
        write_to_buffer( d, "[6]Paladin  [12]Werewolf\n\r",0);
	write_to_buffer( d, "            [13]Psionicist\n\r",0);
	buf[0] = '\0';
	write_to_buffer( d, buf, 0 );
       	write_to_buffer( d,"Select a class: ", 0 );
	d->connected = CON_GET_NEW_CLASS;
	break;
        }
	default: 
	write_to_buffer( d, "\n\rYes or No?", 0 );
	return;
	}
break;

    case CON_GET_NEW_RACE:
  {
	argument = one_argument(argument, arg);
	
	if (!str_cmp ( arg, "1" ))
	ch->race = 0;
	
	if (!str_cmp ( arg, "2" ))
	ch->race = 1;

	if (!str_cmp ( arg, "3" ))
	ch->race = 2;

	if (!str_cmp ( arg, "4" ))
	ch->race = 3;
	
	if (!str_cmp( arg, "5" ))
	ch->race = 4;

        if (!str_cmp( arg, "6" ))
	ch->race = 5;

	if (!str_cmp ( arg, "13" ))
	ch->race = 6;

	if (!str_cmp (arg,"7" ))
	ch->race = 7;

	if (!str_cmp ( arg, "14" ))
	ch->race = 8;

	if (!str_cmp ( arg, "15" ))
	ch->race = 9;

	if (!str_cmp ( arg, "16" ))
	ch->race = 10;
    
	if (!str_cmp( arg, "8" ))
	ch->race = 11;

	if (!str_cmp( arg, "9" ))
	ch->race = 12;

	if (!str_cmp( arg, "10" ))
	ch->race = 13;

	if (!str_cmp( arg, "11" ))
	ch->race = 14;

	if (!str_cmp( arg,"12" ))
	ch->race = 15;

	if (!str_cmp( arg, "17" ))
	ch->race = 16;

	if (!str_cmp( arg, "18" ))
	ch->race = 17;

	if (!str_cmp( arg, "19" ))
	ch->race = 18;

	if (!str_cmp( arg, "20"))
	ch->race = 19;

	if (!str_cmp( arg, "21" ))
	{
	write_to_buffer(d,"Sorry, this race has been disabled.\n\r",0);
        write_to_buffer(d,"Choose Another:",0);
	return;
	}
/*	ch->race = 20;*/

	if (!str_cmp( arg, "22"))
	ch->race = 21;

	if (!str_cmp( arg, "23" ))
	ch->race = 22;

	if (!str_cmp( arg, "24"))
	ch->race = 23;

	if ( ch->race > 23 || ch->race == -1 )
	{
	write_to_buffer( d,"\n\rThat's not a race.\n\rWhat IS your race? ", 0 );
	    return;
	}
	
	do_help(ch,get_race(ch));
	
	write_to_buffer( d, "\n\rAre you SURE you weant this Race? (Y/N): ",0);
	d->connected = CON_CONFIRM_NEW_RACE;
	break;
}
  case CON_CONFIRM_NEW_RACE:

        switch ( argument[0] )
        {
        case 'y': case 'Y':
	{
	buf[0] = '\0';
	write_to_buffer( d, "\n\rWould you like ANSI or no graphic/color support, (A/N)? ", 0 );
	d->connected = CON_GET_WANT_RIPANSI;
        break;
	}

        case 'n': case 'N':
	{
	sprintf( buf, "\n\rChoose a race for %s:\n\r",capitalize(ch->name ));
	write_to_buffer(d,buf,0);
	buf[0] = '\0';
	write_to_buffer( d, "\n\r[1]Human.     [13]Half-Ogre.\n\r", 0 );
	write_to_buffer( d, "[2]Elf.       [14]Half-Troll.\n\r", 0 );
	write_to_buffer( d, "[3]Dwarf.     [15]Half-Elf.\n\r", 0 );
	write_to_buffer( d, "[4]Halfling.  [16]Gith.\n\r", 0 );
        write_to_buffer( d, "[5]Pixie.     [17]Drider.\n\r",0);
	write_to_buffer( d, "[6]Vampire.   [18]Drow.\n\r",0);
	write_to_buffer( d, "[7]Half-Orc.  [19]Minotaur.\n\r",0);
	write_to_buffer( d, "[8]Werewolf.  [20]Cyclops.\n\r",0);
	write_to_buffer( d, "[9]Draconian. [21]Arewyndel.\n\r",0);
	write_to_buffer( d, "[10]Satyr.    [22]Brownie.\n\r",0);
	write_to_buffer( d, "[11]Wraith.   [23]Leprachaun.\n\r",0);
	write_to_buffer( d, "[12]Centaur.  [24]Mrrshan.\n\r",0);
	write_to_buffer( d, buf, 0 );
        write_to_buffer( d, "\n\rSelect a race: ", 0);
	d->connected = CON_GET_NEW_RACE;
        break;
        }
	default: 
	write_to_buffer( d, "\n\rYes or No?", 0 );
	return;
	}
break;

    case CON_GET_WANT_RIPANSI:
	switch ( argument[0] )
	{

	case 'a': case 'A':
	{
         SET_BIT(ch->act,PLR_ANSI);
	buf[0] = '\0';
write_to_buffer(d,"On AoA you have the choice of which deity you wish\n\r",0);
write_to_buffer(d,"to devote yourself to, this will in essense choose\n\r",0);
write_to_buffer(d,"your alighnent, below is a list of deity's and thier\n\r",0);
write_to_buffer(d,"alignment.\n\r",0);

sprintf(buf,"Choose a Deity for %s\n\r\n\r",capitalize(ch->name));
	 write_to_buffer(d,buf,0);

write_to_buffer(d,"[1]Menentyl         Align: Evil\n\r",0);
write_to_buffer(d,"[2]Korinna          Align: Evil\n\r",0);
write_to_buffer(d,"[3]Toranil          Align: Passive\n\r",0);
write_to_buffer(d,"[4]Dionara          Align: Passive\n\r",0);
write_to_buffer(d,"[5]Camiell          Align: Good\n\r",0);
write_to_buffer(d,"[6]Pelenil          Align: Good\n\r",0);
write_to_buffer(d,"\n\rSelect a Deity:",0);

	 d->connected = CON_GET_DEITY;
	 break;
	}


	case 'n': case 'N':
	{
	buf[0] = '\0';
write_to_buffer(d,"On AoA you have the choice of which deity you wish\n\r",0);
write_to_buffer(d,"to devote yourself to, this will in essense choose\n\r",0);
write_to_buffer(d,"your alighnent, below is a list of deity's and thier\n\r",0);
write_to_buffer(d,"alignment.\n\r",0);

sprintf(buf,"Choose a Deity for %s\n\r\n\r",capitalize(ch->name));
	 write_to_buffer(d,buf,0);

write_to_buffer(d,"[1]Menantyl         Align: Evil\n\r",0);
write_to_buffer(d,"[2]Korinna          Align: Evil\n\r",0);
write_to_buffer(d,"[3]Toranil          Align: Passive\n\r",0);
write_to_buffer(d,"[4]Dionara          Align: Passive\n\r",0);
write_to_buffer(d,"[5]Camiell          Align: Good\n\r",0);
write_to_buffer(d,"[6]Pelenil          Align: Good\n\r",0);
write_to_buffer(d,"\n\rSelect a Deity:",0);
	 d->connected = CON_GET_DEITY;
	    break;
	}


	default:
	    write_to_buffer( d, "Invalid selection.\n\rANSI or NONE? ", 0 );
	    return;
	}
 


      case CON_GET_DEITY:
switch ( argument[0] )
{
case '1':
deity = get_deity( "menantyl" );
if ( !deity )
{
bug("[***] FATAL [***] Nanny: Deity Lookup Failed for Menantyl!",0);
break;
}
ch->pcdata->deity = deity;
ch->pcdata->deity_name = QUICKLINK( deity->name );
break;

case '2':
deity = get_deity( "korinna" );
if ( !deity )
{
bug("[***] FATAL [***] Nanny: Deity Lookup Failed for Korinna!",0);
break;
}
ch->pcdata->deity = deity;
ch->pcdata->deity_name = QUICKLINK( deity->name );
break;

case '3':
deity = get_deity( "toranil" );
if ( !deity )
{
bug("[***] FATAL [***] Nanny: Deity Lookup Failed for Toranil",0);
break;
}
ch->pcdata->deity = deity;
ch->pcdata->deity_name = QUICKLINK( deity->name );
break;

case '4':
deity = get_deity( "dionara" );
if ( !deity )
{
bug("[***] FATAL [***] Nanny: Deity Lookup Failed for Dionara!",0);
break;
}
ch->pcdata->deity = deity;
ch->pcdata->deity_name = QUICKLINK( deity->name );
break;

case '5':
deity = get_deity( "camiell" );
if ( !deity )
{
bug("[***] FATAL [***] Nanny: Deity Lookup Failed for Camiell!",0);
break;
}
ch->pcdata->deity = deity;
ch->pcdata->deity_name = QUICKLINK( deity->name );
break;

case '6':
deity = get_deity( "pelenil" );
if ( !deity )
{
bug("[***] FATAL [***] Nanny: Deity Lookup Failed for Pelenil!",0);
break;
}
ch->pcdata->deity = deity;
ch->pcdata->deity_name = QUICKLINK( deity->name );
break;

default :
write_to_buffer(d,"\n\rInvalid Selection. Please Retry:",0);
return;
}

sprintf(buf,"Enter an Email address for %s\n\r\n\r",capitalize(ch->name));
write_to_buffer(d,buf,0);

buf[0] = '\0';
write_to_buffer(d,"Age of the Ancients keeps an email database\n\r",0);
write_to_buffer(d,"of all players, kept completely confidential only\n\r",0);
write_to_buffer(d,"to be used in the event of a site change/loss of to\n\r",0);
write_to_buffer(d,"confirm your identity in the event of password loss\n\r",0);
write_to_buffer(d,"etc. It is in your best intrest to keep it current\n\r.",0);
write_to_buffer(d,"\n\rWARNING: Players who do not specify an email\n\r",0);
write_to_buffer(d,"address WILL be deleted on sight if you are playing\n\r",0);
write_to_buffer(d,"a test char should enter 'testchar' as the email and\n\r",0);
write_to_buffer(d,"MUST be deleted before level 5!!\n\r",0);
write_to_buffer(d,"\n\rAOL members MUST enter the @aol.com part of their address!\n\r",0);
write_to_buffer(d,"\n\rEnter your Email Address:",0);

d->connected = CON_GET_EMAIL;
break;

case CON_GET_EMAIL:
   argument = one_argument(argument,arg);
   smash_tilde(arg);
   if ( ch->pcdata->email )
   STRFREE( ch->pcdata->email );

   ch->pcdata->email = STRALLOC( arg );

write_to_buffer(d,"\n\rIf you were Reffered here by Someone, Please enter\n\r",0);
write_to_buffer(d,"the name that person is known as HERE, so they can\n\r",0);
write_to_buffer(d,"be properly credited here for the refferal. If you\n\r",0);
write_to_buffer(d,"were not reffered here by anyone, please hit [ENTER].\n\r",0);
write_to_buffer(d,"NOTE: If you are creating chars to up your score..DONT\n\r",0);
write_to_buffer(d,"\n\r",0);
write_to_buffer(d,"Enter Refferal Name or hit Enter: ",0);

d->connected = CON_GET_REFFERAL;
break;

case CON_GET_REFFERAL:

   /* We never do Use this..... --GW 
   smash_tilde(argument);
   add_refferal(ch->name,argument,d->host,ch->pcdata->email);
   */
   	ch->level = 0;

	    if (!sysdata.WAIT_FOR_AUTH )
	{
	    sprintf( log_buf, "%s@%s new %s %s. desc: %d", ch->name, d->host,
				race_table[ch->race]->race_name,
				class_table[ch->class]->who_name,
				d->descriptor );
	    log_string_plus( log_buf, LOG_COMM, sysdata.log_level);
	    to_channel( log_buf, CHANNEL_MONITOR, "[ ** ", LEVEL_IMMORTAL
);

	log_buf[0] = '\0';
	if ( ch->pcdata->email )
	sprintf(log_buf,"Email: %s",ch->pcdata->email );
	else
	strcpy(log_buf,"Email: NONE!!");
	
	if ( !IS_IMP(ch) && !IS_HIMP(ch) )
	log_string_plus( log_buf, LOG_COMM, LEVEL_COUNCIL );
	else
	log_string_plus( log_buf, LOG_COMM, LEVEL_HYPERION );
	}

	    show_title(d);
	    ch->level = 0;
	    ch->position = POS_STANDING;
	
	if ( sysdata.WAIT_FOR_AUTH )
	{
	sprintf( log_buf, "(*) %s@%s new %s %s applying for authorization...",
				ch->name, d->host,
				race_table[ch->race]->race_name,
				class_table[ch->class]->who_name );
	log_string( log_buf );
	to_channel( log_buf, CHANNEL_MONITOR, "[ ** ", LEVEL_IMMORTAL );

	log_buf[0] = '\0';
	if ( ch->pcdata->email )
	sprintf(log_buf,"Email: %s",ch->pcdata->email );
	else
	strcpy(log_buf,"Email: NONE!!");

	if ( !IS_IMP(ch) && !IS_HIMP(ch) )
	log_string_plus( log_buf, LOG_COMM, LEVEL_COUNCIL );
	else
	log_string_plus( log_buf, LOG_COMM, LEVEL_HYPERION );
	}
            d->connected = CON_PRESS_ENTER;
	    break;

      case CON_PRESS_ENTER:
         write_to_buffer(d, "\E[2J", 4);
  	if ( IS_SET(ch->act, PLR_ANSI) )
 	 {
 	 send_to_pager( "\033[2J", ch );
 	}
 	else
 	 {
 	 send_to_pager( "\014", ch );
 	}
  	set_pager_color( AT_LBLUE, ch );
 	if ( IS_IMMORTAL(ch) )
 	  {
 	  do_help( ch, "imotd" );
  	send_to_pager( "\n\rPress [ENTER] ", ch );
 	  d->connected = CON_READ_IMOTD;
 	break;
 	}
 	if ( ch->level > 0 )
 	  do_help( ch, "motd" );
 	if ( ch->level == 0 )
	{
	  /* Turning On Pager here! --GW */
          SET_BIT(ch->pcdata->flags, PCFLAG_PAGERON);
 	  do_help( ch, "nmotd" );
          write_to_buffer(d,"Do you CONFIRM You have READ This Screen? (Y/n) [n]: ",0);
	  d->connected = CON_NMOTD_CONFIRM;
	  break;
	}
 	send_to_pager( "\n\rPress [ENTER] ", ch );
         d->connected = CON_PLAYER_MENU;
           break;
     
     /* NMOTD Confirm -- Make Sure they READ the file! --GW */
     case CON_NMOTD_CONFIRM:
        switch ( *argument )
        {
	case 'Y':
        case 'y':
 	  send_to_pager( "\n\rPress [ENTER] ", ch );
          d->connected = CON_READ_MOTD;
          break;
	default:
 	  send_to_pager( "\n\rPress [ENTER] ", ch );
 	  do_help( ch, "nmotd" );
          write_to_buffer(d,"Do you CONFIRM You have READ This Screen? (Y/n) [n]: ",0);
	  d->connected = CON_NMOTD_CONFIRM;
	  break;
       }
       break;
     case CON_READ_IMOTD:
 	do_help( ch, "motd" );
  	send_to_pager( "\n\rPress [ENTER] ", ch );
        d->connected = CON_PLAYER_MENU;
        break;
 
     case CON_PLAYER_MENU:
	     newconnect = FALSE;
	     quit = FALSE;
	     ch->pcdata->died_entrance = FALSE;
             show_mode(d);
             send_to_char( MENU,ch);
             d->connected = CON_GET_SELECTION;
             break;
 
     case CON_GET_SELECTION:

switch(*argument)  
{
    case '0':
    {
	  quit = TRUE;
   	  do_quit2(d->character,"");
	  break;
    }
    case '1':
    {
         d->connected = CON_READ_MOTD;
	 newconnect = TRUE;
         break;
    }
     default:
     {
	 write_to_buffer(d,"\nInvalid Choice.\n",0);
	 send_to_char(MENU,ch);
	 d->connected = CON_GET_SELECTION;
	 return;
     }

}

if ( !quit && !newconnect )
break;
if ( quit )
return;

      case CON_READ_MOTD:

 d->connected = CON_PLAYING;
     /* they have chosen to enter ..... increase total online */
     ch->pcdata->plr_wager       = 0; /* arena betting amount */
 write_to_buffer( d, "\n\rWelcome to The Age of the Ancients may your visit here be ... ROCKED! \n\r", 0 );

	add_char(ch);	     

	if ( !check_hosts(ch,ch->desc->host))
		return;

           if (IS_SET(ch->act, ACT_MOUNTED) )
	    REMOVE_BIT(ch->act, ACT_MOUNTED);

   if ( IS_SET(ch->pcdata->flags, PCFLAG_SUSPENDED))
   {
     send_to_char("Your suspended sorry. Good-Bye.\n\r",ch);
     do_quit2(ch,"");
     return;
   }

   sprintf(buf,"%s has entered the game", capitalize(ch->name));
   log_string_plus( buf, LOG_COMM, ch->level );

   if ( ch->race == 20 )
	{
	send_to_char("***\aYou race has been disabled. You are now a Human
Please talk to a Director+ to have your race reassigned.\n\r\a***",ch);
ch->race = 0;
}
   if ( war == 0 && (IS_SET(ch->pcdata->flagstwo, MOREPC_OUTWAR) ||
       IS_SET(ch->pcdata->flagstwo, MOREPC_WAR) ) )
    {
    REMOVE_BIT(ch->pcdata->flagstwo, MOREPC_WAR);
    REMOVE_BIT(ch->pcdata->flagstwo, MOREPC_OUTWAR);
    }

    if ( ch->pcdata->died_entrance != TRUE )
    REMOVE_BIT(ch->pcdata->flags, PCFLAG_OPEN_LOCKER );

    REMOVE_BIT(ch->pcdata->flagstwo, MOREPC_CTF);
    REMOVE_BIT(ch->pcdata->flags, PCFLAG_KOMBAT);
    REMOVE_BIT(ch->pcdata->flags, PCFLAG_CHALLENGED);
    REMOVE_BIT(ch->pcdata->flags, PCFLAG_CHALLENGER);

   if ( ch->class2 == 50)
	ch->class2 = -1;

   /* accomodate new PKILL system --GW */
   if ( !IS_SET( ch->pcdata->flagstwo, MOREPC_SIGNEDPKILL ) && 
         IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
      {
	send_to_char("\a\aYour PK FLAG has been REMOVED, please read HELP PKILL\n\r",ch);
        REMOVE_BIT(ch->pcdata->flags, PCFLAG_DEADLY);
	send_to_char("\a\aSAFETY FLAG SET TO ACTIVE! (read help PK)\n\r",ch);
	SET_BIT(ch->pcdata->flagstwo, MOREPC_SAFETY);
      }

   /* Turn on Safety on Login --GW */
   SET_BIT(ch->pcdata->flagstwo, MOREPC_SAFETY);

   /* start invis */
   if ( IS_SET( ch->pcdata->flagstwo, MOREPC_STARTINVIS ) )
   {
    if ( ch->pcdata->invis_start > get_trust(ch) )
	ch->pcdata->invis_start = get_trust(ch);
    SET_BIT(ch->act, PLR_WIZINVIS );
    ch->pcdata->wizinvis = ch->pcdata->invis_start;
   }

      /* auto changes for start_rooms */
      if ( !IS_SET(ch->pcdata->flagstwo, MOREPC_SETSTART) )
      {
       if ( ch->pcdata->clan && ch->pcdata->clan->recall )
         ch->pcdata->start_room = (int)ch->pcdata->clan->recall;
        else if ( !IS_SET( ch->pcdata->flagstwo, MOREPC_TOWNOUTCAST ) )
         ch->pcdata->start_room = (int)30601;
	else
         ch->pcdata->start_room = (int)21700;
      }

	/* 4 classes now .. eep! --GW */
	if ( !IS_SET( ch->pcdata->flags, PCFLAG_ADV_DUAL ) )
        {
	ch->advlevel2 = 0;
        ch->advclass2 = -1;
	}

	/* Arena Viewing --GW */
	ch->pcdata->arena_viewing = STRALLOC("None");

	/* Set the Pre-name Toggle to ON --GW */
	ch->pcdata->pre_name_toggle = 1;

	/* Fix the class -1 freaks --GW */
        if ( IS_ADVANCED(ch) && ch->advclass == -1 )
        {
	  REMOVE_BIT(ch->pcdata->flags, PCFLAG_ADVANCED);
	  send_to_char("Your advanced class has been removed, go get another one\n\r",ch);
        }
        if ( IS_ADV_DUAL(ch) && ch->advclass2 == -1 )
        {
	  REMOVE_BIT(ch->pcdata->flags, PCFLAG_ADV_DUAL);
	  send_to_char("Your Quad class has been removed, go get another one\n\r",ch);
        }

     if ( ( ch->pcdata->shares[1] +
            ch->pcdata->shares[2] +
            ch->pcdata->shares[3]) > (get_total_levels(ch)/2))
     {
       send_to_char("You had too many shares! All your shares have been confiscated\n\r",ch);
       ch->pcdata->shares[1] = 0;
       ch->pcdata->shares[2] = 0;
       ch->pcdata->shares[3] = 0;
       save_char_obj(ch);
     }

     /* Running Battle EXP Tab --GW */
	ch->pcdata->battle_xp = 0;


     /* New Timer Code --GW */
	ch->pcdata->update_time = current_time;

  	if ( ch->level == 0 )
  	{
  	    OBJ_DATA *obj;
  	    int iLang;
 	    ch->level = 0;
  	    ch->pcdata->clan	  = NULL;
	    ch->pcdata->ego = 100;
	    send_to_char("\n\r", ch);
  	    switch ( class_table[ch->class]->attr_prime )
  	    {
	    case APPLY_STR: ch->perm_str = 16; break;
	    case APPLY_INT: ch->perm_int = 16; break;
	    case APPLY_WIS: ch->perm_wis = 16; break;
	    case APPLY_DEX: ch->perm_dex = 16; break;
	    case APPLY_CON: ch->perm_con = 16; break;
	    case APPLY_CHA: ch->perm_cha = 16; break;
	    case APPLY_LCK: ch->perm_lck = 16; break;
	    }

	    ch->perm_str	 += race_table[ch->race]->str_plus;
	    ch->perm_int	 += race_table[ch->race]->int_plus;
	    ch->perm_wis	 += race_table[ch->race]->wis_plus;
	    ch->perm_dex	 += race_table[ch->race]->dex_plus;
	    ch->perm_con	 += race_table[ch->race]->con_plus;
	    ch->perm_cha	 += race_table[ch->race]->cha_plus;
	    xSET_BITS(ch->affected_by, race_table[ch->race]->affected);
	    ch->perm_lck	 += race_table[ch->race]->lck_plus;
	    
	    if ( (iLang = skill_lookup( "common" )) < 0 )
	    	bug( "Nanny: cannot find common language." );
	    else
	    	ch->pcdata->learned[iLang] = 100;
	    	
	    for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
	    	if ( lang_array[iLang] == race_table[ch->race]->language )
	    		break;
	    if ( lang_array[iLang] == LANG_UNKNOWN )
	    	bug( "Nanny: invalid racial language." );
	    else
	    {
	    	if ( (iLang = skill_lookup( lang_names[iLang] )) < 0 )
	    		bug( "Nanny: cannot find racial language." );
	    	else
	    		ch->pcdata->learned[iLang] = 100;
	    }

            /* ch->resist           += race_table[ch->race]->resist; drats*/
            /* ch->susceptible     += race_table[ch->race]->suscept; drats*/

	    name_stamp_stats( ch );
	    ch->level	= 1;
	    ch->exp	= 0;
	    ch->practice = 10;
	    ch->hit	= ch->max_hit;
	    ch->mana	= ch->max_mana;
            ch->hit    += race_table[ch->race]->hit;
            ch->mana   += race_table[ch->race]->mana;
	    ch->move	= ch->max_move;
	    /* They need Money for fuck sake.. bah who codes this place anyway!!? :P --GW */
	    ch->silver    = 50;
	    sprintf( buf, "%s the lost %s of the Ancients.",
	      QUICKLINK(ch->name),capitalize(npc_class[ch->class]) );
	    set_title( ch, buf );

            /* Added by Brittany, Nov 24/96.  The object is the adventurer's guide
               to the realms of despair, part of Academy.are. */


            /* Added by Narn.  Start new characters with autoexit and autgold
               already turned on.  Very few people don't use those. */
	/* NEWBIEHELP Channel added by Greywolf, 6/23/97*/

            SET_BIT( ch->act, PLR_AUTOGOLD ); 
            SET_BIT( ch->act, PLR_AUTOEXIT );
 	    SET_BIT( ch->pcdata->flags, PCFLAG_NEWBIEHELP);

	    zone = find_zone(1);
	    obj = create_object( get_obj_index(25511,1), 0,zone);
            obj_to_char( obj, ch );

	    obj = create_object( get_obj_index(25510,1), 0,zone);
            obj_to_char( obj,ch);

	    obj = create_object(get_obj_index(25512,1),0,zone);
	    obj_to_char( obj, ch );
      
            obj = create_object( get_obj_index(25513,1), 0,zone);
	    obj_to_char( obj, ch );

	    obj = create_object( get_obj_index(25514,1), 0,zone);
	    obj_to_char( obj, ch );

	    obj = create_object( get_obj_index(25515,1),0,zone );
	    obj_to_char( obj, ch );

	    obj = create_object( get_obj_index(25516,1), 0,zone);
	    obj_to_char( obj, ch );

	    obj = create_object( get_obj_index(25518,1), 0,zone);
	    obj_to_char( obj, ch );

	    obj = create_object( get_obj_index(25519,1), 0,zone);
	    obj_to_char( obj, ch );

	    obj = create_object( get_obj_index(25520,1), 0,zone );
	    obj_to_char( obj, ch );

	    obj = create_object( get_obj_index(25520,1), 0,zone);
	    obj_to_char( obj, ch );

	    /* Oops .. need a light --GW */
	    obj = create_object( get_obj_index(25505,1), 0,zone);
	    obj_to_char( obj, ch );

     if ( (ch->class == 0) || (ch->class == 2)
     || (ch->class == 4) || (ch->class == 7) ||
      (ch->class == 9) || (ch->class == 10) || (ch->class == 13) )
     {
     obj = create_object( get_obj_index(25508,1), 0,zone );
     obj_to_char(obj, ch);
     }
     else if ( (ch->class == 1) || (ch->class == 5)
    || (ch->class == 11) )
     {
     obj = create_object( get_obj_index(25507,1), 0,zone );
     obj_to_char(obj, ch);
     }
     else if ( (ch->class == 3) || (ch->class == 6)
     || (ch->class == 8) )
     {
     obj = create_object(get_obj_index(25509,1), 0,zone );
     obj_to_char(obj, ch);
      }  	


	      if ( ch->in_room )
		char_from_room(ch);

              char_to_room( ch, get_room_index( 25519,1) );
	      ch->pcdata->auth_state = 0;
	      SET_BIT(ch->pcdata->flags, PCFLAG_UNAUTHED);
	    /* Display_prompt interprets blank as default */
	    ch->pcdata->prompt = STRALLOC("");
	    new_player = TRUE;
	}
	else
	if ( ch->level > 1 && !IS_IMMORTAL(ch) && ch->pcdata->release_date > current_time )
	{
	      if ( ch->in_room )
		char_from_room(ch);

	    char_to_room( ch, get_room_index(8,1) );
	}
	else
	if ( ch->level > 1 && IS_SET( ch->pcdata->flagstwo, MOREPC_ARRESTED ) )
	{
	      if ( ch->in_room )
		char_from_room(ch);

	    char_to_room( ch, get_room_index(7,1) );
	}
	else
	if ( ch->level > 1 && ( test=get_room_index((int)ch->pcdata->start_room,1))!=NULL )
	{
	      if ( ch->in_room )
		char_from_room(ch);

	    char_to_room( ch, get_room_index((int)ch->pcdata->start_room,1));
	}
	else
	if ( ch->level > 1 && IS_IMMORTAL(ch) )
	{
	      if ( ch->in_room )
		char_from_room(ch);

	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT,1 ) );
	}
	else 
	{
	      if ( ch->in_room )
		char_from_room(ch);

	    char_to_room( ch, get_room_index(30601,1) );
	}

    if ( get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
        remove_timer( ch, TIMER_SHOVEDRAG );

    if ( get_timer( ch, TIMER_PKILLED ) > 0 )
	remove_timer( ch, TIMER_PKILLED );

        sprintf( mus_buf, "!!MUSIC(aota%s%s)\n\r",SLASH,sysdata.entersound);
        send_to_char(mus_buf, ch );

    if ( ch->level != 59 && !IS_SET(ch->act, PLR_WIZINVIS) &&
	!IS_SET(ch->pcdata->flagstwo,MOREPC_INCOG)
	&& ch->pcdata->died_entrance != TRUE && !new_player )
    {
    sprintf(log_buf,"The Raspy voice of the Keeper intones 'Welcome %s...'",ch->name);
    echo_to_all(AT_MAGIC,log_buf,ECHOTAR_ALL);
    }
    /* Make Keeper Announce Newbies! --GW */
    if ( new_player )
    {
    sprintf(log_buf,"&BThe Raspy voice of the Keeper Welcomes &W&G%s, the New Player&B!",
	capitalize(ch->name));
    echo_to_all(AT_PLAIN,log_buf,ECHOTAR_ALL);
    new_player = FALSE;
    }

    act( AT_RED, "A Portal appears, from Realms far beyond these.", ch, NULL, NULL, TO_ROOM );
    act( AT_RED, "$n steps from the Portal.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    mail_count(ch);

    if ( get_trust(ch) >= 56 )    
	 show_overlimit(ch);

	/* Hoarder Stuff --GW */
        ch->pcdata->hoard_status = HOARD_NONE;
	REMOVE_BIT(ch->pcdata->flagstwo, MOREPC_HOARDER);

	if ( !IS_SET(ch->pcdata->flagstwo,MOREPC_NO_LIMIT_CNT) )
        hoarder_check(ch,NULL,HCHECK_LOGIN);

    if ( ch->pcdata->died_entrance == TRUE )
    {
     CHAR_DATA *wch;

        /*
         * Make things a little fancier                         -Thoric
         */
        if ( ( ( wch = get_char_room( ch, "newbie" ) ) != NULL ) ||
        ( ( wch = get_char_room( ch, "guard" ) ) != NULL ) ||
        ( ( wch = get_char_room( ch, "spawn" ) ) != NULL ) )
        {
            act( AT_MAGIC, "$n mutters a few incantations, waves $s hands and points $s finger.",wch, NULL, NULL, TO_ROOM );
            act( AT_MAGIC, "$n appears from some strange swirling mists!",ch, NULL, NULL, TO_NOTVICT );
            sprintf(buf, "Welcome back to the land of the living, %s",
                    capitalize( ch->name ) );
            do_say( wch, buf );
        }
        else
            act( AT_MAGIC, "$n appears from some strange swirling mists!",ch, NULL,NULL,TO_NOTVICT);
        ch->position = POS_RESTING;
        return;
    }

    if ( !sysdata.WAIT_FOR_AUTH && ch->level == 1 )
    {

   send_to_char("\n\r************************************************** \n\r",ch);
   send_to_char("***** AUTO AUTHORIZATION IS ON - AUTHORIZING *****\n\r",ch);
  send_to_char("**************************************************\n\r",ch);

        if ( ( location = find_location( ch,loc2,1) ) == NULL )
        {
            log_string( "AUTO-AUTH FAILED!!!: No such location");
            return;
        }

    REMOVE_BIT(ch->pcdata->flags, PCFLAG_UNAUTHED);
    ch->level = 2;
    advance_level(ch,TRUE,1);
    stop_fighting( ch, TRUE );
    char_from_room( ch );
    char_to_room( ch, location );
    do_look( ch, "auto" );
    do_save(ch,"");
    }

    break;
        /* Far too many possible screwups if we do it this way. -- Altrag */
/*        case CON_NEW_LANGUAGE:
        for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
		if ( !str_prefix( argument, lang_names[iLang] ) )
			if ( can_learn_lang( ch, lang_array[iLang] ) )
			{
				add_char( ch );
				SET_BIT( ch->speaks, lang_array[iLang] );
				set_char_color( AT_SAY, ch );
				ch_printf( ch, "You can now speak %s.\n\r", lang_names[iLang] );
				d->connected = CON_PLAYING;
				return;
			}
	set_char_color( AT_SAY, ch );
	write_to_buffer( d, "You may not learn that language.  Please choose another.\n\r"
				  "New language: ", 0 );
	break;*/

/* Death --GW */

    case CON_DEATH_SELECT:
	switch( *argument )
	{
	 case 'Y':
	 case 'y':
	    do_char_ressurect( d->character );
	    send_to_char("So be it.....\n\r",d->character);
	    send_to_char("Your Constitution Drops by 1.\n\r",d->character);
	    send_to_char("\n\rPress [ENTER]",ch);
	    d->connected = CON_READ_MOTD;
	    break;

	case 'N':
	case 'n':
	    do_char_notressurect( d->character );
	    send_to_char("So be it.....\n\r",d->character);
	    send_to_char("You drop 1 level.\n\r",ch);
	    send_to_char("\n\rPress [ENTER]",ch);
	    d->connected = CON_READ_MOTD;
	    break;

	default:
          write_to_buffer(d,"\nInvalid Choice.\n",0);
          send_to_char(DEATH_MENU,ch);
          d->connected = CON_DEATH_SELECT;
          return;
     }

}
    return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
   
    /*
     * Reserved words.
     */
    if ( is_name( name, "all auto immortal self newbie someone god supreme demigod dog guard cityguard cat cornholio spock hicaine hithoric death ass fuck shit piss crap quit" ) )
	return FALSE;

    /*
     * Length restrictions.
     */
    if ( strlen(name) <  3 )
	return FALSE;

    if ( strlen(name) > 14 )
	return FALSE;

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;
	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    /*
     * Code that followed here used to prevent players from naming
     * themselves after mobs... this caused much havoc when new areas
     * would go in...
     */

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;
/*    char buf[MSL];*/


    for ( ch = first_char; ch; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&& ( !fConn || !ch->desc )
	&&    ch->name
	&&   !str_cmp( name, ch->name ) )
	{
	  /*  if ( fConn && ch->switched )
	    {
	      write_to_buffer( d, "Already playing.\n\rName: ", 0 );
	      d->connected = CON_GET_NAME;
	      if ( d->character )
	      {
		clear descriptor pointer to get rid of bug message inlog 
		 d->character->desc = NULL;
		 free_char( d->character );
		 d->character = NULL;
	      }
	      return BERR;
	    }*/
	    if ( fConn == FALSE )
	    {
		DISPOSE( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		/* clear descriptor pointer to get rid of bug message in log */
		d->character->desc = NULL;
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		send_to_char( "Reconnecting.\n\r", ch );
		act( AT_ACTION, "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
		sprintf( log_buf, "%s@%s(%s) reconnected.", ch->name, d->host, d->user );
		ch->pcdata->pre_name_toggle = 1;
/*
    sprintf(buf,"gunzip -f %s%c/%s",PLAYER_DIR,tolower(name[0]),
    capitalize(name));
    system(buf);
*/
		log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->level ) );
/*
		if ( ch->level < LEVEL_SAVIOR )
		  to_channel( log_buf, CHANNEL_MONITOR, "[ ** ", ch->level
);
*/
		d->connected = CON_PLAYING;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name, bool kick )
{
    CHAR_DATA *ch;

    DESCRIPTOR_DATA *dold;
    int	cstate;

    for ( dold = first_descriptor; dold; dold = dold->next )
    {
	if ( dold != d
	&& (  dold->character || dold->original )
	&&   !str_cmp( name, dold->original
		 ? dold->original->name : dold->character->name ) )
	{
	    cstate = dold->connected;
	    ch = dold->original ? dold->original : dold->character;
	    if ( !ch->name
	    || ( cstate != CON_PLAYING && cstate != CON_EDITING ) )
	    {
		write_to_buffer( d, "Already connected - try again.\n\r", 0 );
		sprintf( log_buf, "%s already connected.", ch->name );
		log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
		return BERR;
	    }
	    if ( !kick )
	      return TRUE;

	    
//   	    if ( check_hosts(dold->character,d->host))
//		return TRUE;

	    write_to_buffer( d, "Already playing... Kicking off old connection.\n\r", 0 );
	    write_to_buffer( dold, "Kicking off old connection... bye!\n\r", 0 );
	    close_socket( dold, FALSE );
	    /* clear descriptor pointer to get rid of bug message in log */
	    d->character->desc = NULL;
	    free_char( d->character );
	    d->character = ch;
	    ch->desc	 = d;
	    ch->timer	 = 0;
/*	    if ( ch->switched )
	      do_return( ch->switched, "" );
	    ch->switched = NULL; */
	    send_to_char( "Reconnecting.\n\r", ch );
	    act( AT_ACTION, "$n has reconnected, kicking off old link.",
	         ch, NULL, NULL, TO_ROOM );
	    sprintf( log_buf, "%s@%s reconnected, kicking off old link.",
	             ch->name, d->host );
	    log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->level ) );
	    ch->pcdata->pre_name_toggle = 1;
/*
	    if ( ch->level < LEVEL_SAVIOR )
	      to_channel( log_buf, CHANNEL_MONITOR, "[ ** ", ch->level );
*/
	    d->connected = cstate;
	    return TRUE;
	}
    }

    return FALSE;
}


/* Bugged?
void old_stop_idling( CHAR_DATA *ch )
{
    if ( !ch
    ||   !ch->desc
    ||    ch->desc->connected != CON_PLAYING
    ||   !ch->was_in_room
    ||    ch->in_room != get_room_index( ROOM_VNUM_LIMBO,1 ) )
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( AT_ACTION, "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}
*/

void stop_idling( CHAR_DATA *ch )
{
    if ( !ch
    ||   !ch->desc
    ||    ch->desc->connected != CON_PLAYING
    ||  (ch->timer < 2))
	return;

    ch->timer = 0;
    return;
}



/*
 * Write to one char.
 */
/*void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( !ch )
    {
      bug( "Send_to_char: NULL *ch" );
      return;
    }

    if ( strlen(txt) > MAX_STRING_LENGTH )
       {
       bug("Send_to_char: string length to long! (*FATAL* -- REPORT THIS!)");
       return;
       }

    if ( txt && ch->desc )
        {
	write_to_buffer( ch->desc, txt, strlen(txt) );
        }

    return;
}
*/

/*
 * Same as above, but converts &color codes to ANSI sequences..
 */
bool send_to_char_color( char *txt, CHAR_DATA *ch )
{
  DESCRIPTOR_DATA *d;
  char *colstr;
  const char *prevstr = txt;
  char colbuf[20];
  int ln;
  
  if ( !ch )
  {
    bug( "Send_to_char_color: NULL *ch" );
    return FALSE;
  }
  if ( !txt || !ch->desc )
    return FALSE;

  d = ch->desc;

/* DONOT use this --GW */
/*  if ( d->outsize > 1000000 )
  {
    bug("outsize: %d: %s",d->outsize,txt);
   return FALSE;
  }
*/
  /* Clear out old color stuff */
/*  make_color_sequence(NULL, NULL, NULL);*/
  while ( (colstr = strpbrk(prevstr, "&^")) != NULL )
  {
    /* if we pass length=0, then write_to_buffer will do a strlen! */
    if (colstr!=prevstr)
    {
      if (( write_to_buffer(d, prevstr, (colstr-prevstr)))==FALSE)
      {
	return FALSE;
      }
    }
    ln = make_color_sequence(colstr, colbuf, d);
    if ( ln < 0 )
    {
      prevstr = colstr+1;
      break;
    }
    else if ( ln > 0 )
    {
     if (( write_to_buffer(d, colbuf, ln))==FALSE)
     {
	return FALSE;
     }
    }
    prevstr = colstr+2;
  }
  if ( *prevstr )
  {
   if( (  write_to_buffer(d, prevstr, 0))==FALSE)
   {
	return FALSE;
   }
  }
   return TRUE;
}

/*
 * Ripoff of Send to Char Color - Sends to a Desc Color (for login screen) --GW
 */
bool write_to_desc_color( char *txt, DESCRIPTOR_DATA *d )
{
  char *colstr;
  const char *prevstr = txt;
  char colbuf[20];
  int ln;
  
  if ( !d )
  {
    bug( "write_to_desc_color: NULL *d" );
    return FALSE;
  }

  if ( !txt )
    return FALSE;

/* Clear out old color stuff */
/*  make_color_sequence(NULL, NULL, NULL);*/
  while ( (colstr = strpbrk(prevstr, "&^")) != NULL )
  {
    /* if we pass length=0, then write_to_buffer will do a strlen! */
    if (colstr!=prevstr)
    {
      if (( write_to_buffer(d, prevstr, (colstr-prevstr)))==FALSE)
      {
	return FALSE;
      }
    }
    ln = make_color_sequence(colstr, colbuf, d);
    if ( ln < 0 )
    {
      prevstr = colstr+1;
      break;
    }
    else if ( ln > 0 )
    {
     if (( write_to_buffer(d, colbuf, ln))==FALSE)
     {
	return FALSE;
     }
    }
    prevstr = colstr+2;
  }
  if ( *prevstr )
  {
   if( (  write_to_buffer(d, prevstr, 0))==FALSE)
   {
	return FALSE;
   }
  }
   return TRUE;
}

bool write_to_pager( DESCRIPTOR_DATA *d, const char *txt, int length )
{
  if ( length <= 0 )
    length = strlen(txt);
  if ( length == 0 )
    return FALSE;
  if ( !d->pagebuf )
  {
    d->pagesize = MAX_STRING_LENGTH;
    CREATE( d->pagebuf, char, d->pagesize );
  }
  if ( !d->pagepoint )
  {
    d->pagepoint = d->pagebuf;
    d->pagetop = 0;
    d->pagecmd = '\0';
  }
  if ( d->pagetop == 0 && !d->fcommand )
  {
    d->pagebuf[0] = '\n';
    d->pagebuf[1] = '\r';
    d->pagetop = 2;
  }
  while ( d->pagetop + length >= d->pagesize )
  {
    if ( d->pagesize > 32000 )
    {
      bug( "Pager overflow.  Ignoring.\n\r" );
      d->pagetop = 0;
      d->pagepoint = NULL;
      DISPOSE(d->pagebuf);
      d->pagesize = MAX_STRING_LENGTH;
      return FALSE;
    }
    d->pagesize *= 2;
    RECREATE(d->pagebuf, char, d->pagesize);
  }
  strncpy(d->pagebuf+d->pagetop, txt, length);
  d->pagetop += length;
  d->pagebuf[d->pagetop] = '\0';
  return TRUE;
}

/*void send_to_pager( const char *txt, CHAR_DATA *ch )
{
  if ( !ch )
  {
    bug( "Send_to_pager: NULL *ch" );
    return;
  }
  if ( txt && ch->desc )
  {
    DESCRIPTOR_DATA *d = ch->desc;
    
    ch = d->original ? d->original : d->character;
    if ( IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
    {
	send_to_char(txt, d->character);
	return;
    }
    write_to_pager(d, txt, 0);
  }
  return;
}
*/

bool send_to_pager_color( char *txt, CHAR_DATA *ch )
{
  DESCRIPTOR_DATA *d;
  char *colstr;
  const char *prevstr = txt;
  char colbuf[20];
  int ln;
  
  if ( !ch )
  {
    bug( "Send_to_pager_color: NULL *ch" );
    return FALSE;
  }
  if ( !txt || !ch->desc )
    return FALSE;
  d = ch->desc;
  ch = d->original ? d->original : d->character;

  if ( IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
  {
   if ( ( send_to_char_color(txt, d->character) )==FALSE)
     return FALSE;
   else
    return TRUE;
  }

  /* Clear out old color stuff */
/*  make_color_sequence(NULL, NULL, NULL);*/
  while ( (colstr = strpbrk(prevstr, "&^")) != NULL )
  {
    if ( colstr > prevstr )
    {
      if ( ( write_to_pager(d, prevstr, (colstr-prevstr)) )==FALSE)
       return FALSE;
    } 
    ln = make_color_sequence(colstr, colbuf, d);
    if ( ln < 0 )
    {
      prevstr = colstr+1;
      break;
    }
    else if ( ln > 0 )
    {
    if( ( write_to_pager(d, colbuf, ln) )==FALSE)
	return FALSE;
    }
    prevstr = colstr+2;
  }
  if ( *prevstr )
  {
   if( (  write_to_pager(d, prevstr, 0))==FALSE)
	return FALSE;
  }
  return TRUE;
}

void set_char_color( sh_int AType, CHAR_DATA *ch )
{
    char buf[16];
    CHAR_DATA *och;
    
    if ( !ch || !ch->desc )
      return;
    
    och = ch;
    if ( !IS_NPC(och) && IS_SET(och->act, PLR_ANSI) )
    {
	if ( AType == 7 )
	  strcpy( buf, "\033[m" );
	else
	  sprintf(buf, "\033[0;%d;%s%dm", (AType & 8) == 8,
	        (AType > 15 ? "5;" : ""), (AType & 7)+30);

	write_to_buffer( ch->desc, buf, strlen(buf) );
    }
    return;
}

void set_pager_color( sh_int AType, CHAR_DATA *ch )
{
    char buf[16];
    CHAR_DATA *och;
    
    if ( !ch || !ch->desc )
      return;
    
    och = (ch->desc->original ? ch->desc->original : ch);
    if ( !IS_NPC(och) && IS_SET(och->act, PLR_ANSI) )
    {
	if ( AType == 7 )
	  strcpy( buf, "\033[m" );
	else
	  sprintf(buf, "\033[0;%d;%s%dm", (AType & 8) == 8,
	        (AType > 15 ? "5;" : ""), (AType & 7)+30);
	send_to_pager( buf, ch );
	ch->desc->pagecolor = AType;
    }
    return;
}


/* source: EOD, by John Booth <???> */
void ch_printf(CHAR_DATA *ch, char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];	/* better safe than sorry */
    va_list args;

    if ( fmt == NULL )
	return;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
	
    send_to_char(buf, ch);
}

void pager_printf(CHAR_DATA *ch, char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];
    va_list args;

    if ( fmt == NULL )
	return;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
	
    send_to_pager(buf, ch);
}

/*  From Erwin  */

void log_printf(char *fmt, ...)
{
  char buf[MAX_STRING_LENGTH*2];
  va_list args;

  if ( fmt == NULL )
	return;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  log_string(buf);
}

char *obj_short( OBJ_DATA *obj )
{
    static char buf[MAX_STRING_LENGTH];

    if ( obj->count > 1 )
    {
	sprintf( buf, "%s (%d)", obj->short_descr, obj->count );
	return buf;
    }
    return obj->short_descr;
}

/*
 * The primary output interface for formatted output.
 */
/* Major overhaul. -- Alty */
#define NAME(ch)	(IS_NPC(ch) ? ch->short_descr : ch->name)
char *act_string(const char *format, CHAR_DATA *to, CHAR_DATA *ch,
		 const void *arg1, const void *arg2)
{
  static char * const he_she  [] = { "it",  "he",  "she" };
  static char * const him_her [] = { "it",  "him", "her" };
  static char * const his_her [] = { "its", "his", "her" };
  static char buf[MAX_STRING_LENGTH];
  char fname[MAX_INPUT_LENGTH];
  char *point = buf;
  const char *str = format;
  const char *i;
  CHAR_DATA *vch = (CHAR_DATA *) arg2;
  OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
  OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;

  while ( *str != '\0' )
  {
    if ( *str != '$' )
    {
      *point++ = *str++;
      continue;
    }
    ++str;

    /* The $p crasher --GW */
    if ( isdigit( *str+1 ) )
    {
      *point++ = *str++;
      continue;
    }

    if ( !arg2 && *str >= 'A' && *str <= 'Z' )
    {
      bug( "Act: missing arg2 for code %c:", *str );
      bug( format );
      i = " <@@@> ";
    }
    else
    {
      switch ( *str )
      {
      default:  bug( "Act: bad code %c.", *str );
		i = " <@@@> ";						break;
      case 't': i = (char *) arg1;					break;
      case 'T': i = (char *) arg2;					break;
      case 'n': i = (to ? PERS( ch, to) : NAME( ch));			break;
      case 'N': i = (to ? PERS(vch, to) : NAME(vch));			break;
      case 'e': if (ch->sex > 2 || ch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", ch->name,
		      ch->sex);
		  i = "it";
		}
		else
		  i = he_she [URANGE(0,  ch->sex, 2)];
		break;
      case 'E': if (vch->sex > 2 || vch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", vch->name,
		      vch->sex);
		  i = "it";
		}
		else
		  i = he_she [URANGE(0, vch->sex, 2)];
		break;
      case 'm': if (ch->sex > 2 || ch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", ch->name,
		      ch->sex);
		  i = "it";
		}
		else
		  i = him_her[URANGE(0,  ch->sex, 2)];
		break;
      case 'M': if (vch->sex > 2 || vch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", vch->name,
		      vch->sex);
		  i = "it";
		}
		else
		  i = him_her[URANGE(0, vch->sex, 2)];
		break;
      case 's': if (ch->sex > 2 || ch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", ch->name,
		      ch->sex);
		  i = "its";
		}
		else
		  i = his_her[URANGE(0,  ch->sex, 2)];
		break;
      case 'S': if (vch->sex > 2 || vch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", vch->name,
		      vch->sex);
		  i = "its";
		}
		else
		  i = his_her[URANGE(0, vch->sex, 2)];
		break;
      case 'q': i = (to == ch) ? "" : "s";				break;
      case 'Q': i = (to == ch) ? "your" :
		    his_her[URANGE(0,  ch->sex, 2)];			break;
      case 'p': i = (!to || can_see_obj(to, obj1)
		  ? obj_short(obj1) : "something");			break;
      case 'P': i = (!to || can_see_obj(to, obj2)
		  ? obj_short(obj2) : "something");			break;
      case 'd':
        if ( !arg2 || ((char *) arg2)[0] == '\0' )
          i = "door";
        else
        {
          one_argument((char *) arg2, fname);
          i = fname;
        }
        break;
      }
    }
    ++str;
    while ( (*point = *i) != '\0' )
      ++point, ++i;
  }
  strcpy(point, "\n\r");
  buf[0] = UPPER(buf[0]);
  return buf;
}
#undef NAME
  
  
void act( sh_int AType, char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type )
{
    char *txt;
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *)arg2;
    CHAR_DATA *from_char;
    int num1 = 0;
    int num2 = 0;
    bool bad;
    char temp[MSL];

     txt = NULL;
     temp[0] = '\0';

    /* set bad to false */
    bad = FALSE;

    /*
     * Discard null and zero-length messages.
     */
    if ( !format || format[0] == '\0' )
	return;

    if ( !ch )
    {
	bug( "Act: null ch. (%s)", format );
	return;
    }

    if ( !ch->in_room )
    {
	bug( "Act: null ch->in_room --> (%s)",ch->name );
	return;
    }

    /* hopefully this puts a noose on the crap bug ..--GW */
    strcpy( temp, format );
    num1 = strlen(temp);
    /* scan it for crap .. break if there is any*/
    for ( num2 = 0; num2 <= num1; num2++ )
    {
      if ( temp[num2] == '\0' )
      break;

      if ( isspace(temp[num2]) || ( temp[num2] == ' ' ) 
      || ( temp[num2] == '$' ) || ( temp[num2] == '.' ) 
      || ( isgraph(temp[num2] ) ) || ( isalnum(temp[num2]) ) 
      || ( isprint( temp[num2]) ) )
      continue;
            
      bad = TRUE;
      bug("Act: Bad Format!(%s) at vnum: %d.",temp,ch->in_room->vnum);  
      break;
      
    }

    /* kick us out if its bad text */
    if ( bad == TRUE )
    return;


	to = NULL;

    from_char = ch;

    if ( !ch || !ch->in_room || !ch->in_room->first_person )
      to = NULL; 
    else if ( type == TO_CHAR )
      to = ch;
    else
      to = ch->in_room->first_person;

    /*
     * ACT_SECRETIVE handling
     */
    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_SECRETIVE) && type != TO_CHAR )
	return;

    if ( type == TO_VICT )
    {
	if ( !vch )
	{
	    bug( "Act: null vch with TO_VICT." );
	    bug( "%s (%s)", ch->name, format );
	    return;
	}
	if ( !vch->in_room )
	{
	    bug( "Act: vch in NULL room!" );
	    bug( "%s -> %s (%s)", ch->name, vch->name, format );
	    return;
	}
	to = vch;
    }

    if ( to == NULL )
    {
      return;
    } 
    if ( to->in_room == NULL )
    {
	return;
    }
   
   
    if ( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
    {
      OBJ_DATA *to_obj;

      txt = act_string(format, NULL, ch, arg1, arg2);

	if ( txt == NULL )
	{
	 bug("Act: NULL txt!");
	 return;
	}      

	if ( ch == NULL || to == NULL )
	{
	bug("Act: NULL ch!!");
	return;
	}

        if ( ch->in_room == NULL || to->in_room == NULL )
        return;

      if ( to && to->in_room && 
	  to->in_room->progtypes && IS_SET(to->in_room->progtypes,ACT_PROG) )
        rprog_act_trigger(txt, to->in_room, ch, (OBJ_DATA *)arg1, (void*)arg2);

      for ( to_obj = to->in_room->first_content; to_obj;
            to_obj = to_obj->next_content )
	  {
            if ( to_obj->pIndexData && IS_SET(to_obj->pIndexData->progtypes, ACT_PROG) )
            oprog_act_trigger(txt, to_obj, ch, (OBJ_DATA *)arg1, (void *)arg2);
	  }
    }

    /* Anyone feel like telling me the point of looping through the whole
       room when we're only sending to one char anyways..? -- Alty */
    for ( ; to; to = (type == TO_CHAR || type == TO_VICT)
                     ? NULL : to->next_in_room )
    {
	if ((!to->desc 
	&& (  IS_NPC(to) && !IS_SET(to->pIndexData->progtypes, ACT_PROG) ))
	||   !IS_AWAKE(to) )
	    continue;

	if ( type == TO_CHAR && to != ch )
	    continue;
	if ( type == TO_VICT && ( to != vch || to == ch ) )
	    continue;
	if ( type == TO_ROOM && to == ch )
	    continue;
	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;

	/* ignore */
	if ( is_ignoring( to, from_char ) )
	    continue;

	/* Vehicles */
/*	if ( !IS_NPC(ch) && ch->pcdata->vehicle && type != TO_CHAR &&
             !IS_NPC(to) && ( !to->pcdata->vehicle ||
           ( to->pcdata->vehicle != ch->pcdata->vehicle ) ) )
	   continue;*/

	txt = act_string(format, to, ch, arg1, arg2);
	if (to->desc)
	{
	  set_char_color(AType, to);
	  send_to_char_color(txt,to);
	}

	if (MOBtrigger)
        {
          /* Note: use original string, not string with ANSI. -- Alty */
	  mprog_act_trigger( txt, to, ch, (OBJ_DATA *)arg1, (void *)arg2 );
        }
    }
    MOBtrigger = TRUE;
    format = NULL;
    return;
}



void do_name( CHAR_DATA *ch, char *argument )
{
    char fname[1024];
    char fname2[1024];
    struct stat fst;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  bool fOld;

  fOld = FALSE;

one_argument( argument, arg);

if ( ( victim = get_char_world( ch, arg ) ) == NULL )
{
send_to_char( "They aren't playing.\n\r", ch);
return;
}

one_argument( (argument+strlen(victim->name)+1), arg2);

if ( arg[0] == '\0' || arg2[0] == '\0' )
{
  send_to_char("Syntax: rename <char> <new name>",ch);
  return;
}


if (get_trust(ch) <= get_trust(victim) && ch != victim )
{
send_to_char("No......... Idiot.\n\r",ch);
return;
}

  if (!check_parse_name(arg2))
  {
    send_to_char("Illegal name, try another.\n\r", ch);
    return;
  }

  sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(arg2[0]),
                        capitalize( arg2 ) );
  sprintf( fname2, "%s%c/%s", PLAYER_DIR, tolower(victim->name[0]),
			capitalize( victim->name ) );

  if ( stat( fname, &fst ) != -1 )
  {
    send_to_char("That name is already taken.  Please choose another.\n\r", ch);
    return;
  }


  arg2[0] = UPPER(arg2[0]);

  STRFREE( victim->name );
  victim->name = STRALLOC( arg2 );
  sprintf(buf,"%s has renamed you to '%s'.",QUICKLINK(ch->name),QUICKLINK(victim->name ));
  send_to_char(buf,victim);
  send_to_char("\n\r",victim);
  remove(fname2);
  do_title(victim,"");
  if ( IS_IMMORTAL( victim ) )
  {
  do_bamfin(victim,"");
  do_bamfout(victim,"");
  }
  do_save(victim,"");
  send_to_char("Done.\n\r",ch);
  return;
}
  
char *default_prompt( CHAR_DATA *ch )
{
  static char buf[60];

  strcpy(buf, "&w<&R%h/%Hhp ");
  if ( IS_VAMPIRE(ch) )
    strcat(buf, "&R%b/%Bbp");
  else
    strcat(buf, "&g%m/%Mm");
  strcat(buf, " &b%v/%Vmv &r<&W%c&r>&w> ");
  if ( IS_NPC(ch) || IS_IMMORTAL(ch) )
    strcat(buf, "%i%R");
  return buf;
}

int getcolor(char clr)
{
  static const char colors[16] = "xrgObpcwzRGYBPCW";
  int r;
  
  for ( r = 0; r < 16; r++ )
    if ( clr == colors[r] )
      return r;
  return -1;
}

void display_prompt( DESCRIPTOR_DATA *d )
{
  CHAR_DATA *ch = d->character;
  CHAR_DATA *och = (d->original ? d->original : d->character);
  bool ansi = (!IS_NPC(och) && IS_SET(och->act, PLR_ANSI));
  const char *prompt;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char fbuf[MAX_STRING_LENGTH];
  char wound[MAX_STRING_LENGTH];
  char *pbuf = buf;
  int stat;
  int percent;
  CHAR_DATA *victim;

  if ( !ch )
  {
    bug( "display_prompt: NULL ch" );
    return;
  }

  if ( !IS_NPC(ch) && ch->substate != SUB_NONE && ch->pcdata->subprompt
  &&   ch->pcdata->subprompt[0] != '\0' )
    prompt = ch->pcdata->subprompt;
  else
  if ( IS_NPC(ch) || !ch->pcdata->prompt || !*ch->pcdata->prompt )
    prompt = default_prompt(ch);
  else
    prompt = ch->pcdata->prompt;

  if ( ansi )
  {
    strcpy(pbuf, "\033[m");
    d->prevcolor = 0x07;
    pbuf += 3;
  }
  /* Clear out old color stuff */
/*  make_color_sequence(NULL, NULL, NULL);*/
  for ( ; *prompt; prompt++ )
  {
    /*
     * '&' = foreground color/intensity bit
     * '^' = background color/blink bit
     * '%' = prompt commands
     * Note: foreground changes will revert background to 0 (black)
     */
    if ( *prompt != '&' && *prompt != '^' && *prompt != '%' )
    {
      *(pbuf++) = *prompt;
      continue;
    }
    ++prompt;
    if ( !*prompt )
      break;
    if ( *prompt == *(prompt-1) )
    {
      *(pbuf++) = *prompt;
      continue;
    }
    switch(*(prompt-1))
    {
    default:
      bug( "Display_prompt: bad command char '%c'.", *(prompt-1) );
      break;
    case '&':
    case '^':
      stat = make_color_sequence(&prompt[-1], pbuf, d);
      if ( stat < 0 )
        --prompt;
      else if ( stat > 0 )
        pbuf += stat;
      break;
    case '%':
      *pbuf = '\0';
      stat = 0x80000000;
      switch(*prompt)
      {
      case '%':
	*pbuf++ = '%';
	*pbuf = '\0';
	break;
      case 'a':
	if ( ch->level >= 10 )
	  stat = ch->alignment;
	else if ( IS_GOOD(ch) )
	  strcpy(pbuf, "good");
	else if ( IS_EVIL(ch) )
	  strcpy(pbuf, "evil");
	else
	  strcpy(pbuf, "neutral");
	break;
        case 'p' :
	  stat = get_power(ch);
	  break;
	case 'P' :
	  if( ch->fighting && (victim = ch->fighting->who) != NULL)
	  {
	     stat = get_power(victim);
	  }
	  else
	     stat = 0;
	  break; 
        case 'Y' :
/* This graphical bar shows the player's condition -- Cal */
  
        if (ch->fighting && (victim = ch->fighting->who) != NULL)
        {
         if (ch->max_hit > 0)
                percent = ch->hit * 100 / ch->max_hit;
            else
                percent = -1;
            if (percent >= 100)
                sprintf(wound,"&WYou: [^r___^O___^g____^x&W]");
            else if (percent >= 90)
                sprintf(wound,"&WYou: [^r___^O___^g___^x &W]");
            else if (percent >= 80)
                sprintf(wound,"&WYou: [^r___^O___^g__^x  &W]");
            else if (percent >= 70)
                sprintf(wound,"&WYou: [^r___^O___^g_^x   &W]");
            else if (percent >= 58) 
                sprintf(wound,"&WYou: [^r___^O___^x    &W]");
            else if (percent >= 45)
                sprintf(wound,"&WYou: [^r___^O__^x     &W]");
            else if (percent >= 30)
                sprintf(wound,"&WYou: [^r___^O_^x      &W]");  
            else if (percent >= 28)
                sprintf(wound,"&WYou: [^r___&W^x       &W]");  
            else if (percent >= 15)
                sprintf(wound,"&WYou: [^r__&W^x        &W]");  
            else if (percent >= 8) 
                sprintf(wound,"&WYou: [^r_&W^x         &W]");  
            else 
                sprintf(wound,"&WYou: [   &RDEAD!!!  ^x&W]");  
            sprintf(buf2," %s",wound);
            strcpy(pbuf,buf2);
        }
        else
           strcpy(pbuf, "<>");
        break;
/* Ok yes that was a cheap copy of the one below :P but in the interests
 * of being complete 
 */
        case 'Z' :

/*    this is the graphical battle damage prompt
 *
 *    <- Gothar 1997 ->
 */

        if (ch->fighting && (victim = ch->fighting->who) != NULL)
        {
            if (victim->max_hit > 0)
                percent = victim->hit * 100 / victim->max_hit;
            else
                percent = -1;
            if (percent >= 100)
                sprintf(wound,"&WEnemy: [^r___^O___^g____^x&W]");
            else if (percent >= 90)
                sprintf(wound,"&WEnemy: [^r___^O___^g___^x &W]");
            else if (percent >= 80)
                sprintf(wound,"&WEnemy: [^r___^O___^g__^x  &W]");
            else if (percent >= 70)
                sprintf(wound,"&WEnemy: [^r___^O___^g_^x   &W]");
            else if (percent >= 58)
                sprintf(wound,"&WEnemy: [^r___^O___^x    &W]");
            else if (percent >= 45)
                sprintf(wound,"&WEnemy: [^r___^O__^x     &W]");
            else if (percent >= 30)
                sprintf(wound,"&WEnemy: [^r___^O_^x      &W]");
            else if (percent >= 28)
                sprintf(wound,"&WEnemy: [^r___&W^x       &W]");
            else if (percent >= 15)
                sprintf(wound,"&WEnemy: [^r__&W^x        &W]");
            else if (percent >= 8)
                sprintf(wound,"&WEnemy: [^r_&W^x         &W]");
            else
                sprintf(wound,"&WEnemy: [   &RDEAD!!!  ^x&W]");
            sprintf(buf2," %s",wound);
            strcpy(pbuf,buf2);
        }
        else
           strcpy(pbuf, "<>");
        break;
        case 'z' :

/*    this is the percentage battle damage prompt
 *    The prompt changes colour to show the
 *    condition of the mob.
 *    <- Gothar 1997 ->
 */
           if (ch->fighting && (victim = ch->fighting->who) != NULL)
           {
              percent = victim->hit * 100 / victim->max_hit;
              if(percent >= 65)
                 sprintf(buf2," &WEnemy: &O%d&W%%",percent);
              else if(percent >= 25 && percent < 65)
                 sprintf(buf2," &WEnemy: &O%d&W%%",percent);
              else
                 sprintf(buf2," &WEnemy: &r%d&W%%",percent);
              strcpy(pbuf,buf2);
           }
           else
              strcpy(pbuf, "None");
           break;

      case 'f':
	if ( ch->pcdata->deity )
	  stat = ch->pcdata->favor;
	else
	  strcpy(pbuf, "no favor");
	break;
      case 'h':
	stat = ch->hit;
	break;
      case 'H':
	stat = ch->max_hit;
	break;
      case 'm':
	if ( IS_VAMPIRE(ch) )
	  stat = 0;
	else
	  stat = ch->mana;
	break;
      case 'M':
	if ( IS_VAMPIRE(ch) )
	  stat = 0;
	else
	  stat = ch->max_mana;
	break;
      case 'b':
	if ( IS_VAMPIRE(ch) )
	  stat = ch->pcdata->condition[COND_BLOODTHIRST];
	else
	  stat = 0;
	break;
      case 'B':
	if ( IS_VAMPIRE(ch) )
	  stat = get_bloodthirst(ch);
	else
	  stat = 0;
	break;
      case 'u':
	stat = get_mortal_num_desc(ch);
	break;
      case 'U':
	stat = sysdata.maxplayers;
	break;
      case 'v':
	stat = ch->move;
	break;
      case 'V':
	stat = ch->max_move;
	break;
      case 'g':
	stat = ch->gold;
	break;
      case 'r':
	if ( IS_IMMORTAL(och) )
	  stat = ch->in_room->vnum;
	break;
      case 'R':
	if ( IS_SET(och->act, PLR_ROOMVNUM) )
	  sprintf(pbuf, "<#%d> ", ch->in_room->vnum);
	break;
      case 'x':
	stat = ch->exp;
	break;
      case 'X':
	stat = exp_level(ch, ch->level+1) - ch->exp;
	break;
      case 'i':
	if ( (!IS_NPC(ch) && IS_SET(ch->act, PLR_WIZINVIS)) ||
	      (IS_NPC(ch) && IS_SET(ch->act, ACT_MOBINVIS)) )
	  sprintf(pbuf, "(Invis %d) ", (IS_NPC(ch) ? ch->mobinvis : ch->pcdata->wizinvis));
	else
	if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
	  sprintf(pbuf, "(Invis) " );
	if ( ( !IS_NPC(ch) && IS_SET(ch->pcdata->flagstwo, MOREPC_INCOG) ) &&
	((!IS_NPC(ch) && IS_SET(ch->act, PLR_WIZINVIS)) ||
	      (IS_NPC(ch) && IS_SET(ch->act, ACT_MOBINVIS)) ) )
	  sprintf(pbuf,"(Incog %d) (Invis %d) ", ch->pcdata->incog_level,
		ch->pcdata->wizinvis );
	  else if ( ( !IS_NPC(ch) && IS_SET(ch->pcdata->flagstwo,MOREPC_INCOG) ) )
	  sprintf(pbuf,"(Incog %d) ", ch->pcdata->incog_level);
	break;
      case 'I':
	stat = (IS_NPC(ch) ? (IS_SET(ch->act, ACT_MOBINVIS) ? ch->mobinvis : 0)
	     : (IS_SET(ch->act, PLR_WIZINVIS) ? ch->pcdata->wizinvis : 0));
	break;
      case 'c':

	 if(ch->fighting && ch->fighting->who )
	 {

    if ( ch->fighting->who->max_hit > 0 )
        percent = ( 100 * ch->fighting->who->hit ) / ch->fighting->who->max_hit;
    else
        percent = -1;

         if ( percent >= 100 ) strcpy( fbuf, "perfect health");
    else if ( percent >=  90 ) strcpy( fbuf, "slightly scratched");
    else if ( percent >=  80 ) strcpy( fbuf, "few bruises");
    else if ( percent >=  70 ) strcpy( fbuf, "some cuts");
    else if ( percent >=  60 ) strcpy( fbuf, "several wounds");
    else if ( percent >=  50 ) strcpy( fbuf, "nasty wounds");
    else if ( percent >=  40 ) strcpy( fbuf, "bleeding freely");
    else if ( percent >=  30 ) strcpy( fbuf, "covered in blood");
    else if ( percent >=  20 ) strcpy( fbuf, "leaking guts");
    else if ( percent >=  10 ) strcpy( fbuf, "almost dead");
    else                       strcpy( fbuf, "DYING");

    fbuf[0] = UPPER(fbuf[0]);
      }
	 sprintf(pbuf,"%s",ch->fighting ? fbuf : " ");          
	break;

      }
      if ( stat != 0x80000000 )
	sprintf(pbuf, "%d", stat);
      pbuf += strlen(pbuf);
      break;
    }
  }
  *pbuf = '\0';
  send_to_char(buf, ch);
  return;
}

/*
 * Dont let people using %u see invis immortals --GW
 */
int get_mortal_num_desc( CHAR_DATA *ch )
{
int num;
DESCRIPTOR_DATA *d;

num = 0;
for ( d = first_descriptor; d; d = d->next )
{
if ( can_see( ch, d->character ) )
num++;
}

return num;
}


int make_color_sequence(const char *col, char *buf, DESCRIPTOR_DATA *d)
{
  int ln;
  const char *ctype = col;
  unsigned char cl;
  CHAR_DATA *och;
  bool ansi=TRUE;
  
  och = (d->original ? d->original : d->character);

  if ( och )
  ansi = (!IS_NPC(och) && IS_SET(och->act, PLR_ANSI));

  col++;
  if ( !*col )
    ln = -1;
  else if ( *ctype != '&' && *ctype != '^' )
  {
    bug("Make_color_sequence: command '%c' not '&' or '^'.", *ctype);
    ln = -1;
  }
  else if ( *col == *ctype )
  {
    buf[0] = *col;
    buf[1] = '\0';
    ln = 1;
  }
  else if ( !ansi )
    ln = 0;
  else
  {
    cl = d->prevcolor;
    switch(*ctype)
    {
    default:
      bug( "Make_color_sequence: bad command char '%c'.", *ctype );
      ln = -1;
      break;
    case '&':
      if ( *col == '-' )
      {
        buf[0] = '~';
        buf[1] = '\0';
        ln = 1;
        break;
      }
    case '^':
      {
        int newcol;
        
        if ( (newcol = getcolor(*col)) < 0 )
        {
          ln = 0;
          break;
        }
        else if ( *ctype == '&' )
          cl = (cl & 0xF0) | newcol;
        else
          cl = (cl & 0x0F) | (newcol << 4);
      }
      if ( cl == d->prevcolor )
      {
        ln = 0;
        break;
      }
      strcpy(buf, "\033[");
      if ( (cl & 0x88) != (d->prevcolor & 0x88) )
      {
        strcat(buf, "m\033[");
        if ( (cl & 0x08) )
          strcat(buf, "1;");
        if ( (cl & 0x80) )
          strcat(buf, "5;");
        d->prevcolor = 0x07 | (cl & 0x88);
        ln = strlen(buf);
      }
      else
        ln = 2;
      if ( (cl & 0x07) != (d->prevcolor & 0x07) )
      {
        sprintf(buf+ln, "3%d;", cl & 0x07);
        ln += 3;
      }
      if ( (cl & 0x70) != (d->prevcolor & 0x70) )
      {
        sprintf(buf+ln, "4%d;", (cl & 0x70) >> 4);
        ln += 3;
      }
      if ( buf[ln-1] == ';' )
        buf[ln-1] = 'm';
      else
      {
        buf[ln++] = 'm';
        buf[ln] = '\0';
      }
      d->prevcolor = cl;
    }
  }
  if ( ln <= 0 )
    *buf = '\0';
  return ln;
}

void set_pager_input( DESCRIPTOR_DATA *d, char *argument )
{
  while ( isspace(*argument) )
    argument++;
  d->pagecmd = *argument;
  return;
}

bool pager_output( DESCRIPTOR_DATA *d )
{
  register char *last;
  CHAR_DATA *ch;
  int pclines;
  register int lines;
  bool ret;

  if ( !d || !d->pagepoint || d->pagecmd == -1 )
    return TRUE;
  ch = d->original ? d->original : d->character;
  pclines = UMAX(ch->pcdata->pagerlen, 5) - 1;
  switch(LOWER(d->pagecmd))
  {
  default:
    lines = 0;
    break;
  case 'b':
    lines = -1-(pclines*2);
    break;
  case 'r':
    lines = -1-pclines;
    break;
  case 'q':
    d->pagetop = 0;
    d->pagepoint = NULL;
    flush_buffer(d, TRUE);
    DISPOSE(d->pagebuf);
    d->pagesize = MAX_STRING_LENGTH;
    return TRUE;
  }
  while ( lines < 0 && d->pagepoint >= d->pagebuf )
    if ( *(--d->pagepoint) == '\n' )
      ++lines;
  if ( *d->pagepoint == '\n' && *(++d->pagepoint) == '\r' )
      ++d->pagepoint;
  if ( d->pagepoint < d->pagebuf )
    d->pagepoint = d->pagebuf;
  for ( lines = 0, last = d->pagepoint; lines < pclines; ++last )
    if ( !*last )
      break;
    else if ( *last == '\n' )
      ++lines;
  if ( *last == '\r' )
    ++last;
  if ( last != d->pagepoint )
  {
    if ( !write_to_descriptor(d->descriptor, d->pagepoint,
          (last-d->pagepoint)) )
      return FALSE;
    d->pagepoint = last;
  }
  while ( isspace(*last) )
    ++last;
  if ( !*last )
  {
    d->pagetop = 0;
    d->pagepoint = NULL;
    flush_buffer(d, TRUE);
    DISPOSE(d->pagebuf);
    d->pagesize = MAX_STRING_LENGTH;
    return TRUE;
  }
  d->pagecmd = -1;
  if ( IS_SET( ch->act, PLR_ANSI ) )
      if ( write_to_descriptor(d->descriptor, "\033[1;36m", 7) == FALSE )
	return FALSE;
  if ( (ret=write_to_descriptor(d->descriptor,
	"(C)ontinue, (R)efresh, (B)ack, (Q)uit: [C] ", 0)) == FALSE )
	return FALSE;
  if ( IS_SET( ch->act, PLR_ANSI ) )
  {
      char buf[32];

      if ( d->pagecolor == 7 )
	strcpy( buf, "\033[m" );
      else
	sprintf(buf, "\033[0;%d;%s%dm", (d->pagecolor & 8) == 8,
		(d->pagecolor > 15 ? "5;" : ""), (d->pagecolor & 7)+30);
      ret = write_to_descriptor( d->descriptor, buf, 0 );
  }
  return ret;
}

/*  Warm reboot stuff, gotta make sure to thank Erwin for this :) */

void do_copyover (CHAR_DATA *ch, char * argument)
{
  FILE *fp;
  DESCRIPTOR_DATA *d, *d_next;
  char buf [100], buf2[100], buf3[100], buf4[100], buf5[100], buf6[100];
  disable_timer_abort = TRUE;

  fp = fopen (COPYOVER_FILE, "w");

  if (!fp && ch )
   {
      send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
      log_printf ("Could not write to copyover file: %s", COPYOVER_FILE);
      perror ("do_copyover:fopen");
      return;
   }

    /* Save the boot time and max players this boot --GW */
	sysdata.boottime = boot_time;
        sysdata.max_plrs_boot = sysdata.maxplayers; 
	save_sysdata(sysdata);

    /* Fix here for invis imms doing a warmboot, dont want name displayed*/
    /* --GW */

/*    strcpy(name,ch->name);
    
    if (IS_SET(ch->act,PLR_WIZINVIS))
    strcpy(name,"Someone");*/

    /* Nuke any challenges in progress */
    challenge_cancel(NULL,NULL,"all");
    stop_viewers();

    /* Consider changing all saved areas here, if you use OLC */

    /* do_asave (NULL, ""); - autosave changed areas */
    strcpy(buf, "\n\rSwirls of Black Chaos Suddenly Surround You, as Your World Stops...\n\r ");
     /* For each playing descriptor, save its state */
    for (d = first_descriptor; d ; d = d_next)
     {
       CHAR_DATA * och = CH(d);
       d_next = d->next; /* We delete from the list , so need to save this */
       if (!d->character || d->connected < 0) /* drop those logging on */
         {
            write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting."
               " Come back in a few minutes.\n\r", 0);
            close_socket (d, FALSE); /* throw'em out */
          }
        else
          {
             fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
             if (och->level == 1)
               {
                  write_to_descriptor (d->descriptor, "Since you are level one,"
                     "and level one characters do not save, you gain a free level!\n\r",
                      0);
                  advance_level (och,TRUE,1);
                  och->level++; /* Advance_level doesn't do that */
               }
		  do_locker( och, "close" );
                  save_char_obj (och);
                  write_to_descriptor (d->descriptor, buf, 0);
           }
      }
        fprintf (fp, "-1\n");
        new_fclose (fp);

        /* Close reserve and other always-open files and release other resources */

        /* Shutdown Web Server */
	if ( sysdata.webserv == 1 )
        shutdown_web(); 

//        new_fclose (fpReserve);
        new_fclose (fpLOG);

        /* exec - descriptors are inherited */
	/* Changed this abit .. as I have added 'arguments' to execution,
	   this adds the -w if it is not there, which means 'Warmboot' --GW */


        sprintf (buf, "%d", port);
        sprintf (buf2, "%d", control);

     if ( !mudarg_scan('W') ) /* Add the W if it isnt already there --GW */ 
	sprintf (buf6, "-%sW", mudargs);
     else
	sprintf (buf6, "-%s", mudargs);

        execl (EXE_FILE, "AOTA", buf, buf6, buf2, buf3,
          buf4, buf5, (char *) NULL);

        /* Failed - sucessful exec will not return */

        perror ("do_copyover: execl");
//        send_to_char ("Copyover FAILED!\n\r",ch);

        /* Here you might want to reopen fpReserve */
        /* Since I'm a neophyte type guy, I'll assume this is
           a good idea and cut and past from main()  */

/*        if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
         {
           perror( NULL_FILE );
           exit( 1 );
         }*/
        if ( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
         {
           perror( NULL_FILE );
           exit( 1 );
         }

}

/* Recover from a copyover - load players */
void copyover_recover ()
{
  DESCRIPTOR_DATA *d=NULL;
  FILE *fp;
  char name [100];
  char host[MAX_STRING_LENGTH];
  int desc;
  bool fOld;
  ROOM_INDEX_DATA *location=NULL;
  char buf[MSL];

  log_string ("Copyover recovery initiated");

  num_descriptors=0;
  sysdata.maxplayers = sysdata.max_plrs_boot;

  fp = fopen (COPYOVER_FILE, "r");

  if (!fp) /* there are some descriptors open which will hang forever then ? */
        {
          perror ("copyover_recover:fopen");
          log_string("Copyover file not found. Exitting.\n\r");
           exit (1);
        }

  unlink (COPYOVER_FILE); /* In case something crashes
                              - doesn't prevent reading */

  for (;;)
   {
     fscanf (fp, "%d %s %s\n", &desc, name, host);
     if (desc == -1)
       break;

        /* Write something, and check if it goes error-free */
     if (!write_to_descriptor (desc, "\n\rThe Planets Begin to realign, as Your World Slowly Takes Shape.\n\r",0))
       {
         close (desc); /* nope */
         continue;
        }

      d=NULL;
      CREATE(d, DESCRIPTOR_DATA, 1);
      init_descriptor (d, desc); /* set up various stuff */

      d->host = STRALLOC( host );

      LINK( d, first_descriptor, last_descriptor, next, prev );
      d->connected = CON_COPYOVER_RECOVER; /* negative so close_socket
                                              will cut them off */

      num_descriptors++;

       /* Now, find the pfile */

      fOld = load_char_obj (d, name, FALSE);

      if (!fOld) /* Player file not found?! */
       {
          write_to_descriptor (desc, "\n\rSomehow, your character was lost in the copyover sorry.\n\r", 0);
          close_socket (d, FALSE);
       }
      else /* ok! */
       {
          write_to_descriptor (desc, "\n\rA Blinding Light Fills Your Eyes, as a Stunning New Dawn Encompasses this World!\n\r",0);

	  /* put the char back in the room --GW */
	  location=NULL;
	  if ((location=get_room_index((int)d->character->pcdata->warm_to_room,1))!=NULL)
          {
		if ( d->character->in_room )
		  char_from_room(d->character);

		char_to_room(d->character,location);

		sprintf(buf,"WARMBOOT: putting %s back in room %d",capitalize(d->character->name),location->vnum);
		log_string_plus(buf,LOG_NORMAL,d->character->level);
	  }

           /* Just In Case,  Someone said this isn't necassary, but _why_
              do we want to dump someone in limbo? */
           if (!d->character->in_room)
                char_to_room(d->character,get_room_index(ROOM_VNUM_TEMPLE,1));

           /* Insert in the char_list */
/*           LINK( d->character, first_char, last_char, next, prev );*/

/*-->*/	     add_char(d->character);
/*might use this instead ..*/
	   area_reset_check(d->character->in_room);
//           char_to_room (d->character, d->character->in_room);
	   d->character->pcdata->pre_name_toggle = 1;
           do_look (d->character, "auto noprog");
           act (AT_ACTION, "$n materializes!", d->character, NULL, NULL, TO_ROOM);
	   d->character->pcdata->arena_viewing = STRALLOC("None");
           d->connected = CON_PLAYING;
	   disable_timer_abort = FALSE;
       }

   }
}

