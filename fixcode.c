/* fixcode.c -- fixes address of 5 character code in record */
/* code and record specified in command */

static char VERSION[] = "137@(#) fixcode.c 137.1@(#) 10/2/00 14:27:58";

#include        "iiimacro.h"
#include		<stdio.h>
#include		<ctype.h>
#include		<string.h>
#include		"cdebug.h"
#include		"iiitypes.h"
#include		"gethash.h"
#include		"wrec.h"
#include 		"cfix.h"
#include 		"cset.h"
#include 		"fldutil.h"
#include 		"range.h"
#include 		"ndx.h"
#include 		"menu.h"
#include 		"input.h"
#include 		"getput.h"
#include 		"adf.h"

#define         MAX_RANGE   100

typedef	struct
	{
	int		ndx_address;
	char	*name;
	} code_rec;

/* this order corresponds to the ndx module */
static code_rec CodeArray[] = {
	0,			"",                 /* unused */
	NDX_BRANCH,	"branch",
	NDX_FUND, 	"fbal",
	NDX_VENDOR, "vendors",
	NDX_FIRM, 	"firmfile" };

typedef	struct
	{
	int	ndx_address;
	int	rectype;
	int	offset;
	} offset_rec;

/* no particular order necessary, update usage as well as here */
static offset_rec OffsetArray[] = {
	NDX_BRANCH, 'b', f_b_branch,
	NDX_BRANCH, 'o', f_o_branch,
	NDX_BRANCH, 'c', f_c_branch,
	NDX_BRANCH, 'i', f_i_location,
	NDX_BRANCH, 'p', f_p_location,
	NDX_BRANCH, 'r', f_r_location,
	NDX_FUND,   'o', f_o_fund,
	NDX_VENDOR, 'o', f_o_vendor,
	NDX_VENDOR, 'c', f_c_vendor,
	NDX_FIRM,   'p', f_p_firm,
	0,          ' ', 0,   /* end of struct */
	};

static int		Verbose = 0;
/* resent modification for using address to change code --lle*/
static int              USE_ADDR = 0;
static int		ReportOnly = 0;
static char		RangeSt[ MAX_RANGE ];

#ifdef HAS_PROTOTYPE
#define P(a)		a
#else
#define P(a)		()
#endif

static char	*get_code_loc P((workrec *, int));

static void decide_fix P((struct t_ndx *, workrec *, char *, char *, int ));
static void fix_code P((struct t_ndx *, workrec *, char *, int));
static void fix_one_code P((struct t_ndx *, workrec *, char *, int));
static void fix_tag_code P((struct t_ndx *, workrec *, char *, int));
static void main_loop P((int));
static void process P((struct t_ndx *, workrec *, int, int, int));
static void start_up P((int, int));
static void usage P((void));
static void user_stop P((int));
static void set_unit P((int, char *));

static int none_special(char *, char *, int , int *);
static int multi_special(char *, char *, int , int * );
static int valid_field(struct t_ndx *, int, char *, char *);
/*-------------------------------------------------------------------*/
main( argc,argv )
int		argc;
char	**argv;
{
int			code_index = 0;
int 		confirm_range = 0;
int         optc;
extern char	*optarg;

iiistart;

while ( ( optc =getopt( argc, argv, "ubn:cd:f:r:v:a" ) ) != EOF )
    {
    switch ( optc )
		{
		case 'u':
			ReportOnly = TRUE;
			break;
		case 'v':
		    puts( VERSION );
			exit( 0 );
		case 'n': /* index for file to fix */
			code_index = atol( optarg );
			break;
		case 'b':
			Verbose = 1;
			break;
		case 'c':
			confirm_range = 1;
			break;
		case 'r':
			strcpy( RangeSt, optarg );
		    break;
/* resent modification for using address to change code --lle*/
		case 'a':                      
		        USE_ADDR = 1;
			break;
		case 'd':
#ifdef INCLUDE_DEBUG
	    	*c_debug = 1;
	    	if ( !set_debug( optarg ) )
				{
	       		fprintf( stderr, "set-debug failed" );
	       		return( 2 );
	    		}
#endif
	    	break;
		default:
			usage();
		}
    }

start_up( code_index, confirm_range );
main_loop( code_index );

exit( 0 );
}
/*------------- -end main-- -- -------------------------------*/
static void
start_up( code_index, confirm_range )
int		code_index, confirm_range;
{
if ( code_index == 0 )
	usage();

if ( CodeArray[code_index].ndx_address == NDX_FUND ||
	 CodeArray[code_index].ndx_address == NDX_VENDOR )
	set_unit( code_index, RangeSt );

if ( confirm_range && range_greater_than( RangeSt, MAX_RANGE ) )
	{
    puts( "\007Range is more than 100 records!!!" );
	if ( !yes_or_no( "Are you sure you want to do this?" ) )
		exit( 0 );
	}
}
/*-------------------------------------------------------------*/
static void
main_loop( code_index )
int		code_index;
{
struct t_ndx	*id = ndx_open( CodeArray[code_index].ndx_address );
workrec 		*ww = wrec_new( 0 );
int				recnum;
char			rectype;

if ( ww == NULL )
	{
	puts( "cannot malloc work rec" );
	exit( 1 );	
	}

while ( range_nextnum( RangeSt, &rectype, &recnum ) == 0 )
	{
	user_stop( recnum );
	process( id, ww, rectype, recnum, code_index );
	}

ndx_close( id );
wrec_free( ww );
putchar( '\n' );
}
/*-------------------------------------------------------------*/
static void
user_stop( num )
int     num;
{
if ( c_ifkey() && inset( (unsigned) c_readkbd(), "csqCSQ\033" ) )
	{
	printf( "\n\nnow at %d\n\n\n", num );
	cmenu_init( 20, 18, 60, 23, 1 );
	cmenu_load( "C > CONTINUE Processing" );
	cmenu_load( "Q > QUIT" );
	if ( ( cmenu_go( (int *) 0 ) == 'q' ) || *c_escape )
		exit( 0 );
	puts( "\n\n\n\n" );
	}
}
/*-------------------------------------------------------------------*/
static void
usage()
{
puts( "Usage: fixcode [-r RangeStr] [options] args" );
puts( "      -b          verbose" );
puts( "      -n num      index of code to fix" );
puts( "          1 -- branches, brname ( bocipr )" );
puts( "          2 -- fbal, fundname ( o )" );
puts( "          3 -- vendors, venname ( oc )" );
puts( "          4 -- firmfile, firmname ( p )" );
puts( "      -a use address to fix code");
puts( "       -u          report only, no updates performed" );
puts( "       -v          print version" );
puts( "       -d nums     set debug flags" );

exit( 0 );
}
/*-------------------------------------------------------------------*/
static void
set_unit( code_index, range_st )
int		code_index;
char	*range_st;
{
#define UNIT_QUERY "Select the account unit within which to check the codes: "

workrec *ww = wrec_new( 0 );
int     new_unit;
int     max_unit;
int		recnum;
char    *cur_unit = getenv( "ACCTUNIT" );
char    *recpos;
char    rectype;

if ( !cur_unit )
	cur_unit = "0";

recpos = strchr( range_st, ORDER_TYPE );
if ( !recpos )
	{
	if ( CodeArray[code_index].ndx_address == NDX_FUND )
		return;

	recpos = strchr( range_st, CHECK_TYPE );
	if ( !recpos )
		return;

	max_unit = max_serialunit();
	}
else
	max_unit = max_acctunit();

if ( max_unit > 0 )
	{
	if ( isdigit( recpos[1] ) )
		{
		if ( range_nextnum( range_st, &rectype, &recnum ) == -1 )
			exit( 1 );
		range_clear();

		wrec_setnum( ww, rectype, recnum );
		new_unit = rec_unit( ww );
		if ( new_unit == -1 )
			{
			printf( "Failed to determin appropriate unit for %c%d", rectype,
					recnum );
			exit( 1 );
			}
		
		if ( new_unit == atoi( cur_unit ) )
			 return;

		msg_waitf( "\007Changing account/serial unit to %d\n", new_unit );
		fflush( stdout );
		}
	else if ( !memcmp( cur_unit, "0", 2 ) )
		{
		new_unit = get_a_number( UNIT_QUERY, 1, max_unit );
		if ( *c_escape )
			exit( 0 );
		}
	else
		return;

	if ( acctunit_set( new_unit ) == -1 )
		{
		puts( "\007Failed to set account/serial unit" );
		exit( 1 );
		}
	}
}
#undef UNIT_QUERY
/*-------------------------------------------------------------------*/
static void
process( id, ww, rectype, recnum, code_index )
struct t_ndx	*id;
workrec 		*ww;
char    		rectype;
int				recnum, code_index;
{
char	*code_loc;

wrec_clear( ww );
wrec_setnum( ww, rectype, recnum );
if ( wrec_load( ww, "r" ) == 1 )
	{
	code_loc = get_code_loc( ww, code_index );
	if ( code_loc )
		fix_code( id, ww, code_loc, code_index );
	else if ( Verbose )
		printf("\nFailed to locate code in record %c%d\n", rectype, recnum);
	}
else if ( Verbose )
	printf( "\n%s:%c%d:%s", CodeArray[code_index].name, rectype, recnum,
			wrec_error( ww ) );
}
/*-------------------------------------------------------------------*/
static void
fix_tag_code( id, ww, valid_tags, code_index )
struct t_ndx	*id;
workrec			*ww;
char			*valid_tags;
int				code_index;
{
int		total;
char	*fld = fld_find( ww, CNULL, valid_tags, FALSE );
char	*ptr;

if ( fld )
	{
	total = (fld_len( fld ) - 3) / 9;
	for ( ptr = fld + 7; total--; ptr += 9 )
		fix_one_code( id, ww, ptr + 2, code_index );
	}
else
	printf( "\nrecord %c%d has %s code \"multi\" but no %c field\n",
			ww->rectype, ww->recnum, CodeArray[code_index].name, *valid_tags );
}
/*-------------------------------------------------------------------*/
static void
fix_code( id, ww, code_loc, code_index )
struct t_ndx	*id;
workrec 		*ww;
char   			*code_loc;
int     		code_index;
{
if ( get16( code_loc ) == -1 )	/* multi code */
	{
	if ( CodeArray[code_index].ndx_address == NDX_BRANCH )
		decide_fix( id, ww, "1", code_loc, code_index );
	else if ( CodeArray[code_index].ndx_address == NDX_FUND )
		decide_fix( id, ww, "2", code_loc, code_index );
	else
		fix_one_code( id, ww, code_loc, code_index );
	}
else
	fix_one_code( id, ww, code_loc, code_index );
}
/*-------------------------------------------------------------------*/
static void
decide_fix( id, ww, tag, code_loc, code_index )
struct t_ndx	*id;
workrec			*ww;
char			*tag, *code_loc;
int				code_index;
{
if ( memcmp( code_loc + 2, "multi", 5 ) )	/* if not multi */
	fix_one_code( id, ww, code_loc, code_index );
else										/* fix multi field */
	fix_tag_code( id, ww, tag, code_index );
}
/*-------------------------------------------------------------------*/
static void
fix_one_code( id, ww, loc, code_index )
struct t_ndx	*id;
workrec			*ww;
char			*loc;
int				code_index;
{
int			addr;
int			loc_addr;
char		old_code[ 6 ];
char            new_code[ 6 ];
memcpy( old_code, loc + 2, 5 );
old_code[ 5 ] = 0;
/* resent modification for using address to change code --lle*/
loc_addr = get16(loc);
if ( Verbose )
  printf( "\n%s:%c%d:%s", CodeArray[code_index].name, ww->rectype,
			ww->recnum, old_code );

if ( multi_special(old_code,new_code,loc_addr,&addr) ) {}
    
else if ( none_special(old_code,new_code,loc_addr,&addr)){}

/* resent modification for using address to change code --lle*/
else if ( (addr = valid_field(id,loc_addr,old_code,new_code)) < 1 )
  {
    if ( !Verbose )
      printf( "\n%s:%c%d", CodeArray[code_index].name, ww->rectype,
	      ww->recnum );
    
    if ( addr == -1 )
      printf( ":Failed to access index file for %s",
	      CodeArray[code_index].name );
    else if(USE_ADDR)
      printf( ":ERROR  the address %d does not exist", loc_addr);
    else
      printf( ":ERROR  the code '%s' does not exist", old_code );
    
    if ( !ReportOnly )
      printf( ":no change" );
    
    return;
  }	
 
if ( USE_ADDR )
  
  if (memcmp(old_code,new_code,5) != 0) 
      {
	if ( ReportOnly )
	  {
	    if ( !Verbose )
	      printf( "\n%s:%c%d", CodeArray[code_index].name, ww->rectype,
		      ww->recnum );
	    printf( ":requires code changed from %s to %s", old_code, new_code );
	  }
	else
	  {
	    if ( Verbose )
	      printf( ":code changed from %s to %s", old_code, new_code );
	    put16(loc,addr);
	    memcpy(loc+2,new_code,5);
	    c_alter_data_file( ww->data, ww->data + fld_reclen( ww ), &ww->recptr );
	  }
      }
  else
    IFDB( 1, if ( strncmp(old_code,new_code) == 0 ) printf( "code is same\n" ); );
else
  if (addr != loc_addr) 
    {
    if ( ReportOnly )
      {
	if ( !Verbose )
	  printf( "\n%s:%c%d", CodeArray[code_index].name, ww->rectype,
		  ww->recnum );
	printf( ":requires address changed from %d to %d", loc_addr, addr );
      }
    else
      {
	if ( Verbose )
	  printf( ":address changed from %d to %d", loc_addr, addr );
	
	put16( loc, addr );
	c_alter_data_file( ww->data, ww->data + fld_reclen( ww ), &ww->recptr );
      }
    }  
  else
    IFDB( 1, if ( addr == loc_addr ) printf( "addr is same\n" ); );

}
/*-------------------------------------------------------------------*/
static char *
get_code_loc( ww, code_index )
workrec		*ww;
int			code_index;
{
int			fnum = -1;
int			i;
static int	offset;
static int	old_rectype = ' ';

if ( ww->rectype != old_rectype )
	{
	for ( i = 0; OffsetArray[i].ndx_address; i++ )
		{
		if ( ww->rectype == OffsetArray[i].rectype &&
			 CodeArray[code_index].ndx_address == OffsetArray[i].ndx_address )
			{
			fnum = i;
			break;
			}
		}

	if ( fnum == -1 )
		{
		old_rectype = ' ';
		return( CNULL );
		}
	
	old_rectype = ww->rectype;
	offset = OffsetArray[ fnum ].offset;
	}

return( cfix_loc( ww, offset ) );
}

static int
valid_field(id, loc, old_code, new_code)
struct t_ndx *id;
int loc;
char *old_code;
char *new_code;
{

int count;
if(!USE_ADDR)
  return ndx_find_addr( id, old_code, 0 );
else if(count = ndx_addr2count(id,loc))
    return ndx_count2addr(id,count,new_code);
else
  return (0);
}

static int 
multi_special(old, new, loc_addr, addr)
char *old;
char *new;
int loc_addr;
int *addr;
{
if(USE_ADDR) {
  if(loc_addr == -1) {
    memcpy(new,"multi",5);
    return 1;
  }
  else {
    return 0;
  }
}
else {
  if(memcmp(old, "multi",5) == 0) {
    *addr = -1;
    return 1;
  }
  else
    return 0;
 }
}

static int 
none_special(old, new, loc_addr, addr)
char *old;
char *new;
int loc_addr;
int *addr;
{
if(USE_ADDR) {
  if(loc_addr == 0) {
    memcpy(new,"none ",5);
    return 1;
  }
  else {
    return 0;

  }
}
else {
  if(memcmp(old, "none ",5) == 0) {
    *addr = 0;
    return 1;
  }
  else
    return 0;
 }
}
