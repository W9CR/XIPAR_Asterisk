/* #define	NEW_ASTERISK */
/* #define OLD_ASTERISK */
/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2002-2009, Jim Dixon, WB6NIL
 *
 * Jim Dixon, WB6NIL <jim@lambdatel.com>
 * Serious contributions by Steve RoDgers, WA6ZFT <hwstar@rodgers.sdcoxmail.com>
 * Steven Henke, W9SH, <w9sh@xelatec.com> added documentation and features.
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */
 /*

	Activity Summary - 2010-01-01 "Steven Henke"<sph@xelatec.com>
	- updates for new Asterisk/Dahdi & O/S

 	Activity Summary - 2009-06-17 - project merge
	Rev 296 COP 49 & 50 for No Incoming Connections
	Elected to not add uChameleon functions.
    GPIO/DAQ handled generically in other module.
	299 0.189 nounkeyct and holdofftelem configuration options
    300 0.190 fix holdofftelem always enabled bug
	304 0.191 add beaconing id option
	306 0.191 Fixed Mantis bug 0000032: ID Talkover CW loops causes RX audio to chop
	replace raw printf to ast_log for remote debugging of tx systems
	add extras with mdc and notch

    To Do:
	2 Tone Paging - decide on old method or new
	Resolve DAHDI_CONF_REALANDPSEUDO this shouldnt have to be this way.

 */
/*! \file
 *
 * \brief Radio Repeater / Remote Base program
 *  version 0.524-AM 2013/07/25 1316 EDT + 2014/12/08 12:20 PST
 *
 * \author Jim Dixon, WB6NIL <jim@lambdatel.com>
 * \ingroup applications
 *
 * Serious contributions by Steve RoDgers, WA6ZFT <hwstar@rodgers.sdcoxmail.com>
 *
 * This version is developed and maintained by
 * Steven Henke, W9SH, <w9sh@xelatec.com>
 * There are some additional features that may be of interest.
 * See http://www.xelatec.com/xipar for amateur radio issues and
 * http://www.xelatec.com for commercial applications, products and services.
 *
 * Repeater / Remote Functions:
 * "Simple" Mode:  * - autopatch access, # - autopatch hangup
 * Normal mode:
 * See the function list in rpt.conf (autopatchup, autopatchdn)
 * autopatchup can optionally take comma delimited setting=value pairs:
 *
 *
 * context=string		:	Override default context with "string"
 * dialtime=ms			:	Specify the max number of milliseconds between phone number digits (1000 milliseconds = 1 second)
 * farenddisconnect=1		:	Automatically disconnect when called party hangs up
 * noct=1			:	Don't send repeater courtesy tone during autopatch calls
 * quiet=1			:	Don't send dial tone, or connect messages. Do not send patch down message when called party hangs up
 *
 *
 * Example: 123=autopatchup,dialtime=20000,noct=1,farenddisconnect=1
 *
 *  To send an asterisk (*) while dialing or talking on phone,
 *  use the autopatch acess code.
 *
 *
 * status cmds:
 *
 *  1 - Force ID (global)
 *  2 - Give Time of Day (global)
 *  3 - Give software Version (global)
 *  11 - Force ID (local only)
 *  12 - Give Time of Day (local only)
 *
 * cop (control operator) cmds:
 *
 *  1 - System warm boot
 *  2 - System enable
 *  3 - System disable
 *  4 - Test Tone On/Off
 *  5 - Dump System Variables on Console (debug)
 *  6 - PTT (phone mode only)
 *  7 - Time out timer enable
 *  8 - Time out timer disable
 *  9 - Autopatch enable
 *  10 - Autopatch disable
 *  11 - Link enable
 *  12 - Link disable
 *  13 - Query System State
 *  14 - Change System State
 *  15 - Scheduler Enable
 *  16 - Scheduler Disable
 *  17 - User functions (time, id, etc) enable
 *  18 - User functions (time, id, etc) disable
 *  19 - Select alternate hang timer
 *  20 - Select standard hang timer
 *  21 - Enable Parrot Mode
 *  22 - Disable Parrot Mode
 *  23 - Birdbath (Current Parrot Cleanup/Flush)
 *  24 - Flush all telemetry
 *  25 - Query last node un-keyed
 *  26 - Query all nodes keyed/unkeyed
 *  27 - Reset DAQ minimum on a pin
 *  28 - Reset DAQ maximum on a pin
 *  30 - Recall Memory Setting in Attached Xcvr
 *  31 - Channel Selector for Parallel Programmed Xcvr
 *  32 - Touchtone pad test: command + Digit string + # to playback all digits pressed
 *  33 - Local Telemetry Output Enable
 *  34 - Local Telemetry Output Disable
 *  35 - Local Telemetry Output on Demand
 *  36 - Foreign Link Local Output Path Enable
 *  37 - Foreign Link Local Output Path Disable
 *  38 - Foreign Link Local Output Path Follows Local Telemetry
 *  39 - Foreign Link Local Output Path on Demand
 *  40 - IRLP announce Enable
 *  41 - IRLP announce Disable
 *  42 - Echolink announce node # only
 *  43 - Echolink announce node Callsign only
 *  44 - Echolink announce node # & Callsign
 *  45 - Link Activity timer enable
 *  46 - Link Activity timer disable
 *  47 - Reset "Link Config Changed" Flag
 *  48 - Send Page Tone (Tone specs separated by parenthesis)
 *  49 - Disable incoming connections (control state notice)
 *  50 - Enable incoming connections (control state notice)
 *	51 - Reserved for Amateur Radio Use
 *       ...
 *  55 - Parrot Once if parrot mode is disabled
 *  56 - Rx CTCSS Enable
 *  57 - Rx CTCSS Disable
 *  58 - Tx CTCSS On Input only Enable
 *  59 - Tx CTCSS On Input only Disable

 *       Xelatec Range Starts
 *  80 - Change Default Squelch Code

 *
 * ilink cmds:
 *
 *  1 - Disconnect specified link
 *  2 - Connect specified link -- monitor only
 *  3 - Connect specified link -- tranceive
 *  4 - Enter command mode on specified link
 *  5 - System status
 *  6 - Disconnect all links
 *  11 - Disconnect a previously permanently connected link
 *  12 - Permanently connect specified link -- monitor only
 *  13 - Permanently connect specified link -- tranceive
 *  15 - Full system status (all nodes)
 *  16 - Reconnect links disconnected with "disconnect all links"
 *  200 thru 215 - (Send DTMF 0-9,*,#,A-D) (200=0, 201=1, 210=*, etc)
 *
 * remote cmds:
 *
 *  1 - Recall Memory MM  (*000-*099) (Gets memory from rpt.conf)
 *  2 - Set VFO MMMMM*KKK*O   (Mhz digits, Khz digits, Offset)
 *  3 - Set Rx PL Tone HHH*D*
 *  4 - Set Tx PL Tone HHH*D* (Not currently implemented with DHE RBI-1)
 *  5 - Link Status (long)
 *  6 - Set operating mode M (FM, USB, LSB, AM, etc)
 *  100 - RX PL off (Default)
 *  101 - RX PL On
 *  102 - TX PL Off (Default)
 *  103 - TX PL On
 *  104 - Low Power
 *  105 - Med Power
 *  106 - Hi Power
 *  107 - Bump Down 20 Hz
 *  108 - Bump Down 100 Hz
 *  109 - Bump Down 500 Hz
 *  110 - Bump Up 20 Hz
 *  111 - Bump Up 100 Hz
 *  112 - Bump Up 500 Hz
 *  113 - Scan Down Slow
 *  114 - Scan Down Medium
 *  115 - Scan Down Fast
 *  116 - Scan Up Slow
 *  117 - Scan Up Medium
 *  118 - Scan Up Fast
 *  119 - Transmit allowing auto-tune
 *  140 - Link Status (brief)
 *  200 thru 215 - (Send DTMF 0-9,*,#,A-D) (200=0, 201=1, 210=*, etc)
 *
 * playback cmds:
 *  specify the name of the file to be played globally (for example, 25=rpt/foo)
 *
 * localplay cmds:
 * specify the name of the file to be played locally (for example, 25=rpt/foo)
 *
 * 'duplex' modes:  (defaults to duplex=2)
 *
 * 0 - Only remote links key Tx and no main repeat audio.
 * 1 - Everything other then main Rx keys Tx, no main repeat audio.
 * 2 - Normal mode
 * 3 - Normal except no main repeat audio.
 * 4 - Normal except no main repeat audio during autopatch only
 *
*/

/*** MODULEINFO
	<depend>dahdi</depend>
	<depend>tonezone</depend>
	<defaultenabled>yes</defaultenabled>
 ***/

/* Un-comment the following to include support for MDC-1200 digital tone
   signalling protocol (using KA6SQG's GPL'ed implementation) */
/* #include "mdc_decode.c" */

/* Un-comment the following to include support for notch filters in the
   rx audio stream (using Tony Fisher's mknotch (mkfilter) implementation) */
/* #include "rpt_notch.c" */

/* maximum digits in DTMF buffer, and seconds after * for DTMF command timeout */

/* Build with Network Mark Debug and Trace */
/* #define	OPT_NETMARK  */

#ifdef 	OLD_ASTERISK
#define ast_free free
#define ast_malloc malloc
#define ast_strdup strdup
#endif

#define	MAXDTMF 32
#define	MAXMACRO 2048
#define	MAXLINKLIST 5120
#define	LINKLISTTIME 10000
#define	LINKLISTSHORTTIME 200
#define	LINKPOSTTIME 30000
#define	LINKPOSTSHORTTIME 200
#define	KEYPOSTTIME 30000
#define	KEYPOSTSHORTTIME 200

#define	MACROTIME 100
#define	MACROPTIME 500
#define	MACRONTIME 3
#define	DTMF_TIMEOUT 3
#define	KENWOOD_RETRIES 5
#define	TOPKEYN 32
#define	TOPKEYWAIT 3
#define	TOPKEYMAXSTR 30
#define	NEWKEYTIME 2000

#define	AUTHTELLTIME 7000
#define	AUTHTXTIME 1000
#define	AUTHLOGOUTTIME 25000

#ifdef	__RPT_NOTCH
#define	MAXFILTERS 10
#endif

#define	DISC_TIME 10000  /* report disc after 10 seconds of no connect */
#define	MAX_RETRIES 5
#define	MAX_RETRIES_PERM 1000000000

#define	REDUNDANT_TX_TIME 2000

#define	RETRY_TIMER_MS 5000

#define	PATCH_DIALPLAN_TIMEOUT 1500

#ifdef OLD_ASTERISK
#define	START_DELAY 10
#else
#define	START_DELAY 2
#endif

#define	RPT_LOCKOUT_SECS 10

#define MAXPEERSTR 31
#define	MAXREMSTR 15

#define	DELIMCHR ','
#define	QUOTECHR 34

#define	MONITOR_DISK_BLOCKS_PER_MINUTE 38

#define	DEFAULT_MONITOR_MIN_DISK_BLOCKS 10000
#define	DEFAULT_REMOTE_INACT_TIMEOUT (15 * 60)
#define	DEFAULT_REMOTE_TIMEOUT (60 * 60)
#define	DEFAULT_REMOTE_TIMEOUT_WARNING (3 * 60)
#define	DEFAULT_REMOTE_TIMEOUT_WARNING_FREQ 30

#define	DEFAULT_ERXGAIN "-3.0"
#define	DEFAULT_ETXGAIN "3.0"
#define	DEFAULT_IRXGAIN "-10.0"
#define	DEFAULT_ITXGAIN "10.0"

#define	DEFAULT_IRLPANN 1
#define	DEFAULT_EANNMODE 1

#define	DEFAULT_RXBURST_TIME 250
#define	DEFAULT_RXBURST_THRESHOLD 16

#define TONE_SAMPLE_RATE 8000
#define TONE_SAMPLES_IN_FRAME 160

#define	NODES "nodes"
#define	EXTNODES "extnodes"
#define MEMORY "memory"
#define MACRO "macro"
#define	FUNCTIONS "functions"
#define TELEMETRY "telemetry"
#define MORSE "morse"
#define	TONEMACRO "tonemacro"
#define	FUNCCHAR '*'
#define	ENDCHAR '#'
#define	EXTNODEFILE "/var/lib/asterisk/rpt_extnodes"
#define	NODENAMES "rpt/nodenames"
#define	PARROTFILE "/tmp/parrot_%s_%u"

#define	PARROTTIME 1000

#define	TELEM_HANG_TIME 120000
#define	LINK_HANG_TIME 120000

#define	DEFAULT_IOBASE 0x378

#define	DEFAULT_CIV_ADDR 0x58

#define	MAXCONNECTTIME 5000
#define	MAXCONNECTTIME_IRLP 25000

#define MAXNODESTR 300

#define MAXNODELEN 16

#define MAXIDENTLEN 32

#define MAXPATCHCONTEXT 100

#define ACTIONSIZE 32

#define TELEPARAMSIZE 400

#define REM_SCANTIME 100

#define	DTMF_LOCAL_TIME 250
#define	DTMF_LOCAL_STARTTIME 500

#define	IC706_PL_MEMORY_OFFSET 50

#define	VOX_ON_DEBOUNCE_COUNT 3
#define	VOX_OFF_DEBOUNCE_COUNT 20
#define	VOX_MAX_THRESHOLD 10000.0
#define	VOX_MIN_THRESHOLD 3000.0
#define	VOX_TIMEOUT_MS 10000
#define	VOX_RECOVER_MS 2000
#define	SIMPLEX_PATCH_DELAY 25
#define	SIMPLEX_PHONE_DELAY 25

#define	RX_LINGER_TIME 70		   	/*!< \brief rx key timeout after no voice packets in ms */

#define	REQUIRED_DAHDI_VERSION 'A'

#define	STATPOST_PROGRAM "/usr/bin/wget,-q,--output-document=/dev/null,--no-check-certificate"

#define	ALLOW_LOCAL_CHANNELS

#define EL_DB_ROOT "echolink"

#ifdef  OPT_NETMARK
#define NETMARK(a) {char str[64];                   \
sprintf(str,"echo '_mark_xelatec_'%i____line____%u> \
/dev/udp/10.25.163.211/5063",a,__LINE__); ast_safe_system(str); }
#else
#define NETMARK(a)
#endif

/*
 * DAQ subsystem
 */
#define MAX_DAQ_RANGES 16  /* Max number of entries for range() */
#define MAX_DAQ_ENTRIES 10 /* Max number of DAQ devices */
#define MAX_DAQ_NAME 32 /* Max length of a device name */
#define MAX_DAQ_DEV 64 /* Max length of a daq device path */
#define MAX_METER_FILES 10 /* Max number of sound files in a meter def. */
#define DAQ_RX_TIMEOUT 50 /* Receive time out for DAQ subsystem */
#define DAQ_ADC_ACQINT 10 /* Acquire interval in sec. for ADC channels */
#define ADC_HIST_TIME 300 /* Time  in sec. to calculate short term avg, high and low peaks from. */
#define ADC_HISTORY_DEPTH ADC_HIST_TIME/DAQ_ADC_ACQINT

enum {REM_OFF,REM_MONITOR,REM_TX};

enum {LINKMODE_OFF,LINKMODE_ON,LINKMODE_FOLLOW,LINKMODE_DEMAND,
	LINKMODE_GUI,LINKMODE_PHONE,LINKMODE_ECHOLINK,LINKMODE_IRLP};

enum{ID,PROC,TERM,COMPLETE,UNKEY,REMDISC,REMALREADY,REMNOTFOUND,REMGO,
	CONNECTED,CONNFAIL,STATUS,TIMEOUT,ID1, STATS_TIME, PLAYBACK,
	LOCALPLAY, STATS_VERSION, IDTALKOVER, ARB_ALPHA, TEST_TONE, REV_PATCH,
	TAILMSG, MACRO_NOTFOUND, MACRO_BUSY, LASTNODEKEY, FULLSTATUS,
	MEMNOTFOUND, INVFREQ, REMMODE, REMLOGIN, REMXXX, REMSHORTSTATUS,
	REMLONGSTATUS, LOGINREQ, SCAN, SCANSTAT, TUNE, SETREMOTE, TOPKEY,
	TIMEOUT_WARNING, ACT_TIMEOUT_WARNING, LINKUNKEY, UNAUTHTX, PARROT,
	STATS_TIME_LOCAL, VARCMD, LOCUNKEY, METER, USEROUT, PAGE};

enum {REM_SIMPLEX,REM_MINUS,REM_PLUS,REM_WILD};

enum {REM_LOWPWR,REM_MEDPWR,REM_HIPWR};

enum {DC_INDETERMINATE, DC_REQ_FLUSH, DC_ERROR, DC_COMPLETE, DC_COMPLETEQUIET, DC_DOKEY};

enum {SOURCE_RPT, SOURCE_LNK, SOURCE_RMT, SOURCE_PHONE, SOURCE_DPHONE, SOURCE_ALT};

enum {DLY_TELEM, DLY_ID, DLY_UNKEY, DLY_CALLTERM, DLY_COMP, DLY_LINKUNKEY, DLY_PARROT};

enum {REM_MODE_FM,REM_MODE_USB,REM_MODE_LSB,REM_MODE_AM};

enum {HF_SCAN_OFF,HF_SCAN_DOWN_SLOW,HF_SCAN_DOWN_QUICK,
      HF_SCAN_DOWN_FAST,HF_SCAN_UP_SLOW,HF_SCAN_UP_QUICK,HF_SCAN_UP_FAST};

/*
 * DAQ Subsystem
 */

enum{DAQ_PS_IDLE = 0, DAQ_PS_START, DAQ_PS_BUSY, DAQ_PS_IN_MONITOR};
enum{DAQ_CMD_IN, DAQ_CMD_ADC, DAQ_CMD_OUT, DAQ_CMD_PINSET, DAQ_CMD_MONITOR};
enum{DAQ_SUB_CUR = 0, DAQ_SUB_MIN, DAQ_SUB_MAX, DAQ_SUB_STMIN, DAQ_SUB_STMAX, DAQ_SUB_STAVG};
enum{DAQ_PT_INADC = 1, DAQ_PT_INP, DAQ_PT_IN, DAQ_PT_OUT};
enum{DAQ_TYPE_UCHAMELEON};


#define DEFAULT_TELEMDUCKDB "-9"
#define	DEFAULT_RPT_TELEMDEFAULT 1
#define	DEFAULT_RPT_TELEMDYNAMIC 1
#define	DEFAULT_GUI_LINK_MODE LINKMODE_ON
#define	DEFAULT_GUI_LINK_MODE_DYNAMIC 1
#define	DEFAULT_PHONE_LINK_MODE LINKMODE_ON
#define	DEFAULT_PHONE_LINK_MODE_DYNAMIC 1
#define	DEFAULT_ECHOLINK_LINK_MODE LINKMODE_DEMAND
#define	DEFAULT_ECHOLINK_LINK_MODE_DYNAMIC 1
#define	DEFAULT_IRLP_LINK_MODE LINKMODE_DEMAND
#define	DEFAULT_IRLP_LINK_MODE_DYNAMIC 1

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <search.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/io.h>
#include <sys/vfs.h>
#include <math.h>
#ifdef OLD_ASTERISK
#include <dahdi/user.h>
#include <dahdi/tonezone.h>
#else
#include <dahdi/user.h>
#include <dahdi/tonezone.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>

#include "asterisk/utils.h"
#include "asterisk/lock.h"
#include "asterisk/file.h"
#include "asterisk/logger.h"
#include "asterisk/channel.h"
#include "asterisk/callerid.h"
#include "asterisk/pbx.h"
#include "asterisk/module.h"
#include "asterisk/translate.h"
#include "asterisk/features.h"
#include "asterisk/options.h"
#include "asterisk/cli.h"
#include "asterisk/config.h"
#include "asterisk/say.h"
#include "asterisk/localtime.h"
#include "asterisk/cdr.h"
#include "asterisk/options.h"
#include "asterisk/manager.h"
#include "asterisk/astdb.h"
#include "asterisk/app.h"
#include <termios.h>

#ifdef	NEW_ASTERISK
struct ast_flags config_flags = { CONFIG_FLAG_WITHCOMMENTS };
#endif


/* Start a tone-list going */
int ast_playtones_start(struct ast_channel *chan, int vol, const char* tonelist, int interruptible);
/*! Stop the tones from playing */
void ast_playtones_stop(struct ast_channel *chan);

static  char *tdesc = "Radio Repeater / Remote Base version 0.524-AM 2013/07/25 1316 EDT + 2014/12/08 12:20 PST";

static char *app = "Rpt";

static char *synopsis = "Radio Repeater/Remote Base Control System";

static char *descrip =
"  Rpt(nodename[|options][|M][|*]):  \n"
"    Radio Remote Link or Remote Base Link Endpoint Process.\n"
"\n"
"    Not specifying an option puts it in normal endpoint mode (where source\n"
"    IP and nodename are verified).\n"
"\n"
"    Options are as follows:\n"
"\n"
"        X - Normal endpoint mode WITHOUT security check. Only specify\n"
"            this if you have checked security already (like with an IAX2\n"
"            user/password or something).\n"
"\n"
"        Rannounce-string[|timeout[|timeout-destination]] - Amateur Radio\n"
"            Reverse Autopatch. Caller is put on hold, and announcement (as\n"
"            specified by the 'announce-string') is played on radio system.\n"
"            Users of radio system can access autopatch, dial specified\n"
"            code, and pick up call. Announce-string is list of names of\n"
"            recordings, or \"PARKED\" to substitute code for un-parking,\n"
"            or \"NODE\" to substitute node number.\n"
"\n"
"        P - Phone Control mode. This allows a regular phone user to have\n"
"            full control and audio access to the radio system. For the\n"
"            user to have DTMF control, the 'phone_functions' parameter\n"
"            must be specified for the node in 'rpt.conf'. An additional\n"
"            function (cop,6) must be listed so that PTT control is available.\n"
"\n"
"        D - Dumb Phone Control mode. This allows a regular phone user to\n"
"            have full control and audio access to the radio system. In this\n"
"            mode, the PTT is activated for the entire length of the call.\n"
"            For the user to have DTMF control (not generally recomended in\n"
"            this mode), the 'dphone_functions' parameter must be specified\n"
"            for the node in 'rpt.conf'. Otherwise no DTMF control will be\n"
"            available to the phone user.\n"
"\n"
"        S - Simplex Dumb Phone Control mode. This allows a regular phone user\n"
"            audio-only access to the radio system. In this mode, the\n"
"            transmitter is toggled on and off when the phone user presses the\n"
"            funcchar (*) key on the telephone set. In addition, the transmitter\n"
"            will turn off if the endchar (#) key is pressed. When a user first\n"
"            calls in, the transmitter will be off, and the user can listen for\n"
"            radio traffic. When the user wants to transmit, they press the *\n"
"            key, start talking, then press the * key again or the # key to turn\n"
"            the transmitter off.  No other functions can be executed by the\n"
"            user on the phone when this mode is selected. Note: If your\n"
"            radio system is full-duplex, we recommend using either P or D\n"
"            modes as they provide more flexibility.\n"
"\n"
"        N - Non-Message Trunking. Transmission Trunking Call. \n"
"\n"
"        q - Query Status. Sets channel variables and returns + 101 in plan.\n"
"\n"
"        M - Memory Channel Steer as MXX where XX is the memory channel number.\n"
"\n"
"        * - Alt Macro to execute (e.g. *7 for status)\n"
"\n";
;

static int debug = 0;  /* Set this >0 for extra debug output */
static int nrpts = 0;

static char remdtmfstr[] = "0123456789*#ABCD";

enum {TOP_TOP,TOP_WON,WON_BEFREAD,BEFREAD_AFTERREAD};

int max_chan_stat [] = {22000,1000,22000,100,22000,2000,22000};

#define NRPTSTAT 7

struct rpt_chan_stat
{
	struct timeval last;
	long long total;
	unsigned long count;
	unsigned long largest;
	struct timeval largest_time;
};

char *discstr    = "!!DISCONNECT!!";
char *newkeystr  = "!NEWKEY!";
char *newkey1str = "!NEWKEY1!";
char *iaxkeystr  = "!IAXKEY!";					/*!< \brief IAX keying client parameter marker */

static char *remote_rig_ft950="ft950";
static char *remote_rig_ft897="ft897";
static char *remote_rig_rbi="rbi";
static char *remote_rig_kenwood="kenwood";
static char *remote_rig_tm271="tm271";
static char *remote_rig_ic706="ic706";
static char *remote_rig_rtx150="rtx150";
static char *remote_rig_rtx450="rtx450";
static char *remote_rig_ppp16="ppp16";	  		// parallel port programmable 16 channels

#define ISRIG_RTX(x) ((!strcmp(x,remote_rig_rtx150)) || (!strcmp(x,remote_rig_rtx450)))
#define	IS_XPMR(x) (!strncasecmp(x->rxchanname,"rad",3))

static char* dtmf_tones[] = {
	"!941+1336/200,!0/200",	/* 0 */
	"!697+1209/200,!0/200",	/* 1 */
	"!697+1336/200,!0/200",	/* 2 */
	"!697+1477/200,!0/200",	/* 3 */
	"!770+1209/200,!0/200",	/* 4 */
	"!770+1336/200,!0/200",	/* 5 */
	"!770+1477/200,!0/200",	/* 6 */
	"!852+1209/200,!0/200",	/* 7 */
	"!852+1336/200,!0/200",	/* 8 */
	"!852+1477/200,!0/200",	/* 9 */
	"!697+1633/200,!0/200",	/* A */
	"!770+1633/200,!0/200",	/* B */
	"!852+1633/200,!0/200",	/* C */
	"!941+1633/200,!0/200",	/* D */
	"!941+1209/200,!0/200",	/* * */
	"!941+1477/200,!0/200" };	/* # */

#ifdef	OLD_ASTERISK
STANDARD_LOCAL_USER;
LOCAL_USER_DECL;
#endif

#define	MSWAIT 20
#define	HANGTIME 5000
#define	TOTIME 180000
#define	IDTIME 300000
#define	MAXRPTS 32
#define MAX_STAT_LINKS 32
#define POLITEID 30000
#define FUNCTDELAY 1500

#define	MAXXLAT 20
#define	MAXXLATTIME 3

#define MAX_SYSSTATES 10

#define FT897_SERIAL_DELAY 75000		/* # of usec to wait between some serial commands on FT-897 */

struct vox {
	float	speech_energy;
	float	noise_energy;
	int		enacount;
	char	voxena;
	char	lastvox;
	int		offdebcnt;
	int		ondebcnt;
	int		voxminth;
	int		voxmaxth;
};

#define	mymax(x,y) ((x > y) ? x : y)
#define	mymin(x,y) ((x < y) ? x : y)

struct rpt_topkey
{
char	node[TOPKEYMAXSTR];
int	timesince;
int	keyed;
} ;

struct rpt_xlat
{
char	funccharseq[MAXXLAT];
char	endcharseq[MAXXLAT];
char	passchars[MAXXLAT];
int	funcindex;
int	endindex;
time_t	lastone;
} ;

static time_t	starttime = 0;

static  pthread_t rpt_master_thread;

struct rpt;

/*!
 * \brief Radio Transceiver Link (i.e. phone,another rpt,iaxRpt)
 */
struct rpt_link
{
	struct 	rpt_link *next;
	struct 	rpt_link *prev;
	void	*parent;			/*!< \brief parent structure */
	char	mode;				/*!< \brief 1 if in tx mode */
	char	isremote;			/*!< \brief Link is to a Remote */
	char	phonemode;			/*!< \brief Link is to a Phone */
	char	phonevox;			/*!< \brief Link is to a Phone with VOX */
	char	phonemonitor;		/*!< \brief no tx or functions for the phone */
	char	name[MAXNODESTR];	/*!< \brief identifier (routing) string */
	char	lasttx;				/*!< \brief Transmitter State Last Sent to Radio */
	char	lasttx1;
	char    lasttxhandled;	  	/*!< \brief Transmitter State Last Handled */
	char	lastrx;
	char	lastrealrx;
	char	lastrx1;			/*!< \brief Receiver Squelch State Last Handled */
	char    lastrxhandled;		/*!< \brief Receiver Squelch State Last Handled */
	char    lastrxraw;			/*!< \brief Receiver Squelch State Raw regardless of squelch code */
	char	connected;
	char	hasconnected;
	char	perma;				/*!< \brief Permanent Link. Restore if down. */
	char	thisconnected;
	char	outbound;			/*!< \brief This is the calling link. */
	char	disced;
	char	killme;
	long	elaptime;
	long	disctime;
	long 	retrytimer;
	long	retxtimer;			/*!< \brief To send recurring ACRK messages */
	long	rerxtimer;			/*!< \brief Loss of Packets Timer */
	long	rxlingertimer;		/*!< \brief Loss of Voice Packets Timer */
	int     rssi_report_timer;
	int     rssi;
	int		retries;
	int		max_retries;
	int		reconnects;
	long long connecttime;
	struct 	ast_channel *chan;	   /*!< \brief Tx stream to Link from psuedo */
	struct 	ast_channel *pchan;	   /*!< \brief Rx stream from Link to psuedo */
	int		pchblen;			   /*!< \brief psuedo channel buffer len      */
	char	linklist[MAXLINKLIST];
	time_t	linklistreceived;
	long	linklisttimer;
	int		dtmfed;
	int 	linkunkeytocttimer;
	struct 	timeval lastlinktv;
	struct	ast_frame *lastf1,*lastf2;
	struct	rpt_chan_stat chan_stat[NRPTSTAT];
	struct 	vox vox;
	char 	wasvox;
	int 	voxtotimer;
	char 	voxtostate;
	char 	newkey;			/*!< \brief Link is to new keying client */
	int     iaxkey;		    /*!< \brief Link iax control type */
	int 	linkmode;
	int 	newkeytimer;
	char    gott;			/*!< \brief Got T message flag  */
	int 	iaxrpt;			/*!< \brief Link is to iaxRpt client */
	int 	csmonitor;		/*!< \brief Squelch Code Monitor Mode */
	char 	cstxcode[16];	/*!< \brief Squelch Code for local Tx to send if keyed by this link */
	char 	csrxcode[16];	/*!< \brief Squelch Code for link Rx to be recognized locally */
	char 	cstxnow[16];	/*!< \brief Squelch Code Tx to link now */
	char 	csrxnow[16];	/*!< \brief Squelch Code Rx from link now */
	char    csm;			/*!< \brief Squelch Code Match */
	char    csmm;			/*!< \brief Squelch Code Master Match. Send code info on the link. */
	char 	lnwku_update;
	int		voterlink;      /*!< \brief set if node is defined as a voter rx */
	int		votewinner;		/*!< \brief set if node won the rssi competition */
	time_t	lastkeytime;
	time_t	lastunkeytime;

#ifdef OLD_ASTERISK
        AST_LIST_HEAD(, ast_frame) rxq;
#else
	AST_LIST_HEAD_NOLOCK(, ast_frame) rxq;
#endif
#ifdef OLD_ASTERISK
        AST_LIST_HEAD(, ast_frame) textq;
#else
	AST_LIST_HEAD_NOLOCK(, ast_frame) textq;
#endif
} ;

struct rpt_lstat
{
	struct	rpt_lstat *next;
	struct	rpt_lstat *prev;
	char	peer[MAXPEERSTR];
	char	name[MAXNODESTR];
	char	mode;
	char	outbound;
	char	reconnects;
	char	thisconnected;
	long long	connecttime;
	struct	rpt_chan_stat chan_stat[NRPTSTAT];
} ;

struct rpt_tele
{
	struct rpt_tele *next;
	struct rpt_tele *prev;
	struct rpt *rpt;
	struct ast_channel *chan;
	int	mode;
	struct rpt_link mylink;
	char param[TELEPARAMSIZE];
	int	submode;
	unsigned int parrot;
	char killed;
	pthread_t threadid;
} ;

struct function_table_tag
{
	char action[ACTIONSIZE];
	int (*function)(struct rpt *myrpt, char *param, char *digitbuf,
		int command_source, struct rpt_link *mylink);
} ;

/* Used to store the morse code patterns */

struct morse_bits
{
	int len;
	int ddcomb;
} ;

struct telem_defaults
{
	char name[20];
	char value[80];
} ;


struct sysstate
{
	char txdisable;
	char totdisable;
	char linkfundisable;
	char autopatchdisable;
	char schedulerdisable;
	char userfundisable;
	char alternatetail;
	char noincomingconns;
};

/* rpt cmd support */
#define CMD_DEPTH 1
#define CMD_STATE_IDLE 0
#define CMD_STATE_BUSY 1
#define CMD_STATE_READY 2
#define CMD_STATE_EXECUTING 3

struct rpt_cmd_struct
{
    int state;
    int functionNumber;
    char param[MAXDTMF];
    char digits[MAXDTMF];
    int command_source;
};

/*!
 * \brief Tone Burst Detection
 */
typedef struct {
	int v2;
	int v3;
	int chunky;
	int fac;
	int samples;
} goertzel_state_t;

typedef struct {
	int value;
	int power;
} goertzel_result_t;

typedef struct
{
	int freq;
	int block_size;
	int squelch;		/* Remove (squelch) tone */
	goertzel_state_t tone;
	float energy;		/* Accumulated energy of the current block */
	int samples_pending;	/* Samples remain to complete the current block */
	int mute_samples;	/* How many additional samples needs to be muted to suppress already detected tone */

	int hits_required;	/* How many successive blocks with tone we are looking for */
	float threshold;	/* Energy of the tone relative to energy from all other signals to consider a hit */

	int hit_count;		/* How many successive blocks we consider tone present */
	int last_hit;		/* Indicates if the last processed block was a hit */

} tone_detect_state_t;


/*!
 * \brief Radio Transceiver
 */
static struct rpt
{
	ast_mutex_t lock;
	ast_mutex_t remlock;
	ast_mutex_t statpost_lock;
	struct ast_config *cfg;
	char reload;
	char xlink;		 							// cross link state of a share repeater/remote radio
	unsigned int statpost_seqno;

	char *name;
	char *rxchanname;
	char *txchanname;
	char remote;
	char *remoterig;
	struct	rpt_chan_stat chan_stat[NRPTSTAT];
	unsigned int scram;		/*!< \brief to scramble/randomize channel handling? */

	/* the following are set in rpt.conf */
	struct {
		char *ourcontext;
		char *ourcallerid;
		char *acctcode;
		char *ident;
		char *tonezone;
		char simple;
		char *functions;
		char *link_functions;
		char *phone_functions;
		char *dphone_functions;
		char *alt_functions;
		char *nodes;
		char *extnodes;
		char *extnodefile;
		char *lnkactmacro;
		char *lnkacttimerwarn;
		int hangtime;
		int althangtime;
		int totime;
		int idtime;
		int tailmessagetime;
		int tailsquashedtime;
		int lnkacttime;
		int duplex;
		int politeid;
		char *tailmessages[500];
		int tailmessagemax;
		char	*memory;
		char	*macro;
		char	*tonemacro;
		char	*startupmacro;
		int iobase;
		char *ioport;
		char funcchar;
		char endchar;
		char nobusyout;
		char notelemtx;
		char propagate_dtmf;
		char propagate_phonedtmf;
		char linktolink;
		unsigned char civaddr;
		struct rpt_xlat inxlat;
		struct rpt_xlat outxlat;
		char *archivedir;
		int authlevel;
		char *csstanzaname;
		char *skedstanzaname;
		char *txlimitsstanzaname;
		long monminblocks;
		int remoteinacttimeout;
		int remotetimeout;
		int remotetimeoutwarning;
		int remotetimeoutwarningfreq;
		int sysstate_cur;
		struct sysstate s[MAX_SYSSTATES];
		char parrotmode;
		int parrottime;
		char *rptnode;
		char remote_mars;
		int voxtimeout_ms;
		int voxrecover_ms;
		int voxminth;
		int voxmaxth;
		int simplexpatchdelay;
		int simplexphonedelay;
		char telemdefault;
		char telemdynamic;
		char lnkactenable;
		char *statpost_program;
		char *statpost_url;
		char linkmode[10];
		char linkmodedynamic[10];
		char *locallist[16];
		int nlocallist;
		char ctgroup[16];
		float telemnomgain;		/*!< \brief nominal gain adjust for telemetry */
		float telemduckgain;	/*!< \brief duck on busy gain adjust for telemetry */
		float erxgain;
		float etxgain;
		float irxgain;
		float itxgain;
		char irlpann;
		char eannmode; /* {NONE,NODE,CALL,BOTH} */
		char *discpgm;
		char *connpgm;
		char nolocallinkct;
		char nounkeyct;					/*!< \brief No Unkey Connect Telemetry */
		char holdofftelem;
		char beaconing;
		int rxburstfreq;				/*!< \brief Tone Burst Squelch */
		int rxbursttime;
		int rxburstthreshold;
		char nonodelist;	 			/*!< \brief Inhibit sending node list text messages */
		char *codemaster[16];
		int ncodemasters;
		char *ncomnum;				    /*!< \brief SIP address for NCOM call 	*/
		int itxctcss;					/*!< \brief inhibit tx ctcss */
		int votertype;					/*!< \brief 0 none, 1 repeater, 2 voter rx 		*/
		int votermode;					/*!< \brief 0 none, 1 one shot, 2 continuous 	*/
        int votermargin;				/*!< \brief rssi margin to win a vote           */
	} p;

	struct rpt_link links;
	int unkeytocttimer;
	time_t lastkeyedtime;
	time_t lasttxkeyedtime;
	char keyed;							/*!< \brief Receiver Key State from rxchannel */
	char txkeyed;  						/*!< \brief Transmitter Key State */
	char rxchankeyed;					/*!< \brief Receiver RxChan Key State */

	char exttx;
	char localtx;
	char lnwku_update;
	char remrx;							/*!< \brief Indicate keying from a link */
	char remoterx;
	char remotetx;
	char remoteon;
	char remtxfreqok;
	char tounkeyed;
	char tonotify;
	char dtmfbuf[MAXDTMF];
	char macrobuf[MAXMACRO];
	char rem_dtmfbuf[MAXDTMF];
	char lastdtmfcommand[MAXDTMF];
	char cmdnode[50];
	char nowchan;						// channel now
	char waschan;						// channel selected initially or by command
	char bargechan;						// barge in channel
	char macropatch;					// autopatch via tonemacro state
	char parrotstate;
	char parrotonce;
	char linkactivityflag;
	int  parrottimer;
	char lastitx;				 		/*!< \brief Last state of inhibit tx ctcss flag */
	unsigned int parrotcnt;
	int telemmode;
	struct ast_channel *rxchannel,*txchannel, *monchannel, *parrotchannel;
	struct ast_channel *pchannel,*txpchannel, *zaprxchannel, *zaptxchannel;
	struct ast_channel *telechannel;  	/*!< \brief pseudo channel between telemetry conference and txconf */
	struct ast_channel *btelechannel;  	/*!< \brief pseudo channel buffer between telemetry conference and txconf */
	struct ast_channel *voxchannel;
	struct ast_channel *whokeyed;		/*!< \brief Who keyed the Transmitter */
	struct ast_frame *lastf1,*lastf2;
	struct rpt_tele tele;
	struct timeval lasttv,curtv;
	pthread_t rpt_call_thread,rpt_thread;
	time_t dtmf_time,rem_dtmf_time,dtmf_time_rem;
	int calldigittimer;
	int tailtimer,totimer,idtimer,txconf,conf;
	int callmode; 								/*!< \brief state of call/patch */
	int cidx,scantimer,tmsgtimer,skedtimer,linkactivitytimer;
	int mustid,tailid;
	int tailevent;
	int teleconf;								/*!< \brief telemetry conference id */
	int telemrefcount;
	int dtmfidx,rem_dtmfidx;
	int dailytxtime,dailykerchunks,totalkerchunks,dailykeyups,totalkeyups,timeouts;
	int totalexecdcommands, dailyexecdcommands;
	long	retxtimer;
	long	rerxtimer;
	long long totaltxtime;
	char mydtmf;
	char exten[AST_MAX_EXTENSION];
	char freq[MAXREMSTR],rxpl[MAXREMSTR],txpl[MAXREMSTR],freqtx[MAXREMSTR];
	char offset;
	char powerlevel;
	char txplon;
	char rxplon;
	char remmode;
	char tunerequest;
	char hfscanmode;
	int hfscanstatus;
	char hfscanstop;
	char lastlinknode[MAXNODESTR];
	char savednodes[MAXNODESTR];
	int stopgen;
	char patchfarenddisconnect;
	char patchnoct;
	char patchquiet;
	char patchcontext[MAXPATCHCONTEXT];
	int patchdialtime;
	int macro_longest;
	int phone_longestfunc;
	int alt_longestfunc;
	int dphone_longestfunc;
	int link_longestfunc;
	int longestfunc;
	int longestnode;
	int threadrestarts;
	int tailmessagen;
	time_t disgorgetime;
	time_t lastthreadrestarttime;
	long	macrotimer;
	char	lastnodewhichkeyedusup[MAXNODESTR];
	int	dtmf_local_timer;
	char	dtmf_local_str[100];
	struct ast_filestream *monstream,*parrotstream;
	char	loginuser[50];
	char	loginlevel[10];
	long	authtelltimer;
	long	authtimer;
	int iofd;
	time_t start_time,last_activity_time;
	char	lasttone[32];					/*!< \brief last squelch code processed for squelch code macros */
	struct rpt_tele *active_telem;
	struct 	rpt_topkey topkey[TOPKEYN];
	int topkeystate;
	time_t topkeytime;
	int topkeylong;
	struct vox vox;
	char wasvox;
	int voxtotimer;
	char voxtostate;
	int linkposttimer;
	int keyposttimer;

	int	 phonelinks;		    /*! \brief Count of Phone Links generated by numlinks() */
	char callcode;				/*!< \brief indicate call had custom code */

	char csrxnow[16];       	/*!< \brief squelch code receiving now */
	char cstxnow[16];           /*!< \brief squelch code transmiting now */

	char csrxcode[16];			/*!< \brief squelch code to receive */
	char cstxcode[16];		  	/*!< \brief squelch code to transmit */

	char hasnoradio;			/*!< \brief node has no radio attached */
	char radiotaken;			/*!< \brief radio taken by another node */
	char newkey;				/*!< \brief node keying type */
	char iaxkey;				/*!< \brief node control type */
	char inpadtest;
	long rxlingertimer;			/*!< \brief rx key timer after no voice packets */
	char localoverride;
	char innox;					/*!< \brief Indicate in NCOM TX call */
	char radiofail;			    /*!< \brief Indicate radio/channel fail */
	char lastrxburst;			/*!< \brief Tone Burst */
	char reallykeyed;
	char localteleminhibit;		/*!< \brief local telemetry inhibit */
	char noduck;				/*!< \brief no ducking of telemetry  */

	struct rpt_link *voted_link; /*!< \brief last winning link or NULL */
	int  rxrssi;				/*!< \brief rx rssi from the rxchannel */
	int  voted_rssi;			/*!< \brief last winning rssi */
	int  vote_counter;			/*!< \brief count to frame used to vote the winner */
	int  voter_oneshot;
	int  votewinner;
    int  voteremrx;             /* 0 no voters are keyed, 1 at least one voter is keyed */

	tone_detect_state_t burst_tone_state;
#ifdef OLD_ASTERISK
	AST_LIST_HEAD(, ast_frame) txq;
#else
	AST_LIST_HEAD_NOLOCK(, ast_frame) txq;
#endif
#ifdef OLD_ASTERISK
	AST_LIST_HEAD(, ast_frame) rxq;
#else
	AST_LIST_HEAD_NOLOCK(, ast_frame) rxq;
#endif
	char txrealkeyed;
#ifdef	__RPT_NOTCH
	struct rptfilter
	{
		char	desc[100];
		float	x0;
		float	x1;
		float	x2;
		float	y0;
		float	y1;
		float	y2;
		float	gain;
		float	const0;
		float	const1;
		float	const2;
	} filters[MAXFILTERS];
#endif
#ifdef	_MDC_DECODE_H_
	mdc_decoder_t *mdc;
	unsigned short lastunit;
#endif
	struct rpt_cmd_struct cmdAction;
} rpt_vars[MAXRPTS];

struct nodelog {
struct nodelog *next;
struct nodelog *prev;
time_t	timestamp;
char archivedir[MAXNODESTR];
char str[MAXNODESTR * 2];
} nodelog;

static int service_scan(struct rpt *myrpt);
static int set_mode_ft897(struct rpt *myrpt, char newmode);
static int set_mode_ic706(struct rpt *myrpt, char newmode);
static int simple_command_ft897(struct rpt *myrpt, char command);
static int setrem(struct rpt *myrpt);
static int setrtx_check(struct rpt *myrpt);
static int channel_revert(struct rpt *myrpt);
static int channel_steer(struct rpt *myrpt, char *data);
static void rpt_telemetry(struct rpt *myrpt,int mode, void *data);

AST_MUTEX_DEFINE_STATIC(nodeloglock);

AST_MUTEX_DEFINE_STATIC(nodelookuplock);

#ifdef	APP_RPT_LOCK_DEBUG

#warning COMPILING WITH LOCK-DEBUGGING ENABLED!!

#define	MAXLOCKTHREAD 100

#define rpt_mutex_lock(x) _rpt_mutex_lock(x,myrpt,__LINE__)
#define rpt_mutex_unlock(x) _rpt_mutex_unlock(x,myrpt,__LINE__)

struct lockthread
{
	pthread_t id;
	int lockcount;
	int lastlock;
	int lastunlock;
} lockthreads[MAXLOCKTHREAD];


struct by_lightning
{
	int line;
	struct timeval tv;
	struct rpt *rpt;
	struct lockthread lockthread;
} lock_ring[32];

int lock_ring_index = 0;

AST_MUTEX_DEFINE_STATIC(locklock);

static struct lockthread *get_lockthread(pthread_t id)
{
int	i;

	for(i = 0; i < MAXLOCKTHREAD; i++)
	{
		if (lockthreads[i].id == id) return(&lockthreads[i]);
	}
	return(NULL);
}

static struct lockthread *put_lockthread(pthread_t id)
{
int	i;

	for(i = 0; i < MAXLOCKTHREAD; i++)
	{
		if (lockthreads[i].id == id)
			return(&lockthreads[i]);
	}
	for(i = 0; i < MAXLOCKTHREAD; i++)
	{
		if (!lockthreads[i].id)
		{
			lockthreads[i].lockcount = 0;
			lockthreads[i].lastlock = 0;
			lockthreads[i].lastunlock = 0;
			lockthreads[i].id = id;
			return(&lockthreads[i]);
		}
	}
	return(NULL);
}


static void rpt_mutex_spew(void)
{
	struct by_lightning lock_ring_copy[32];
	int lock_ring_index_copy;
	int i,j;
	long long diff;
	char a[100];
	struct timeval lasttv;

	ast_mutex_lock(&locklock);
	memcpy(&lock_ring_copy, &lock_ring, sizeof(lock_ring_copy));
	lock_ring_index_copy = lock_ring_index;
	ast_mutex_unlock(&locklock);

	lasttv.tv_sec = lasttv.tv_usec = 0;
	for(i = 0 ; i < 32 ; i++)
	{
		j = (i + lock_ring_index_copy) % 32;
		strftime(a,sizeof(a) - 1,"%m/%d/%Y %H:%M:%S",
			localtime(&lock_ring_copy[j].tv.tv_sec));
		diff = 0;
		if(lasttv.tv_sec)
		{
			diff = (lock_ring_copy[j].tv.tv_sec - lasttv.tv_sec)
				* 1000000;
			diff += (lock_ring_copy[j].tv.tv_usec - lasttv.tv_usec);
		}
		lasttv.tv_sec = lock_ring_copy[j].tv.tv_sec;
		lasttv.tv_usec = lock_ring_copy[j].tv.tv_usec;
		if (!lock_ring_copy[j].tv.tv_sec) continue;
		if (lock_ring_copy[j].line < 0)
		{
			ast_log(LOG_NOTICE,"LOCKDEBUG [#%d] UNLOCK app_rpt.c:%d node %s pid %x diff %lld us at %s.%06d\n",
				i - 31,-lock_ring_copy[j].line,lock_ring_copy[j].rpt->name,(int) lock_ring_copy[j].lockthread.id,diff,a,(int)lock_ring_copy[j].tv.tv_usec);
		}
		else
		{
			ast_log(LOG_NOTICE,"LOCKDEBUG [#%d] LOCK app_rpt.c:%d node %s pid %x diff %lld us at %s.%06d\n",
				i - 31,lock_ring_copy[j].line,lock_ring_copy[j].rpt->name,(int) lock_ring_copy[j].lockthread.id,diff,a,(int)lock_ring_copy[j].tv.tv_usec);
		}
	}
}


static void _rpt_mutex_lock(ast_mutex_t *lockp, struct rpt *myrpt, int line)
{
struct lockthread *t;
pthread_t id;

	id = pthread_self();
	ast_mutex_lock(&locklock);
	t = put_lockthread(id);
	if (!t)
	{
		ast_mutex_unlock(&locklock);
		return;
	}
	if (t->lockcount)
	{
		int lastline = t->lastlock;
		ast_mutex_unlock(&locklock);
		ast_log(LOG_NOTICE,"rpt_mutex_lock: Double lock request line %d node %s pid %x, last lock was line %d\n",line,myrpt->name,(int) t->id,lastline);
		rpt_mutex_spew();
		return;
	}
	t->lastlock = line;
	t->lockcount = 1;
	gettimeofday(&lock_ring[lock_ring_index].tv, NULL);
	lock_ring[lock_ring_index].rpt = myrpt;
	memcpy(&lock_ring[lock_ring_index].lockthread,t,sizeof(struct lockthread));
	lock_ring[lock_ring_index++].line = line;
	if(lock_ring_index == 32)
		lock_ring_index = 0;
	ast_mutex_unlock(&locklock);
	ast_mutex_lock(lockp);
}


static void _rpt_mutex_unlock(ast_mutex_t *lockp, struct rpt *myrpt, int line)
{
struct lockthread *t;
pthread_t id;

	id = pthread_self();
	ast_mutex_lock(&locklock);
	t = put_lockthread(id);
	if (!t)
	{
		ast_mutex_unlock(&locklock);
		return;
	}
	if (!t->lockcount)
	{
		int lastline = t->lastunlock;
		ast_mutex_unlock(&locklock);
		ast_log(LOG_NOTICE,"rpt_mutex_lock: Double un-lock request line %d node %s pid %x, last un-lock was line %d\n",line,myrpt->name,(int) t->id,lastline);
		rpt_mutex_spew();
		return;
	}
	t->lastunlock = line;
	t->lockcount = 0;
	gettimeofday(&lock_ring[lock_ring_index].tv, NULL);
	lock_ring[lock_ring_index].rpt = myrpt;
	memcpy(&lock_ring[lock_ring_index].lockthread,t,sizeof(struct lockthread));
	lock_ring[lock_ring_index++].line = -line;
	if(lock_ring_index == 32)
		lock_ring_index = 0;
	ast_mutex_unlock(&locklock);
	ast_mutex_unlock(lockp);
}

#else  /* APP_RPT_LOCK_DEBUG */

#define rpt_mutex_lock(x) ast_mutex_lock(x)
#define rpt_mutex_unlock(x) ast_mutex_unlock(x)

#endif  /* APP_RPT_LOCK_DEBUG */

/*
* Return 1 if rig is multimode capable
*/

static int multimode_capable(struct rpt *myrpt)
{
	if(!strcmp(myrpt->remoterig, remote_rig_ft897))
		return 1;
	if(!strcmp(myrpt->remoterig, remote_rig_ft950))
		return 1;
	if(!strcmp(myrpt->remoterig, remote_rig_ic706))
		return 1;
	return 0;
}

static void voxinit_rpt(struct rpt *myrpt,char enable)
{
	myrpt->vox.speech_energy = 0.0;
	myrpt->vox.noise_energy = 0.0;
	myrpt->vox.enacount = 0;
	myrpt->vox.voxena = 0;
	if (!enable) myrpt->vox.voxena = -1;
	myrpt->vox.lastvox = 0;
	myrpt->vox.ondebcnt = VOX_ON_DEBOUNCE_COUNT;
	myrpt->vox.offdebcnt = VOX_OFF_DEBOUNCE_COUNT;
	myrpt->vox.voxminth = myrpt->p.voxminth;
	myrpt->vox.voxmaxth = myrpt->p.voxmaxth;
	myrpt->wasvox = 0;
	myrpt->voxtotimer = 0;
	myrpt->voxtostate = 0;
}

static void voxinit_link(struct rpt_link *mylink,char enable)
{
	mylink->vox.speech_energy = 0.0;
	mylink->vox.noise_energy = 0.0;
	mylink->vox.enacount = 0;
	mylink->vox.voxena = 0;
	if (!enable) mylink->vox.voxena = -1;
	mylink->vox.lastvox = 0;
	mylink->vox.ondebcnt = VOX_ON_DEBOUNCE_COUNT;
	mylink->vox.offdebcnt = VOX_OFF_DEBOUNCE_COUNT;

	if(mylink->parent)
	{
		struct rpt *myrpt;
		myrpt=(struct rpt *)(mylink->parent);
		mylink->vox.voxminth = myrpt->p.voxminth;
		mylink->vox.voxmaxth = myrpt->p.voxmaxth;
	}
	else
	{
		mylink->vox.voxminth = VOX_MIN_THRESHOLD;
		mylink->vox.voxmaxth = VOX_MAX_THRESHOLD;
	}

	mylink->wasvox = 0;
	mylink->voxtotimer = 0;
	mylink->voxtostate = 0;
}

static int dovox(struct vox *v,short *buf,int bs)
{
	int i;
	float	esquare = 0.0;
	float	energy = 0.0;
	float	threshold = 0.0;

	if (v->voxena < 0) return(v->lastvox);
	for(i = 0; i < bs; i++)
	{
		esquare += (float) buf[i] * (float) buf[i];
	}
	energy = sqrt(esquare);

	if (energy >= v->speech_energy)
		v->speech_energy += (energy - v->speech_energy) / 4;
	else
		v->speech_energy += (energy - v->speech_energy) / 64;

	if (energy >= v->noise_energy)
		v->noise_energy += (energy - v->noise_energy) / 64;
	else
		v->noise_energy += (energy - v->noise_energy) / 4;

	if (v->voxena)
		threshold = v->speech_energy / 8;
	else
	{
		threshold = mymax(v->speech_energy / 16,v->noise_energy * 2);
		threshold = mymin(threshold,v->voxmaxth);
	}
	threshold = mymax(threshold,v->voxminth);
	if (energy > threshold)
	{
		if (v->voxena) v->noise_energy *= 0.75;
		v->voxena = 1;
	} else 	v->voxena = 0;
	if (v->lastvox != v->voxena)
	{
		if (v->enacount++ >= ((v->lastvox) ? v->offdebcnt : v->ondebcnt))
		{
			v->lastvox = v->voxena;
			v->enacount = 0;
		}
	} else v->enacount = 0;
	return(v->lastvox);
}

/*! \brief Determine if linking to a coded squelch master and set flag */
static int check_linkmodemaster(struct rpt *myrpt, struct rpt_link *l)
{
	l->csmm = 0;
	if (myrpt->p.ncodemasters)
	{
		int x;
		for(x = 0; x < myrpt->p.ncodemasters; x++)
		{
			if (!strcasecmp(l->name,myrpt->p.codemaster[x]))
			{
				l->csmm = 1;
				break;
			}
		}
	}
	if ( debug > 1 )
		ast_log(LOG_NOTICE,
			"[%s]  link=%s/%s set codemaster flag csmm = %i \n",
			myrpt->name,l->name,l->chan->name,l->csmm);

	return(l->csmm);
}


/*
* CLI extensions
*/

/* Debug mode */
static int rpt_do_debug(int fd, int argc, char *argv[]);
static int rpt_do_dump(int fd, int argc, char *argv[]);
static int rpt_do_stats(int fd, int argc, char *argv[]);
static int rpt_do_lstats(int fd, int argc, char *argv[]);
static int rpt_do_nodes(int fd, int argc, char *argv[]);
static int rpt_do_local_nodes(int fd, int argc, char *argv[]);
static int rpt_do_reload(int fd, int argc, char *argv[]);
static int rpt_do_restart(int fd, int argc, char *argv[]);
static int rpt_do_playback(int fd, int argc, char *argv[]);
static int rpt_do_localplay(int fd, int argc, char *argv[]);
static int rpt_do_irlpplay(int fd, int argc, char *argv[]);
static int rpt_do_fun(int fd, int argc, char *argv[]);
static int rpt_do_fun1(int fd, int argc, char *argv[]);
static int rpt_do_cmd(int fd, int argc, char *argv[]);

static char debug_usage[] =
"Usage: rpt debug level {0-7}\n"
"       Enables debug messages in app_rpt\n";

static char dump_usage[] =
"Usage: rpt dump <nodename>\n"
"       Dumps struct debug info to log\n";

static char dump_stats[] =
"Usage: rpt stats <nodename>\n"
"       Dumps node statistics to console\n";

static char dump_lstats[] =
"Usage: rpt lstats <nodename>\n"
"       Dumps link statistics to console\n";

static char dump_nodes[] =
"Usage: rpt nodes <nodename>\n"
"       Dumps a list of directly and indirectly connected nodes to the console\n";

static char usage_local_nodes[] =
"Usage: rpt localnodes\n"
"       Dumps a list of the locally configured node numbers to the console.\n";

static char reload_usage[] =
"Usage: rpt reload\n"
"       Reloads app_rpt running config parameters\n";

static char restart_usage[] =
"Usage: rpt restart\n"
"       Restarts app_rpt\n";

static char playback_usage[] =
"Usage: rpt playback <nodename> <sound_file_base_name>\n"
"       Send an Audio File to a node, send to all other connected nodes (global)\n";

static char localplay_usage[] =
"Usage: rpt localplay <nodename> <sound_file_base_name>\n"
"       Send an Audio File to a node, do not send to other connected nodes (local)\n";

static char irlpplay_usage[] =
"Usage: rpt irlpplay <nodename> <sound_file_base_name>\n"
"       Send an Audio File to a node, do not send to other connected nodes (local)\n";

static char fun_usage[] =
"Usage: rpt fun <nodename> <command>\n"
"       Send a DTMF function to a node\n";

static char cmd_usage[] =
"Usage: rpt cmd <nodename> <cmd-name> <cmd-index> <cmd-args.\n"
"       Send a command to a node.\n        i.e. rpt cmd 2000 ilink 3 2001\n";

#ifndef	NEW_ASTERISK

static struct ast_cli_entry  cli_debug =
        { { "rpt", "debug", "level" }, rpt_do_debug,
		"Enable app_rpt debugging", debug_usage };

static struct ast_cli_entry  cli_dump =
        { { "rpt", "dump" }, rpt_do_dump,
		"Dump app_rpt structs for debugging", dump_usage };

static struct ast_cli_entry  cli_stats =
        { { "rpt", "stats" }, rpt_do_stats,
		"Dump node statistics", dump_stats };

static struct ast_cli_entry  cli_nodes =
        { { "rpt", "nodes" }, rpt_do_nodes,
		"Dump node list", dump_nodes };

static struct ast_cli_entry  cli_local_nodes =
        { { "rpt", "localnodes" }, rpt_do_local_nodes,
		"Dump list of local node numbers", usage_local_nodes };

static struct ast_cli_entry  cli_lstats =
        { { "rpt", "lstats" }, rpt_do_lstats,
		"Dump link statistics", dump_lstats };

static struct ast_cli_entry  cli_reload =
        { { "rpt", "reload" }, rpt_do_reload,
		"Reload app_rpt config", reload_usage };

static struct ast_cli_entry  cli_restart =
        { { "rpt", "restart" }, rpt_do_restart,
		"Restart app_rpt", restart_usage };

static struct ast_cli_entry  cli_playback =
        { { "rpt", "playback" }, rpt_do_playback,
		"Play Back an Audio File Globally", playback_usage };

static struct ast_cli_entry  cli_localplay =
        { { "rpt", "localplay" }, rpt_do_localplay,
                "Play Back an Audio File Locally", localplay_usage };

static struct ast_cli_entry  cli_irlpplay =
        { { "rpt", "irlpplay" }, rpt_do_irlpplay,
                "Play Back an Audio File Locally", irlpplay_usage };

static struct ast_cli_entry  cli_fun =
        { { "rpt", "fun" }, rpt_do_fun,
		"Execute a DTMF function", fun_usage };

static struct ast_cli_entry  cli_fun1 =
        { { "rpt", "fun1" }, rpt_do_fun1,
		"Execute a DTMF function", fun_usage };

static struct ast_cli_entry  cli_cmd =
        { { "rpt", "cmd" }, rpt_do_cmd,
		"Execute a DTMF function", cmd_usage };

#endif

/*
* Telemetry defaults
*/


static struct telem_defaults tele_defs[] = {
	{"ct1","|t(350,0,100,3072)(500,0,100,3072)(660,0,100,3072)"},
	{"ct2","|t(660,880,150,3072)"},
	{"ct3","|t(440,0,150,3072)"},
	{"ct4","|t(550,0,150,3072)"},
	{"ct5","|t(660,0,150,3072)"},
	{"ct6","|t(880,0,150,3072)"},
	{"ct7","|t(660,440,150,3072)"},
	{"ct8","|t(700,1100,150,3072)"},
	{"remotemon","|t(1600,0,75,2048)"},
	{"remotetx","|t(2000,0,75,2048)(0,0,75,0)(1600,0,75,2048)"},
	{"cmdmode","|t(900,904,200,2048)"},
	{"functcomplete","|t(1000,0,100,2048)(0,0,100,0)(1000,0,100,2048)"}
} ;

/*
	Tone Burst Detection
	Inline for speed?
*/
static inline void goertzel_sample(goertzel_state_t *s, short sample)
{
	int v1;

	v1 = s->v2;
	s->v2 = s->v3;

	s->v3 = (s->fac * s->v2) >> 15;
	s->v3 = s->v3 - v1 + (sample >> s->chunky);
	if (abs(s->v3) > 32768) {
		s->chunky++;
		s->v3 = s->v3 >> 1;
		s->v2 = s->v2 >> 1;
		v1 = v1 >> 1;
	}
}

static inline void goertzel_update(goertzel_state_t *s, short *samps, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		goertzel_sample(s, samps[i]);
	}
}


static inline float goertzel_result(goertzel_state_t *s)
{
	goertzel_result_t r;
	r.value = (s->v3 * s->v3) + (s->v2 * s->v2);
	r.value -= ((s->v2 * s->v3) >> 15) * s->fac;
	r.power = s->chunky * 2;
	return (float)r.value * (float)(1 << r.power);
}

static inline void goertzel_init(goertzel_state_t *s, float freq, int samples)
{
	s->v2 = s->v3 = s->chunky = 0.0;
	s->fac = (int)(32768.0 * 2.0 * cos(2.0 * M_PI * freq / TONE_SAMPLE_RATE));
	s->samples = samples;
}

static inline void goertzel_reset(goertzel_state_t *s)
{
	s->v2 = s->v3 = s->chunky = 0.0;
}

static void tone_detect_init(tone_detect_state_t *s, int freq, int duration, int amp)
{
	int duration_samples;
	float x;
	int periods_in_block;

	s->freq = freq;

	/* Desired tone duration in samples */
	duration_samples = duration * TONE_SAMPLE_RATE / 1000;
	/* We want to allow 10% deviation of tone duration */
	duration_samples = duration_samples * 9 / 10;

	/* If we want to remove tone, it is important to have block size not
	   to exceed frame size. Otherwise by the moment tone is detected it is too late
 	   to squelch it from previous frames */
	s->block_size = TONE_SAMPLES_IN_FRAME;

	periods_in_block = s->block_size * freq / TONE_SAMPLE_RATE;

	/* Make sure we will have at least 5 periods at target frequency for analisys.
	   This may make block larger than expected packet and will make squelching impossible
	   but at least we will be detecting the tone */
	if (periods_in_block < 5)
		periods_in_block = 5;

	/* Now calculate final block size. It will contain integer number of periods */
	s->block_size = periods_in_block * TONE_SAMPLE_RATE / freq;

	/* tone_detect is currently only used to detect fax tones and we
	   do not need suqlching the fax tones */
	s->squelch = 0;

	/* Account for the first and the last block to be incomplete
	   and thus no tone will be detected in them */
	s->hits_required = (duration_samples - (s->block_size - 1)) / s->block_size;

	goertzel_init(&s->tone, freq, s->block_size);

	s->samples_pending = s->block_size;
	s->hit_count = 0;
	s->last_hit = 0;
	s->energy = 0.0;

	/* We want tone energy to be amp decibels above the rest of the signal (the noise).
	   According to Parseval's theorem the energy computed in time domain equals to energy
	   computed in frequency domain. So subtracting energy in the frequency domain (Goertzel result)
	   from the energy in the time domain we will get energy of the remaining signal (without the tone
	   we are detecting). We will be checking that
		10*log(Ew / (Et - Ew)) > amp
	   Calculate threshold so that we will be actually checking
		Ew > Et * threshold
	*/

	x = pow(10.0, amp / 10.0);
	s->threshold = x / (x + 1);

	ast_log(LOG_DEBUG,"Setup tone %d Hz, %d ms, block_size=%d, hits_required=%d\n", freq, duration, s->block_size, s->hits_required);
}


static int tone_detect(tone_detect_state_t *s, int16_t *amp, int samples)
{
	float tone_energy;
	int i;
	int hit = 0;
	int limit;
	int res = 0;
	int16_t *ptr;
	int start, end;

	for (start = 0;  start < samples;  start = end) {
		/* Process in blocks. */
		limit = samples - start;
		if (limit > s->samples_pending) {
			limit = s->samples_pending;
		}
		end = start + limit;

		for (i = limit, ptr = amp ; i > 0; i--, ptr++) {
			/* signed 32 bit int should be enough to suqare any possible signed 16 bit value */
			s->energy += (int32_t) *ptr * (int32_t) *ptr;

			goertzel_sample(&s->tone, *ptr);
		}

		s->samples_pending -= limit;

		if (s->samples_pending) {
			/* Finished incomplete (last) block */
			break;
		}

		tone_energy = goertzel_result(&s->tone);

		/* Scale to make comparable */
		tone_energy *= 2.0;
		s->energy *= s->block_size;

		hit = 0;
		ast_log(LOG_DEBUG,"tone %d, Ew=%.2E, Et=%.2E, s/n=%10.2f\n", s->freq, tone_energy, s->energy, tone_energy / (s->energy - tone_energy));
		if (tone_energy > s->energy * s->threshold) {
			ast_log(LOG_DEBUG,"Hit! count=%d\n", s->hit_count);
			hit = 1;
		}

		if (s->hit_count) {
			s->hit_count++;
		}

		if (hit == s->last_hit) {
			if (!hit) {
				/* Two successive misses. Tone ended */
				s->hit_count = 0;
			} else if (!s->hit_count) {
				s->hit_count++;
			}

		}

		if (s->hit_count >= s->hits_required) {
			ast_log(LOG_DEBUG,"%d Hz tone detected\n", s->freq);
			res = 1;
		}

		s->last_hit = hit;

		/* Reinitialise the detector for the next block */
		/* Reset for the next block */
		goertzel_reset(&s->tone);

		/* Advance to the next block */
		s->energy = 0.0;
		s->samples_pending = s->block_size;

		amp += limit;
	}

	return res;
}

/*
* Forward decl's - these suppress compiler warnings when funcs coded further down the file than their invocation
*/

static int setrbi(struct rpt *myrpt);
static int set_ft897(struct rpt *myrpt);
static int set_ft950(struct rpt *myrpt);
static int set_ic706(struct rpt *myrpt);
static int setkenwood(struct rpt *myrpt);
static int set_tm271(struct rpt *myrpt);
static int setrbi_check(struct rpt *myrpt);
static int setxpmr(struct rpt *myrpt, int dotx);



/*
* Define function protos for function table here
*/

static int function_ilink(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink);
static int function_autopatchup(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink);
static int function_autopatchdn(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink);
static int function_status(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink);
static int function_cop(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink);
static int function_remote(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink);
static int function_macro(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink);
static int function_playback(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink);
static int function_localplay(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink);

/*
* Function table
*/

static struct function_table_tag function_table[] = {
	{"cop", function_cop},
	{"autopatchup", function_autopatchup},
	{"autopatchdn", function_autopatchdn},
	{"ilink", function_ilink},
	{"status", function_status},
	{"remote", function_remote},
	{"macro", function_macro},
	{"playback", function_playback},
	{"localplay", function_localplay}

} ;

static long diskavail(struct rpt *myrpt)
{
struct	statfs statfsbuf;

	if (!myrpt->p.archivedir) return(0);
	if (statfs(myrpt->p.archivedir,&statfsbuf) == -1)
	{
		ast_log(LOG_WARNING,"Cannot get filesystem size for %s node %s\n",
			myrpt->p.archivedir,myrpt->name);
		return(-1);
	}
	return(statfsbuf.f_bavail);
}

static void flush_telem(struct rpt *myrpt)
{
	struct rpt_tele *telem;
	if(debug)ast_log(LOG_NOTICE, "[%s] start ",myrpt->name);
	rpt_mutex_lock(&myrpt->lock);
	telem = myrpt->tele.next;
	while(telem != &myrpt->tele)
	{
		if (telem->mode != SETREMOTE) ast_softhangup(telem->chan,AST_SOFTHANGUP_DEV);
		telem = telem->next;
	}
	rpt_mutex_unlock(&myrpt->lock);
}
/*
	return via error priority
*/
static int priority_jump(struct rpt *myrpt, struct ast_channel *chan)
{
	int res=0;
	if(debug)ast_log(LOG_NOTICE, "[%s] start ",myrpt->name);
	// if (ast_test_flag(&flags,OPT_JUMP) && ast_goto_if_exists(chan, chan->context, chan->exten, chan->priority + 101) == 0){
	if (ast_goto_if_exists(chan, chan->context, chan->exten, chan->priority + 101) == 0){
		res = 0;
	} else {
		res = -1;
	}
	return res;
}
/*
*/

static void init_linkmode(struct rpt *myrpt, struct rpt_link *mylink, int linktype)
{
	if (!myrpt) return;
	if (!mylink) return;
	if(debug)ast_log(LOG_NOTICE, "[%s] start ",myrpt->name);
	switch(myrpt->p.linkmode[linktype])
	{
	    case LINKMODE_OFF:
		mylink->linkmode = 0;
		break;
	    case LINKMODE_ON:
		mylink->linkmode = 0x7fffffff;
		break;
	    case LINKMODE_FOLLOW:
		mylink->linkmode = 0x7ffffffe;
		break;
	    case LINKMODE_DEMAND:
		mylink->linkmode = 1;
		break;
	}
	return;
}

static void set_linkmode(struct rpt_link *mylink, int linkmode)
{
	if (!mylink) return;
	if(debug)ast_log(LOG_NOTICE, "start link=%s",mylink->name);
 	switch(linkmode)
	{
	    case LINKMODE_OFF:
		mylink->linkmode = 0;
		break;
	    case LINKMODE_ON:
		mylink->linkmode = 0x7fffffff;
		break;
	    case LINKMODE_FOLLOW:
		mylink->linkmode = 0x7ffffffe;
		break;
	    case LINKMODE_DEMAND:
		mylink->linkmode = 1;
		break;
	}
	return;
}

static void rpt_telem_select(struct rpt *myrpt, int command_source, struct rpt_link *mylink)
{
int	src;

	if(debug)ast_log(LOG_NOTICE, "[%s] start",myrpt->name);
	if (mylink && mylink->chan)
	{
		src = LINKMODE_GUI;
		if (mylink->phonemode) src = LINKMODE_PHONE;
		else if (!strncasecmp(mylink->chan->name,"echolink",8)) src = LINKMODE_ECHOLINK;
		else if (!strncasecmp(mylink->chan->name,"irlp",4)) src = LINKMODE_IRLP;
		if (myrpt->p.linkmodedynamic[src] && (mylink->linkmode >= 1) &&
		    (mylink->linkmode < 0x7ffffffe))
				mylink->linkmode = LINK_HANG_TIME;
	}
	if (!myrpt->p.telemdynamic) return;
	if (myrpt->telemmode == 0) return;
	if (myrpt->telemmode == 0x7fffffff) return;
	myrpt->telemmode = TELEM_HANG_TIME;
	return;
}

/*
	For the referenced repeater/node and link,
	returns 1 if the link is not to a Foreign Link.
	A foreign link is to another network or system.
	Echolink is an example of a foreign link.
	The purpose is to not transmit telemetry to foreign links.
*/
static int altlink(struct rpt *myrpt,struct rpt_link *mylink)
{
	if (!myrpt) return(0);
	if (!mylink) return(0);
	if (!mylink->chan) return(0);
	/* if doesn't qualify as a foreign link */
	if ((mylink->name[0] != '0') && (!mylink->phonemode) &&
	    strncasecmp(mylink->chan->name,"echolink",8) &&
		strncasecmp(mylink->chan->name,"irlp",4)) return(0);
	if ((myrpt->p.duplex < 2) && (myrpt->tele.next == &myrpt->tele)) return(0);
	if ((mylink->iaxrpt) && (myrpt->tele.next == &myrpt->tele)) return(0);
	if (mylink->linkmode < 2) return(0);
	if (mylink->linkmode == 0x7fffffff) return(1);
	if (mylink->linkmode < 0x7ffffffe) return(1);
	if (myrpt->telemmode > 1) return(1);
	return(0);
}

/*
	Returns 1 if Link is not to a Foreign Link and
	this node has queued telemetry or tones to output.
*/
static int altlink1(struct rpt *myrpt,struct rpt_link *mylink)
{
struct  rpt_tele *tlist;
int	nonlocals;

	if (!myrpt) return(0);
	if (!mylink) return(0);
	if (!mylink->chan) return(0);
	nonlocals = 0;
	tlist = myrpt->tele.next;
    if (tlist != &myrpt->tele)
    {
        while(tlist != &myrpt->tele)
		{
			if ((tlist->mode == PLAYBACK) ||
				(tlist->mode == TEST_TONE)) nonlocals++;
			tlist = tlist->next;
		}
	}
	if ((!myrpt->p.duplex) || (!nonlocals)) return(0);
	/* if doesnt qualify as a foreign link */
	if ((mylink->name[0] != '0') && (!mylink->phonemode) &&
	    strncasecmp(mylink->chan->name,"echolink",8) &&
		strncasecmp(mylink->chan->name,"irlp",4)) return(1);
	if (mylink->linkmode < 2) return(0);
	if (mylink->linkmode == 0x7fffffff) return(1);
	if (mylink->linkmode < 0x7ffffffe) return(1);
	if (myrpt->telemmode > 1) return(1);
	return(0);
}

static void rpt_qwrite(struct rpt_link *l,struct ast_frame *f)
{
struct	ast_frame *f1;

	if (!l->chan) return;
	f1 = ast_frdup(f);
	memset(&f1->frame_list,0,sizeof(f1->frame_list));
	AST_LIST_INSERT_TAIL(&l->textq,f1,frame_list);
	return;
}

static int linkcount(struct rpt *myrpt)
{
	struct	rpt_link *l;
	char *reverse_patch_state;
 	int numoflinks;

	reverse_patch_state = "DOWN";
	numoflinks = 0;
	myrpt->phonelinks=0;
	l = myrpt->links.next;
	while(l && (l != &myrpt->links)){
		if(numoflinks >= MAX_STAT_LINKS){
			ast_log(LOG_WARNING,
			"[%s] maximum number of links exceeds %d in rpt_do_stats()!\n",myrpt->name,MAX_STAT_LINKS);
			break;
		}
		//if (l->name[0] == '0'){ /* Skip '0' nodes */
		//	reverse_patch_state = "UP";
		//	l = l->next;
		//	continue;
		//}
		if(l->phonemode)myrpt->phonelinks++;
		numoflinks++;

		l = l->next;
	}
	if(debug)ast_log(LOG_NOTICE,"[%s] numoflinks=%i\n",myrpt->name,numoflinks);
	return numoflinks;
}
/* Considers repeater received RSSI and all voter link RSSI information and
   set values in myrpt structure.
*/
static int FindBestRssi(struct rpt *myrpt)
{
	struct	rpt_link *l;
	struct	rpt_link *bl;
	int maxrssi;
 	int numoflinks;
    char newboss;

	bl=NULL;
	maxrssi=0;
	numoflinks=0;
    newboss=0;

    myrpt->voted_rssi=0;
    if(myrpt->votewinner&&myrpt->rxchankeyed)
        myrpt->voted_rssi=myrpt->rxrssi;
    else if(myrpt->voted_link!=NULL && myrpt->voted_link->lastrealrx)
        myrpt->voted_rssi=myrpt->voted_link->rssi;

	if(myrpt->rxchankeyed)
		maxrssi=myrpt->rxrssi;

	l = myrpt->links.next;
	while(l && (l != &myrpt->links)){
		if(numoflinks >= MAX_STAT_LINKS){
			ast_log(LOG_WARNING,
			"[%s] number of links exceeds limit of %d \n",myrpt->name,MAX_STAT_LINKS);
			break;
		}
		if(l->lastrealrx && (l->rssi > maxrssi))
		{
			maxrssi=l->rssi;
			bl=l;
		}
		l->votewinner=0;
		numoflinks++;
		l = l->next;
	}

	if( !myrpt->voted_rssi ||
	    (myrpt->voted_link==NULL && !myrpt->votewinner) ||
	    (maxrssi>(myrpt->voted_rssi+myrpt->p.votermargin))
	  )
	{
        newboss=1;
        myrpt->votewinner=0;
		if(bl==NULL && myrpt->rxchankeyed)
			myrpt->votewinner=1;
		else if(bl!=NULL)
			bl->votewinner=1;
		myrpt->voted_link=bl;
		myrpt->voted_rssi=maxrssi;
    }

	if(debug>4)
	    ast_log(LOG_NOTICE,"[%s] links=%i best rssi=%i from %s%s\n",
	        myrpt->name,numoflinks,maxrssi,bl==NULL?"rpt":bl->name,newboss?"*":"");

	return numoflinks;
}
/*
 * Retrieve a memory channel
 * Return 0 if sucessful,
 * -1 if channel not found,
 *  1 if parse error
 */
static int retreive_memory(struct rpt *myrpt, char *memory)
{
	char tmp[30], *s, *s1,*s2, *val;

	if (debug)ast_log(LOG_NOTICE,"[%s] start memory=%s context=%s\n",myrpt->name,memory,myrpt->p.memory);

	val = (char *) ast_variable_retrieve(myrpt->cfg, myrpt->p.memory, memory);
	if (!val){
		return -1;
	}
	strncpy(tmp,val,sizeof(tmp) - 1);
	tmp[sizeof(tmp)-1] = 0;

	s = strchr(tmp,',');
	if (!s)
		return 1;
	*s++ = 0;
	s1 = strchr(s,',');
	if (!s1)
		return 1;
	*s1++ = 0;
	s2 = strchr(s1,',');
	if (s2)
		*s2++ = 0;

	strncpy(myrpt->freq, tmp, sizeof(myrpt->freq) - 1);
	strncpy(myrpt->rxpl, s, sizeof(myrpt->rxpl) - 1);
	strncpy(myrpt->txpl, s, sizeof(myrpt->rxpl) - 1);

	if (s2)
		strncpy(myrpt->freqtx, s2, sizeof(myrpt->freqtx) - 1);

	myrpt->remmode = REM_MODE_FM;
	myrpt->offset = REM_SIMPLEX;
	myrpt->powerlevel = REM_MEDPWR;
	myrpt->txplon = myrpt->rxplon = 0;
	while(*s1){
		switch(*s1++){
			case 'A':
			case 'a':
				strcpy(myrpt->rxpl, "100.0");
				strcpy(myrpt->txpl, "100.0");
				myrpt->remmode = REM_MODE_AM;
				break;
			case 'B':
			case 'b':
				strcpy(myrpt->rxpl, "100.0");
				strcpy(myrpt->txpl, "100.0");
				myrpt->remmode = REM_MODE_LSB;
				break;
			case 'F':
				myrpt->remmode = REM_MODE_FM;
				break;
			case 'L':
			case 'l':
				myrpt->powerlevel = REM_LOWPWR;
				break;
			case 'H':
			case 'h':
				myrpt->powerlevel = REM_HIPWR;
				break;

			case 'M':
			case 'm':
				myrpt->powerlevel = REM_MEDPWR;
				break;

			case '-':
				myrpt->offset = REM_MINUS;
				break;

			case '+':
				myrpt->offset = REM_PLUS;
				break;

			case 'S':
			case 's':
				myrpt->offset = REM_SIMPLEX;
				break;

			case 'W':
			case 'w':
				myrpt->offset = REM_WILD;
				break;

			case 'T':
			case 't':
				myrpt->txplon = 1;
				break;

			case 'R':
			case 'r':
				myrpt->rxplon = 1;
				break;

			case 'U':
			case 'u':
				strcpy(myrpt->rxpl, "100.0");
				strcpy(myrpt->txpl, "100.0");
				myrpt->remmode = REM_MODE_USB;
				break;
			default:
				return 1;
		}
	}
	return 0;
}
/*

*/
static void birdbath(struct rpt *myrpt)
{
	struct rpt_tele *telem;
	if(debug)ast_log(LOG_NOTICE, "[%s] start",myrpt->name);
	rpt_mutex_lock(&myrpt->lock);
	telem = myrpt->tele.next;
	while(telem != &myrpt->tele)
	{
		if (telem->mode == PARROT) ast_softhangup(telem->chan,AST_SOFTHANGUP_DEV);
		telem = telem->next;
	}
	rpt_mutex_unlock(&myrpt->lock);
}


static void do_dtmf_phone(struct rpt *myrpt, struct rpt_link *mylink, char c)
{
struct        rpt_link *l;

       l = myrpt->links.next;
       /* go thru all the links */
       while(l != &myrpt->links)
       {
               if (!l->phonemode)
               {
                       l = l->next;
                       continue;
               }
               /* dont send to self */
               if (mylink && (l == mylink))
               {
                       l = l->next;
                       continue;
               }
#ifdef	NEW_ASTERISK
               if (l->chan) ast_senddigit(l->chan,c,0);
#else
               if (l->chan) ast_senddigit(l->chan,c);
#endif
               l = l->next;
       }
       return;
}

/* node logging function */
static void donodelog(struct rpt *myrpt,char *str)
{
struct nodelog *nodep;
char	datestr[100];

	if (!myrpt->p.archivedir) return;
	nodep = (struct nodelog *)ast_malloc(sizeof(struct nodelog));
	if (nodep == NULL)
	{
		ast_log(LOG_ERROR,"[%s] Cannot get memory for node log",myrpt->name);
		return;
	}
	time(&nodep->timestamp);
	strncpy(nodep->archivedir,myrpt->p.archivedir,
		sizeof(nodep->archivedir) - 1);
	strftime(datestr,sizeof(datestr) - 1,"%Y%m%d%H%M%S",
		localtime(&nodep->timestamp));
	snprintf(nodep->str,sizeof(nodep->str) - 1,"%s %s,%s\n",
		myrpt->name,datestr,str);
	ast_mutex_lock(&nodeloglock);
	insque((struct qelem *) nodep, (struct qelem *) nodelog.prev);
	ast_mutex_unlock(&nodeloglock);
}

/* must be called with rpt locked */
static void do_dtmf_local(struct rpt *myrpt, char c)
{
int	i;
char	digit;

	if (c)
	{
		if(debug)
			ast_log(LOG_NOTICE,"[%s] start dtmf=%c\n",myrpt->name,c);

		snprintf(myrpt->dtmf_local_str + strlen(myrpt->dtmf_local_str),sizeof(myrpt->dtmf_local_str) - 1,"%c",c);
		if (!myrpt->dtmf_local_timer)
			 myrpt->dtmf_local_timer = DTMF_LOCAL_STARTTIME;
	}
	/* if at timeout */
	if (myrpt->dtmf_local_timer == 1)
	{
		if(debug > 6)
			ast_log(LOG_NOTICE,"time out dtmf_local_timer=%i\n",myrpt->dtmf_local_timer);

		/* if anything in the string */
		if (myrpt->dtmf_local_str[0])
		{
			digit = myrpt->dtmf_local_str[0];
			myrpt->dtmf_local_str[0] = 0;
			for(i = 1; myrpt->dtmf_local_str[i]; i++)
			{
				myrpt->dtmf_local_str[i - 1] =
					myrpt->dtmf_local_str[i];
			}
			myrpt->dtmf_local_str[i - 1] = 0;
			myrpt->dtmf_local_timer = DTMF_LOCAL_TIME;
			rpt_mutex_unlock(&myrpt->lock);
			if (!strncasecmp(myrpt->txchannel->name,"rtpdir",6))
			{
				ast_senddigit(myrpt->txchannel,digit);
			}
			else
			{
				if (digit >= '0' && digit <='9')
					ast_playtones_start(myrpt->txchannel, 0, dtmf_tones[digit-'0'], 0);
				else if (digit >= 'A' && digit <= 'D')
					ast_playtones_start(myrpt->txchannel, 0, dtmf_tones[digit-'A'+10], 0);
				else if (digit == '*')
					ast_playtones_start(myrpt->txchannel, 0, dtmf_tones[14], 0);
				else if (digit == '#')
					ast_playtones_start(myrpt->txchannel, 0, dtmf_tones[15], 0);
				else {
					/* not handled */
					ast_log(LOG_DEBUG, "Unable to generate DTMF tone '%c' for '%s'\n", digit, myrpt->txchannel->name);
				}
			}
			rpt_mutex_lock(&myrpt->lock);
		}
		else
		{
			myrpt->dtmf_local_timer = 0;
		}
	}
}

static int setdtr(int fd, int enable)
{
struct termios mode;

	if (fd < 0) return -1;
	if (tcgetattr(fd, &mode)) {
		ast_log(LOG_WARNING, "Unable to get serial parameters for dtr: %s\n", strerror(errno));
		return -1;
	}
	if (enable)
	{
		cfsetspeed(&mode, B9600);
	}
	else
	{
		cfsetspeed(&mode, B0);
		usleep(100000);
	}
	if (tcsetattr(fd, TCSADRAIN, &mode)) {
		ast_log(LOG_WARNING, "Unable to set serial parameters for dtr: %s\n", strerror(errno));
		return -1;
	}
	if (enable) usleep(100000);
	return 0;
}

static int openserial(struct rpt *myrpt,char *fname)
{
	struct termios mode;
	int fd;

	fd = open(fname,O_RDWR);
	if (fd == -1)
	{
		ast_log(LOG_WARNING,"Cannot open serial port %s\n",fname);
		return -1;
	}
	memset(&mode, 0, sizeof(mode));
	if (tcgetattr(fd, &mode)) {
		ast_log(LOG_WARNING, "Unable to get serial parameters on %s: %s\n", fname, strerror(errno));
		return -1;
	}
#ifndef	SOLARIS
	cfmakeraw(&mode);
#else
        mode.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                        |INLCR|IGNCR|ICRNL|IXON);
        mode.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
        mode.c_cflag &= ~(CSIZE|PARENB|CRTSCTS);
        mode.c_cflag |= CS8;
	mode.c_cc[VTIME] = 3;
	mode.c_cc[VMIN] = 1;
#endif

	cfsetispeed(&mode, B9600);
	cfsetospeed(&mode, B9600);
	if (tcsetattr(fd, TCSANOW, &mode))
		ast_log(LOG_WARNING, "Unable to set serial parameters on %s: %s\n", fname, strerror(errno));
	if(!strcmp(myrpt->remoterig, remote_rig_kenwood)) setdtr(fd,0);
	usleep(100000);
	if (debug)ast_log(LOG_NOTICE,"Opened serial port %s\n",fname);
	return(fd);
}

static void mdc1200_notify(struct rpt *myrpt,char *fromnode, unsigned int unit)
{
	if (!fromnode)
	{
		ast_verbose("Got MDC-1200 ID %04X from local system (%s)\n",
			unit,myrpt->name);
	}
	else
	{
		ast_verbose("Got MDC-1200 ID %04X from node %s (%s)\n",
			unit,fromnode,myrpt->name);
	}
}

#ifdef	_MDC_DECODE_H_

static void mdc1200_send(struct rpt *myrpt, unsigned int unit)
{
struct rpt_link *l;
struct	ast_frame wf;
char	str[200];

	sprintf(str,"I %s %04X",myrpt->name,unit);

	wf.frametype = AST_FRAME_TEXT;
	wf.subclass = 0;
	wf.offset = 0;
	wf.mallocd = 0;
	wf.datalen = strlen(str) + 1;
	wf.samples = 0;
	wf.src = "mdc1200_send";

	l = myrpt->links.next;
	/* otherwise, send it to all of em */
	while(l != &myrpt->links)
	{
		if (l->name[0] == '0')
		{
			l = l->next;
			continue;
		}
		wf.data = str;
		if (l->chan) rpt_qwrite(l,&wf);
		l = l->next;
	}
	return;
}
#endif

/*
	rssi_send() Send rx rssi out on all links.

*/
static void rssi_send(struct rpt *myrpt)
{
struct rpt_link *l;
struct	ast_frame wf;
char	str[200];

	sprintf(str,"R %i",myrpt->rxrssi);

	wf.frametype = AST_FRAME_TEXT;
	wf.subclass = 0;
	wf.offset = 0;
	wf.mallocd = 0;
	wf.datalen = strlen(str) + 1;
	wf.samples = 0;
	wf.src = "rssi_send";

	l = myrpt->links.next;
	/* otherwise, send it to all of em */
	while(l != &myrpt->links)
	{
		if (l->name[0] == '0')
		{
			l = l->next;
			continue;
		}
		wf.data = str;
		if(debug>5)ast_log(LOG_NOTICE, "[%s] rssi=%i to %s\n", myrpt->name,myrpt->rxrssi,l->name);
		if (l->chan) rpt_qwrite(l,&wf);
		l = l->next;
	}
}


static char func_xlat(struct rpt *myrpt,char c,struct rpt_xlat *xlat)
{
time_t	now;
int	gotone;

	time(&now);
	gotone = 0;
	/* if too much time, reset the skate machine */
	if ((now - xlat->lastone) > MAXXLATTIME)
	{
		xlat->funcindex = xlat->endindex = 0;
	}
	if (xlat->funccharseq[0] && (c == xlat->funccharseq[xlat->funcindex++]))
	{
		time(&xlat->lastone);
		gotone = 1;
		if (!xlat->funccharseq[xlat->funcindex])
		{
			xlat->funcindex = xlat->endindex = 0;
			return(myrpt->p.funcchar);
		}
	} else xlat->funcindex = 0;
	if (xlat->endcharseq[0] && (c == xlat->endcharseq[xlat->endindex++]))
	{
		time(&xlat->lastone);
		gotone = 1;
		if (!xlat->endcharseq[xlat->endindex])
		{
			xlat->funcindex = xlat->endindex = 0;
			return(myrpt->p.endchar);
		}
	} else xlat->endindex = 0;
	/* if in middle of decode seq, send nothing back */
	if (gotone) return(0);
	/* if no pass chars specified, return em all */
	if (!xlat->passchars[0]) return(c);
	/* if a "pass char", pass it */
	if (strchr(xlat->passchars,c)) return(c);
	return(0);
}

/*
 * Return a pointer to the first non-whitespace character
 */

static char *eatwhite(char *s)
{
	while((*s == ' ') || (*s == 0x09)){ /* get rid of any leading white space */
		if(!*s)
			break;
		s++;
	}
	return s;
}
/*
* Break up a delimited string into a table of substrings
*
* str - delimited string ( will be modified )
* strp- list of pointers to substrings (this is built by this function), NULL will be placed at end of list
* limit- maximum number of substrings to process
* delim- user specified delimeter
* quote- user specified quote for escaping a substring. Set to zero to escape nothing.
*
* Note: This modifies the string str, be suer to save an intact copy if you need it later.
*
* Returns number of substrings found.
*/


static int explode_string(char *str, char *strp[], int limit, char delim, char quote)
{
int     i,l,inquo;

        inquo = 0;
        i = 0;
        strp[i++] = str;
        if (!*str)
           {
                strp[0] = 0;
                return(0);
           }
        for(l = 0; *str && (l < limit) ; str++)
        {
		if(quote)
		{
                	if (*str == quote)
                   	{
                        	if (inquo)
                           	{
                                	*str = 0;
                                	inquo = 0;
                           	}
                        	else
                           	{
                                	strp[i - 1] = str + 1;
                                	inquo = 1;
                           	}
			}
		}
                if ((*str == delim) && (!inquo))
                {
                        *str = 0;
			l++;
                        strp[i++] = str + 1;
                }
        }
        strp[i] = 0;
        return(i);

}

/*
* Break up a delimited string into a table of substrings
*
* str - delimited string ( will be modified )
* strp- list of pointers to substrings (this is built by this function), NULL will be placed at end of list
* limit- maximum number of substrings to process
*/
static int finddelim(char *str, char *strp[], int limit)
{
	return explode_string(str, strp, limit, DELIMCHR, QUOTECHR);
}

#if 1==0
/*! \brief Determine if radio channel is free
	option 0=tx|rx, 1=rx, 2=tx
	returns 1 if channel is free for the indicated purpose
	This is under development for interface to another popular radio format.
*/
static int rpt_check_channel(struct rpt *myrpt, struct rpt_link *mylink, char option)
{
	int result,rxok,txok;

	result=rxok=txok=1;

	if (  myrpt->keyed   			&&
	      mylink->csrxcode[0]!=0 	&&
	      myrpt->csrxnow[0]!=0 		&&
	      (strcmp(myrpt->csrxnow,mylink->csrxcode)!=0)
	   ) {
	   rxok=0; // not free
   	   if(option==0||option==1)result=0;
	}

	if (  myrpt->txkeyed          	&&
	      mylink->cstxcode[0]!=0 	&&
	      myrpt->cstxnow[0]!=0    	&&
	      (strcmp(myrpt->cstxnow,mylink->cstxcode)!=0)
	   ) {
	   txok=0;
   	   if(option==0||option==2)result=0;
	}

	if (debug){
		ast_log(LOG_NOTICE, "  keyed=%i csrxcode=%s csrxnow=%s\n",myrpt->keyed,mylink->csrxcode,myrpt->csrxnow);
		ast_log(LOG_NOTICE, "txkeyed=%i cstxcode=%s cstxnow=%s\n",myrpt->txkeyed,mylink->cstxcode,myrpt->cstxnow);
		ast_log(LOG_NOTICE, " option=%i    result=%i\n",option,result);
	}

	return(result);
}
#endif

/*!
 * \brief Send text frame to USB radio device channel driver
 */
static int send_usb_txt(struct rpt *myrpt, char *txt)
{
	struct ast_frame wf;

	if (debug)ast_log(LOG_NOTICE,"[%s] txt=%s\n",myrpt->name,txt);
	wf.frametype = AST_FRAME_TEXT;
	wf.subclass = 0;
	wf.offset = 0;
	wf.mallocd = 0;
	wf.datalen = strlen(txt) + 1;
	wf.data = txt;
	wf.samples = 0;
	wf.src = "send_usb_txt";
	ast_write(myrpt->txchannel,&wf);
	return 0;
}
/* must be called locked */
static void __mklinklist(struct rpt *myrpt, struct rpt_link *mylink, char *buf)
{
struct rpt_link *l;
char mode;
int	i,spos;

	buf[0] = 0; /* clear output buffer */
	if (myrpt->remote) return;
	/* go thru all links */
	for(l = myrpt->links.next; l != &myrpt->links; l = l->next)
	{
		/* if is not a real link, ignore it */
		if (l->name[0] == '0') continue;
		/* dont count our stuff */
		if (l == mylink) continue;
		if (mylink && (!strcmp(l->name,mylink->name))) continue;
		/* figure out mode to report */
		mode = 'T'; /* use Tranceive by default */
		if (!l->mode) mode = 'R'; /* indicate RX for our mode */
		if (!l->thisconnected) 	mode = 'C'; /* indicate connecting */
		spos = strlen(buf); /* current buf size (b4 we add our stuff) */
		if (spos)
		{
			strcat(buf,",");
			spos++;
		}
		/* add nodes into buffer */
		if (l->linklist[0])
		{
			snprintf(buf + spos,MAXLINKLIST - spos,
				"%c%s,%s",mode,l->name,l->linklist);
		}
		else /* if no nodes, add this node into buffer */
		{
			snprintf(buf + spos,MAXLINKLIST - spos,
				"%c%s",mode,l->name);
		}
		/* if we are in tranceive mode, let all modes stand */
		if (mode == 'T') continue;
		/* downgrade everyone on this node if appropriate */
		for(i = spos; buf[i]; i++)
		{
			if (buf[i] == 'T') buf[i] = mode;
			if ((buf[i] == 'R') && (mode == 'C')) buf[i] = mode;
		}
	}
	return;
}


/* must be called locked */
static void __mklinklist2(struct rpt *myrpt, struct rpt_link *mylink, char *buf)
{
struct rpt_link *l;
char mode;
int	i,spos,cpos;

	buf[0] = 0; /* clear output buffer */
	cpos=0;
	if (myrpt->remote) return;
	/* go thru all links */
	for(l = myrpt->links.next; l != &myrpt->links; l = l->next)
	{
		/* if is not a real link, ignore it */
		if (l->name[0] == '0') continue;
		/* dont count our stuff */
		if (l == mylink) continue;
		if (mylink && (!strcmp(l->name,mylink->name))) continue;
		/* figure out mode to report */
		mode = 'T'; /* use Tranceive by default */
		if (!l->mode) mode = 'R'; /* indicate RX for our mode */
		if (!l->thisconnected) 	mode = 'C'; /* indicate connecting */
		spos = strlen(buf); /* current buf size (b4 we add our stuff) */
		/* add nodes into buffer */
		if (l->linklist[0])
		{
			snprintf(buf + spos,MAXLINKLIST - spos,
				"/%c%s,%s",mode,l->name,l->linklist);
		}
		else /* if no nodes, add this node into buffer */
		{
			snprintf(buf + spos,MAXLINKLIST - spos,
				"/%c%s",mode,l->name);
		}
		/* if we are in tranceive mode, let all modes stand */
		if (mode == 'T') continue;
		/* downgrade everyone on this node if appropriate */
		for(i = spos; buf[i]; i++)
		{
			if (buf[i] == 'T') buf[i] = mode;
			if ((buf[i] == 'R') && (mode == 'C')) buf[i] = mode;
		}
	}
	return;
}

/* must be called locked */
static void __kickshort(struct rpt *myrpt)
{
struct rpt_link *l;

	for(l = myrpt->links.next; l != &myrpt->links; l = l->next)
	{
		/* if is not a real link, ignore it */
		if (l->name[0] == '0') continue;
		l->linklisttimer = LINKLISTSHORTTIME;
	}
	myrpt->linkposttimer = LINKPOSTSHORTTIME;
	return;
}

static void dodispgm(struct rpt *myrpt,char *them)
{
char 	*a;
int	i;

	if (!myrpt->p.discpgm) return;
	i = strlen(them) + strlen(myrpt->p.discpgm) + 100;
	a = ast_malloc(i);
	if (!a)
	{
		ast_log(LOG_NOTICE,"Unable to alloc");
		return;
	}
	memset(a,0,i);
	sprintf(a,"%s %s %s &",myrpt->p.discpgm,
		myrpt->name,them);
	ast_safe_system(a);
	free(a);
	return;
}

static void doconpgm(struct rpt *myrpt,char *them)
{
char 	*a;
int	i;

	if (!myrpt->p.connpgm) return;
	i = strlen(them) + strlen(myrpt->p.connpgm) +  + 100;
	a = ast_malloc(i);
	if (!a)
	{
		ast_log(LOG_NOTICE,"Unable to alloc");
		return;
	}
	memset(a,0,i);
	sprintf(a,"%s %s %s &",myrpt->p.connpgm,
		myrpt->name,them);
	ast_safe_system(a);
	ast_free(a);
	return;
}

static void statpost(struct rpt *myrpt,char *pairs)
{
char *str,*astr;
char *astrs[100];
int	n,pid;
time_t	now;
unsigned int seq;

	if (!myrpt->p.statpost_url) return;
	if(debug)ast_log(LOG_NOTICE, "[%s] start %s\n",myrpt->name,myrpt->p.statpost_url);
	str = ast_malloc(strlen(pairs) + strlen(myrpt->p.statpost_url) + 200);
	astr = ast_strdup(myrpt->p.statpost_program);
	if ((!str) || (!astr)) return;
	n = finddelim(astr,astrs,100);
	if (n < 1) return;
	ast_mutex_lock(&myrpt->statpost_lock);
	seq = ++myrpt->statpost_seqno;
	ast_mutex_unlock(&myrpt->statpost_lock);
	astrs[n++] = str;
	astrs[n] = NULL;
	time(&now);
	sprintf(str,"%s?node=%s&time=%u&seqno=%u",myrpt->p.statpost_url,
		myrpt->name,(unsigned int) now,seq);
	if (pairs) sprintf(str + strlen(str),"&%s",pairs);
	if (!(pid = fork()))
	{
		execv(astrs[0],astrs);
		ast_log(LOG_ERROR, "exec of %s failed.\n", astrs[0]);
		perror("asterisk");
		exit(0);
	}
	if(debug>5)ast_log(LOG_NOTICE, "[%s] execv(%s,%s)\n",myrpt->name,astrs[0],str);
	ast_free(astr);
	ast_free(str);
	return;
}

static char *node_lookup(struct rpt *myrpt,char *digitbuf)
{

char *val;
int longestnode,j;
struct stat mystat;
static struct ast_config *ourcfg = NULL;
struct ast_variable *vp;

	if (debug)ast_log(LOG_NOTICE, "%s\n",digitbuf);

	/* try to look it up locally first */
	val = (char *) ast_variable_retrieve(myrpt->cfg, myrpt->p.nodes, digitbuf);
	if (val)
	{
		if (debug)ast_log(LOG_NOTICE,"val=%s\n",val);
		return(val);
	}
	ast_mutex_lock(&nodelookuplock);
	/* if file does not exist */
	if (stat(myrpt->p.extnodefile,&mystat) == -1)
	{
		if (ourcfg) ast_config_destroy(ourcfg);
		ourcfg = NULL;
		ast_mutex_unlock(&nodelookuplock);
		return(NULL);
	}
	if (ourcfg) ast_config_destroy(ourcfg);
#ifdef	NEW_ASTERISK
	ourcfg = ast_config_load(myrpt->p.extnodefile,config_flags);
#else
	ourcfg = ast_config_load(myrpt->p.extnodefile);
#endif
	/* if file not there, just bail */
	if (!ourcfg)
	{
		ast_mutex_unlock(&nodelookuplock);
		return(NULL);
	}

	/* determine longest node length again */
	longestnode = 0;
	vp = ast_variable_browse(myrpt->cfg, myrpt->p.nodes);
	while(vp){
		j = strlen(vp->name);
		if (j > longestnode)
			longestnode = j;
		vp = vp->next;
	}
	vp = ast_variable_browse(ourcfg, myrpt->p.extnodes);
	while(vp){
		j = strlen(vp->name);
		if (j > longestnode)
			longestnode = j;
		vp = vp->next;
	}

	myrpt->longestnode = longestnode;
	val = NULL;
	if (ourcfg)
		val = (char *) ast_variable_retrieve(ourcfg, myrpt->p.extnodes, digitbuf);
	ast_mutex_unlock(&nodelookuplock);
	if (debug)ast_log(LOG_NOTICE,"val=%s\n",val);
	return(val);
}

/*
* Match a keyword in a list, and return index of string plus 1 if there was a match,* else return 0.
* If param is passed in non-null, then it will be set to the first character past the match
*/

static int matchkeyword(char *string, char **param, char *keywords[])
{
int	i,ls;
	for( i = 0 ; keywords[i] ; i++){
		ls = strlen(keywords[i]);
		if(!ls){
			if(param)
				*param = NULL;
			return 0;
		}
		if(!strncmp(string, keywords[i], ls)){
			if(param)
				*param = string + ls;
			return i + 1;
		}
	}
	if(param)
		*param = NULL;
	return 0;
}

/*
* Skip characters in string which are in charlist, and return a pointer to the
* first non-matching character
*/

static char *skipchars(char *string, char *charlist)
{
int i;
	while(*string){
		for(i = 0; charlist[i] ; i++){
			if(*string == charlist[i]){
				string++;
				break;
			}
		}
		if(!charlist[i])
			return string;
	}
	return string;
}



static int myatoi(char *str)
{
int	ret;

	if (str == NULL) return -1;
	/* leave this %i alone, non-base-10 input is useful here */
	if (sscanf(str,"%i",&ret) != 1) return -1;
	return ret;
}

static int mycompar(const void *a, const void *b)
{
char	**x = (char **) a;
char	**y = (char **) b;
int	xoff,yoff;

	if ((**x < '0') || (**x > '9')) xoff = 1; else xoff = 0;
	if ((**y < '0') || (**y > '9')) yoff = 1; else yoff = 0;
	return(strcmp((*x) + xoff,(*y) + yoff));
}

static int topcompar(const void *a, const void *b)
{
struct rpt_topkey *x = (struct rpt_topkey *) a;
struct rpt_topkey *y = (struct rpt_topkey *) b;

	return(x->timesince - y->timesince);
}

#ifdef	__RPT_NOTCH

/* rpt filter routine */
static void rpt_filter(struct rpt *myrpt, volatile short *buf, int len)
{
int	i,j;
struct	rptfilter *f;

	for(i = 0; i < len; i++)
	{
		for(j = 0; j < MAXFILTERS; j++)
		{
			f = &myrpt->filters[j];
			if (!*f->desc) continue;
			f->x0 = f->x1; f->x1 = f->x2;
		        f->x2 = ((float)buf[i]) / f->gain;
		        f->y0 = f->y1; f->y1 = f->y2;
		        f->y2 =   (f->x0 + f->x2) +   f->const0 * f->x1
		                     + (f->const1 * f->y0) + (f->const2 * f->y1);
			buf[i] = (short)f->y2;
		}
	}
}

#endif


/*
 Get the time for the machine's time zone
 Note: Asterisk requires a copy of localtime
 in the /etc directory for this to work properly.
 If /etc/localtime is not present, you will get
 GMT time! This is especially important on systems
 running embedded linux distributions as they don't usually
 have support for locales.

 If OLD_ASTERISK is defined, then the older localtime_r
 function will be used. The /etc/localtime file is not
 required in this case. This provides backward compatibility
 with Asterisk 1.2 systems.

*/

#ifdef	NEW_ASTERISK
static void rpt_localtime( time_t * t, struct ast_tm *lt)
{
struct timeval tv;

	tv.tv_sec = *t;
	tv.tv_usec = 0;
	ast_localtime(&tv, lt, NULL);

}

#else
static void rpt_localtime( time_t * t, struct tm *lt)
{
#ifdef OLD_ASTERISK
	localtime_r(t, lt);
#else
	ast_localtime(t, lt, NULL);
#endif
}
#endif


/* Retrieve an int from a config file */

static int retrieve_astcfgint(struct rpt *myrpt,char *category, char *name, int min, int max, int defl)
{
        char *var;
        int ret;
	char include_zero = 0;

	if(min < 0){ /* If min is negative, this means include 0 as a valid entry */
		min = -min;
		include_zero = 1;
	}

        var = (char *) ast_variable_retrieve(myrpt->cfg, category, name);
        if(var){
                ret = myatoi(var);
		if(include_zero && !ret)
			return 0;
                if(ret < min)
                        ret = min;
                if(ret > max)
                        ret = max;
        }
        else
                ret = defl;
        return ret;
}


static void load_rpt_vars(int n,int init)
{
char *this,*val;
int	i,j,longestnode;
struct ast_variable *vp;
struct ast_config *cfg;
char *strs[100];
char s1[256];
static char *cs_keywords[] = {"rptena","rptdis","apena","apdis","lnkena","lnkdis","totena","totdis","skena","skdis",
				"ufena","ufdis","atena","atdis","noice","noicd",NULL};

	if (option_verbose > 2)
		ast_verbose(VERBOSE_PREFIX_3 "%s config for repeater %s\n",
			(init) ? "Loading initial" : "Re-Loading",rpt_vars[n].name);
	ast_mutex_lock(&rpt_vars[n].lock);
	if (rpt_vars[n].cfg) ast_config_destroy(rpt_vars[n].cfg);
#ifdef	NEW_ASTERISK
	cfg = ast_config_load("rpt.conf",config_flags);
#else
	cfg = ast_config_load("rpt.conf");
#endif
	if (!cfg) {
		ast_mutex_unlock(&rpt_vars[n].lock);
 		ast_log(LOG_NOTICE, "Unable to open radio repeater configuration rpt.conf.  Radio Repeater disabled.\n");
		pthread_exit(NULL);
	}
	rpt_vars[n].cfg = cfg;
	this = rpt_vars[n].name;
 	memset(&rpt_vars[n].p,0,sizeof(rpt_vars[n].p));
	if (init)
	{
		char *cp;
		int savearea = (char *)&rpt_vars[n].p - (char *)&rpt_vars[n];

		cp = (char *) &rpt_vars[n].p;
		memset(cp + sizeof(rpt_vars[n].p),0,
			sizeof(rpt_vars[n]) - (sizeof(rpt_vars[n].p) + savearea));
		rpt_vars[n].tele.next = &rpt_vars[n].tele;
		rpt_vars[n].tele.prev = &rpt_vars[n].tele;
		rpt_vars[n].rpt_thread = AST_PTHREADT_NULL;
		rpt_vars[n].tailmessagen = 0;
	}
#ifdef	__RPT_NOTCH
	/* zot out filters stuff */
	memset(&rpt_vars[n].filters,0,sizeof(rpt_vars[n].filters));
#endif

    val = (char *) ast_variable_retrieve(cfg,this,"debuglevel");
	if (val) debug = atoi(val);

	val = (char *) ast_variable_retrieve(cfg,this,"context");
	if (val) rpt_vars[n].p.ourcontext = val;
	else rpt_vars[n].p.ourcontext = this;
	val = (char *) ast_variable_retrieve(cfg,this,"callerid");
	if (val) rpt_vars[n].p.ourcallerid = val;
	val = (char *) ast_variable_retrieve(cfg,this,"accountcode");
	if (val) rpt_vars[n].p.acctcode = val;
	val = (char *) ast_variable_retrieve(cfg,this,"idrecording");
	if (val) rpt_vars[n].p.ident = val;
	val = (char *) ast_variable_retrieve(cfg,this,"hangtime");
	if (val) rpt_vars[n].p.hangtime = atoi(val);
		else rpt_vars[n].p.hangtime = HANGTIME;
	val = (char *) ast_variable_retrieve(cfg,this,"althangtime");
	if (val) rpt_vars[n].p.althangtime = atoi(val);
		else rpt_vars[n].p.althangtime = HANGTIME;
	val = (char *) ast_variable_retrieve(cfg,this,"totime");
	if (val) rpt_vars[n].p.totime = atoi(val);
		else rpt_vars[n].p.totime = TOTIME;
	val = (char *) ast_variable_retrieve(cfg,this,"voxtimeout");
	if (val) rpt_vars[n].p.voxtimeout_ms = atoi(val);
		else rpt_vars[n].p.voxtimeout_ms = VOX_TIMEOUT_MS;

	val = (char *) ast_variable_retrieve(cfg,this,"voxrecover");
	if (val)
		rpt_vars[n].p.voxrecover_ms = atoi(val);
	else
		rpt_vars[n].p.voxrecover_ms = VOX_RECOVER_MS;

	val = (char *) ast_variable_retrieve(cfg,this,"voxmaxth");
	if (val)
		rpt_vars[n].p.voxmaxth = atoi(val);
	else
		rpt_vars[n].p.voxmaxth = VOX_MAX_THRESHOLD;

	val = (char *) ast_variable_retrieve(cfg,this,"voxminth");
	if (val)
		rpt_vars[n].p.voxminth = atoi(val);
	else
		rpt_vars[n].p.voxminth = VOX_MIN_THRESHOLD;

	val = (char *) ast_variable_retrieve(cfg,this,"simplexpatchdelay");
	if (val) rpt_vars[n].p.simplexpatchdelay = atoi(val);
		else rpt_vars[n].p.simplexpatchdelay = SIMPLEX_PATCH_DELAY;
	val = (char *) ast_variable_retrieve(cfg,this,"simplexphonedelay");
	if (val) rpt_vars[n].p.simplexphonedelay = atoi(val);
		else rpt_vars[n].p.simplexphonedelay = SIMPLEX_PHONE_DELAY;
	val = (char *) ast_variable_retrieve(cfg,this,"statpost_program");
	if (val) rpt_vars[n].p.statpost_program = val;
		else rpt_vars[n].p.statpost_program = STATPOST_PROGRAM;
	rpt_vars[n].p.statpost_url =
		(char *) ast_variable_retrieve(cfg,this,"statpost_url");
	rpt_vars[n].p.tailmessagetime = retrieve_astcfgint(&rpt_vars[n],this, "tailmessagetime", 0, 2400000, 0);
	rpt_vars[n].p.tailsquashedtime = retrieve_astcfgint(&rpt_vars[n],this, "tailsquashedtime", 0, 2400000, 0);
	rpt_vars[n].p.duplex = retrieve_astcfgint(&rpt_vars[n],this,"duplex",0,4,2);
	rpt_vars[n].p.idtime = retrieve_astcfgint(&rpt_vars[n],this, "idtime", -60000, 2400000, IDTIME);	/* Enforce a min max including zero */
	rpt_vars[n].p.politeid = retrieve_astcfgint(&rpt_vars[n],this, "politeid", 30000, 300000, POLITEID); /* Enforce a min max */
	val = (char *) ast_variable_retrieve(cfg,this,"tonezone");
	if (val) rpt_vars[n].p.tonezone = val;
	rpt_vars[n].p.tailmessages[0] = 0;
	rpt_vars[n].p.tailmessagemax = 0;
	val = (char *) ast_variable_retrieve(cfg,this,"tailmessagelist");
	if (val) rpt_vars[n].p.tailmessagemax = finddelim(val, rpt_vars[n].p.tailmessages, 500);
	val = (char *) ast_variable_retrieve(cfg,this,"memory");
	if (!val) val = MEMORY;
	rpt_vars[n].p.memory = val;
	val = (char *) ast_variable_retrieve(cfg,this,"macro");
	if (!val) val = MACRO;
	rpt_vars[n].p.macro = val;
	val = (char *) ast_variable_retrieve(cfg,this,"tonemacro");
	if (!val) val = TONEMACRO;
	rpt_vars[n].p.tonemacro = val;
	val = (char *) ast_variable_retrieve(cfg,this,"startup_macro");
	if (val) rpt_vars[n].p.startupmacro = val;
	val = (char *) ast_variable_retrieve(cfg,this,"iobase");
	/* do not use atoi() here, we need to be able to have
		the input specified in hex or decimal so we use
		sscanf with a %i */
	if ((!val) || (sscanf(val,"%i",&rpt_vars[n].p.iobase) != 1))
		rpt_vars[n].p.iobase = DEFAULT_IOBASE;
	val = (char *) ast_variable_retrieve(cfg,this,"ioport");
	rpt_vars[n].p.ioport = val;
	val = (char *) ast_variable_retrieve(cfg,this,"functions");
	if (!val)
		{
			val = FUNCTIONS;
			rpt_vars[n].p.simple = 1;
		}
	rpt_vars[n].p.functions = val;
	val =  (char *) ast_variable_retrieve(cfg,this,"link_functions");
	if (val) rpt_vars[n].p.link_functions = val;
	else
		rpt_vars[n].p.link_functions = rpt_vars[n].p.functions;
	val = (char *) ast_variable_retrieve(cfg,this,"phone_functions");
	if (val) rpt_vars[n].p.phone_functions = val;
	val = (char *) ast_variable_retrieve(cfg,this,"dphone_functions");
	if (val) rpt_vars[n].p.dphone_functions = val;
	val = (char *) ast_variable_retrieve(cfg,this,"alt_functions");
	if (val) rpt_vars[n].p.alt_functions = val;
	val = (char *) ast_variable_retrieve(cfg,this,"funcchar");
	if (!val) rpt_vars[n].p.funcchar = FUNCCHAR; else
		rpt_vars[n].p.funcchar = *val;
	val = (char *) ast_variable_retrieve(cfg,this,"endchar");
	if (!val) rpt_vars[n].p.endchar = ENDCHAR; else
		rpt_vars[n].p.endchar = *val;
	val = (char *) ast_variable_retrieve(cfg,this,"nobusyout");
	if (val) rpt_vars[n].p.nobusyout = ast_true(val);
	val = (char *) ast_variable_retrieve(cfg,this,"notelemtx");
	if (val) rpt_vars[n].p.notelemtx = ast_true(val);
	val = (char *) ast_variable_retrieve(cfg,this,"propagate_dtmf");
	if (val) rpt_vars[n].p.propagate_dtmf = ast_true(val);
	val = (char *) ast_variable_retrieve(cfg,this,"propagate_phonedtmf");
	if (val) rpt_vars[n].p.propagate_phonedtmf = ast_true(val);
	val = (char *) ast_variable_retrieve(cfg,this,"linktolink");
	if (val) rpt_vars[n].p.linktolink = ast_true(val);
	val = (char *) ast_variable_retrieve(cfg,this,"nodes");
	if (!val) val = NODES;
	rpt_vars[n].p.nodes = val;
	val = (char *) ast_variable_retrieve(cfg,this,"extnodes");
	if (!val) val = EXTNODES;
	rpt_vars[n].p.extnodes = val;
	val = (char *) ast_variable_retrieve(cfg,this,"extnodefile");
	if (!val) val = EXTNODEFILE;
	rpt_vars[n].p.extnodefile = val;
	val = (char *) ast_variable_retrieve(cfg,this,"archivedir");
	if (val) rpt_vars[n].p.archivedir = val;
	val = (char *) ast_variable_retrieve(cfg,this,"authlevel");
	if (val) rpt_vars[n].p.authlevel = atoi(val);
	else rpt_vars[n].p.authlevel = 0;
	val = (char *) ast_variable_retrieve(cfg,this,"parrot");
	if (val) rpt_vars[n].p.parrotmode = ast_true(val) * 2;
	else rpt_vars[n].p.parrotmode = 0;
	val = (char *) ast_variable_retrieve(cfg,this,"parrottime");
	if (val) rpt_vars[n].p.parrottime = atoi(val);
	else rpt_vars[n].p.parrottime = PARROTTIME;
	val = (char *) ast_variable_retrieve(cfg,this,"rptnode");
	rpt_vars[n].p.rptnode = val;
	val = (char *) ast_variable_retrieve(cfg,this,"mars");
	if (val) rpt_vars[n].p.remote_mars = atoi(val);
	else rpt_vars[n].p.remote_mars = 0;
	val = (char *) ast_variable_retrieve(cfg,this,"monminblocks");
	if (val) rpt_vars[n].p.monminblocks = atol(val);
	else rpt_vars[n].p.monminblocks = DEFAULT_MONITOR_MIN_DISK_BLOCKS;
	val = (char *) ast_variable_retrieve(cfg,this,"remote_inact_timeout");
	if (val) rpt_vars[n].p.remoteinacttimeout = atoi(val);
	else rpt_vars[n].p.remoteinacttimeout = DEFAULT_REMOTE_INACT_TIMEOUT;
	val = (char *) ast_variable_retrieve(cfg,this,"civaddr");
	if (val) rpt_vars[n].p.civaddr = atoi(val);
	else rpt_vars[n].p.civaddr = DEFAULT_CIV_ADDR;
	val = (char *) ast_variable_retrieve(cfg,this,"remote_timeout");
	if (val) rpt_vars[n].p.remotetimeout = atoi(val);
	else rpt_vars[n].p.remotetimeout = DEFAULT_REMOTE_TIMEOUT;
	val = (char *) ast_variable_retrieve(cfg,this,"remote_timeout_warning");
	if (val) rpt_vars[n].p.remotetimeoutwarning = atoi(val);
	else rpt_vars[n].p.remotetimeoutwarning = DEFAULT_REMOTE_TIMEOUT_WARNING;
	val = (char *) ast_variable_retrieve(cfg,this,"remote_timeout_warning_freq");
	if (val) rpt_vars[n].p.remotetimeoutwarningfreq = atoi(val);
	else rpt_vars[n].p.remotetimeoutwarningfreq = DEFAULT_REMOTE_TIMEOUT_WARNING_FREQ;
	val = (char *) ast_variable_retrieve(cfg,this,"ncomnum");
	rpt_vars[n].p.ncomnum = val;
	val = (char *) ast_variable_retrieve(cfg,this,"erxgain");
	if (!val) val = DEFAULT_ERXGAIN;
	rpt_vars[n].p.erxgain = pow(10.0,atof(val) / 20.0);
	val = (char *) ast_variable_retrieve(cfg,this,"etxgain");
	if (!val) val = DEFAULT_ETXGAIN;
	rpt_vars[n].p.etxgain = pow(10.0,atof(val) / 20.0);
	val = (char *) ast_variable_retrieve(cfg,this,"irxgain");
	if (!val) val = DEFAULT_IRXGAIN;
	rpt_vars[n].p.irxgain = pow(10.0,atof(val) / 20.0);
	val = (char *) ast_variable_retrieve(cfg,this,"itxgain");
	if (!val) val = DEFAULT_ITXGAIN;
	rpt_vars[n].p.itxgain = pow(10.0,atof(val) / 20.0);
	val = (char *) ast_variable_retrieve(cfg,this,"irlpann");
	if (val) rpt_vars[n].p.irlpann = ast_true(val);
	else rpt_vars[n].p.irlpann = DEFAULT_IRLPANN;
	val = (char *) ast_variable_retrieve(cfg,this,"eannmode");
	if (val) rpt_vars[n].p.eannmode = atoi(val);
	else rpt_vars[n].p.eannmode = DEFAULT_EANNMODE;
	if (rpt_vars[n].p.eannmode < 1) rpt_vars[n].p.eannmode = 1;
	if (rpt_vars[n].p.eannmode > 3) rpt_vars[n].p.eannmode = 3;
	val = (char *) ast_variable_retrieve(cfg,this,"discpgm");
	rpt_vars[n].p.discpgm = val;
	val = (char *) ast_variable_retrieve(cfg,this,"connpgm");
	rpt_vars[n].p.connpgm = val;
	val = (char *) ast_variable_retrieve(cfg,this,"lnkactenable");
	if (val) rpt_vars[n].p.lnkactenable = ast_true(val);
	rpt_vars[n].p.lnkacttime = retrieve_astcfgint(&rpt_vars[n],this, "lnkacttime", -120, 90000, 0);	/* Enforce a min max including zero */
	val = (char *) ast_variable_retrieve(cfg, this, "lnkactmacro");
	rpt_vars[n].p.lnkactmacro = val;
	val = (char *) ast_variable_retrieve(cfg, this, "lnkacttimerwarn");
	rpt_vars[n].p.lnkacttimerwarn = val;
	val = (char *) ast_variable_retrieve(cfg, this, "nolocallinkct");
	rpt_vars[n].p.nolocallinkct = ast_true(val);
	val = (char *) ast_variable_retrieve(cfg, this, "nounkeyct");
	rpt_vars[n].p.nounkeyct = ast_true(val);
	val = (char *) ast_variable_retrieve(cfg, this, "holdofftelem");
	rpt_vars[n].p.holdofftelem = ast_true(val);
	val = (char *) ast_variable_retrieve(cfg, this, "beaconing");
	rpt_vars[n].p.beaconing = ast_true(val);
	val = (char *) ast_variable_retrieve(cfg,this,"rxburstfreq");
	if (val) rpt_vars[n].p.rxburstfreq = atoi(val);
	else rpt_vars[n].p.rxburstfreq = 0;
	val = (char *) ast_variable_retrieve(cfg,this,"rxbursttime");
	if (val) rpt_vars[n].p.rxbursttime = atoi(val);
	else rpt_vars[n].p.rxbursttime = DEFAULT_RXBURST_TIME;
	val = (char *) ast_variable_retrieve(cfg,this,"rxburstthreshold");
	if (val) rpt_vars[n].p.rxburstthreshold = atoi(val);
	else rpt_vars[n].p.rxburstthreshold = DEFAULT_RXBURST_THRESHOLD;
	val = (char *) ast_variable_retrieve(cfg, this, "nonodelist");
	rpt_vars[n].p.nonodelist = ast_true(val);
	val = (char *) ast_variable_retrieve(cfg,this,"itxctcss");
	if (val) rpt_vars[n].p.itxctcss = ast_true(val);

#ifdef	__RPT_NOTCH
	val = (char *) ast_variable_retrieve(cfg,this,"rxnotch");
	if (val) {
		i = finddelim(val,strs,MAXFILTERS * 2);
		i &= ~1; /* force an even number, rounded down */
		if (i >= 2) for(j = 0; j < i; j += 2)
		{
			rpt_mknotch(atof(strs[j]),atof(strs[j + 1]),
			  &rpt_vars[n].filters[j >> 1].gain,
			    &rpt_vars[n].filters[j >> 1].const0,
				&rpt_vars[n].filters[j >> 1].const1,
				    &rpt_vars[n].filters[j >> 1].const2);
			sprintf(rpt_vars[n].filters[j >> 1].desc,"%s Hz, BW = %s",
				strs[j],strs[j + 1]);
		}
	}
#endif
	val = (char *) ast_variable_retrieve(cfg,this,"votertype");
	if (!val) val = "0";
	rpt_vars[n].p.votertype=atoi(val);

	val = (char *) ast_variable_retrieve(cfg,this,"votermode");
	if (!val) val = "0";
	rpt_vars[n].p.votermode=atoi(val);

	val = (char *) ast_variable_retrieve(cfg,this,"votermargin");
	if (!val) val = "10";
	rpt_vars[n].p.votermargin=atoi(val);

	val = (char *) ast_variable_retrieve(cfg,this,"telemnomdb");
	if (!val) val = "0";
	rpt_vars[n].p.telemnomgain = pow(10.0,atof(val) / 20.0);
	val = (char *) ast_variable_retrieve(cfg,this,"telemduckdb");
	if (!val) val = DEFAULT_TELEMDUCKDB;
	rpt_vars[n].p.telemduckgain = pow(10.0,atof(val) / 20.0);
	val = (char *) ast_variable_retrieve(cfg,this,"telemdefault");
	if (val) rpt_vars[n].p.telemdefault = atoi(val);
	else rpt_vars[n].p.telemdefault = DEFAULT_RPT_TELEMDEFAULT;
	val = (char *) ast_variable_retrieve(cfg,this,"telemdynamic");
	if (val) rpt_vars[n].p.telemdynamic = ast_true(val);
	else rpt_vars[n].p.telemdynamic = DEFAULT_RPT_TELEMDYNAMIC;
	if (!rpt_vars[n].p.telemdefault)
		rpt_vars[n].telemmode = 0;
	else if (rpt_vars[n].p.telemdefault == 2)
		rpt_vars[n].telemmode = 1;
	else rpt_vars[n].telemmode = 0x7fffffff;

	val = (char *) ast_variable_retrieve(cfg,this,"guilinkdefault");
	if (val) rpt_vars[n].p.linkmode[LINKMODE_GUI] = atoi(val) + 4;
	else rpt_vars[n].p.linkmode[LINKMODE_GUI] = DEFAULT_GUI_LINK_MODE;
	val = (char *) ast_variable_retrieve(cfg,this,"guilinkdynamic");
	if (val) rpt_vars[n].p.linkmodedynamic[LINKMODE_GUI] = ast_true(val);
	else rpt_vars[n].p.linkmodedynamic[LINKMODE_GUI] = DEFAULT_GUI_LINK_MODE_DYNAMIC;

	val = (char *) ast_variable_retrieve(cfg,this,"phonelinkdefault");
	if (val) rpt_vars[n].p.linkmode[LINKMODE_PHONE] = atoi(val) + 4;
	else rpt_vars[n].p.linkmode[LINKMODE_PHONE] = DEFAULT_PHONE_LINK_MODE;
	val = (char *) ast_variable_retrieve(cfg,this,"phonelinkdynamic");
	if (val) rpt_vars[n].p.linkmodedynamic[LINKMODE_PHONE] = ast_true(val);
	else rpt_vars[n].p.linkmodedynamic[LINKMODE_PHONE] = DEFAULT_PHONE_LINK_MODE_DYNAMIC;

	val = (char *) ast_variable_retrieve(cfg,this,"echolinkdefault");
	if (val) rpt_vars[n].p.linkmode[LINKMODE_ECHOLINK] = atoi(val) + 4;
	else rpt_vars[n].p.linkmode[LINKMODE_ECHOLINK] = DEFAULT_ECHOLINK_LINK_MODE;
	val = (char *) ast_variable_retrieve(cfg,this,"echolinkdynamic");
	if (val) rpt_vars[n].p.linkmodedynamic[LINKMODE_ECHOLINK] = ast_true(val);
	else rpt_vars[n].p.linkmodedynamic[LINKMODE_ECHOLINK] = DEFAULT_ECHOLINK_LINK_MODE_DYNAMIC;

	val = (char *) ast_variable_retrieve(cfg,this,"irlplinkdefault");
	if (val) rpt_vars[n].p.linkmode[LINKMODE_IRLP] = atoi(val) + 4;
	else rpt_vars[n].p.linkmode[LINKMODE_IRLP] = DEFAULT_IRLP_LINK_MODE;
	val = (char *) ast_variable_retrieve(cfg,this,"irlplinkdynamic");
	if (val) rpt_vars[n].p.linkmodedynamic[LINKMODE_IRLP] = ast_true(val);
	else rpt_vars[n].p.linkmodedynamic[LINKMODE_IRLP] = DEFAULT_IRLP_LINK_MODE_DYNAMIC;

	val = (char *) ast_variable_retrieve(cfg,this,"locallist");
	if (val) {
		memset(rpt_vars[n].p.locallist,0,sizeof(rpt_vars[n].p.locallist));
		rpt_vars[n].p.nlocallist = finddelim(val,rpt_vars[n].p.locallist,16);
	}

	val = (char *) ast_variable_retrieve(cfg,this,"codemasters");
	if (val) {
		memset(rpt_vars[n].p.codemaster,0,sizeof(rpt_vars[n].p.codemaster));
		rpt_vars[n].p.ncodemasters = finddelim(val,rpt_vars[n].p.codemaster,16);
	}

	val = (char *) ast_variable_retrieve(cfg,this,"inxlat");
	if (val) {
		memset(&rpt_vars[n].p.inxlat,0,sizeof(struct rpt_xlat));
		i = finddelim(val,strs,3);
		if (i) strncpy(rpt_vars[n].p.inxlat.funccharseq,strs[0],MAXXLAT - 1);
		if (i > 1) strncpy(rpt_vars[n].p.inxlat.endcharseq,strs[1],MAXXLAT - 1);
		if (i > 2) strncpy(rpt_vars[n].p.inxlat.passchars,strs[2],MAXXLAT - 1);
	}
	val = (char *) ast_variable_retrieve(cfg,this,"outxlat");
	if (val) {
		memset(&rpt_vars[n].p.outxlat,0,sizeof(struct rpt_xlat));
		i = finddelim(val,strs,3);
		if (i) strncpy(rpt_vars[n].p.outxlat.funccharseq,strs[0],MAXXLAT - 1);
		if (i > 1) strncpy(rpt_vars[n].p.outxlat.endcharseq,strs[1],MAXXLAT - 1);
		if (i > 2) strncpy(rpt_vars[n].p.outxlat.passchars,strs[2],MAXXLAT - 1);
	}
	/* retreive the stanza name for the control states if there is one */
	val = (char *) ast_variable_retrieve(cfg,this,"controlstates");
	rpt_vars[n].p.csstanzaname = val;

	/* retreive the stanza name for the scheduler if there is one */
	val = (char *) ast_variable_retrieve(cfg,this,"scheduler");
	rpt_vars[n].p.skedstanzaname = val;

	/* retreive the stanza name for the txlimits */
	val = (char *) ast_variable_retrieve(cfg,this,"txlimits");
	rpt_vars[n].p.txlimitsstanzaname = val;

	longestnode = 0;

	vp = ast_variable_browse(cfg, rpt_vars[n].p.nodes);

	while(vp){
		j = strlen(vp->name);
		if (j > longestnode)
			longestnode = j;
		vp = vp->next;
	}

	rpt_vars[n].longestnode = longestnode;

	/*
	* For this repeater, Determine the length of the longest function
	*/
	rpt_vars[n].longestfunc = 0;
	vp = ast_variable_browse(cfg, rpt_vars[n].p.functions);
	while(vp){
		j = strlen(vp->name);
		if (j > rpt_vars[n].longestfunc)
			rpt_vars[n].longestfunc = j;
		vp = vp->next;
	}
	/*
	* For this repeater, Determine the length of the longest function
	*/
	rpt_vars[n].link_longestfunc = 0;
	vp = ast_variable_browse(cfg, rpt_vars[n].p.link_functions);
	while(vp){
		j = strlen(vp->name);
		if (j > rpt_vars[n].link_longestfunc)
			rpt_vars[n].link_longestfunc = j;
		vp = vp->next;
	}
	rpt_vars[n].phone_longestfunc = 0;
	if (rpt_vars[n].p.phone_functions)
	{
		vp = ast_variable_browse(cfg, rpt_vars[n].p.phone_functions);
		while(vp){
			j = strlen(vp->name);
			if (j > rpt_vars[n].phone_longestfunc)
				rpt_vars[n].phone_longestfunc = j;
			vp = vp->next;
		}
	}
	rpt_vars[n].dphone_longestfunc = 0;
	if (rpt_vars[n].p.dphone_functions)
	{
		vp = ast_variable_browse(cfg, rpt_vars[n].p.dphone_functions);
		while(vp){
			j = strlen(vp->name);
			if (j > rpt_vars[n].dphone_longestfunc)
				rpt_vars[n].dphone_longestfunc = j;
			vp = vp->next;
		}
	}
	rpt_vars[n].alt_longestfunc = 0;
	if (rpt_vars[n].p.alt_functions)
	{
		vp = ast_variable_browse(cfg, rpt_vars[n].p.alt_functions);
		while(vp){
			j = strlen(vp->name);
			if (j > rpt_vars[n].alt_longestfunc)
				rpt_vars[n].alt_longestfunc = j;
			vp = vp->next;
		}
	}
	rpt_vars[n].macro_longest = 1;
	vp = ast_variable_browse(cfg, rpt_vars[n].p.macro);
	while(vp){
		j = strlen(vp->name);
		if (j > rpt_vars[n].macro_longest)
			rpt_vars[n].macro_longest = j;
		vp = vp->next;
	}

	/* Browse for control states */
	if(rpt_vars[n].p.csstanzaname)
		vp = ast_variable_browse(cfg, rpt_vars[n].p.csstanzaname);
	else
		vp = NULL;
	for( i = 0 ; vp && (i < MAX_SYSSTATES) ; i++){ /* Iterate over the number of control state lines in the stanza */
		int k,nukw,statenum;
		statenum=atoi(vp->name);
		strncpy(s1, vp->value, 255);
		s1[255] = 0;
		nukw  = finddelim(s1,strs,32);

		for (k = 0 ; k < nukw ; k++){ /* for each user specified keyword */
			for(j = 0 ; cs_keywords[j] != NULL ; j++){ /* try to match to one in our internal table */
				if(!strcmp(strs[k],cs_keywords[j])){
					switch(j){
						case 0: /* rptena */
							rpt_vars[n].p.s[statenum].txdisable = 0;
							break;
						case 1: /* rptdis */
							rpt_vars[n].p.s[statenum].txdisable = 1;
							break;

						case 2: /* apena */
							rpt_vars[n].p.s[statenum].autopatchdisable = 0;
							break;

						case 3: /* apdis */
							rpt_vars[n].p.s[statenum].autopatchdisable = 1;
							break;

						case 4: /* lnkena */
							rpt_vars[n].p.s[statenum].linkfundisable = 0;
							break;

						case 5: /* lnkdis */
							rpt_vars[n].p.s[statenum].linkfundisable = 1;
							break;

						case 6: /* totena */
							rpt_vars[n].p.s[statenum].totdisable = 0;
							break;

						case 7: /* totdis */
							rpt_vars[n].p.s[statenum].totdisable = 1;
							break;

						case 8: /* skena */
							rpt_vars[n].p.s[statenum].schedulerdisable = 0;
							break;

						case 9: /* skdis */
							rpt_vars[n].p.s[statenum].schedulerdisable = 1;
							break;

						case 10: /* ufena */
							rpt_vars[n].p.s[statenum].userfundisable = 0;
							break;

						case 11: /* ufdis */
							rpt_vars[n].p.s[statenum].userfundisable = 1;
							break;

						case 12: /* atena */
							rpt_vars[n].p.s[statenum].alternatetail = 1;
							break;

						case 13: /* atdis */
							rpt_vars[n].p.s[statenum].alternatetail = 0;
							break;

						case 14: /* noice */
							rpt_vars[n].p.s[statenum].noincomingconns = 1;
							break;

						case 15: /* noicd */
							rpt_vars[n].p.s[statenum].noincomingconns = 0;
							break;

						default:
							ast_log(LOG_WARNING,
								"Unhandled control state keyword %s", cs_keywords[i]);
							break;
					}
				}
			}
		}
		vp = vp->next;
	}
	ast_mutex_unlock(&rpt_vars[n].lock);
}

/*
* Enable or disable debug output at a given level at the console
*/

static int rpt_do_debug(int fd, int argc, char *argv[])
{
	int newlevel;

        if (argc != 4)
                return RESULT_SHOWUSAGE;
        newlevel = myatoi(argv[3]);
        if((newlevel < 0) || (newlevel > 7))
                return RESULT_SHOWUSAGE;
        if(newlevel)
                ast_cli(fd, "app_rpt Debugging enabled, previous level: %d, new level: %d\n", debug, newlevel);
        else
                ast_cli(fd, "app_rpt Debugging disabled\n");

        debug = newlevel;
        return RESULT_SUCCESS;
}

/*
* Dump rpt struct debugging onto console
*/

static int rpt_do_dump(int fd, int argc, char *argv[])
{
	int i;

        if (argc != 3)
                return RESULT_SHOWUSAGE;

	for(i = 0; i < nrpts; i++)
	{
		if (!strcmp(argv[2],rpt_vars[i].name))
		{
			rpt_vars[i].disgorgetime = time(NULL) + 1; /* Do it 1 seconds later */
		        ast_cli(fd, "app_rpt struct dump requested for node %s\n",argv[2]);
		        return RESULT_SUCCESS;
		}
	}
	if(debug) ast_log(LOG_WARNING,"target node %s not found.\n",argv[2]);

	return RESULT_FAILURE;
}

/*
* Dump statistics onto console
*/

static int rpt_do_stats(int fd, int argc, char *argv[])
{
	int i,j,numoflinks;
	int dailytxtime, dailykerchunks;
	time_t now;
	int totalkerchunks, dailykeyups, totalkeyups, timeouts;
	int totalexecdcommands, dailyexecdcommands, hours, minutes, seconds;
	int uptime;
	long long totaltxtime;
	struct	rpt_link *l;
	char *listoflinks[MAX_STAT_LINKS];
	char *lastdtmfcommand,*parrot_ena;
	char *tot_state, *ider_state, *patch_state;
	char *reverse_patch_state, *sys_ena, *tot_ena, *link_ena, *patch_ena;
	char *sch_ena, *input_signal, *called_number, *user_funs, *tail_type;
	char *iconns;
	struct rpt *myrpt;

	static char *not_applicable = "N/A";

	if(argc != 3)
		return RESULT_SHOWUSAGE;

	tot_state = ider_state =
	patch_state = reverse_patch_state =
	input_signal = not_applicable;
	called_number = lastdtmfcommand = NULL;

	time(&now);
	for(i = 0; i < nrpts; i++)
	{
		if (!strcmp(argv[2],rpt_vars[i].name)){
			/* Make a copy of all stat variables while locked */
			myrpt = &rpt_vars[i];
			rpt_mutex_lock(&myrpt->lock); /* LOCK */
			uptime = (int)(now - starttime);
			dailytxtime = myrpt->dailytxtime;
			totaltxtime = myrpt->totaltxtime;
			dailykeyups = myrpt->dailykeyups;
			totalkeyups = myrpt->totalkeyups;
			dailykerchunks = myrpt->dailykerchunks;
			totalkerchunks = myrpt->totalkerchunks;
			dailyexecdcommands = myrpt->dailyexecdcommands;
			totalexecdcommands = myrpt->totalexecdcommands;
			timeouts = myrpt->timeouts;

			/* Traverse the list of connected nodes */
			reverse_patch_state = "DOWN";
			numoflinks = 0;
			l = myrpt->links.next;
			while(l && (l != &myrpt->links)){
				if(numoflinks >= MAX_STAT_LINKS){
					ast_log(LOG_NOTICE,
					"maximum number of links exceeds %d in rpt_do_stats()!",MAX_STAT_LINKS);
					break;
				}
				if (l->name[0] == '0'){ /* Skip '0' nodes */
					reverse_patch_state = "UP";
					l = l->next;
					continue;
				}
				listoflinks[numoflinks] = ast_strdup(l->name);
				if(listoflinks[numoflinks] == NULL){
					break;
				}
				else{
					numoflinks++;
				}
				l = l->next;
			}

			if(myrpt->keyed)
				input_signal = "YES";
			else
				input_signal = "NO";

			if(myrpt->p.parrotmode)
				parrot_ena = "ENABLED";
			else
				parrot_ena = "DISABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].txdisable)
				sys_ena = "DISABLED";
			else
				sys_ena = "ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].totdisable)
				tot_ena = "DISABLED";
			else
				tot_ena = "ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].linkfundisable)
				link_ena = "DISABLED";
			else
				link_ena = "ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].autopatchdisable)
				patch_ena = "DISABLED";
			else
				patch_ena = "ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].schedulerdisable)
				sch_ena = "DISABLED";
			else
				sch_ena = "ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].userfundisable)
				user_funs = "DISABLED";
			else
				user_funs = "ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].alternatetail)
				tail_type = "ALTERNATE";
			else
				tail_type = "STANDARD";

			if(myrpt->p.s[myrpt->p.sysstate_cur].noincomingconns)
				iconns = "DISABLED";
			else
				iconns = "ENABLED";

			if(!myrpt->totimer)
				tot_state = "TIMED OUT!";
			else if(myrpt->totimer != myrpt->p.totime)
				tot_state = "ARMED";
			else
				tot_state = "RESET";

			if(myrpt->tailid)
				ider_state = "QUEUED IN TAIL";
			else if(myrpt->mustid)
				ider_state = "QUEUED FOR CLEANUP";
			else
				ider_state = "CLEAN";

			switch(myrpt->callmode){
				case 1:
					patch_state = "DIALING";
					break;
				case 2:
					patch_state = "CONNECTING";
					break;
				case 3:
					patch_state = "UP";
					break;

				case 4:
					patch_state = "CALL FAILED";
					break;

				default:
					patch_state = "DOWN";
			}

			if(strlen(myrpt->exten)){
				called_number = ast_strdup(myrpt->exten);
			}

			if(strlen(myrpt->lastdtmfcommand)){
				lastdtmfcommand = ast_strdup(myrpt->lastdtmfcommand);
			}
			rpt_mutex_unlock(&myrpt->lock); /* UNLOCK */

			ast_cli(fd, "************************ NODE %s STATISTICS *************************\n\n", myrpt->name);
			ast_cli(fd, "Selected system state............................: %d\n", myrpt->p.sysstate_cur);
			ast_cli(fd, "Signal on input..................................: %s\n", input_signal);
			ast_cli(fd, "System...........................................: %s\n", sys_ena);
			ast_cli(fd, "Parrot Mode......................................: %s\n", parrot_ena);
			ast_cli(fd, "Scheduler........................................: %s\n", sch_ena);
			ast_cli(fd, "Tail Time........................................: %s\n", tail_type);
			ast_cli(fd, "Time out timer...................................: %s\n", tot_ena);
			ast_cli(fd, "Incoming connections.............................: %s\n", iconns);
			ast_cli(fd, "Time out timer state.............................: %s\n", tot_state);
			ast_cli(fd, "Time outs since system initialization............: %d\n", timeouts);
			ast_cli(fd, "Identifier state.................................: %s\n", ider_state);
			ast_cli(fd, "Kerchunks today..................................: %d\n", dailykerchunks);
			ast_cli(fd, "Kerchunks since system initialization............: %d\n", totalkerchunks);
			ast_cli(fd, "Keyups today.....................................: %d\n", dailykeyups);
			ast_cli(fd, "Keyups since system initialization...............: %d\n", totalkeyups);
			ast_cli(fd, "DTMF commands today..............................: %d\n", dailyexecdcommands);
			ast_cli(fd, "DTMF commands since system initialization........: %d\n", totalexecdcommands);
			ast_cli(fd, "Last DTMF command executed.......................: %s\n",

			(lastdtmfcommand && strlen(lastdtmfcommand)) ? lastdtmfcommand : not_applicable);
			hours = dailytxtime/3600000;
			dailytxtime %= 3600000;
			minutes = dailytxtime/60000;
			dailytxtime %= 60000;
			seconds = dailytxtime/1000;
			dailytxtime %= 1000;

			ast_cli(fd, "TX time today....................................: %02d:%02d:%02d.%d\n",
				hours, minutes, seconds, dailytxtime);

			hours = (int) totaltxtime/3600000;
			totaltxtime %= 3600000;
			minutes = (int) totaltxtime/60000;
			totaltxtime %= 60000;
			seconds = (int)  totaltxtime/1000;
			totaltxtime %= 1000;

			ast_cli(fd, "TX time since system initialization..............: %02d:%02d:%02d.%d\n",
				 hours, minutes, seconds, (int) totaltxtime);

                       	hours = uptime/3600;
                        uptime %= 3600;
                        minutes = uptime/60;
                        uptime %= 60;

                        ast_cli(fd, "Uptime...........................................: %02d:%02d:%02d\n",
                                hours, minutes, uptime);

			ast_cli(fd, "Nodes currently connected to us..................: ");
                        if(!numoflinks){
  	                      ast_cli(fd,"<NONE>");
                        }
			else{
				for(j = 0 ;j < numoflinks; j++){
					ast_cli(fd, "%s", listoflinks[j]);
					if(j % 4 == 3){
						ast_cli(fd, "\n");
						ast_cli(fd, "                                                 : ");
					}
					else{
						if((numoflinks - 1) - j  > 0)
							ast_cli(fd, ", ");
					}
				}
			}
			ast_cli(fd,"\n");

			ast_cli(fd, "Autopatch........................................: %s\n", patch_ena);
			ast_cli(fd, "Autopatch state..................................: %s\n", patch_state);
			ast_cli(fd, "Autopatch called number..........................: %s\n",
			(called_number && strlen(called_number)) ? called_number : not_applicable);
			ast_cli(fd, "Reverse patch/IAXRPT connected...................: %s\n", reverse_patch_state);
			ast_cli(fd, "User linking commands............................: %s\n", link_ena);
			ast_cli(fd, "User functions...................................: %s\n", user_funs);
			ast_cli(fd, "Coded Squelch Received...........................: %s\n", myrpt->csrxnow);
			for(j = 0; j < myrpt->p.ncodemasters; j++){
				ast_cli(fd, "Code Master.............................. %d: %s\n", j,myrpt->p.codemaster[j]);
			}

			for(j = 0; j < numoflinks; j++){ /* ast_free() all link names */
				ast_free(listoflinks[j]);
			}
			if(called_number){
				ast_free(called_number);
			}
			if(lastdtmfcommand){
				ast_free(lastdtmfcommand);
			}
		        return RESULT_SUCCESS;
		}
	}
	return RESULT_FAILURE;
}

/*
* Link stats function
*/

static int rpt_do_lstats(int fd, int argc, char *argv[])
{
	int i,j;
	char *connstate;
	struct rpt *myrpt;
	struct rpt_link *l;
	struct rpt_lstat *s,*t;
	struct rpt_lstat s_head;
	if(argc != 3)
		return RESULT_SHOWUSAGE;

	s = NULL;
	s_head.next = &s_head;
	s_head.prev = &s_head;

	for(i = 0; i < nrpts; i++)
	{
		if (!strcmp(argv[2],rpt_vars[i].name)){
			/* Make a copy of all stat variables while locked */
			myrpt = &rpt_vars[i];
			rpt_mutex_lock(&myrpt->lock); /* LOCK */
			/* Traverse the list of connected nodes */
			j = 0;
			l = myrpt->links.next;
			while(l && (l != &myrpt->links)){
				if (l->name[0] == '0'){ /* Skip '0' nodes */
					l = l->next;
					continue;
				}
				if((s = (struct rpt_lstat *) ast_malloc(sizeof(struct rpt_lstat))) == NULL){
					ast_log(LOG_ERROR, "Malloc failed in rpt_do_lstats\n");
					rpt_mutex_unlock(&myrpt->lock); /* UNLOCK */
					return RESULT_FAILURE;
				}
				memset(s, 0, sizeof(struct rpt_lstat));
				strncpy(s->name, l->name, MAXREMSTR - 1);
				if (l->chan) pbx_substitute_variables_helper(l->chan, "${IAXPEER(CURRENTCHANNEL)}", s->peer, MAXPEERSTR - 1);
				else strcpy(s->peer,"(none)");
				s->mode = l->mode;
				s->outbound = l->outbound;
				s->reconnects = l->reconnects;
				s->connecttime = l->connecttime;
				s->thisconnected = l->thisconnected;
				memcpy(s->chan_stat,l->chan_stat,NRPTSTAT * sizeof(struct rpt_chan_stat));
				insque((struct qelem *) s, (struct qelem *) s_head.next);
				memset(l->chan_stat,0,NRPTSTAT * sizeof(struct rpt_chan_stat));
				l = l->next;
			}
			rpt_mutex_unlock(&myrpt->lock); /* UNLOCK */
			ast_cli(fd, "NODE      PEER                RECONNECTS  DIRECTION  CONNECT TIME        CONNECT STATE\n");
			ast_cli(fd, "----      ----                ----------  ---------  ------------        -------------\n");

			for(s = s_head.next; s != &s_head; s = s->next){
				int hours, minutes, seconds;
				long long connecttime = s->connecttime;
				char conntime[21];
				hours = (int) connecttime/3600000;
				connecttime %= 3600000;
				minutes = (int) connecttime/60000;
				connecttime %= 60000;
				seconds = (int)  connecttime/1000;
				connecttime %= 1000;
				snprintf(conntime, 20, "%02d:%02d:%02d.%d",
					hours, minutes, seconds, (int) connecttime);
				conntime[20] = 0;
				if(s->thisconnected)
					connstate  = "ESTABLISHED";
				else
					connstate = "CONNECTING";
				ast_cli(fd, "%-10s%-20s%-12d%-11s%-20s%-20s\n",
					s->name, s->peer, s->reconnects, (s->outbound)? "OUT":"IN", conntime, connstate);
			}
			/* destroy our local link queue */
			s = s_head.next;
			while(s != &s_head){
				t = s;
				s = s->next;
				remque((struct qelem *)t);
				ast_free(t);
			}
			return RESULT_SUCCESS;
		}
	}
	return RESULT_FAILURE;
}

/*
* List all nodes connected, directly or indirectly
*/

static int rpt_do_nodes(int fd, int argc, char *argv[])
{
	int i,j;
	char ns;
	char lbuf[MAXLINKLIST],*strs[MAXLINKLIST];
	struct rpt *myrpt;
	if(argc != 3)
		return RESULT_SHOWUSAGE;

	for(i = 0; i < nrpts; i++)
	{
		if (!strcmp(argv[2],rpt_vars[i].name)){
			/* Make a copy of all stat variables while locked */
			myrpt = &rpt_vars[i];
			rpt_mutex_lock(&myrpt->lock); /* LOCK */
			__mklinklist(myrpt,NULL,lbuf);
			rpt_mutex_unlock(&myrpt->lock); /* UNLOCK */
			/* parse em */
			ns = finddelim(lbuf,strs,MAXLINKLIST);
			/* sort em */
			if (ns) qsort((void *)strs,ns,sizeof(char *),mycompar);
			ast_cli(fd,"\n");
			ast_cli(fd, "************************* CONNECTED NODES *************************\n\n");
			for(j = 0 ;; j++){
				if(!strs[j]){
					if(!j){
						ast_cli(fd,"<NONE>");
					}
					break;
				}
				ast_cli(fd, "%s", strs[j]);
				if(j % 8 == 7){
					ast_cli(fd, "\n");
				}
				else{
					if(strs[j + 1])
						ast_cli(fd, ", ");
				}
			}
			ast_cli(fd,"\n\n");
			return RESULT_SUCCESS;
		}
	}
	return RESULT_FAILURE;
}

/*
* List all locally configured nodes
*/

static int rpt_do_local_nodes(int fd, int argc, char *argv[])
{

    int i;
    ast_cli(fd, "\nNode\n----\n");
    for (i=0; i< nrpts; i++)
    {
        ast_cli(fd, "%s\n", rpt_vars[i].name);
    } /* for i */
    ast_cli(fd,"\n");
    return RESULT_SUCCESS;
}


/*
* reload vars
*/

static int rpt_do_reload(int fd, int argc, char *argv[])
{
int	n;

        if (argc > 2) return RESULT_SHOWUSAGE;

	for(n = 0; n < nrpts; n++) rpt_vars[n].reload = 1;

	return RESULT_FAILURE;
}

/*
* restart app_rpt
*/

static int rpt_do_restart(int fd, int argc, char *argv[])
{
int	i;

	if (argc > 2) return RESULT_SHOWUSAGE;
	for(i = 0; i < nrpts; i++)
	{
		if (rpt_vars[i].rxchannel) ast_softhangup(rpt_vars[i].rxchannel,AST_SOFTHANGUP_DEV);
	}
	return RESULT_FAILURE;
}


/*
* send an app_rpt DTMF function from the CLI
*/

static int rpt_do_fun(int fd, int argc, char *argv[])
{
	int	i,busy=0;

        if (argc != 4) return RESULT_SHOWUSAGE;

	for(i = 0; i < nrpts; i++){
		if(!strcmp(argv[2], rpt_vars[i].name)){
			struct rpt *myrpt = &rpt_vars[i];
			rpt_mutex_lock(&myrpt->lock);
			if ((MAXMACRO - strlen(myrpt->macrobuf)) < strlen(argv[3])){
				rpt_mutex_unlock(&myrpt->lock);
				busy=1;
			}
			if(!busy){
				myrpt->macrotimer = MACROTIME;
				strncat(myrpt->macrobuf,argv[3],MAXMACRO - 1);
			}
			rpt_mutex_unlock(&myrpt->lock);
		}
	}
	if(busy){
		ast_cli(fd, "Function decoder busy");
	}
	return RESULT_FAILURE;
}
/*
* send an Audio File from the CLI
*/

static int rpt_do_playback(int fd, int argc, char *argv[])
{
	int	i,busy=0;

        if (argc != 4) return RESULT_SHOWUSAGE;

	for(i = 0; i < nrpts; i++){
		if(!strcmp(argv[2], rpt_vars[i].name)){
			struct rpt *myrpt = &rpt_vars[i];
			rpt_telemetry(myrpt,PLAYBACK,argv[3]);
		}
	}
	if(busy){
		ast_cli(fd, "Function decoder busy");
	}
	return RESULT_FAILURE;
}

static int rpt_do_localplay(int fd, int argc, char *argv[])
{
        int     i,busy=0;

        if (argc != 4) return RESULT_SHOWUSAGE;

        for(i = 0; i < nrpts; i++){
                if(!strcmp(argv[2], rpt_vars[i].name)){
                        struct rpt *myrpt = &rpt_vars[i];
                        rpt_telemetry(myrpt,LOCALPLAY,argv[3]);
                }
        }
        if(busy){
                ast_cli(fd, "Function decoder busy");
        }
        return RESULT_FAILURE;
}

static int rpt_do_irlpplay(int fd, int argc, char *argv[])
{
        int     i;

        if (argc != 4) return RESULT_SHOWUSAGE;

        for(i = 0; i < nrpts; i++){
		if (!rpt_vars[i].p.irlpann) continue;
                if(!strcmp(argv[2], rpt_vars[i].name)){
                        struct rpt *myrpt = &rpt_vars[i];
                        rpt_telemetry(myrpt,LOCALPLAY,argv[3]);
                }
        }
        return RESULT_SUCCESS;
}


/*
	the convention is that macros in the data from the rpt() application
	are all at the end of the data, separated by the | and start with a *
	when put into the macro buffer, the characters have their high bit
	set so the macro processor knows they came from the application data
	and to use the alt-functions table.
	sph:
*/
static int rpt_push_alt_macro(struct rpt *myrpt, char *sptr)
{
	int	busy=0;

	rpt_mutex_lock(&myrpt->lock);
	if ((MAXMACRO - strlen(myrpt->macrobuf)) < strlen(sptr)){
		rpt_mutex_unlock(&myrpt->lock);
		busy=1;
	}
	if(!busy){
		int x;
		if (debug)ast_log(LOG_NOTICE, "rpt_push_alt_macro %s\n",sptr);
		myrpt->macrotimer = MACROTIME;
		for(x = 0; *(sptr + x); x++)
		    myrpt->macrobuf[x] = *(sptr + x) | 0x80;
		*(sptr + x) = 0;
	}
	rpt_mutex_unlock(&myrpt->lock);

	if(busy)ast_log(LOG_WARNING, "Function decoder busy on app_rpt command macro.\n");

	return busy;
}
/*
	allows us to test rpt() application data commands
*/
static int rpt_do_fun1(int fd, int argc, char *argv[])
{
	int	i;

    if (argc != 4) return RESULT_SHOWUSAGE;

	for(i = 0; i < nrpts; i++){
		if(!strcmp(argv[2], rpt_vars[i].name)){
			struct rpt *myrpt = &rpt_vars[i];
			rpt_push_alt_macro(myrpt,argv[3]);
		}
	}
	return RESULT_FAILURE;
}
/*
* send an app_rpt **command** from the CLI
*/

static int rpt_do_cmd(int fd, int argc, char *argv[])
{
	int i, l;
	int busy=0;
	int maxActions = sizeof(function_table)/sizeof(struct function_table_tag);

	int thisRpt = -1;
	int thisAction = -1;
	struct rpt *myrpt = NULL;
	if (argc != 6) return RESULT_SHOWUSAGE;

	for(i = 0; i < nrpts; i++)
	{
		if(!strcmp(argv[2], rpt_vars[i].name))
		{
			thisRpt = i;
			myrpt = &rpt_vars[i];
			break;
		} /* if !strcmp... */
	} /* for i */

	if (thisRpt < 0)
	{
		ast_cli(fd, "Unknown node number %s.\n", argv[2]);
		return RESULT_FAILURE;
	} /* if thisRpt < 0 */

	/* Look up the action */
	l = strlen(argv[3]);
	for(i = 0 ; i < maxActions; i++)
	{
		if(!strncasecmp(argv[3], function_table[i].action, l))
		{
			thisAction = i;
			break;
		} /* if !strncasecmp... */
	} /* for i */

	if (thisAction < 0)
	{
		ast_cli(fd, "Unknown action name %s.\n", argv[3]);
		return RESULT_FAILURE;
	} /* if thisAction < 0 */

	/* at this point, it looks like all the arguments make sense... */

	rpt_mutex_lock(&myrpt->lock);

	if (rpt_vars[thisRpt].cmdAction.state == CMD_STATE_IDLE)
	{
		rpt_vars[thisRpt].cmdAction.state = CMD_STATE_BUSY;
		rpt_vars[thisRpt].cmdAction.functionNumber = thisAction;
		strncpy(rpt_vars[thisRpt].cmdAction.param, argv[4], MAXDTMF);
		strncpy(rpt_vars[thisRpt].cmdAction.digits, argv[5], MAXDTMF);
		rpt_vars[thisRpt].cmdAction.command_source = SOURCE_RPT;
		rpt_vars[thisRpt].cmdAction.state = CMD_STATE_READY;
		if (debug)ast_log(LOG_NOTICE,"cmdAction.state READY  %i  %s  %s\n",thisAction,argv[4],argv[5]);

	} /* if (rpt_vars[thisRpt].cmdAction.state == CMD_STATE_IDLE */
	else
	{
		busy = 1;
	} /* if (rpt_vars[thisRpt].cmdAction.state == CMD_STATE_IDLE */
	rpt_mutex_unlock(&myrpt->lock);

	return (busy ? RESULT_FAILURE : RESULT_SUCCESS);
} /* rpt_do_cmd() */

static int play_tone_pair(struct ast_channel *chan, int f1, int f2, int duration, int amplitude)
{
	int res;

        if ((res = ast_tonepair_start(chan, f1, f2, duration, amplitude)))
                return res;

        while(chan->generatordata) {
		if (ast_safe_sleep(chan,1)) return -1;
	}

        return 0;
}

static int play_tone(struct ast_channel *chan, int freq, int duration, int amplitude)
{
	return play_tone_pair(chan, freq, 0, duration, amplitude);
}

#ifdef	NEW_ASTERISK

static char *res2cli(int r)

{
	switch (r)
	{
	    case RESULT_SUCCESS:
		return(CLI_SUCCESS);
	    case RESULT_SHOWUSAGE:
		return(CLI_SHOWUSAGE);
	    default:
		return(CLI_FAILURE);
	}
}

static char *handle_cli_debug(struct ast_cli_entry *e,
	int cmd, struct ast_cli_args *a)
{
        switch (cmd) {
        case CLI_INIT:
                e->command = "rpt debug level";
                e->usage = debug_usage;
                return NULL;
        case CLI_GENERATE:
                return NULL;
	}
	return res2cli(rpt_do_debug(a->fd,a->argc,a->argv));
}

static char *handle_cli_dump(struct ast_cli_entry *e,
	int cmd, struct ast_cli_args *a)
{
        switch (cmd) {
        case CLI_INIT:
                e->command = "rpt dump level";
                e->usage = dump_usage;
                return NULL;
        case CLI_GENERATE:
                return NULL;
	}
	return res2cli(rpt_do_dump(a->fd,a->argc,a->argv));
}


static char *handle_cli_stats(struct ast_cli_entry *e,
	int cmd, struct ast_cli_args *a)
{
        switch (cmd) {
        case CLI_INIT:
                e->command = "rpt stats";
                e->usage = dump_stats;
                return NULL;
        case CLI_GENERATE:
                return NULL;
	}
	return res2cli(rpt_do_stats(a->fd,a->argc,a->argv));
}

static char *handle_cli_nodes(struct ast_cli_entry *e,
	int cmd, struct ast_cli_args *a)
{
        switch (cmd) {
        case CLI_INIT:
                e->command = "rpt nodes";
                e->usage = dump_nodes;
                return NULL;
        case CLI_GENERATE:
                return NULL;
	}
	return res2cli(rpt_do_nodes(a->fd,a->argc,a->argv));
}

static char *handle_cli_local_nodes(struct ast_cli_entry *e,
	int cmd, struct ast_cli_args *a)
{
        switch (cmd) {
        case CLI_INIT:
                e->command = "rpt localnodes";
                e->usage = usage_local_nodes;
                return NULL;
        case CLI_GENERATE:
                return NULL;
	}
	return res2cli(rpt_do_local_nodes(a->fd,a->argc,a->argv));
}

static char *handle_cli_lstats(struct ast_cli_entry *e,
	int cmd, struct ast_cli_args *a)
{
        switch (cmd) {
        case CLI_INIT:
                e->command = "rpt lstats";
                e->usage = dump_lstats;
                return NULL;
        case CLI_GENERATE:
                return NULL;
	}
	return res2cli(rpt_do_lstats(a->fd,a->argc,a->argv));
}

static char *handle_cli_reload(struct ast_cli_entry *e,
	int cmd, struct ast_cli_args *a)
{
        switch (cmd) {
        case CLI_INIT:
                e->command = "rpt reload";
                e->usage = reload_usage;
                return NULL;
        case CLI_GENERATE:
                return NULL;
	}
	return res2cli(rpt_do_reload(a->fd,a->argc,a->argv));
}

static char *handle_cli_restart(struct ast_cli_entry *e,
	int cmd, struct ast_cli_args *a)
{
        switch (cmd) {
        case CLI_INIT:
                e->command = "rpt restart";
                e->usage = restart_usage;
                return NULL;
        case CLI_GENERATE:
                return NULL;
	}
	return res2cli(rpt_do_restart(a->fd,a->argc,a->argv));
}

static char *handle_cli_fun(struct ast_cli_entry *e,
	int cmd, struct ast_cli_args *a)
{
        switch (cmd) {
        case CLI_INIT:
                e->command = "rpt fun";
                e->usage = fun_usage;
                return NULL;
        case CLI_GENERATE:
                return NULL;
	}
	return res2cli(rpt_do_fun(a->fd,a->argc,a->argv));
}

static char *handle_cli_playback(struct ast_cli_entry *e,
	int cmd, struct ast_cli_args *a)
{
        switch (cmd) {
        case CLI_INIT:
                e->command = "rpt playback";
                e->usage = playback_usage;
                return NULL;
        case CLI_GENERATE:
                return NULL;
	}
	return res2cli(rpt_do_playback(a->fd,a->argc,a->argv));
}

static char *handle_cli_fun1(struct ast_cli_entry *e,
	int cmd, struct ast_cli_args *a)
{
        switch (cmd) {
        case CLI_INIT:
                e->command = "rpt fun1";
                e->usage = fun_usage;
                return NULL;
        case CLI_GENERATE:
                return NULL;
	}
	return res2cli(rpt_do_fun1(a->fd,a->argc,a->argv));
}

static char *handle_cli_cmd(struct ast_cli_entry *e,
	int cmd, struct ast_cli_args *a)
{
        switch (cmd) {
        case CLI_INIT:
                e->command = "rpt cmd";
                e->usage = cmd_usage;
                return NULL;
        case CLI_GENERATE:
                return NULL;
	}
	return res2cli(rpt_do_cmd(a->fd,a->argc,a->argv));
}

static struct ast_cli_entry rpt_cli[] = {
	AST_CLI_DEFINE(handle_cli_debug,"Enable app_rpt debugging"),
	AST_CLI_DEFINE(handle_cli_dump,"Dump app_rpt structs for debugging"),
	AST_CLI_DEFINE(handle_cli_stats,"Dump node statistics"),
	AST_CLI_DEFINE(handle_cli_nodes,"Dump node list"),
	AST_CLI_DEFINE(handle_cli_local_nodes,	"Dump list of local node numbers"),
	AST_CLI_DEFINE(handle_cli_lstats,"Dump link statistics"),
	AST_CLI_DEFINE(handle_cli_reload,"Reload app_rpt config"),
	AST_CLI_DEFINE(handle_cli_restart,"Restart app_rpt"),
	AST_CLI_DEFINE(handle_cli_playback,"Play Back an Audio File"),
	AST_CLI_DEFINE(handle_cli_fun,"Execute a DTMF function"),
	AST_CLI_DEFINE(handle_cli_fun1,"Execute a DTMF function"),
	AST_CLI_DEFINE(handle_cli_cmd,"Execute a DTMF function")
};

#endif

static int morse_cat(char *str, int freq, int duration)
{
	char *p;
	int len;

	if(!str)
		return -1;

	len = strlen(str);
	p = str+len;

	if(len){
		*p++ = ',';
		*p = '\0';
	}

	snprintf(p, 62,"!%d/%d", freq, duration);

	return 0;
}

static int send_morse(struct ast_channel *chan, char *string, int speed, int freq, int amplitude)
{

static struct morse_bits mbits[] = {
		{0, 0}, /* SPACE */
		{0, 0},
		{6, 18},/* " */
		{0, 0},
		{7, 72},/* $ */
		{0, 0},
		{0, 0},
		{6, 30},/* ' */
		{5, 13},/* ( */
		{6, 29},/* ) */
		{0, 0},
		{5, 10},/* + */
		{6, 51},/* , */
		{6, 33},/* - */
		{6, 42},/* . */
		{5, 9}, /* / */
		{5, 31},/* 0 */
		{5, 30},/* 1 */
		{5, 28},/* 2 */
		{5, 24},/* 3 */
		{5, 16},/* 4 */
		{5, 0}, /* 5 */
		{5, 1}, /* 6 */
		{5, 3}, /* 7 */
		{5, 7}, /* 8 */
		{5, 15},/* 9 */
		{6, 7}, /* : */
		{6, 21},/* ; */
		{0, 0},
		{5, 33},/* = */
		{0, 0},
		{6, 12},/* ? */
		{0, 0},
        	{2, 2}, /* A */
 		{4, 1}, /* B */
		{4, 5}, /* C */
		{3, 1}, /* D */
		{1, 0}, /* E */
		{4, 4}, /* F */
		{3, 3}, /* G */
		{4, 0}, /* H */
		{2, 0}, /* I */
		{4, 14},/* J */
		{3, 5}, /* K */
		{4, 2}, /* L */
		{2, 3}, /* M */
		{2, 1}, /* N */
		{3, 7}, /* O */
		{4, 6}, /* P */
		{4, 11},/* Q */
		{3, 2}, /* R */
		{3, 0}, /* S */
		{1, 1}, /* T */
		{3, 4}, /* U */
		{4, 8}, /* V */
		{3, 6}, /* W */
		{4, 9}, /* X */
		{4, 13},/* Y */
		{4, 3}  /* Z */
	};


	int dottime;
	int dashtime;
	int intralettertime;
	int interlettertime;
	int interwordtime;
	int len, ddcomb;
	int res;
	int c;
	char *str = NULL;

	res = 0;

	str = ast_malloc(12*8*strlen(string)); /* 12 chrs/element max, 8 elements/letter max */
	if(!str)
		return -1;
	str[0] = '\0';

	/* Approximate the dot time from the speed arg. */

	dottime = 900/speed;

	/* Establish timing relationships */

	dashtime = 3 * dottime;
	intralettertime = dottime;
	interlettertime = dottime * 4 ;
	interwordtime = dottime * 7;

	for(;(*string) && (!res); string++){

		c = *string;

		/* Convert lower case to upper case */

		if((c >= 'a') && (c <= 'z'))
			c -= 0x20;

		/* Can't deal with any char code greater than Z, skip it */

		if(c  > 'Z')
			continue;

		/* If space char, wait the inter-word time */

		if(c == ' '){
			if(!res){
				if((res = morse_cat(str, 0, interwordtime)))
					break;
			}
			continue;
		}

		/* Subtract out control char offset to match our table */

		c -= 0x20;

		/* Get the character data */

		len = mbits[c].len;
		ddcomb = mbits[c].ddcomb;

		/* Send the character */

		for(; len ; len--){
			if(!res)
				res = morse_cat(str, freq, (ddcomb & 1) ? dashtime : dottime);
			if(!res)
				res = morse_cat(str, 0, intralettertime);
			ddcomb >>= 1;
		}

		/* Wait the interletter time */

		if(!res)
			res = morse_cat(str, 0, interlettertime - intralettertime);
	}

	/* Wait for all the characters to be sent */

	if(!res){
		if(debug > 4)
			ast_log(LOG_NOTICE,"Morse string: %s\n", str);
		ast_safe_sleep(chan,100);
		ast_playtones_start(chan, amplitude, str, 0);
		while(chan->generatordata){
			if(ast_safe_sleep(chan, 20)){
				res = -1;
				break;
			}
		}

	}
	if(str)
		ast_free(str);
	return res;
}

static int send_tone_telemetry(struct ast_channel *chan, char *tonestring)
{
	char *p,*stringp;
	char *tonesubset;
	int f1,f2;
	int duration;
	int amplitude;
	int res;
	int i;
	int flags;

	res = 0;

	if(!tonestring)
		return res;

	p = stringp = ast_strdup(tonestring);

	for(;tonestring;){
		tonesubset = strsep(&stringp,")");
		if(!tonesubset)
			break;
		if(sscanf(tonesubset,"(%d,%d,%d,%d", &f1, &f2, &duration, &amplitude) != 4)
			break;
		res = play_tone_pair(chan, f1, f2, duration, amplitude);
		if(res)
			break;
	}
	if(p)
		ast_free(p);
	if(!res)
		res = play_tone_pair(chan, 0, 0, 100, 0); /* This is needed to ensure the last tone segment is timed correctly */

	if (!res)
		res = ast_waitstream(chan, "");

	ast_stopstream(chan);

	/*
	* Wait for the zaptel driver to physically write the tone blocks to the hardware
	*/

	for(i = 0; i < 20 ; i++){
		flags =  DAHDI_IOMUX_WRITEEMPTY | DAHDI_IOMUX_NOWAIT;
		res = ioctl(chan->fds[0], DAHDI_IOMUX, &flags);
		if(flags & DAHDI_IOMUX_WRITEEMPTY)
			break;
		if( ast_safe_sleep(chan, 50)){
			res = -1;
			break;
		}
	}

	return res;

}

static int sayfile(struct ast_channel *mychannel,char *fname)
{
int	res;

	res = ast_streamfile(mychannel, fname, mychannel->language);
	if (!res)
		res = ast_waitstream(mychannel, "");
	else
		 ast_log(LOG_WARNING, "ast_streamfile %s failed on %s\n", fname, mychannel->name);
	ast_stopstream(mychannel);
	return res;
}

static int saycharstr(struct ast_channel *mychannel,char *str)
{
int	res;

	res = ast_say_character_str(mychannel,str,NULL,mychannel->language);
	if (!res)
		res = ast_waitstream(mychannel, "");
	else
		 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
	ast_stopstream(mychannel);
	return res;
}

static int saynum(struct ast_channel *mychannel, int num)
{
	int res;
	res = ast_say_number(mychannel, num, NULL, mychannel->language, NULL);
	if(!res)
		res = ast_waitstream(mychannel, "");
	else
		ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
	ast_stopstream(mychannel);
	return res;
}

static int sayphoneticstr(struct ast_channel *mychannel,char *str)
{
int	res;

	res = ast_say_phonetic_str(mychannel,str,NULL,mychannel->language);
	if (!res)
		res = ast_waitstream(mychannel, "");
	else
		 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
	ast_stopstream(mychannel);
	return res;
}

/* say a node and nodename. Try to look in dir referred to by nodenames in
config, and see if there's a custom node file to play, and if so, play it */

static int saynode(struct rpt *myrpt, struct ast_channel *mychannel, char *name)
{
int	res = 0;
char	*val,fname[300],dbstr[100],actstr[100];

	if ((name[0] != '3') || (myrpt->p.eannmode != 2))
	{
		val = (char *) ast_variable_retrieve(myrpt->cfg, myrpt->name, "nodenames");
		if (!val) val = NODENAMES;
		snprintf(fname,sizeof(fname) - 1,"%s/%s",val,name);
		if (ast_fileexists(fname,NULL,mychannel->language) > 0)
			return(sayfile(mychannel,fname));
		res = sayfile(mychannel,"rpt/node");
		if (!res)
			res = ast_say_character_str(mychannel,name,NULL,mychannel->language);
	}
	if (name[0] != '3') return res;
	if (myrpt->p.eannmode < 2) return res;
	if (ast_db_get(EL_DB_ROOT,"active",actstr,sizeof(actstr) - 1)) return res;
	sprintf(dbstr,"%s/nodenum-call/%d",actstr,atoi(name + 1));
	if (ast_db_get(EL_DB_ROOT,dbstr,fname,sizeof(fname))) return res;
	res = sayphoneticstr(mychannel,fname);
	return res;
}

static int telem_any(struct rpt *myrpt,struct ast_channel *chan, char *entry)
{
	int res;
	char c;

	static int morsespeed;
	static int morsefreq;
	static int morseampl;
	static int morseidfreq = 0;
	static int morseidampl;
	static char mcat[] = MORSE;

	res = 0;

	if(!morseidfreq){ /* Get the morse parameters if not already loaded */
		morsespeed = retrieve_astcfgint(myrpt, mcat, "speed", 5, 20, 20);
        	morsefreq = retrieve_astcfgint(myrpt, mcat, "frequency", 300, 3000, 800);
        	morseampl = retrieve_astcfgint(myrpt, mcat, "amplitude", 200, 8192, 4096);
		morseidampl = retrieve_astcfgint(myrpt, mcat, "idamplitude", 200, 8192, 2048);
		morseidfreq = retrieve_astcfgint(myrpt, mcat, "idfrequency", 300, 3000, 330);
	}

	/* Is it a file, or a tone sequence? */

	if(entry[0] == '|'){
		c = entry[1];
		if((c >= 'a')&&(c <= 'z'))
			c -= 0x20;

		switch(c){
			case 'I': /* Morse ID */
				res = send_morse(chan, entry + 2, morsespeed, morseidfreq, morseidampl);
				break;

			case 'M': /* Morse Message */
				res = send_morse(chan, entry + 2, morsespeed, morsefreq, morseampl);
				break;

			case 'T': /* Tone sequence */
				res = send_tone_telemetry(chan, entry + 2);
				break;
			default:
				res = -1;
		}
	}
	else
		res = sayfile(chan, entry); /* File */
	return res;
}

/*
* This function looks up a telemetry name in the config file, and does a telemetry response as configured.
*
* 4 types of telemtry are handled: Morse ID, Morse Message, Tone Sequence, and a File containing a recording.
*/
static int telem_lookup(struct rpt *myrpt,struct ast_channel *chan, char *node, char *name)
{
	int res;
	int i;
	char *entry;
	char *telemetry;
	char *telemetry_save;

	res = 0;
	telemetry_save = NULL;
	entry = NULL;

	if(debug)ast_log(LOG_NOTICE,"[%s] %s\n",myrpt->name,name);

	/* Retrieve the section name for telemetry from the node section */
	telemetry = (char *) ast_variable_retrieve(myrpt->cfg, node, TELEMETRY);
	if(telemetry ){
		telemetry_save = ast_strdup(telemetry);
		if(!telemetry_save){
			ast_log(LOG_WARNING,"ast_strdup() failed in telem_lookup()\n");
			return res;
		}
		entry = (char *) ast_variable_retrieve(myrpt->cfg, telemetry_save, name);
	}

	/* Try to look up the telemetry name */
	if(!entry){
		/* Telemetry name wasn't found in the config file, use the default */
		for(i = 0; i < sizeof(tele_defs)/sizeof(struct telem_defaults) ; i++){
			if(!strcasecmp(tele_defs[i].name, name))
				entry = tele_defs[i].value;
		}
	}
	if(entry){
		if(strlen(entry))
			if (chan) telem_any(myrpt,chan, entry);
	}
	else{
		res = -1;
	}
	if(telemetry_save)
		ast_free(telemetry_save);
	return res;
}

/*
* Retrieve a wait interval
*/

static int get_wait_interval(struct rpt *myrpt, int type)
{
        int interval;
        char *wait_times;
        char *wait_times_save;

		if(debug)ast_log(LOG_NOTICE,"[%s] start type=%i\n",myrpt->name,type);

        wait_times_save = NULL;
        wait_times = (char *) ast_variable_retrieve(myrpt->cfg, myrpt->name, "wait_times");

        if(wait_times){
                wait_times_save = ast_strdup(wait_times);
                if(!wait_times_save)
			return 0;
        }

        switch(type){
                case DLY_TELEM:
                        if(wait_times)
                                interval = retrieve_astcfgint(myrpt,wait_times_save, "telemwait", 500, 5000, 1000);
                        else
                                interval = 1000;
                        break;

                case DLY_ID:
                        if(wait_times)
                                interval = retrieve_astcfgint(myrpt,wait_times_save, "idwait",250,5000,500);
                        else
                                interval = 500;
                        break;

                case DLY_UNKEY:
                        if(wait_times)
                                interval = retrieve_astcfgint(myrpt,wait_times_save, "unkeywait",50,5000,1000);
                        else
                                interval = 1000;
                        break;

                case DLY_LINKUNKEY:
                        if(wait_times)
                                interval = retrieve_astcfgint(myrpt,wait_times_save, "linkunkeywait",500,5000,1000);
                        else
                                interval = 1000;
                        break;

                case DLY_CALLTERM:
                        if(wait_times)
                                interval = retrieve_astcfgint(myrpt,wait_times_save, "calltermwait",500,5000,1500);
                        else
                                interval = 1500;
                        break;

                case DLY_COMP:
                        if(wait_times)
                                interval = retrieve_astcfgint(myrpt,wait_times_save, "compwait",500,5000,200);
                        else
                                interval = 200;
                        break;

                case DLY_PARROT:
                        if(wait_times)
                                interval = retrieve_astcfgint(myrpt,wait_times_save, "parrotwait",500,5000,200);
                        else
                                interval = 200;
                        break;

                default:
			interval = 0;
			break;
        }
	if(wait_times_save)
       		ast_free(wait_times_save);
	return interval;
}


/*
* Wait a configurable interval of time
*/
static void wait_interval(struct rpt *myrpt, int type, struct ast_channel *chan)
{
	int interval;

	do {
		while (myrpt->p.holdofftelem &&
			(myrpt->keyed || (myrpt->remrx && (type != DLY_ID))))
		{
			if (ast_safe_sleep(chan,100) < 0) return;
		}

		interval = get_wait_interval(myrpt, type);
		if(debug)
			ast_log(LOG_NOTICE,"Delay interval = %d\n", interval);
		if(interval)
			ast_safe_sleep(chan,interval);
		if(debug)
			ast_log(LOG_NOTICE,"Delay complete\n");
	}
	while (myrpt->p.holdofftelem &&
		(myrpt->keyed || (myrpt->remrx && (type != DLY_ID))));
	return;
}

static int split_freq(char *mhz, char *decimals, char *freq);

static void handle_varcmd_tele(struct rpt *myrpt,struct ast_channel *mychannel,char *varcmd)
{
char	*strs[100],*p;
int	i,n,res,vmajor,vminor;
time_t	t;
unsigned int t1;
struct	tm localtm;

	if (debug) ast_log(LOG_NOTICE,"[%s] %s\n",myrpt->name,varcmd);

	n = finddelim(varcmd,strs,100);
	if (n < 1) return;
	if (!strcasecmp(strs[0],"REMGO"))
	{
		wait_interval(myrpt, DLY_TELEM, mychannel);
		sayfile(mychannel, "rpt/remote_go");
		return;
	}
	if (!strcasecmp(strs[0],"REMALREADY"))
	{
		wait_interval(myrpt, DLY_TELEM, mychannel);
		sayfile(mychannel, "rpt/remote_already");
		return;
	}
	if (!strcasecmp(strs[0],"REMNOTFOUND"))
	{
		wait_interval(myrpt, DLY_TELEM, mychannel);
		sayfile(mychannel, "rpt/remote_notfound");
		return;
	}
	if (!strcasecmp(strs[0],"COMPLETE"))
	{
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = telem_lookup(myrpt,mychannel, myrpt->name, "functcomplete");
		if (!res)
			res = ast_waitstream(mychannel, "");
		else
			 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
		ast_stopstream(mychannel);
		return;
	}
	if (!strcasecmp(strs[0],"PROC"))
	{
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = telem_lookup(myrpt, mychannel, myrpt->name, "patchup");
		if(res < 0){ /* Then default message */
			sayfile(mychannel, "rpt/callproceeding");
		}
		return;
	}
	if (!strcasecmp(strs[0],"TERM"))
	{
		/* wait a little bit longer */
		wait_interval(myrpt, DLY_CALLTERM, mychannel);
		res = telem_lookup(myrpt, mychannel, myrpt->name, "patchdown");
		if(res < 0){ /* Then default message */
			sayfile(mychannel, "rpt/callterminated");
		}
		return;
	}
	if (!strcasecmp(strs[0],"MACRO_NOTFOUND"))
	{
		wait_interval(myrpt, DLY_TELEM, mychannel);
		sayfile(mychannel, "rpt/macro_notfound");
		return;
	}
	if (!strcasecmp(strs[0],"MACRO_BUSY"))
	{
		wait_interval(myrpt, DLY_TELEM, mychannel);
		sayfile(mychannel, "rpt/macro_busy");
		return;
	}
	if (!strcasecmp(strs[0],"CONNECTED"))
	{
		if (n < 3) return;
		wait_interval(myrpt, DLY_TELEM,  mychannel);
		res = saynode(myrpt,mychannel,strs[2]);
		if (!res)
		    res = ast_streamfile(mychannel, "rpt/connected", mychannel->language);
		if (!res)
			res = ast_waitstream(mychannel, "");
		else
			 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
		ast_stopstream(mychannel);
		res = ast_streamfile(mychannel, "digits/2", mychannel->language);
		if (!res)
			res = ast_waitstream(mychannel, "");
		else
			 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
		ast_stopstream(mychannel);
		saynode(myrpt,mychannel,strs[1]);
		return;
	}
	if (!strcasecmp(strs[0],"CONNFAIL"))
	{
		if (n < 2) return;
		res = saynode(myrpt,mychannel,strs[1]);
		if (!res)
		   sayfile(mychannel, "rpt/connection_failed");
		return;
	}
	if (!strcasecmp(strs[0],"REMDISC"))
	{
		if (n < 2) return;
		res = saynode(myrpt,mychannel,strs[1]);
		if (!res)
		   sayfile(mychannel, "rpt/remote_disc");
		return;
	}
	if (!strcasecmp(strs[0],"STATS_TIME"))
	{
		if (n < 2) return;
		if (sscanf(strs[1],"%u",&t1) != 1) return;
		t = t1;
	    	wait_interval(myrpt, DLY_TELEM, mychannel); /* Wait a little bit */
		rpt_localtime(&t, &localtm);
		/* Say the phase of the day is before the time */
		if((localtm.tm_hour >= 0) && (localtm.tm_hour < 12))
			p = "rpt/goodmorning";
		else if((localtm.tm_hour >= 12) && (localtm.tm_hour < 18))
			p = "rpt/goodafternoon";
		else
			p = "rpt/goodevening";
		if (sayfile(mychannel,p) == -1) return;
		/* Say the time is ... */
		if (sayfile(mychannel,"rpt/thetimeis") == -1) return;
		/* Say the time */
	    	res = ast_say_time(mychannel, t, "", mychannel->language);
		if (!res)
			res = ast_waitstream(mychannel, "");
		ast_stopstream(mychannel);
		return;
	}
	if (!strcasecmp(strs[0],"STATS_VERSION"))
	{
		if (n < 2) return;
		if(sscanf(strs[1], "%d.%d", &vmajor, &vminor) != 2) return;
    		wait_interval(myrpt, DLY_TELEM, mychannel); /* Wait a little bit */
		/* Say "version" */
		if (sayfile(mychannel,"rpt/version") == -1) return;
		res = ast_say_number(mychannel, vmajor, "", mychannel->language, (char *) NULL);
		if (!res)
			res = ast_waitstream(mychannel, "");
		ast_stopstream(mychannel);
		if (saycharstr(mychannel,".") == -1) return;
		if(!res) /* Say "Y" */
			ast_say_number(mychannel, vminor, "", mychannel->language, (char *) NULL);
		if (!res){
			res = ast_waitstream(mychannel, "");
			ast_stopstream(mychannel);
		}
		else
			 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
		return;
	}
	if (!strcasecmp(strs[0],"ARB_ALPHA"))
	{
		if (n < 2) return;
	    	wait_interval(myrpt, DLY_TELEM, mychannel); /* Wait a little bit */
	    	saycharstr(mychannel, strs[1]);
		return;
	}
	if (!strcasecmp(strs[0],"REV_PATCH"))
	{
		/* Parts of this section taken from app_parkandannounce */
		char *tpl_working, *tpl_current, *tpl_copy;
		char *tmp[100], *myparm;
		int looptemp=0,i=0, dres = 0;

		if (n < 3) return;
	    	wait_interval(myrpt, DLY_TELEM, mychannel); /* Wait a little bit */

		tpl_working = ast_strdup(strs[2]);
		tpl_copy = tpl_working;
		myparm = strsep(&tpl_working,"^");
		tpl_current=strsep(&tpl_working, ":");
		while(tpl_current && looptemp < sizeof(tmp)) {
			tmp[looptemp]=tpl_current;
			looptemp++;
			tpl_current=strsep(&tpl_working,":");
		}
		for(i=0; i<looptemp; i++) {
			if(!strcmp(tmp[i], "PARKED")) {
				ast_say_digits(mychannel, atoi(myparm), "", mychannel->language);
			} else if(!strcmp(tmp[i], "NODE")) {
				ast_say_digits(mychannel, atoi(strs[1]), "", mychannel->language);
			} else {
				dres = ast_streamfile(mychannel, tmp[i], mychannel->language);
				if(!dres) {
					dres = ast_waitstream(mychannel, "");
				} else {
					ast_log(LOG_WARNING, "ast_streamfile of %s failed on %s\n", tmp[i], mychannel->name);
					dres = 0;
				}
			}
		}
		ast_free(tpl_copy);
		return;
	}
	if (!strcasecmp(strs[0],"LASTNODEKEY"))
	{
		if (n < 2) return;
		if (!atoi(strs[1])) return;
		wait_interval(myrpt, DLY_TELEM, mychannel);
		saynode(myrpt,mychannel,strs[1]);
		return;
	}
	if (!strcasecmp(strs[0],"STATUS"))
	{
		if (n < 3) return;
		wait_interval(myrpt, DLY_TELEM, mychannel);
		saynode(myrpt,mychannel,strs[1]);
		if (atoi(strs[2]) > 0) sayfile(mychannel, "rpt/autopatch_on");
		else if (n == 3)
		{
			sayfile(mychannel,"rpt/repeat_only");
			return;
		}
		for(i = 3; i < n; i++)
		{
			saynode(myrpt,mychannel,strs[i] + 1);
			if (*strs[i] == 'T') sayfile(mychannel,"rpt/tranceive");
			else if (*strs[i] == 'R') sayfile(mychannel,"rpt/monitor");
			else sayfile(mychannel,"rpt/connecting");
		}
		return;
	}
	ast_log(LOG_WARNING,"Got unknown link telemetry command: %s\n",strs[0]);
	return;
}

static void *rpt_tele_thread(void *this)
{
struct dahdi_confinfo ci;  /* conference info */
int	res = 0,haslink,hastx,hasremote,imdone = 0, unkeys_queued, x;
struct	rpt_tele *mytele = (struct rpt_tele *)this;
struct  rpt_tele *tlist;
struct	rpt *myrpt;
struct	rpt_link *l,*l1,linkbase;
struct	ast_channel *mychannel;
int id_malloc, vmajor, vminor, m;
char *p,*ct,*ct_copy,*ident, *nodename,*cp;
time_t t;
#ifdef	NEW_ASTERISK
struct ast_tm localtm;
#else
struct tm localtm;
#endif
char lbuf[MAXLINKLIST],*strs[MAXLINKLIST];
int	i,ns,rbimode;
char mhz[MAXREMSTR];
char decimals[MAXREMSTR];
char	mystr[200];
struct dahdi_params par;

	/* get a pointer to myrpt */
	myrpt = mytele->rpt;

	if(debug)ast_log(LOG_NOTICE,"[%s] start\n",myrpt->name);

	/* Snag copies of a few key myrpt variables */
	rpt_mutex_lock(&myrpt->lock);
	nodename = ast_strdup(myrpt->name);
	if(!nodename)
	{
	    fprintf(stderr,"rpt:Sorry unable strdup nodename\n");
	    rpt_mutex_lock(&myrpt->lock);
	    remque((struct qelem *)mytele);
	    ast_log(LOG_NOTICE,"Telemetry thread aborted at line %d, mode: %d\n",__LINE__, mytele->mode); /*@@@@@@@@@@@*/
	    rpt_mutex_unlock(&myrpt->lock);
	    ast_free(mytele);
	    pthread_exit(NULL);
	}

	if (myrpt->p.ident){
		ident = ast_strdup(myrpt->p.ident);
        if(!ident)
		{
			fprintf(stderr,"rpt:Sorry unable strdup ident\n");
			rpt_mutex_lock(&myrpt->lock);
			remque((struct qelem *)mytele);
			ast_log(LOG_NOTICE,"Telemetry thread aborted at line %d, mode: %d\n",
                	__LINE__, mytele->mode); /*@@@@@@@@@@@*/
            rpt_mutex_unlock(&myrpt->lock);
			ast_free(nodename);
			ast_free(mytele);
            pthread_exit(NULL);
        }
		else
		{
			id_malloc = 1;
		}
	}
	else
	{
		ident = "";
		id_malloc = 0;
	}

	rpt_mutex_unlock(&myrpt->lock);

	/* allocate a pseudo-channel thru asterisk */
	mychannel = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
	if (!mychannel)
	{
		fprintf(stderr,"rpt:Sorry unable to obtain pseudo channel\n");
		rpt_mutex_lock(&myrpt->lock);
		remque((struct qelem *)mytele);
		ast_log(LOG_NOTICE,"Telemetry thread aborted at line %d, mode: %d\n",__LINE__, mytele->mode); /*@@@@@@@@@@@*/
		rpt_mutex_unlock(&myrpt->lock);
		ast_free(nodename);
		if(id_malloc)
		ast_free(ident);
		ast_free(mytele);
		pthread_exit(NULL);
	}
#ifdef	AST_CDR_FLAG_POST_DISABLED
	if (mychannel->cdr)
		ast_set_flag(mychannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
	rpt_mutex_lock(&myrpt->lock);
	mytele->chan = mychannel;
	while (myrpt->active_telem &&
	    (myrpt->active_telem->mode == PAGE))
	{
        rpt_mutex_unlock(&myrpt->lock);
		usleep(100000);
		rpt_mutex_lock(&myrpt->lock);
	}
	rpt_mutex_unlock(&myrpt->lock);
	while((mytele->mode != SETREMOTE) && (mytele->mode != UNKEY) &&
	    (mytele->mode != LINKUNKEY) && (mytele->mode != LOCUNKEY) &&
		(mytele->mode != COMPLETE) && (mytele->mode != REMGO))
	{
        rpt_mutex_lock(&myrpt->lock);
		if ((!myrpt->active_telem) &&
			(myrpt->tele.prev == mytele))
		{
			myrpt->active_telem = mytele;
			rpt_mutex_unlock(&myrpt->lock);
			break;
		}
        rpt_mutex_unlock(&myrpt->lock);
		usleep(100000);
	}

	/* make a conference for the tx */
	ci.chan = 0;
	/* If the telemetry is only intended for a local audience, */
	/* only connect the ID audio to the local tx conference so */
	/* linked systems can't hear it */
	ci.confno = (((mytele->mode == ID1) || (mytele->mode == PLAYBACK) || (mytele->mode == TEST_TONE)) ?
		myrpt->conf : myrpt->teleconf);
	ci.confmode = DAHDI_CONF_CONFANN;
	/* first put the channel on the conference in announce mode */
	if (ioctl(mychannel->fds[0],DAHDI_SETCONF,&ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
		rpt_mutex_lock(&myrpt->lock);
		myrpt->active_telem = NULL;
		remque((struct qelem *)mytele);
		rpt_mutex_unlock(&myrpt->lock);
		ast_log(LOG_NOTICE,"Telemetry thread aborted at line %d, mode: %d\n",__LINE__, mytele->mode); /*@@@@@@@@@@@*/
		ast_free(nodename);
		if(id_malloc)
			ast_free(ident);
		ast_free(mytele);
		ast_hangup(mychannel);
		pthread_exit(NULL);
	}
	ast_stopstream(mychannel);

	if(debug)ast_log(LOG_NOTICE, "[%s] telemetry chan to %s\n",myrpt->name,(ci.confno==myrpt->conf)?"conf":"teleconf");

	switch(mytele->mode)
	{
	    case VARCMD:
		handle_varcmd_tele(myrpt,mychannel,mytele->param);
		imdone = 1;
		break;
	    case ID:
	    case ID1:
		if (*ident)
		{
			/* wait a bit */
			wait_interval(myrpt, (mytele->mode == ID) ? DLY_ID : DLY_TELEM,mychannel);
			res = telem_any(myrpt,mychannel, ident);
		}
		imdone=1;
		break;

	    case TAILMSG:
		/* wait a little bit longer */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = ast_streamfile(mychannel, myrpt->p.tailmessages[myrpt->tailmessagen], mychannel->language);
		break;

	    case IDTALKOVER:
			if(debug)
				ast_log(LOG_NOTICE,"Tracepoint IDTALKOVER: in rpt_tele_thread()\n");
	    	p = (char *) ast_variable_retrieve(myrpt->cfg, nodename, "idtalkover");
	    	if(p)res = telem_any(myrpt,mychannel, p);
		imdone=1;
	    	break;

	    case PROC:
		/* wait a little bit longer */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = telem_lookup(myrpt, mychannel, myrpt->name, "patchup");
		if(res < 0){ /* Then default message */
			res = ast_streamfile(mychannel, "rpt/callproceeding", mychannel->language);
		}
		break;
	    case TERM:
		/* wait a little bit longer */
		wait_interval(myrpt, DLY_CALLTERM, mychannel);
		res = telem_lookup(myrpt, mychannel, myrpt->name, "patchdown");
		if(res < 0){ /* Then default message */
			res = ast_streamfile(mychannel, "rpt/callterminated", mychannel->language);
		}
		break;
	    case COMPLETE:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = telem_lookup(myrpt,mychannel, myrpt->name, "functcomplete");
		break;
	    case MACRO_NOTFOUND:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = ast_streamfile(mychannel, "rpt/macro_notfound", mychannel->language);
		break;
	    case MACRO_BUSY:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = ast_streamfile(mychannel, "rpt/macro_busy", mychannel->language);
		break;

	    case PAGE:
		wait_interval(myrpt, DLY_TELEM,  mychannel);
		res = -1;
		if (mytele->submode)
		{
			myrpt->noduck=1;
			res = ast_playtones_start(myrpt->txchannel,0,
				(char *) mytele->submode,0);
			while(myrpt->txchannel->generatordata)
			{
				if(ast_safe_sleep(myrpt->txchannel, 50))
				{
					res = -1;
					break;
				}
			}
			free((char *)mytele->submode);
		}
		imdone = 1;
		break;

	    case UNKEY:
	    case LOCUNKEY:
		if(myrpt->patchnoct && myrpt->callmode){ /* If no CT during patch configured, then don't send one */
			imdone = 1;
			break;
		}

		/*
		* Reset the Unkey to CT timer
		*/
		x = get_wait_interval(myrpt, DLY_UNKEY);
		rpt_mutex_lock(&myrpt->lock);
		myrpt->unkeytocttimer = x; /* Must be protected as it is changed below */
		rpt_mutex_unlock(&myrpt->lock);

		/*
		* If there's one already queued, don't do another
		*/
		tlist = myrpt->tele.next;
		unkeys_queued = 0;
        if (tlist != &myrpt->tele)
        {
        	rpt_mutex_lock(&myrpt->lock);
        	while(tlist != &myrpt->tele){
            	if ((tlist->mode == UNKEY) ||
            	    (tlist->mode == LOCUNKEY)) unkeys_queued++;
                tlist = tlist->next;
            }
            rpt_mutex_unlock(&myrpt->lock);
		}
		if( unkeys_queued > 1){
			imdone = 1;
			break;
		}

		/* Wait for the telemetry timer to expire */
		/* Periodically check the timer since it can be re-initialized above */
		while(myrpt->unkeytocttimer)
		{
			int ctint;
			if(myrpt->unkeytocttimer > 100)
				ctint = 100;
			else
				ctint = myrpt->unkeytocttimer;
			ast_safe_sleep(mychannel, ctint);
			rpt_mutex_lock(&myrpt->lock);
			if(myrpt->unkeytocttimer < ctint)
				myrpt->unkeytocttimer = 0;
			else
				myrpt->unkeytocttimer -= ctint;
			rpt_mutex_unlock(&myrpt->lock);
		}

		/*
		* Now, the carrier on the rptr rx should be gone.
		* If it re-appeared, then forget about sending the CT
		*/
		if(myrpt->keyed){
			imdone = 1;
			break;
		}

		rpt_mutex_lock(&myrpt->lock); /* Update the kerchunk counters */
		myrpt->dailykerchunks++;
		myrpt->totalkerchunks++;
		rpt_mutex_unlock(&myrpt->lock);

		if ((mytele->mode == LOCUNKEY) &&
		    ((ct = (char *) ast_variable_retrieve(myrpt->cfg, nodename, "localct")))) { /* Local override ct */
			ct_copy = ast_strdup(ct);
			if(ct_copy)
			{
			    myrpt->noduck=1;
				res = telem_lookup(myrpt,mychannel, myrpt->name, ct_copy);
				ast_free(ct_copy);
			}
			else
				res = -1;
			if(res)
			 	ast_log(LOG_WARNING, "telem_lookup:ctx failed on %s\n", mychannel->name);
		}
		haslink = 0;
		hastx = 0;
		hasremote = 0;
		l = myrpt->links.next;
		if (l != &myrpt->links)
		{
			rpt_mutex_lock(&myrpt->lock);
			while(l != &myrpt->links)
			{
				if (l->name[0] == '0')
				{
					l = l->next;
					continue;
				}
				// haslink = 1;	  // asdf !!! ??? sph maw
				if (myrpt->p.nolocallinkct)
				{
					int v,w;

					w = 1;
					for(v = 0; v < nrpts; v++)
					{
						if (&rpt_vars[v] == myrpt) continue;
						if (rpt_vars[v].remote) continue;
						if (strcmp(rpt_vars[v].name,l->name)) continue;
						w = 0;
						break;
					}
					if (w) haslink = 1;
				} else haslink = 1;

				if (l->mode) {
					hastx++;
					if (l->isremote) hasremote++;
				}
				l = l->next;
			}
			rpt_mutex_unlock(&myrpt->lock);
		}
		if (haslink)
		{
			myrpt->noduck=1;
			res = telem_lookup(myrpt,mychannel, myrpt->name, (!hastx) ? "remotemon" : "remotetx");
			if(res)
				ast_log(LOG_WARNING, "telem_lookup:remotexx failed on %s\n", mychannel->name);

			/* if in remote cmd mode, indicate it */
			if (myrpt->cmdnode[0])
			{
				ast_safe_sleep(mychannel,200);
				res = telem_lookup(myrpt,mychannel, myrpt->name, "cmdmode");
				if(res)
				 	ast_log(LOG_WARNING, "telem_lookup:cmdmode failed on %s\n", mychannel->name);
				ast_stopstream(mychannel);
			}
		}
		else if((ct = (char *) ast_variable_retrieve(myrpt->cfg, nodename, "unlinkedct"))){ /* Unlinked Courtesy Tone */
			ct_copy = ast_strdup(ct);
			if(ct_copy)
			{
				myrpt->noduck=1;
				res = telem_lookup(myrpt,mychannel, myrpt->name, ct_copy);
				ast_free(ct_copy);
			}
			else
				res = -1;
			if(res)
			 	ast_log(LOG_WARNING, "telem_lookup:ctx failed on %s\n", mychannel->name);
		}
		if (hasremote && (!myrpt->cmdnode[0]))
		{
			/* set for all to hear */
			ci.chan = 0;
			ci.confno = myrpt->conf;
			ci.confmode = DAHDI_CONF_CONFANN;
			/* first put the channel on the conference in announce mode */
			if (ioctl(mychannel->fds[0],DAHDI_SETCONF,&ci) == -1)
			{
				ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
				rpt_mutex_lock(&myrpt->lock);
				myrpt->active_telem = NULL;
				remque((struct qelem *)mytele);
				rpt_mutex_unlock(&myrpt->lock);
				ast_log(LOG_NOTICE,"Telemetry thread aborted at line %d, mode: %d\n",__LINE__, mytele->mode); /*@@@@@@@@@@@*/
				ast_free(nodename);
				if(id_malloc)
					ast_free(ident);
				ast_free(mytele);
				ast_hangup(mychannel);
				pthread_exit(NULL);
			}
			if(debug)ast_log(LOG_NOTICE, "[%s] telemetry channel output to conf\n", myrpt->name);
			if((ct = (char *) ast_variable_retrieve(myrpt->cfg, nodename, "remotect"))){ /* Unlinked Courtesy Tone */
				ast_safe_sleep(mychannel,200);
				ct_copy = ast_strdup(ct);
				if(ct_copy)
				{
					myrpt->noduck=1;
					res = telem_lookup(myrpt,mychannel, myrpt->name, ct_copy);
					ast_free(ct_copy);
				}
				else
					res = -1;

				if(res)
				 	ast_log(LOG_WARNING, "telem_lookup:ctx failed on %s\n", mychannel->name);
			}
		}
#if	defined(_MDC_DECODE_H_) && defined(MDC_SAY_WHEN_DOING_CT)
		if (myrpt->lastunit)
		{
			char mystr[10];

			ast_safe_sleep(mychannel,200);
			/* set for all to hear */
			ci.chan = 0;
			ci.confno = myrpt->txconf;
			ci.confmode = DAHDI_CONF_CONFANN;
			/* first put the channel on the conference in announce mode */
			if (ioctl(mychannel->fds[0],DAHDI_SETCONF,&ci) == -1)
			{
				ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
				rpt_mutex_lock(&myrpt->lock);
				myrpt->active_telem = NULL;
				remque((struct qelem *)mytele);
				rpt_mutex_unlock(&myrpt->lock);
				ast_log(LOG_NOTICE,"Telemetry thread aborted at line %d, mode: %d\n",__LINE__, mytele->mode); /*@@@@@@@@@@@*/
				ast_free(nodename);
				if(id_malloc)
					ast_free(ident);
				ast_free(mytele);
				ast_hangup(mychannel);
				pthread_exit(NULL);
			}
			sprintf(mystr,"%04x",myrpt->lastunit);
			myrpt->lastunit = 0;
			ast_say_character_str(mychannel,mystr,NULL,mychannel->language);
			break;
		}
#endif

		imdone = 1;
		break;
	    case LINKUNKEY:
		/* if voting and a voter link unkeys but the main or another voter rx is still active */
		if(myrpt->p.votertype==1 && (myrpt->rxchankeyed || myrpt->voteremrx ))
		{
			imdone = 1;
			break;
		}
		if(myrpt->patchnoct && myrpt->callmode){ /* If no CT during patch configured, then don't send one */
			imdone = 1;
			break;
		}
		if (myrpt->p.nolocallinkct) /* No CT if this guy is on local system */
		{
			int v,w;
			w = 0;
			for(v = 0; v < nrpts; v++)
			{
				if (&rpt_vars[v] == myrpt) continue;
				if (rpt_vars[v].remote) continue;
				if (strcmp(rpt_vars[v].name,
					mytele->mylink.name)) continue;
				w = 1;
				break;
			}
			if (w)
			{
				imdone = 1;
				break;
			}
		}
		/*
		* Reset the Unkey to CT timer
		*/
		x = get_wait_interval(myrpt, DLY_LINKUNKEY);
		mytele->mylink.linkunkeytocttimer = x; /* Must be protected as it is changed below */

		/*
		* If there's one already queued, don't do another
		*/
		tlist = myrpt->tele.next;
		unkeys_queued = 0;
        if (tlist != &myrpt->tele){
	        rpt_mutex_lock(&myrpt->lock);
	        while(tlist != &myrpt->tele){
	        	if (tlist->mode == LINKUNKEY) unkeys_queued++;
	        	tlist = tlist->next;
	        }
	        rpt_mutex_unlock(&myrpt->lock);
		}
		if( unkeys_queued > 1){
			imdone = 1;
			break;
		}

		/* Wait for the telemetry timer to expire */
		/* Periodically check the timer since it can be re-initialized above */
		while(mytele->mylink.linkunkeytocttimer)
		{
			int ctint;
			if(mytele->mylink.linkunkeytocttimer > 100)
				ctint = 100;
			else
				ctint = mytele->mylink.linkunkeytocttimer;
			ast_safe_sleep(mychannel, ctint);
			rpt_mutex_lock(&myrpt->lock);
			if(mytele->mylink.linkunkeytocttimer < ctint)
				mytele->mylink.linkunkeytocttimer = 0;
			else
				mytele->mylink.linkunkeytocttimer -= ctint;
			rpt_mutex_unlock(&myrpt->lock);
		}

		l = myrpt->links.next;
		unkeys_queued = 0;
        rpt_mutex_lock(&myrpt->lock);
        while (l != &myrpt->links)
        {
			if (!strcmp(l->name,mytele->mylink.name))
			{
				unkeys_queued = l->lastrx;
				break;
            }
            l = l->next;
		}
        rpt_mutex_unlock(&myrpt->lock);
		if( unkeys_queued ){
			imdone = 1;
			break;
		}
		if((ct = (char *) ast_variable_retrieve(myrpt->cfg, nodename, "linkunkeyct"))){ /* Unlinked Courtesy Tone */
			ct_copy = ast_strdup(ct);
			if(ct_copy){
				res = telem_lookup(myrpt,mychannel, myrpt->name, ct_copy);
				ast_free(ct_copy);
			}
			else
				res = -1;
			if(res)
			 	ast_log(LOG_WARNING, "telem_lookup:ctx failed on %s\n", mychannel->name);
		}
		imdone = 1;
		break;
	    case REMDISC:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		l = myrpt->links.next;
		haslink = 0;
		/* dont report if a link for this one still on system */
		if (l != &myrpt->links)
		{
			rpt_mutex_lock(&myrpt->lock);
			while(l != &myrpt->links)
			{
				if (l->name[0] == '0')
				{
					l = l->next;
					continue;
				}
				if (!strcmp(l->name,mytele->mylink.name))
				{
					haslink = 1;
					break;
				}
				l = l->next;
			}
			rpt_mutex_unlock(&myrpt->lock);
		}
		if (haslink)
		{
			imdone = 1;
			break;
		}
		res = saynode(myrpt,mychannel,mytele->mylink.name);
		if (!res)
		    res = ast_streamfile(mychannel, ((mytele->mylink.hasconnected) ?
			"rpt/remote_disc" : "rpt/remote_busy"), mychannel->language);
		break;
	    case REMALREADY:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = ast_streamfile(mychannel, "rpt/remote_already", mychannel->language);
		break;
	    case REMNOTFOUND:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = ast_streamfile(mychannel, "rpt/remote_notfound", mychannel->language);
		break;
	    case REMGO:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = ast_streamfile(mychannel, "rpt/remote_go", mychannel->language);
		break;
	    case CONNECTED:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM,  mychannel);
		res = saynode(myrpt,mychannel,mytele->mylink.name);
		if (!res)
		    res = ast_streamfile(mychannel, "rpt/connected", mychannel->language);
		if (!res)
			res = ast_waitstream(mychannel, "");
		else
			 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
		ast_stopstream(mychannel);
		res = ast_streamfile(mychannel, "digits/2", mychannel->language);
		if (!res)
			res = ast_waitstream(mychannel, "");
		else
			 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
		ast_stopstream(mychannel);
		res = saynode(myrpt,mychannel,myrpt->name);
		imdone = 1;
		break;
	    case CONNFAIL:
		res = saynode(myrpt,mychannel,mytele->mylink.name);
		if (!res)
		    res = ast_streamfile(mychannel, "rpt/connection_failed", mychannel->language);
		break;
	    case MEMNOTFOUND:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = ast_streamfile(mychannel, "rpt/memory_notfound", mychannel->language);
		break;
	    case PLAYBACK:
	    case LOCALPLAY:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = ast_streamfile(mychannel, mytele->param, mychannel->language);
		if (!res)
			res = ast_waitstream(mychannel, "");
		else
			 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
		ast_stopstream(mychannel);
		if (!strncasecmp(mytele->param,"/home/irlp/astrun",14))
		{
			char fn[100];
			snprintf(fn,sizeof(fn) - 1,"/bin/rm -f %s.*",mytele->param);
			ast_safe_system(fn);
		}
		imdone = 1;
		break;
	    case TOPKEY:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		for(i = 0; i < TOPKEYN; i++)
		{
			if (!myrpt->topkey[i].node[0]) continue;
			if ((!myrpt->topkeylong) && (myrpt->topkey[i].keyed)) continue;
			res = saynode(myrpt, mychannel,	myrpt->topkey[i].node);
			if (!res) res = sayfile(mychannel,(myrpt->topkey[i].keyed) ?
				"rpt/keyedfor" : "rpt/unkeyedfor");
			if (!res) res = saynum(mychannel,
				myrpt->topkey[i].timesince);
			if (!res) res = sayfile(mychannel,"rpt/seconds");
			if (!myrpt->topkeylong) break;
		}
		imdone = 1;
		break;
	    case SETREMOTE:
		ast_mutex_lock(&myrpt->remlock);
		res = 0;
		if(!strcmp(myrpt->remoterig, remote_rig_ft897))
		{
			res = set_ft897(myrpt);
		}
		else if(!strcmp(myrpt->remoterig, remote_rig_ft950))
		{
			res = set_ft950(myrpt);
		}
		else if(!strcmp(myrpt->remoterig, remote_rig_tm271))
		{
			setxpmr(myrpt,0);
			res = set_tm271(myrpt);
		}
		else if(!strcmp(myrpt->remoterig, remote_rig_ic706))
		{
			res = set_ic706(myrpt);
		}
		else if(!strcmp(myrpt->remoterig, remote_rig_rbi)||!strcmp(myrpt->remoterig, remote_rig_ppp16))
		{
			if (ioperm(myrpt->p.iobase,1,1) == -1)
			{
				rpt_mutex_unlock(&myrpt->lock);
				ast_log(LOG_WARNING, "Cant get io permission on IO port %x hex\n",myrpt->p.iobase);
				res = -1;
			}
			else res = setrbi(myrpt);
		}
		else if(!strcmp(myrpt->remoterig, remote_rig_kenwood))
		{
			if (myrpt->iofd >= 0) setdtr(myrpt->iofd,1);
			res = setkenwood(myrpt);
			if (myrpt->iofd >= 0) setdtr(myrpt->iofd,0);
			setxpmr(myrpt,0);
			if (ast_safe_sleep(mychannel,200) == -1)
			{
				ast_mutex_unlock(&myrpt->remlock);
				res = -1;
				break;
			}
			if (myrpt->iofd < 0)
			{
				i = DAHDI_FLUSH_EVENT;
				if (ioctl(myrpt->zaptxchannel->fds[0],DAHDI_FLUSH,&i) == -1)
				{
					ast_mutex_unlock(&myrpt->remlock);
					ast_log(LOG_ERROR,"Cant flush events");
					res = -1;
					break;
				}
				if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_GET_PARAMS,&par) == -1)
				{
					ast_mutex_unlock(&myrpt->remlock);
					ast_log(LOG_ERROR,"Cant get params");
					res = -1;
					break;
				}
				myrpt->remoterx =
					(par.rxisoffhook || (myrpt->tele.next != &myrpt->tele));
			}
		}

		ast_mutex_unlock(&myrpt->remlock);
		if (!res)
		{
			imdone = 1;
			break;
		}
		/* fall thru to invalid freq */
	    case INVFREQ:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = ast_streamfile(mychannel, "rpt/invalid-freq", mychannel->language);
		break;
	    case REMMODE:
		cp = 0;
		wait_interval(myrpt, DLY_TELEM, mychannel);
		switch(myrpt->remmode)
		{
		    case REM_MODE_FM:
			saycharstr(mychannel,"FM");
			break;
		    case REM_MODE_USB:
			saycharstr(mychannel,"USB");
			break;
		    case REM_MODE_LSB:
			saycharstr(mychannel,"LSB");
			break;
		    case REM_MODE_AM:
			saycharstr(mychannel,"AM");
			break;
		}
		wait_interval(myrpt, DLY_COMP, mychannel);
		if (!res) res = telem_lookup(myrpt,mychannel, myrpt->name, "functcomplete");
		break;
	    case LOGINREQ:
		wait_interval(myrpt, DLY_TELEM, mychannel);
		sayfile(mychannel,"rpt/login");
		saycharstr(mychannel,myrpt->name);
		break;
	    case REMLOGIN:
		wait_interval(myrpt, DLY_TELEM, mychannel);
		saycharstr(mychannel,myrpt->loginuser);
		saynode(myrpt,mychannel,myrpt->name);
		wait_interval(myrpt, DLY_COMP, mychannel);
		if (!res) res = telem_lookup(myrpt,mychannel, myrpt->name, "functcomplete");
		break;
	    case REMXXX:
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = 0;
		switch(mytele->submode)
		{
		    case 100: /* RX PL Off */
			sayfile(mychannel, "rpt/rxpl");
			sayfile(mychannel, "rpt/off");
			break;
		    case 101: /* RX PL On */
			sayfile(mychannel, "rpt/rxpl");
			sayfile(mychannel, "rpt/on");
			break;
		    case 102: /* TX PL Off */
			sayfile(mychannel, "rpt/txpl");
			sayfile(mychannel, "rpt/off");
			break;
		    case 103: /* TX PL On */
			sayfile(mychannel, "rpt/txpl");
			sayfile(mychannel, "rpt/on");
			break;
		    case 104: /* Low Power */
			sayfile(mychannel, "rpt/lopwr");
			break;
		    case 105: /* Medium Power */
			sayfile(mychannel, "rpt/medpwr");
			break;
		    case 106: /* Hi Power */
			sayfile(mychannel, "rpt/hipwr");
			break;
		    case 113: /* Scan down slow */
			sayfile(mychannel,"rpt/down");
			sayfile(mychannel, "rpt/slow");
			break;
		    case 114: /* Scan down quick */
			sayfile(mychannel,"rpt/down");
			sayfile(mychannel, "rpt/quick");
			break;
		    case 115: /* Scan down fast */
			sayfile(mychannel,"rpt/down");
			sayfile(mychannel, "rpt/fast");
			break;
		    case 116: /* Scan up slow */
			sayfile(mychannel,"rpt/up");
			sayfile(mychannel, "rpt/slow");
			break;
		    case 117: /* Scan up quick */
			sayfile(mychannel,"rpt/up");
			sayfile(mychannel, "rpt/quick");
			break;
		    case 118: /* Scan up fast */
			sayfile(mychannel,"rpt/up");
			sayfile(mychannel, "rpt/fast");
			break;
		    default:
			res = -1;
		}
		wait_interval(myrpt, DLY_COMP, mychannel);
		if (!res) res = telem_lookup(myrpt,mychannel, myrpt->name, "functcomplete");
		break;
	    case SCAN:
		ast_mutex_lock(&myrpt->remlock);
		if (myrpt->hfscanstop)
		{
			myrpt->hfscanstatus = 0;
			myrpt->hfscanmode = 0;
			myrpt->hfscanstop = 0;
			mytele->mode = SCANSTAT;
			ast_mutex_unlock(&myrpt->remlock);
			if (ast_safe_sleep(mychannel,1000) == -1) break;
			sayfile(mychannel, "rpt/stop");
			imdone = 1;
			break;
		}
		if (myrpt->hfscanstatus > -2) service_scan(myrpt);
		i = myrpt->hfscanstatus;
		myrpt->hfscanstatus = 0;
		if (i) mytele->mode = SCANSTAT;
		ast_mutex_unlock(&myrpt->remlock);
		if (i < 0) sayfile(mychannel, "rpt/stop");
		else if (i > 0) saynum(mychannel,i);
		imdone = 1;
		break;
	    case TUNE:
		ast_mutex_lock(&myrpt->remlock);
		if (!strcmp(myrpt->remoterig,remote_rig_ic706))
		{
			set_mode_ic706(myrpt, REM_MODE_AM);
			if(play_tone(mychannel, 800, 6000, 8192) == -1) break;
			ast_safe_sleep(mychannel,500);
			set_mode_ic706(myrpt, myrpt->remmode);
			myrpt->tunerequest = 0;
			ast_mutex_unlock(&myrpt->remlock);
			imdone = 1;
			break;
		}
		set_mode_ft897(myrpt, REM_MODE_AM);
		simple_command_ft897(myrpt, 8);
		if(play_tone(mychannel, 800, 6000, 8192) == -1) break;
		simple_command_ft897(myrpt, 0x88);
		ast_safe_sleep(mychannel,500);
		set_mode_ft897(myrpt, myrpt->remmode);
		myrpt->tunerequest = 0;
		ast_mutex_unlock(&myrpt->remlock);
		imdone = 1;
		break;
	    case REMSHORTSTATUS:
	    case REMLONGSTATUS:
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = saynode(myrpt,mychannel,myrpt->name);
		if(!res)
			res = sayfile(mychannel,"rpt/frequency");
		if(!res)
			res = split_freq(mhz, decimals, myrpt->freq);
		if (!multimode_capable(myrpt)) decimals[3] = 0;
		if(!res){
			m = atoi(mhz);
			if(m < 100)
				res = saynum(mychannel, m);
			else
				res = saycharstr(mychannel, mhz);
		}
		if(!res)
			res = sayfile(mychannel, "letters/dot");
		if(!res)
			res = saycharstr(mychannel, decimals);

		if(res)	break;
		if(myrpt->remmode == REM_MODE_FM){ /* Mode FM? */
			switch(myrpt->offset){

				case REM_MINUS:
					res = sayfile(mychannel,"rpt/minus");
					break;

				case REM_SIMPLEX:
					res = sayfile(mychannel,"rpt/simplex");
					break;

				case REM_PLUS:
					res = sayfile(mychannel,"rpt/plus");
					break;

				case REM_WILD:
					res = sayfile(mychannel,"rpt/quick");
					break;

				default:
					break;
			}
		}
		else{ /* Must be USB, LSB, or AM */
			switch(myrpt->remmode){

				case REM_MODE_USB:
					res = saycharstr(mychannel, "USB");
					break;

				case REM_MODE_LSB:
					res = saycharstr(mychannel, "LSB");
					break;

				case REM_MODE_AM:
					res = saycharstr(mychannel, "AM");
					break;


				default:
					break;
			}
		}

		if (res == -1) break;

		if(mytele->mode == REMSHORTSTATUS){ /* Short status? */
			wait_interval(myrpt, DLY_COMP, mychannel);
			if (!res) res = telem_lookup(myrpt,mychannel, myrpt->name, "functcomplete");
			break;
		}

		if (strcmp(myrpt->remoterig,remote_rig_ic706))
		{
			switch(myrpt->powerlevel){

				case REM_LOWPWR:
					res = sayfile(mychannel,"rpt/lopwr") ;
					break;
				case REM_MEDPWR:
					res = sayfile(mychannel,"rpt/medpwr");
					break;
				case REM_HIPWR:
					res = sayfile(mychannel,"rpt/hipwr");
					break;
				}
		}

		rbimode = ((!strncmp(myrpt->remoterig,remote_rig_rbi,3))
		  || (!strncmp(myrpt->remoterig,remote_rig_ic706,3)));
		if (res || (sayfile(mychannel,"rpt/rxpl") == -1)) break;
		if (rbimode && (sayfile(mychannel,"rpt/txpl") == -1)) break;
		if ((sayfile(mychannel,"rpt/frequency") == -1) ||
			(saycharstr(mychannel,myrpt->rxpl) == -1)) break;
		if ((!rbimode) && ((sayfile(mychannel,"rpt/txpl") == -1) ||
			(sayfile(mychannel,"rpt/frequency") == -1) ||
			(saycharstr(mychannel,myrpt->txpl) == -1))) break;
		if(myrpt->remmode == REM_MODE_FM){ /* Mode FM? */
			if ((sayfile(mychannel,"rpt/rxpl") == -1) ||
				(sayfile(mychannel,((myrpt->rxplon) ? "rpt/on" : "rpt/off")) == -1) ||
				(sayfile(mychannel,"rpt/txpl") == -1) ||
				(sayfile(mychannel,((myrpt->txplon) ? "rpt/on" : "rpt/off")) == -1))
				{
					break;
				}
		}
		wait_interval(myrpt, DLY_COMP, mychannel);
		if (!res) res = telem_lookup(myrpt,mychannel, myrpt->name, "functcomplete");
		break;
	    case STATUS:
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		hastx = 0;
		linkbase.next = &linkbase;
		linkbase.prev = &linkbase;
		rpt_mutex_lock(&myrpt->lock);
		/* make our own list of links */
		l = myrpt->links.next;
		while(l != &myrpt->links)
		{
			if (l->name[0] == '0')
			{
				l = l->next;
				continue;
			}
			l1 = ast_malloc(sizeof(struct rpt_link));
			if (!l1)
			{
				ast_log(LOG_WARNING, "Cannot alloc memory on %s\n", mychannel->name);
				remque((struct qelem *)mytele);
				myrpt->active_telem = NULL;
				rpt_mutex_unlock(&myrpt->lock);
				ast_log(LOG_NOTICE,"Telemetry thread aborted at line %d, mode: %d\n",__LINE__, mytele->mode); /*@@@@@@@@@@@*/
				ast_free(nodename);
				if(id_malloc)
					ast_free(ident);
				ast_free(mytele);
				ast_hangup(mychannel);
				pthread_exit(NULL);
			}
			memcpy(l1,l,sizeof(struct rpt_link));
			l1->next = l1->prev = NULL;
			insque((struct qelem *)l1,(struct qelem *)linkbase.next);
			l = l->next;
		}
		rpt_mutex_unlock(&myrpt->lock);
		res = saynode(myrpt,mychannel,myrpt->name);
		if (myrpt->callmode)
		{
			hastx = 1;
			res = ast_streamfile(mychannel, "rpt/autopatch_on", mychannel->language);
			if (!res)
				res = ast_waitstream(mychannel, "");
			else
				 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
			ast_stopstream(mychannel);
		}
		l = linkbase.next;
		while(l != &linkbase)
		{
			char *s;

			hastx = 1;
			res = saynode(myrpt,mychannel,l->name);
			s = "rpt/tranceive";
			if (!l->mode) s = "rpt/monitor";
			if (!l->thisconnected) s = "rpt/connecting";
			res = ast_streamfile(mychannel, s, mychannel->language);
			if (!res)
				res = ast_waitstream(mychannel, "");
			else
				ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
			ast_stopstream(mychannel);
			l = l->next;
		}
		if (!hastx)
		{
			res = ast_streamfile(mychannel, "rpt/repeat_only", mychannel->language);
			if (!res)
				res = ast_waitstream(mychannel, "");
			else
				 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
			ast_stopstream(mychannel);
		}
		/* destroy our local link queue */
		l = linkbase.next;
		while(l != &linkbase)
		{
			l1 = l;
			l = l->next;
			remque((struct qelem *)l1);
			ast_free(l1);
		}
		imdone = 1;
		break;
	    case FULLSTATUS:
		rpt_mutex_lock(&myrpt->lock);
		/* get all the nodes */
		__mklinklist(myrpt,NULL,lbuf);
		rpt_mutex_unlock(&myrpt->lock);
		/* parse em */
		ns = finddelim(lbuf,strs,MAXLINKLIST);
		/* sort em */
		if (ns) qsort((void *)strs,ns,sizeof(char *),mycompar);
		/* wait a little bit */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		hastx = 0;
		res = saynode(myrpt,mychannel,myrpt->name);
		if (myrpt->callmode)
		{
			hastx = 1;
			res = ast_streamfile(mychannel, "rpt/autopatch_on", mychannel->language);
			if (!res)
				res = ast_waitstream(mychannel, "");
			else
				 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
			ast_stopstream(mychannel);
		}
		/* go thru all the nodes in list */
		for(i = 0; i < ns; i++)
		{
			char *s,mode = 'T';

			/* if a mode spec at first, handle it */
			if ((*strs[i] < '0') || (*strs[i] > '9'))
			{
				mode = *strs[i];
				strs[i]++;
			}

			hastx = 1;
			res = saynode(myrpt,mychannel,strs[i]);
			s = "rpt/tranceive";
			if (mode == 'R') s = "rpt/monitor";
			if (mode == 'C') s = "rpt/connecting";
			res = ast_streamfile(mychannel, s, mychannel->language);
			if (!res)
				res = ast_waitstream(mychannel, "");
			else
				ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
			ast_stopstream(mychannel);
		}
		if (!hastx)
		{
			res = ast_streamfile(mychannel, "rpt/repeat_only", mychannel->language);
			if (!res)
				res = ast_waitstream(mychannel, "");
			else
				 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
			ast_stopstream(mychannel);
		}
		imdone = 1;
		break;

	    case LASTNODEKEY: /* Identify last node which keyed us up */
		rpt_mutex_lock(&myrpt->lock);
		if(myrpt->lastnodewhichkeyedusup){
			p = ast_strdup(myrpt->lastnodewhichkeyedusup); /* Make a local copy of the node name */
			if(!p){
				ast_log(LOG_WARNING, "ast_strdup failed in telemetry LASTNODEKEY");
				imdone = 1;
				break;
			}
		}
		else
			p = NULL;
		rpt_mutex_unlock(&myrpt->lock);
		if(!p){
			imdone = 1; /* no node previously keyed us up, or the node which did has been disconnected */
			break;
		}
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = saynode(myrpt,mychannel,p);
		ast_free(p);
		imdone = 1;
		break;

	    case UNAUTHTX: /* Say unauthorized transmit frequency */
		wait_interval(myrpt, DLY_TELEM, mychannel);
		res = ast_streamfile(mychannel, "rpt/unauthtx", mychannel->language);
		if (!res)
			res = ast_waitstream(mychannel, "");
		else
			 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
		ast_stopstream(mychannel);
		imdone = 1;
		break;

	    case PARROT: /* Repeat stuff */

		sprintf(mystr,PARROTFILE,myrpt->name,mytele->parrot);
		if (ast_fileexists(mystr,NULL,mychannel->language) <= 0)
		{
			imdone = 1;
			myrpt->parrotstate = 0;
			myrpt->parrotonce = 0;
			break;
		}
		wait_interval(myrpt, DLY_PARROT, mychannel);
		sprintf(mystr,PARROTFILE,myrpt->name,mytele->parrot);
		res = ast_streamfile(mychannel, mystr, mychannel->language);
		if (!res)
			res = ast_waitstream(mychannel, "");
		else
			 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
		ast_stopstream(mychannel);
		sprintf(mystr,PARROTFILE,myrpt->name,mytele->parrot);
		strcat(mystr,".wav");
		unlink(mystr);
		imdone = 1;
		myrpt->parrotstate = 0;
		myrpt->parrotonce = 0;
		break;

	    case TIMEOUT:
		res = saynode(myrpt,mychannel,myrpt->name);
		if (!res)
		   res = ast_streamfile(mychannel, "rpt/timeout", mychannel->language);
		break;

	    case TIMEOUT_WARNING:
		time(&t);
		res = saynode(myrpt,mychannel,myrpt->name);
		if (!res)
		   res = ast_streamfile(mychannel, "rpt/timeout-warning", mychannel->language);
		if (!res)
			res = ast_waitstream(mychannel, "");
		else
			 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
		ast_stopstream(mychannel);
		if(!res) /* Say number of seconds */
			ast_say_number(mychannel, myrpt->p.remotetimeout -
			    (t - myrpt->last_activity_time),
				"", mychannel->language, (char *) NULL);
		if (!res)
			res = ast_waitstream(mychannel, "");
		ast_stopstream(mychannel);
		res = ast_streamfile(mychannel, "queue-seconds", mychannel->language);
		break;

	    case ACT_TIMEOUT_WARNING:
		time(&t);
		res = saynode(myrpt,mychannel,myrpt->name);
		if (!res)
		    res = ast_streamfile(mychannel, "rpt/act-timeout-warning", mychannel->language);
		if (!res)
			res = ast_waitstream(mychannel, "");
		else
			 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
		ast_stopstream(mychannel);
		if(!res) /* Say number of seconds */
			ast_say_number(mychannel, myrpt->p.remoteinacttimeout -
			    (t - myrpt->last_activity_time),
				"", mychannel->language, (char *) NULL);
		if (!res)
			res = ast_waitstream(mychannel, "");
		ast_stopstream(mychannel);
		res = ast_streamfile(mychannel, "queue-seconds", mychannel->language);
		break;

	    case STATS_TIME:
            case STATS_TIME_LOCAL:
	    	wait_interval(myrpt, DLY_TELEM, mychannel); /* Wait a little bit */
		t = time(NULL);
		rpt_localtime(&t, &localtm);
		/* Say the phase of the day is before the time */
		if((localtm.tm_hour >= 0) && (localtm.tm_hour < 12))
			p = "rpt/goodmorning";
		else if((localtm.tm_hour >= 12) && (localtm.tm_hour < 18))
			p = "rpt/goodafternoon";
		else
			p = "rpt/goodevening";
		if (sayfile(mychannel,p) == -1)
		{
			imdone = 1;
			break;
		}
		/* Say the time is ... */
		if (sayfile(mychannel,"rpt/thetimeis") == -1)
		{
			imdone = 1;
			break;
		}
		/* Say the time */
	    	res = ast_say_time(mychannel, t, "", mychannel->language);
		if (!res)
			res = ast_waitstream(mychannel, "");
		ast_stopstream(mychannel);
		imdone = 1;
	    	break;
	    case STATS_VERSION:
		p = strstr(tdesc, "version");
		if(!p)
			break;
		if(sscanf(p, "version %d.%d", &vmajor, &vminor) != 2)
			break;
    		wait_interval(myrpt, DLY_TELEM, mychannel); /* Wait a little bit */
		/* Say "version" */
		if (sayfile(mychannel,"rpt/version") == -1)
		{
			imdone = 1;
			break;
		}
		if(!res) /* Say "X" */
			ast_say_number(mychannel, vmajor, "", mychannel->language, (char *) NULL);
		if (!res)
			res = ast_waitstream(mychannel, "");
		ast_stopstream(mychannel);
		if (saycharstr(mychannel,".") == -1)
		{
			imdone = 1;
			break;
		}
		if(!res) /* Say "Y" */
			ast_say_number(mychannel, vminor, "", mychannel->language, (char *) NULL);
		if (!res){
			res = ast_waitstream(mychannel, "");
			ast_stopstream(mychannel);
		}
		else
			 ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
		imdone = 1;
	    	break;
	    case ARB_ALPHA:
	    	wait_interval(myrpt, DLY_TELEM, mychannel); /* Wait a little bit */
	    	if(mytele->param)
	    		saycharstr(mychannel, mytele->param);
	    	imdone = 1;
		break;
	    case REV_PATCH:
	    	wait_interval(myrpt, DLY_TELEM, mychannel); /* Wait a little bit */
	    	if(mytele->param) {

			/* Parts of this section taken from app_parkandannounce */
			char *tpl_working, *tpl_current;
			char *tmp[100], *myparm;
			int looptemp=0,i=0, dres = 0;

			tpl_working = ast_strdup(mytele->param);
			myparm = strsep(&tpl_working,",");
			tpl_current=strsep(&tpl_working, ":");

			while(tpl_current && looptemp < sizeof(tmp)) {
				tmp[looptemp]=tpl_current;
				looptemp++;
				tpl_current=strsep(&tpl_working,":");
			}

			for(i=0; i<looptemp; i++) {
				if(!strcmp(tmp[i], "PARKED")) {
					ast_say_digits(mychannel, atoi(myparm), "", mychannel->language);
				} else if(!strcmp(tmp[i], "NODE")) {
					ast_say_digits(mychannel, atoi(myrpt->name), "", mychannel->language);
				} else {
					dres = ast_streamfile(mychannel, tmp[i], mychannel->language);
					if(!dres) {
						dres = ast_waitstream(mychannel, "");
					} else {
						ast_log(LOG_WARNING, "ast_streamfile of %s failed on %s\n", tmp[i], mychannel->name);
						dres = 0;
					}
				}
			}
			ast_free(tpl_working);
		}
	    	imdone = 1;
		break;
	    case TEST_TONE:
		imdone = 1;
		if (myrpt->stopgen) break;
		myrpt->stopgen = -1;
	        if ((res = ast_tonepair_start(mychannel, 1004.0, 0, 99999999, 7200.0)))
		{
			myrpt->stopgen = 0;
			break;
		}
	        while(mychannel->generatordata && (myrpt->stopgen <= 0)) {
			if (ast_safe_sleep(mychannel,1)) break;
		    	imdone = 1;
			}
		myrpt->stopgen = 0;
		break;
	    default:
	    	break;
	}
	if (!imdone)
	{
		if (!res)
			res = ast_waitstream(mychannel, "");
		else {
			ast_log(LOG_WARNING, "ast_streamfile failed on %s\n", mychannel->name);
			res = 0;
		}
	}
	ast_stopstream(mychannel);
	rpt_mutex_lock(&myrpt->lock);
	if (mytele->mode == TAILMSG)
	{
		if (!res)
		{
			myrpt->tailmessagen++;
			if(myrpt->tailmessagen >= myrpt->p.tailmessagemax) myrpt->tailmessagen = 0;
		}
		else
		{
			myrpt->tmsgtimer = myrpt->p.tailsquashedtime;
		}
	}
	remque((struct qelem *)mytele);
	myrpt->active_telem = NULL;
	rpt_mutex_unlock(&myrpt->lock);
	ast_free(nodename);
	if(id_malloc)
		ast_free(ident);
	ast_free(mytele);
	ast_hangup(mychannel);
#ifdef  APP_RPT_LOCK_DEBUG
	{
		struct lockthread *t;

		sleep(5);
		ast_mutex_lock(&locklock);
		t = get_lockthread(pthread_self());
		if (t) memset(t,0,sizeof(struct lockthread));
		ast_mutex_unlock(&locklock);
	}
#endif
	myrpt->noduck=0;
	if(debug)
		ast_log(LOG_NOTICE,"[%s] exit\n",myrpt->name);

	pthread_exit(NULL);
}
/* rpt_tele_thread() end  ***********************************************	*/

static void send_tele_link(struct rpt *myrpt,char *cmd);

static void rpt_telemetry(struct rpt *myrpt,int mode, void *data)
{
struct rpt_tele *tele;
struct rpt_link *mylink = NULL;
int res,vmajor,vminor,i;
pthread_attr_t attr;
char *v1, *v2,mystr[300],*p,haslink;
time_t	t;
struct rpt_link *l;

	if(debug)
		ast_log(LOG_NOTICE,"[%s] start mode=%i telemmode=%i remote=%d\n",
			myrpt->name,mode,myrpt->telemmode,myrpt->remote);

	switch(mode)
	{
	    case REMDISC:
		mylink = (struct rpt_link *) data;
		if ((!mylink) || (mylink->name[0] == '0'))
		{
		    /* do not announce if not linked */
			if(debug)
				ast_log(LOG_NOTICE,"mylink=%i\n",mylink?1:0);
			return;
		}
		if ((!mylink->gott) && (mylink->isremote))
		{
			/* do not announce if no T messages received and link it to a remote */
		    if(debug)
				ast_log(LOG_NOTICE,"[%s] gott=%d isremote=%d\n",myrpt->name,mylink->gott,mylink->isremote);
			return;
		}
		break;
	    case VARCMD:
		if (myrpt->telemmode < 2) return;
		break;
	    case UNKEY:
	    case LOCUNKEY:
		/* if voting and the main rx unkeys but a voter link is still active */
		if(myrpt->p.votertype==1 && (myrpt->rxchankeyed || myrpt->voteremrx ))
		{
			return;
		}
		if (myrpt->p.nounkeyct) return;
		/* if any of the following are defined, go ahead and do it,
		   otherwise, don't bother */
		v1 = (char *) ast_variable_retrieve(myrpt->cfg, myrpt->name,
			"unlinkedct");
		v2 = (char *) ast_variable_retrieve(myrpt->cfg, myrpt->name,
			"remotect");
		if (telem_lookup(myrpt,NULL, myrpt->name, "remotemon") &&
		  telem_lookup(myrpt,NULL, myrpt->name, "remotetx") &&
		  telem_lookup(myrpt,NULL, myrpt->name, "cmdmode") &&
		  (!(v1 && telem_lookup(myrpt,NULL, myrpt->name, v1))) &&
		  (!(v2 && telem_lookup(myrpt,NULL, myrpt->name, v2)))) return;
		break;
	    case LINKUNKEY:
		if (!ast_variable_retrieve(myrpt->cfg, myrpt->name, "linkunkeyct"))
			return;
		break;
	    default:
		break;
	}
	l = myrpt->links.next;
	res = 0;
	if (l && (l != &myrpt->links))
	{
		if (l->chan && (l->name[0] != '0') && (l->newkey == 1)) res = 1;
		l = l->next;
	}
	if (myrpt->remote) res = 1;
	if (!res) /* no-one on without newkey */
	{
		if(debug)ast_log(LOG_NOTICE,"[%s] remote=%d\n",myrpt->name,myrpt->remote);

		/* send appropriate commands to everyone on link(s) */
		switch(mode)
		{
		    case REMGO:
			send_tele_link(myrpt,"REMGO");
			return;
		    case REMALREADY:
			send_tele_link(myrpt,"REMALREADY");
			return;
		    case REMNOTFOUND:
			send_tele_link(myrpt,"REMNOTFOUND");
			return;
		    case COMPLETE:
			send_tele_link(myrpt,"COMPLETE");
			return;
		    case PROC:
			send_tele_link(myrpt,"PROC");
			return;
		    case TERM:
			send_tele_link(myrpt,"TERM");
			return;
		    case MACRO_NOTFOUND:
			send_tele_link(myrpt,"MACRO_NOTFOUND");
			return;
		    case MACRO_BUSY:
			send_tele_link(myrpt,"MACRO_BUSY");
			return;
		    case CONNECTED:
			mylink = (struct rpt_link *) data;
			if ((!mylink) || (mylink->name[0] == '0')) return;
			sprintf(mystr,"CONNECTED,%s,%s",myrpt->name,mylink->name);
			send_tele_link(myrpt,mystr);
			return;
		    case CONNFAIL:
			mylink = (struct rpt_link *) data;
			if ((!mylink) || (mylink->name[0] == '0')) return;
			sprintf(mystr,"CONNFAIL,%s",mylink->name);
			send_tele_link(myrpt,mystr);
			return;
		    case REMDISC:
			mylink = (struct rpt_link *) data;
			if ((!mylink) || (mylink->name[0] == '0')) return;
			l = myrpt->links.next;
			haslink = 0;
			/* don't report if a link for this one still on system */
			if (l != &myrpt->links)
			{
				rpt_mutex_lock(&myrpt->lock);
				while(l != &myrpt->links)
				{
					if (l->name[0] == '0')
					{
						l = l->next;
						continue;
					}
					if (!strcmp(l->name,mylink->name))
					{
						haslink = 1;
						break;
					}
					l = l->next;
				}
				rpt_mutex_unlock(&myrpt->lock);
			}
			if(debug)ast_log(LOG_NOTICE,"[%s] haslink=%d\n",myrpt->name,haslink);
			if (haslink) return;
			sprintf(mystr,"REMDISC,%s",mylink->name);
			send_tele_link(myrpt,mystr);
			return;
		    case STATS_TIME:
			t = time(NULL);
			sprintf(mystr,"STATS_TIME,%u",(unsigned int) t);
			send_tele_link(myrpt,mystr);
			return;
		    case STATS_VERSION:
			p = strstr(tdesc, "version");
			if (!p) return;
			if(sscanf(p, "version %d.%d", &vmajor, &vminor) != 2)
				return;
			sprintf(mystr,"STATS_VERSION,%d.%d",vmajor,vminor);
			send_tele_link(myrpt,mystr);
			return;
		    case ARB_ALPHA:
			sprintf(mystr,"ARB_ALPHA,%s",(char *)data);
			send_tele_link(myrpt,mystr);
			return;
		    case REV_PATCH:
			p = (char *)data;
			for(i = 0; p[i]; i++) if (p[i] == ',') p[i] = '^';
			sprintf(mystr,"REV_PATCH,%s,%s",myrpt->name,p);
			send_tele_link(myrpt,mystr);
			return;
		    case LASTNODEKEY:
			if (!myrpt->lastnodewhichkeyedusup[0]) return;
			sprintf(mystr,"LASTNODEKEY,%s",myrpt->lastnodewhichkeyedusup);
			send_tele_link(myrpt,mystr);
			return;
		    case STATUS:
			rpt_mutex_lock(&myrpt->lock);
			sprintf(mystr,"STATUS,%s,%d",myrpt->name,myrpt->callmode);
			/* make our own list of links */
			l = myrpt->links.next;
			while(l != &myrpt->links)
			{
				char s;

				if (l->name[0] == '0')
				{
					l = l->next;
					continue;
				}
				s = 'T';
				if (!l->mode) s = 'R';
				if (!l->thisconnected) s = 'C';
				snprintf(mystr + strlen(mystr),sizeof(mystr),",%c%s",
					s,l->name);
				l = l->next;
			}
			rpt_mutex_unlock(&myrpt->lock);
			send_tele_link(myrpt,mystr);
			return;
		}
	}
	tele = ast_malloc(sizeof(struct rpt_tele));
	if (!tele)
	{
		ast_log(LOG_WARNING, "Unable to allocate memory\n");
		pthread_exit(NULL);
		return;
	}
	/* zero it out */
	memset((char *)tele,0,sizeof(struct rpt_tele));
	tele->rpt = myrpt;
	tele->mode = mode;
	if (mode == PARROT) tele->parrot = (unsigned int) data;
	else mylink = (struct rpt_link *) data;
	rpt_mutex_lock(&myrpt->lock);
	if((mode == CONNFAIL) || (mode == REMDISC) || (mode == CONNECTED) ||
	    (mode == LINKUNKEY)){
		memset(&tele->mylink,0,sizeof(struct rpt_link));
		if (mylink){
			memcpy(&tele->mylink,mylink,sizeof(struct rpt_link));
		}
	}
	else if ((mode == ARB_ALPHA) || (mode == REV_PATCH) ||
	    (mode == PLAYBACK) || (mode == LOCALPLAY) ||
            (mode == VARCMD) || (mode == METER) || (mode == USEROUT)) {
		strncpy(tele->param, (char *) data, TELEPARAMSIZE - 1);
		tele->param[TELEPARAMSIZE - 1] = 0;
	}
	if ((mode == REMXXX) || (mode == PAGE)) tele->submode = (int) data;
	insque((struct qelem *)tele, (struct qelem *)myrpt->tele.next);
	rpt_mutex_unlock(&myrpt->lock);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	res = ast_pthread_create(&tele->threadid,&attr,rpt_tele_thread,(void *) tele);
	if(res < 0){
		rpt_mutex_lock(&myrpt->lock);
		remque((struct qlem *) tele); /* We don't like stuck transmitters, remove it from the queue */
		rpt_mutex_unlock(&myrpt->lock);
		ast_log(LOG_WARNING, "Could not create telemetry thread: %s",strerror(res));
	}
	if(debug)
		ast_log(LOG_NOTICE,"function return\n");

	return;
}

static void *rpt_call(void *this)
{
struct dahdi_confinfo ci;  /* conference info */
struct	rpt *myrpt = (struct rpt *)this;
int	res;
int stopped,congstarted,dialtimer,lastcidx,aborted;
struct ast_channel *mychannel,*genchannel;

	myrpt->mydtmf = 0;
	/* allocate a pseudo-channel thru asterisk */
	mychannel = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
	if (!mychannel)
	{
		fprintf(stderr,"rpt:Sorry unable to obtain pseudo channel\n");
		pthread_exit(NULL);
	}
#ifdef	AST_CDR_FLAG_POST_DISABLED
	if (mychannel->cdr)
		ast_set_flag(mychannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
	ci.chan = 0;
	ci.confno = myrpt->conf; /* use the pseudo conference */
	ci.confmode = DAHDI_CONF_CONF | DAHDI_CONF_TALKER | DAHDI_CONF_LISTENER;
	/* first put the channel on the conference */
	if (ioctl(mychannel->fds[0],DAHDI_SETCONF,&ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
		ast_hangup(mychannel);
		myrpt->callmode = 0;
		pthread_exit(NULL);
	}
	/* allocate a pseudo-channel thru asterisk */
	genchannel = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
	if (!genchannel)
	{
		fprintf(stderr,"rpt:Sorry unable to obtain pseudo channel\n");
		ast_hangup(mychannel);
		pthread_exit(NULL);
	}
#ifdef	AST_CDR_FLAG_POST_DISABLED
	if (genchannel->cdr)
		ast_set_flag(genchannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
	ci.chan = 0;
	ci.confno = myrpt->conf;
	ci.confmode = DAHDI_CONF_CONF | DAHDI_CONF_TALKER | DAHDI_CONF_LISTENER;
	/* first put the channel on the conference */
	if (ioctl(genchannel->fds[0],DAHDI_SETCONF,&ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
		ast_hangup(mychannel);
		ast_hangup(genchannel);
		myrpt->callmode = 0;
		pthread_exit(NULL);
	}
	if (myrpt->p.tonezone && (tone_zone_set_zone(mychannel->fds[0],myrpt->p.tonezone) == -1))
	{
		ast_log(LOG_WARNING, "Unable to set tone zone %s\n",myrpt->p.tonezone);
		ast_hangup(mychannel);
		ast_hangup(genchannel);
		myrpt->callmode = 0;
		pthread_exit(NULL);
	}
	if (myrpt->p.tonezone && (tone_zone_set_zone(genchannel->fds[0],myrpt->p.tonezone) == -1))
	{
		ast_log(LOG_WARNING, "Unable to set tone zone %s\n",myrpt->p.tonezone);
		ast_hangup(mychannel);
		ast_hangup(genchannel);
		myrpt->callmode = 0;
		pthread_exit(NULL);
	}
	/* start dialtone if patchquiet is 0. Special patch modes don't send dial tone */
	if ((!myrpt->patchquiet) && (tone_zone_play_tone(genchannel->fds[0],DAHDI_TONE_DIALTONE) < 0))
	{
		ast_log(LOG_WARNING, "Cannot start dialtone\n");
		ast_hangup(mychannel);
		ast_hangup(genchannel);
		myrpt->callmode = 0;
		pthread_exit(NULL);
	}
	stopped = 0;
	congstarted = 0;
	dialtimer = 0;
	lastcidx = 0;
	myrpt->calldigittimer = 0;
	aborted = 0;

	while ((myrpt->callmode == 1) || (myrpt->callmode == 4))
	{
		if((myrpt->patchdialtime)&&(myrpt->callmode == 1)&&(myrpt->cidx != lastcidx)){
			dialtimer = 0;
			lastcidx = myrpt->cidx;
		}

		if((myrpt->patchdialtime)&&(dialtimer >= myrpt->patchdialtime)){
		    if(debug)
		    	ast_log(LOG_NOTICE, "dialtimer %i > patchdialtime %i\n", dialtimer,myrpt->patchdialtime);
			rpt_mutex_lock(&myrpt->lock);
			aborted = 1;
			myrpt->callmode = 0;
			rpt_mutex_unlock(&myrpt->lock);
			break;
		}

		if ((!myrpt->patchquiet) && (!stopped) && (myrpt->callmode == 1) && (myrpt->cidx > 0))
		{
			stopped = 1;
			/* stop dial tone */
			tone_zone_play_tone(genchannel->fds[0],-1);
		}
		if (myrpt->callmode == 1)
		{
			if(myrpt->calldigittimer > PATCH_DIALPLAN_TIMEOUT)
			{
			    if (debug)ast_log(LOG_NOTICE,"PATCH_DIALPLAN_TIMEOUT\n");
				myrpt->callmode = 2;
				break;
			}
			/* bump timer if active */
			if (myrpt->calldigittimer)
				myrpt->calldigittimer += MSWAIT;
		}
		if (myrpt->callmode == 4)
		{
			if(!congstarted){
				congstarted = 1;
				/* start congestion tone */
				tone_zone_play_tone(genchannel->fds[0],DAHDI_TONE_CONGESTION);
			}
		}
		res = ast_safe_sleep(mychannel, MSWAIT);
		if (res < 0)
		{
		    if(debug)
		    	ast_log(LOG_NOTICE, "ast_safe_sleep=%i\n", res);
			ast_hangup(mychannel);
			ast_hangup(genchannel);
			rpt_mutex_lock(&myrpt->lock);
			myrpt->callmode = 0;
			rpt_mutex_unlock(&myrpt->lock);
			pthread_exit(NULL);
		}
		dialtimer += MSWAIT;
	}
	/* stop any tone generation */
	tone_zone_play_tone(genchannel->fds[0],-1);
	/* end if done */
	if (!myrpt->callmode)
	{
		if(debug)
			ast_log(LOG_NOTICE, "callmode==0\n");
		ast_hangup(mychannel);
		ast_hangup(genchannel);
		rpt_mutex_lock(&myrpt->lock);
		myrpt->callmode = 0;
		myrpt->macropatch=0;
		channel_revert(myrpt);
		rpt_mutex_unlock(&myrpt->lock);
		if((!myrpt->patchquiet) && aborted)
			rpt_telemetry(myrpt, TERM, NULL);
		pthread_exit(NULL);
	}

	if (myrpt->p.ourcallerid && *myrpt->p.ourcallerid){
		char *name, *loc, *instr;
		instr = ast_strdup(myrpt->p.ourcallerid);
		if(instr){
			ast_callerid_parse(instr, &name, &loc);
			if(loc){
				if(mychannel->cid.cid_num)
					ast_free(mychannel->cid.cid_num);
				mychannel->cid.cid_num = ast_strdup(loc);
			}
			if(name){
				if(mychannel->cid.cid_name)
					ast_free(mychannel->cid.cid_name);
				mychannel->cid.cid_name = ast_strdup(name);
			}
			ast_free(instr);
		}
	}

	ast_copy_string(mychannel->exten, myrpt->exten, sizeof(mychannel->exten) - 1);
	ast_copy_string(mychannel->context, myrpt->patchcontext, sizeof(mychannel->context) - 1);

	if (myrpt->p.acctcode)
		ast_cdr_setaccount(mychannel,myrpt->p.acctcode);
	mychannel->priority = 1;
	ast_channel_undefer_dtmf(mychannel);
	if (ast_pbx_start(mychannel) < 0)
	{
		ast_log(LOG_WARNING, "Unable to start PBX!!\n");
		ast_hangup(mychannel);
		ast_hangup(genchannel);
		rpt_mutex_lock(&myrpt->lock);
	 	myrpt->callmode = 0;
		rpt_mutex_unlock(&myrpt->lock);
		pthread_exit(NULL);
	}
	usleep(10000);
	rpt_mutex_lock(&myrpt->lock);
	myrpt->callmode = 3;
	/* set appropriate conference for the pseudo */
	ci.chan = 0;
	ci.confno = myrpt->conf;
	ci.confmode = (myrpt->p.duplex == 2) ? DAHDI_CONF_CONFANNMON :
		(DAHDI_CONF_CONF | DAHDI_CONF_LISTENER | DAHDI_CONF_TALKER);
	/* first put the channel on the conference in announce mode */
	if (ioctl(myrpt->pchannel->fds[0],DAHDI_SETCONF,&ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
		ast_hangup(mychannel);
		ast_hangup(genchannel);
		myrpt->callmode = 0;
		pthread_exit(NULL);
	}
	/* get its channel number */
	if (ioctl(mychannel->fds[0],DAHDI_CHANNO,&res) == -1)
	{
		ast_log(LOG_WARNING, "Unable to get autopatch channel number\n");
		ast_hangup(mychannel);
		myrpt->callmode = 0;
		pthread_exit(NULL);
	}
	ci.chan = 0;
	ci.confno = res;
	ci.confmode = DAHDI_CONF_MONITOR;
	/* put vox channel monitoring on the channel  */
	if (ioctl(myrpt->voxchannel->fds[0],DAHDI_SETCONF,&ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
		ast_hangup(mychannel);
		myrpt->callmode = 0;
		pthread_exit(NULL);
	}
	while(myrpt->callmode)
	{
		if ((!mychannel->pbx) && (myrpt->callmode != 4))
		{
		    /* If patch is setup for far end disconnect */
			if(myrpt->patchfarenddisconnect || (myrpt->p.duplex < 2)){
				if(debug)ast_log(LOG_NOTICE,"callmode=%i, patchfarenddisconnect=%i, duplex=%i\n",\
						myrpt->callmode,myrpt->patchfarenddisconnect,myrpt->p.duplex);
				myrpt->callmode = 0;
				myrpt->macropatch=0;
				if(!myrpt->patchquiet){
					rpt_mutex_unlock(&myrpt->lock);
					rpt_telemetry(myrpt, TERM, NULL);
					rpt_mutex_lock(&myrpt->lock);
				}
			}
			else{ /* Send congestion until patch is downed by command */
				myrpt->callmode = 4;
				rpt_mutex_unlock(&myrpt->lock);
				/* start congestion tone */
				tone_zone_play_tone(genchannel->fds[0],DAHDI_TONE_CONGESTION);
				rpt_mutex_lock(&myrpt->lock);
			}
		}
		if (myrpt->mydtmf)
		{
			struct ast_channel *c;
			struct ast_frame wf = {AST_FRAME_DTMF, } ;

			wf.subclass = myrpt->mydtmf;
			c = ast_bridged_channel(mychannel);
			if (c && c->_state == AST_STATE_UP)
			{
				rpt_mutex_unlock(&myrpt->lock);
				ast_queue_frame(mychannel,&wf);
#ifdef	NEW_ASTERISK
				ast_senddigit(genchannel,myrpt->mydtmf,0);
#else
				ast_senddigit(genchannel,myrpt->mydtmf);
#endif
				rpt_mutex_lock(&myrpt->lock);
			}
			myrpt->mydtmf = 0;
		}
		rpt_mutex_unlock(&myrpt->lock);
		usleep(MSWAIT * 1000);
		rpt_mutex_lock(&myrpt->lock);
	}
	if(debug)
		ast_log(LOG_NOTICE, "exit channel loop\n");
	rpt_mutex_unlock(&myrpt->lock);
	tone_zone_play_tone(genchannel->fds[0],-1);
	if (mychannel->pbx) ast_softhangup(mychannel,AST_SOFTHANGUP_DEV);
	ast_hangup(genchannel);
	rpt_mutex_lock(&myrpt->lock);
	myrpt->callmode = 0;
	myrpt->macropatch=0;
	channel_revert(myrpt);
	rpt_mutex_unlock(&myrpt->lock);
	/* set appropriate conference for the pseudo */
	ci.chan = 0;
	ci.confno = myrpt->conf;
	ci.confmode = ((myrpt->p.duplex == 2) || (myrpt->p.duplex == 4)) ? DAHDI_CONF_CONFANNMON :
		(DAHDI_CONF_CONF | DAHDI_CONF_LISTENER | DAHDI_CONF_TALKER);
	/* first put the channel on the conference in announce mode */
	if (ioctl(myrpt->pchannel->fds[0],DAHDI_SETCONF,&ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
	}
	pthread_exit(NULL);
}

static void send_link_dtmf(struct rpt *myrpt,char c)
{
char	str[300];
struct	ast_frame wf;
struct	rpt_link *l;

	if(debug)ast_log(LOG_NOTICE,"[%s] dtmf=%c\n",myrpt->name,c);

	snprintf(str, sizeof(str), "D %s %s %d %c", myrpt->cmdnode, myrpt->name, ++(myrpt->dtmfidx), c);
	wf.frametype = AST_FRAME_TEXT;
	wf.subclass = 0;
	wf.offset = 0;
	wf.mallocd = 0;
	wf.datalen = strlen(str) + 1;
	wf.samples = 0;
	wf.src = "send_link_dtmf";
	l = myrpt->links.next;
	/* first, see if our dude is there */
	while(l != &myrpt->links)
	{
		if (l->name[0] == '0') 		/* implies iaxrpt client */
		{
			l = l->next;
			continue;
		}
		/* if we found it, write it and were done */
		if (!strcmp(l->name,myrpt->cmdnode))
		{
			wf.data = str;
			if (l->chan) rpt_qwrite(l,&wf);
			return;
		}
		l = l->next;
	}
	l = myrpt->links.next;
	/* if not, give it to everyone */
	while(l != &myrpt->links)
	{
		wf.data = str;
		if (l->chan) rpt_qwrite(l,&wf);
		l = l->next;
	}
	return;
}

static void send_link_keyquery(struct rpt *myrpt)
{
char	str[300];
struct	ast_frame wf;
struct	rpt_link *l;

	rpt_mutex_lock(&myrpt->lock);
	memset(myrpt->topkey,0,sizeof(myrpt->topkey));
	myrpt->topkeystate = 1;
	time(&myrpt->topkeytime);
	rpt_mutex_unlock(&myrpt->lock);
	snprintf(str, sizeof(str), "K? * %s 0 0", myrpt->name);
	wf.frametype = AST_FRAME_TEXT;
	wf.subclass = 0;
	wf.offset = 0;
	wf.mallocd = 0;
	wf.datalen = strlen(str) + 1;
	wf.samples = 0;
	wf.src = "send_link_keyquery";
	l = myrpt->links.next;
	/* give it to everyone */
	while(l != &myrpt->links)
	{
		wf.data = str;
		if (l->chan) rpt_qwrite(l,&wf);
		l = l->next;
	}
	return;
}

static void send_tele_link(struct rpt *myrpt,char *cmd)
{
char	str[400];
struct	ast_frame wf;
struct	rpt_link *l;

	if(debug)ast_log(LOG_NOTICE, "[%s] start %s\n",myrpt->name,cmd);
	snprintf(str, sizeof(str) - 1, "T %s %s", myrpt->name,cmd);
	wf.frametype = AST_FRAME_TEXT;
	wf.subclass = 0;
	wf.offset = 0;
	wf.mallocd = 0;
	wf.datalen = strlen(str) + 1;
	wf.samples = 0;
	wf.src = "send_tele_link";
	l = myrpt->links.next;
	/* give it to everyone */
	while(l != &myrpt->links)
	{
		wf.data = str;
		if (l->chan && l->mode) rpt_qwrite(l,&wf);
		l = l->next;
	}
	if(!myrpt->localteleminhibit)
		rpt_telemetry(myrpt,VARCMD,cmd);
	myrpt->localteleminhibit=0;
	if(debug)ast_log(LOG_NOTICE, "[%s] return %s\n",myrpt->name,cmd);
	return;
}

/*	send iaxkey information after called end answers
    indicate iaxkey = 1, iaxrpt = 0, csmonitor = 0, spare = 0
*/
static void send_iaxkey(struct ast_channel *chan)
{
	char str[32];
	if(debug > 3)
		ast_log(LOG_NOTICE, "send_iaxkey to chan %s\n",chan->name);

	sprintf(str,"%s 1 0 0 0",iaxkeystr);
	ast_sendtext(chan,str);
	return;
}


/* send newkey request */

static void send_newkey(struct ast_channel *chan)
{
	if(debug > 3)
		ast_log(LOG_NOTICE, "send_newkey to chan %s\n",chan->name);
	ast_sendtext(chan,newkey1str);

	send_iaxkey(chan);

	return;
}

static void send_old_newkey(struct ast_channel *chan)
{
	if(debug > 3)
		ast_log(LOG_NOTICE, "send_old_newkey to chan %s\n",chan->name);
	ast_sendtext(chan,newkeystr);
	return;
}


/*
 * Connect a link
 *
 * Return values:
 * -2: Attempt to connect to self
 * -1: No such node
 *  0: Success
 *  1: No match yet
 *  2: Already connected to this node
 */

static int connect_link(struct rpt *myrpt, char* node, int mode, int perma)
{
	char *val, *s, *s1, *s2, *tele,*cp;
	char lstr[MAXLINKLIST],*strs[MAXLINKLIST];
	char tmp[300], deststr[300] = "",modechange = 0;
	char sx[320],*sy;
	struct rpt_link *l;
	int reconnects = 0;
	int i,n;
	int voterlink=0;
	struct dahdi_confinfo ci;  /* conference info */

	/* by convention, nodes beginning with 4 and 5 digits long are IRLP */
	if (node[0] == '4')
	{
		if (strlen(node) < 5) return 1;
		sprintf(tmp,"irlp/%d,NONE",atoi(node + 1));
	}
	/* by convention, nodes beginning with 3 are EchoLink */
	else if (node[0] != '3')
	{
		val = node_lookup(myrpt,node);
		if (!val){
			if(strlen(node) >= myrpt->longestnode)
				return -1; /* No such node */
			return 1; /* No match yet */
		}
		strncpy(tmp,val,sizeof(tmp) - 1);
	}
	else
	{
		char actstr[10],dbstr[40],str1[40];

		if (strlen(node) < 7) return 1;
		if (ast_db_get(EL_DB_ROOT,"active",actstr,sizeof(actstr) - 1))
			return -1;
		sprintf(dbstr,"%s/nodenum/%d",actstr,atoi(node + 1));
		if (ast_db_get(EL_DB_ROOT,dbstr,str1,sizeof(str1))) return -1;
		sprintf(tmp,"echolink/el0/%s,%s",str1,str1);
	}

	if(!strcmp(myrpt->name,node)) /* Do not allow connections to self */
		return -2;

	if(debug > 3)
	{
		ast_log(LOG_NOTICE,"node=%s Connect attempt to node %s\n", myrpt->name,node);
		ast_log(LOG_NOTICE,"Mode: %s\n",(mode)?"Transceive":"Monitor");
		ast_log(LOG_NOTICE,"Connection type: %s\n",(perma)?"Permalink":"Normal");
	}

	s = tmp;
	s1 = strsep(&s,",");
	if (!strchr(s1,':') && strchr(s1,'/') && strncasecmp(s1, "local/", 6) &&
		strncasecmp(s1,"echolink/",9) && strncasecmp(s1,"irlp/",5))
	{
		sy = strchr(s1,'/');
		*sy = 0;
		sprintf(sx,"%s:4569/%s",s1,sy + 1);
		s1 = sx;
	}
	s2 = strsep(&s,",");

	if(debug)ast_log(LOG_NOTICE,"s2=%s s=%s\n",s2,s);
	if(s && strcmp(s,"VOTE")==0)
	{
		voterlink=1;
		if(debug)ast_log(LOG_NOTICE,"NODE is a VOTER.\n");
	}
	rpt_mutex_lock(&myrpt->lock);
	l = myrpt->links.next;
	/* try to find this one in queue */
	while(l != &myrpt->links){
		if (l->name[0] == '0')
		{
			l = l->next;
			continue;
		}
	/* if found matching string */
		if (!strcmp(l->name, node))
			break;
		l = l->next;
	}
	/* if found */
	if (l != &myrpt->links){
	/* if already in this mode, just ignore */
		if ((l->mode == mode) || (!l->chan)) {
			rpt_mutex_unlock(&myrpt->lock);
			return 2; /* Already linked */
		}
		if ((!strncasecmp(l->chan->name,"echolink",8)) ||
			(!strncasecmp(l->chan->name,"irlp",4)))
		{
			l->mode = mode;
			strncpy(myrpt->lastlinknode,node,MAXNODESTR - 1);
			rpt_mutex_unlock(&myrpt->lock);
			return 0;
		}
		reconnects = l->reconnects;
		rpt_mutex_unlock(&myrpt->lock);
		if (l->chan) ast_softhangup(l->chan, AST_SOFTHANGUP_DEV);
		l->retries = l->max_retries + 1;
		l->disced = 2;
		modechange = 1;
	}
	else
	{
		__mklinklist(myrpt,NULL,lstr);
		rpt_mutex_unlock(&myrpt->lock);
		n = finddelim(lstr,strs,MAXLINKLIST);
		for(i = 0; i < n; i++)
		{
			if ((*strs[i] < '0') ||
			    (*strs[i] > '9')) strs[i]++;
			if (!strcmp(strs[i],node))
			{
				return 2; /* Already linked */
			}
		}
	}
	strncpy(myrpt->lastlinknode,node,MAXNODESTR - 1);
	/* establish call */
	l = ast_malloc(sizeof(struct rpt_link));
	if (!l)
	{
		ast_log(LOG_WARNING, "Unable to malloc\n");
		return -1;
	}
	/* zero the new link structure */
	memset((char *)l,0,sizeof(struct rpt_link));
	l->parent=(void *)myrpt;
	l->mode = mode;
	l->outbound = 1;
	l->thisconnected = 0;
	voxinit_link(l,1);
	strncpy(l->name, node, MAXNODESTR - 1);
	l->isremote = (s && ast_true(s));
	if (modechange) l->connected = 1;
	l->hasconnected = l->perma = perma;
	l->newkeytimer = NEWKEYTIME;
	l->newkey = 2;
	l->voterlink=voterlink;
	if ((strncasecmp(s1,"echolink/",9) == 0) || (strncasecmp(s1,"irlp/",5) == 0)) l->newkey = 0;
#ifdef ALLOW_LOCAL_CHANNELS
	if ((strncasecmp(s1,"iax2/", 5) == 0) || (strncasecmp(s1, "local/", 6) == 0) ||
	    (strncasecmp(s1,"echolink/",9) == 0) || (strncasecmp(s1,"irlp/",5) == 0))
#else
	if ((strncasecmp(s1,"iax2/", 5) == 0) || (strncasecmp(s1,"echolink/",9) == 0) ||
	    (strncasecmp(s1,"irlp/",5) == 0))
#endif
        	strncpy(deststr, s1, sizeof(deststr));
	else
	        snprintf(deststr, sizeof(deststr), "IAX2/%s", s1);
	tele = strchr(deststr, '/');
	if (!tele){
		ast_log(LOG_WARNING,"link3:Dial number (%s) must be in format tech/number\n",deststr);
		ast_free(l);
		return -1;
	}
	*tele++ = 0;
	if ((!strncasecmp(deststr,"echolink",8)) ||
	    (!strncasecmp(deststr,"irlp",4)))
	{
		char tel1[100];

		strncpy(tel1,tele,sizeof(tel1) - 1);
		cp = strchr(tel1,'/');
		if (cp) cp++; else cp = tel1;
		if (!strncasecmp(deststr,"irlp",4))
		{
			snprintf(cp,sizeof(tel1) - 1,"%s/%s",
				myrpt->name,node + 1);
		}
		else
		{
			strcpy(cp,node + 1);
		}
		l->chan = ast_request(deststr, AST_FORMAT_SLINEAR,  tel1,NULL);
	}
	else
	{
		l->chan = ast_request(deststr, AST_FORMAT_SLINEAR,tele,NULL);
	}
	if (l->chan){
		ast_set_read_format(l->chan, AST_FORMAT_SLINEAR);
		ast_set_write_format(l->chan, AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
		if (l->chan->cdr)
			ast_set_flag(l->chan->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
#ifndef	NEW_ASTERISK
		l->chan->whentohangup = 0;
#endif
		l->chan->appl = "Apprpt";
		l->chan->data = "(Remote Rx)";
		if (debug > 3)
			ast_log(LOG_NOTICE, "node=%s (remote) initiating call to %s/%s on channel %s\n",
				myrpt->name,deststr, tele, l->chan->name);
		if(l->chan->cid.cid_num)
			ast_free(l->chan->cid.cid_num);
		l->chan->cid.cid_num = ast_strdup(myrpt->name);
		ast_call(l->chan,tele,2000);
	}
	else
	{
		if(debug > 3)
			ast_log(LOG_NOTICE, "Unable to place call to %s/%s\n",
				deststr,tele);
		if (myrpt->p.archivedir)
		{
			char str[100];
			sprintf(str,"LINKFAIL,%s/%s",deststr,tele);
			donodelog(myrpt,str);
		}
		ast_free(l);
		return -1;
	}
	/* allocate a pseudo-channel thru asterisk */
	l->pchan = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
	if (!l->pchan){
		ast_log(LOG_WARNING,"rpt connect: Sorry unable to obtain pseudo channel\n");
		ast_hangup(l->chan);
		ast_free(l);
		return -1;
	}
	ast_set_read_format(l->pchan, AST_FORMAT_SLINEAR);
	ast_set_write_format(l->pchan, AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
	if (l->pchan->cdr)
		ast_set_flag(l->pchan->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
	/* make a conference for the tx */
	ci.chan = 0;
	ci.confno = myrpt->conf;
	ci.confmode = DAHDI_CONF_CONF | DAHDI_CONF_LISTENER | DAHDI_CONF_TALKER;
	/* first put the channel on the conference in proper mode */
	if (ioctl(l->pchan->fds[0], DAHDI_SETCONF, &ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
		ast_hangup(l->chan);
		ast_hangup(l->pchan);
		ast_free(l);
		return -1;
	}
	rpt_mutex_lock(&myrpt->lock);
	if (node[0] == '4') init_linkmode(myrpt,l,LINKMODE_IRLP);
	else if (node[0] == '3') init_linkmode(myrpt,l,LINKMODE_ECHOLINK);
	else l->linkmode = 0;
	l->reconnects = reconnects;
	/* insert at end of queue */
	l->max_retries = MAX_RETRIES;
	if (perma)
		l->max_retries = MAX_RETRIES_PERM;
	if (l->isremote) l->retries = l->max_retries + 1;
	l->rxlingertimer = RX_LINGER_TIME;
	insque((struct qelem *)l,(struct qelem *)myrpt->links.next);
	__kickshort(myrpt);
	rpt_mutex_unlock(&myrpt->lock);
	return 0;
}



/*
* Internet linking functions
* These are commands entered on one link and executed on another
*/
static int function_ilink(struct rpt *myrpt, char *param, char *digits, int command_source, struct rpt_link *mylink)
{
	char *val, *s, *s1, *s2;
	char tmp[300];
	char digitbuf[MAXNODESTR],*strs[MAXLINKLIST];
	char mode,perma;
	char sx[320],*sy;
	struct rpt_link *l;
	int i,r;

	if(!param)
		return DC_ERROR;

	if (myrpt->p.s[myrpt->p.sysstate_cur].txdisable || myrpt->p.s[myrpt->p.sysstate_cur].linkfundisable )
		return DC_ERROR;

	strncpy(digitbuf,digits,MAXNODESTR - 1);

	if(debug)
		ast_log(LOG_NOTICE,"[%s] ilink param = %s, digitbuf = %s\n",myrpt->name, (param)? param : "(null)", digitbuf);

	switch(myatoi(param)){
		case 11: /* Perm Link off */
		case 1: /* Link off */
			if ((digitbuf[0] == '0') && (myrpt->lastlinknode[0]))
				strcpy(digitbuf,myrpt->lastlinknode);
			if (digitbuf[0] == '4')
			{
				if (strlen(digitbuf) < 5) break;
				sprintf(tmp,"irlp/%d,NONE",atoi(digitbuf + 1));
			}
			else if (digitbuf[0] != '3')
			{
				val = node_lookup(myrpt,digitbuf);
				if (!val){
					if(strlen(digitbuf) >= myrpt->longestnode)
						return DC_ERROR;
					break;
				}
				strncpy(tmp,val,sizeof(tmp) - 1);
			}
			else
			{
				char actstr[10],dbstr[40],str1[40];

				if (strlen(digitbuf) < 7) break;
				if (ast_db_get(EL_DB_ROOT,"active",actstr,sizeof(actstr) - 1))
					return DC_ERROR;
				sprintf(dbstr,"%s/nodenum/%d",actstr,atoi(digitbuf + 1));
				if (ast_db_get(EL_DB_ROOT,dbstr,str1,sizeof(str1))) return DC_ERROR;
				sprintf(tmp,"echolink/el0/%s,%s",str1,str1);
			}
			s = tmp;
			s1 = strsep(&s,",");
			if ((!strchr(s1,':')) && strchr(s1,'/') && strncasecmp(s1, "local/", 6) &&
				strncasecmp(s1,"echolink/",9) && strncasecmp(s1, "irlp/",5))
			{
				sy = strchr(s1,'/');
				*sy = 0;
				sprintf(sx,"%s:4569/%s",s1,sy + 1);
				s1 = sx;
			}
			s2 = strsep(&s,",");
			rpt_mutex_lock(&myrpt->lock);
			l = myrpt->links.next;
			/* try to find this one in queue */
			while(l != &myrpt->links){
				if (l->name[0] == '0')
				{
					l = l->next;
					continue;
				}
				/* if found matching string */
				if (!strcmp(l->name, digitbuf))
					break;
				l = l->next;
			}
			if (l != &myrpt->links){ /* if found */
				struct	ast_frame wf;

				/* must use perm command on perm link */
				if ((myatoi(param) < 10) &&
				    (l->max_retries > MAX_RETRIES))
				{
					rpt_mutex_unlock(&myrpt->lock);
					return DC_COMPLETE;
				}
				strncpy(myrpt->lastlinknode,digitbuf,MAXNODESTR - 1);
				l->retries = l->max_retries + 1;
				l->disced = 1;
				l->hasconnected = 1;
				rpt_mutex_unlock(&myrpt->lock);
				memset(&wf,0,sizeof(wf));
				wf.frametype = AST_FRAME_TEXT;
				wf.datalen = strlen(discstr) + 1;
				wf.data = discstr;
				wf.src = "function_ilink:1";
				if (l->chan)
				{
					if (l->thisconnected) ast_write(l->chan,&wf);
					if (ast_safe_sleep(l->chan,250) == -1) return DC_ERROR;
					ast_softhangup(l->chan,AST_SOFTHANGUP_DEV);
				}
				myrpt->linkactivityflag = 1;
				rpt_telem_select(myrpt,command_source,mylink);
				rpt_telemetry(myrpt, COMPLETE, NULL);
				return DC_COMPLETE;
			}
			rpt_mutex_unlock(&myrpt->lock);
			return DC_COMPLETE;
		case 2: /* Link Monitor */
		case 3: /* Link transceive */
		case 12: /* Link Monitor permanent */
		case 13: /* Link transceive permanent */
			if ((digitbuf[0] == '0') && (myrpt->lastlinknode[0]))
				strcpy(digitbuf,myrpt->lastlinknode);
			/* Attempt connection  */
			perma = (atoi(param) > 10) ? 1 : 0;
			mode = (atoi(param) & 1) ? 1 : 0;
			r = connect_link(myrpt, digitbuf, mode, perma);
			switch(r){
				case -2: /* Attempt to connect to self */
					return DC_COMPLETE; /* Silent error */

				case 0:
					myrpt->linkactivityflag = 1;
					rpt_telem_select(myrpt,command_source,mylink);
					rpt_telemetry(myrpt, COMPLETE, NULL);
					return DC_COMPLETE;

				case 1:
					break;

				case 2:
					rpt_telem_select(myrpt,command_source,mylink);
					rpt_telemetry(myrpt, REMALREADY, NULL);
					return DC_COMPLETE;

				default:
					rpt_telem_select(myrpt,command_source,mylink);
					rpt_telemetry(myrpt, CONNFAIL, NULL);
					return DC_COMPLETE;
			}
			break;

		case 4: /* Enter Command Mode */

			/* if doesnt allow link cmd, or no links active, return */
			if (myrpt->links.next == &myrpt->links) return DC_COMPLETE;
 			if ((command_source != SOURCE_RPT) &&
				(command_source != SOURCE_PHONE) &&
				(command_source != SOURCE_ALT) &&
				(command_source != SOURCE_DPHONE) && mylink &&
				strncasecmp(mylink->chan->name,"echolink",8) &&
				strncasecmp(mylink->chan->name,"irlp",4))
					return DC_COMPLETE;

			/* if already in cmd mode, or selected self, fughetabahtit */
			if ((myrpt->cmdnode[0]) || (!strcmp(myrpt->name, digitbuf))){

				rpt_telem_select(myrpt,command_source,mylink);
				rpt_telemetry(myrpt, REMALREADY, NULL);
				return DC_COMPLETE;
			}
			if ((digitbuf[0] == '0') && (myrpt->lastlinknode[0]))
				strcpy(digitbuf,myrpt->lastlinknode);
			/* node must at least exist in list */
			if (digitbuf[0] == '4')
			{
				if (strlen(digitbuf) < 5) break;
			}
			else if (digitbuf[0] != '3')
			{
				val = node_lookup(myrpt,digitbuf);
				if (!val){
					if(strlen(digitbuf) >= myrpt->longestnode)
						return DC_ERROR;
					break;
				}
			}
			else
			{
				if (strlen(digitbuf) < 7) break;
			}
			rpt_mutex_lock(&myrpt->lock);
			strcpy(myrpt->lastlinknode,digitbuf);
			strncpy(myrpt->cmdnode, digitbuf, sizeof(myrpt->cmdnode) - 1);
			rpt_mutex_unlock(&myrpt->lock);
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, REMGO, NULL);
			return DC_COMPLETE;

		case 5: /* Status */
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, STATUS, NULL);
			return DC_COMPLETE;

		case 15: /* Full Status */
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, FULLSTATUS, NULL);
			return DC_COMPLETE;

		case 6: /* All Links Off, including permalinks */
			rpt_mutex_lock(&myrpt->lock);
			myrpt->savednodes[0] = 0;
            l = myrpt->links.next;
            /* loop through all links */
            while(l != &myrpt->links){
				struct	ast_frame wf;
                if (l->name[0] == '0') /* Skip any IAXRPT monitoring */
                {
                    l = l->next;
                    continue;
                }
				/* Make a string of disconnected nodes for possible restoration */
				sprintf(tmp,"%c%c%s",(l->mode) ? 'X' : 'M',(l->perma) ? 'P':'T',l->name);
				if(strlen(tmp) + strlen(myrpt->savednodes) + 1 < MAXNODESTR){
					if(myrpt->savednodes[0])
						strcat(myrpt->savednodes, ",");
					strcat(myrpt->savednodes, tmp);
				}
				l->retries = l->max_retries + 1;
				l->disced = 2; /* Silently disconnect */
				rpt_mutex_unlock(&myrpt->lock);
				/* ast_log(LOG_NOTICE,"dumping link %s\n",l->name); */

				memset(&wf,0,sizeof(wf));
                wf.frametype = AST_FRAME_TEXT;
                wf.datalen = strlen(discstr) + 1;
                wf.data = discstr;
				wf.src = "function_ilink:6";
                if (l->chan)
                {
	                if (l->thisconnected) ast_write(l->chan,&wf);
	                ast_safe_sleep(l->chan,250); /* It's dead already, why check the return value? */
	                ast_softhangup(l->chan,AST_SOFTHANGUP_DEV);
                }
				rpt_mutex_lock(&myrpt->lock);
				l = l->next;
            }
			rpt_mutex_unlock(&myrpt->lock);
			if(debug > 3)
				ast_log(LOG_NOTICE,"Nodes disconnected: %s\n",myrpt->savednodes);
			rpt_telem_select(myrpt,command_source,mylink);
            rpt_telemetry(myrpt, COMPLETE, NULL);
			return DC_COMPLETE;

		case 7: /* Identify last node which keyed us up */
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, LASTNODEKEY, NULL);
			break;

#ifdef	_MDC_DECODE_H_
		case 8:
			myrpt->lastunit = 0xd00d;
			mdc1200_notify(myrpt,NULL,myrpt->lastunit);
			mdc1200_send(myrpt,myrpt->lastunit);
			break;
#endif

		case 16: /* Restore links disconnected with "disconnect all links" command */
			strcpy(tmp, myrpt->savednodes); /* Make a copy */
			finddelim(tmp, strs, MAXLINKLIST); /* convert into substrings */
			for(i = 0; tmp[0] && strs[i] != NULL && i < MAXLINKLIST; i++){
				s1 = strs[i];
				mode = (s1[0] == 'X') ? 1 : 0;
				perma = (s1[1] == 'P') ? 1 : 0;
				connect_link(myrpt, s1 + 2, mode, perma); /* Try to reconnect */
			}
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, COMPLETE, NULL);
			break;

		case 200:
		case 201:
		case 202:
		case 203:
		case 204:
		case 205:
		case 206:
		case 207:
		case 208:
		case 209:
		case 210:
		case 211:
		case 212:
		case 213:
		case 214:
		case 215:
			if (((myrpt->p.propagate_dtmf) &&
			     (command_source == SOURCE_LNK)) ||
			    ((myrpt->p.propagate_phonedtmf) &&
				((command_source == SOURCE_PHONE) ||
				  (command_source == SOURCE_ALT) ||
				    (command_source == SOURCE_DPHONE))))
					do_dtmf_local(myrpt,
						remdtmfstr[myatoi(param) - 200]);
		default:
			return DC_ERROR;

	}

	return DC_INDETERMINATE;
}

/*
* Autopatch up
*/

static int function_autopatchup(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink)
{
	pthread_attr_t attr;
	int i, index, paramlength;
	char *lparam;
	char *value = NULL;
	char *paramlist[20];

	static char *keywords[] = {
	"context",
	"dialtime",
	"farenddisconnect",
	"noct",
	"quiet",
	NULL
	};

	if (myrpt->p.s[myrpt->p.sysstate_cur].txdisable || myrpt->p.s[myrpt->p.sysstate_cur].autopatchdisable)
		return DC_ERROR;

	if(debug)
		ast_log(LOG_NOTICE,"[%s] Autopatch up. digitbuf=%s\n",myrpt->name,digitbuf);

	if(!myrpt->callmode){
		/* Set defaults */
		myrpt->patchnoct = 0;
		myrpt->patchdialtime = 0;
		myrpt->patchfarenddisconnect = 0;
		myrpt->patchquiet = 0;
		strncpy(myrpt->patchcontext, myrpt->p.ourcontext, MAXPATCHCONTEXT);

		if(param){
			/* Process parameter list */
			lparam = ast_strdup(param);
			if(!lparam){
				ast_log(LOG_ERROR,"App_rpt out of memory on line %d\n",__LINE__);
				return DC_ERROR;
			}
			paramlength = finddelim(lparam, paramlist, 20);
			for(i = 0; i < paramlength; i++){
				index = matchkeyword(paramlist[i], &value, keywords);
				if(value)
					value = skipchars(value, "= ");
				switch(index){

					case 1: /* context */
						strncpy(myrpt->patchcontext, value, MAXPATCHCONTEXT - 1) ;
						break;

					case 2: /* dialtime */
						myrpt->patchdialtime = atoi(value);
						break;

					case 3: /* farenddisconnect */
						myrpt->patchfarenddisconnect = atoi(value);
						break;

					case 4:	/* noct */
						myrpt->patchnoct = atoi(value);
						break;

					case 5: /* quiet */
						myrpt->patchquiet = atoi(value);
						break;

					default:
						break;
				}
			}
		ast_free(lparam);
		}
	}

	rpt_mutex_lock(&myrpt->lock);

	/* if on call, force * into current audio stream */

	if ((myrpt->callmode == 2) || (myrpt->callmode == 3)){
		myrpt->mydtmf = myrpt->p.funcchar;
	}
	if (myrpt->callmode){
		rpt_mutex_unlock(&myrpt->lock);
		return DC_COMPLETE;
	}
	myrpt->callmode = 1;
	myrpt->cidx = 0;
	myrpt->exten[myrpt->cidx] = 0;
	rpt_mutex_unlock(&myrpt->lock);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ast_pthread_create(&myrpt->rpt_call_thread,&attr,rpt_call,(void *) myrpt);
	return DC_COMPLETE;
}

/*
* Autopatch down
*/

static int function_autopatchdn(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink)
{
	if (myrpt->p.s[myrpt->p.sysstate_cur].txdisable || myrpt->p.s[myrpt->p.sysstate_cur].autopatchdisable)
		return DC_ERROR;

	if(debug)
		ast_log(LOG_NOTICE,"[%s] Autopatch down\n",myrpt->name);

	rpt_mutex_lock(&myrpt->lock);

	myrpt->macropatch=0;

	if (!myrpt->callmode){
		rpt_mutex_unlock(&myrpt->lock);
		return DC_COMPLETE;
	}

	myrpt->callmode = 0;
	channel_revert(myrpt);
	rpt_mutex_unlock(&myrpt->lock);

	if(!myrpt->innox)
	{
		rpt_telem_select(myrpt,command_source,mylink);
		rpt_telemetry(myrpt, TERM, NULL);
	}
	myrpt->innox=0;

	return DC_COMPLETE;
}

/*
* Status
*/

static int function_status(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink)
{

	if (!param)
		return DC_ERROR;

	if ((myrpt->p.s[myrpt->p.sysstate_cur].txdisable) || (myrpt->p.s[myrpt->p.sysstate_cur].userfundisable))
		return DC_ERROR;

	if(debug)
		ast_log(LOG_NOTICE,"status param = %s, digitbuf = %s\n", (param)? param : "(null)", digitbuf);

	switch(myatoi(param)){
		case 1: /* System ID */
			rpt_telemetry(myrpt, ID1, NULL);
			return DC_COMPLETE;
		case 2: /* System Time */
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, STATS_TIME, NULL);
			return DC_COMPLETE;
		case 3: /* app_rpt.c version */
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, STATS_VERSION, NULL);
			return DC_COMPLETE;
		case 11: /* System ID (local only)*/
		    rpt_telemetry(myrpt, ID , NULL);
            return DC_COMPLETE;
        case 12: /* System Time (local only)*/
            rpt_telemetry(myrpt, STATS_TIME_LOCAL, NULL);
            return DC_COMPLETE;
		default:
			return DC_ERROR;
	}
	return DC_INDETERMINATE;
}
/*
*  Macro-oni (without Salami)
*/
static int function_macro(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink)
{
char	*val;
int	i;
	if (myrpt->remote)
		return DC_ERROR;

	if(debug)
		ast_log(LOG_NOTICE,"[%s] macro-oni param = %s, digitbuf = %s\n", myrpt->name, (param)? param : "(null)", digitbuf);

	if(strlen(digitbuf) < 1) /* needs 1 digit */
		return DC_INDETERMINATE;

	for(i = 0 ; i < digitbuf[i] ; i++) {
		if((digitbuf[i] < '0') || (digitbuf[i] > '9'))
			return DC_ERROR;
	}

	if (*digitbuf == '0') val = myrpt->p.startupmacro;
	else val = (char *) ast_variable_retrieve(myrpt->cfg, myrpt->p.macro, digitbuf);
	/* param was 1 for local buf */
	if (!val){
        if (strlen(digitbuf) < myrpt->macro_longest)
        	return DC_INDETERMINATE;
		rpt_telem_select(myrpt,command_source,mylink);
		rpt_telemetry(myrpt, MACRO_NOTFOUND, NULL);
		return DC_COMPLETE;
	}
	rpt_mutex_lock(&myrpt->lock);
	if ((MAXMACRO - strlen(myrpt->macrobuf)) < strlen(val))
	{
		rpt_mutex_unlock(&myrpt->lock);
		rpt_telem_select(myrpt,command_source,mylink);
		rpt_telemetry(myrpt, MACRO_BUSY, NULL);
		return DC_ERROR;
	}
	myrpt->macrotimer = MACROTIME;
	strncat(myrpt->macrobuf,val,MAXMACRO - 1);
	rpt_mutex_unlock(&myrpt->lock);
	return DC_COMPLETE;
}

/*
*  Playback a recording globally
*/

static int function_playback(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink)
{

	if (myrpt->remote)
		return DC_ERROR;

	if(debug)
		ast_log(LOG_NOTICE,"playback param = %s, digitbuf = %s\n", (param)? param : "(null)", digitbuf);

	if (ast_fileexists(param,NULL,myrpt->rxchannel->language) <= 0)
		return DC_ERROR;

	rpt_telem_select(myrpt,command_source,mylink);
	rpt_telemetry(myrpt,PLAYBACK,param);
	return DC_COMPLETE;
}

/*
 * *  Playback a recording locally
 * */

static int function_localplay(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink)
{

        if (myrpt->remote)
                return DC_ERROR;

        if(debug)
                ast_log(LOG_NOTICE,"localplay param = %s, digitbuf = %s\n", (param)? param : "(null)", digitbuf);

        if (ast_fileexists(param,NULL,myrpt->rxchannel->language) <= 0)
                return DC_ERROR;

        rpt_telemetry(myrpt,LOCALPLAY,param);
        return DC_COMPLETE;
}



/*
* COP - Control OPerator Functions
*/

static int function_cop(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink)
{
	char string[50];
	char paramcopy[500];
	int  argc;
	char *argv[101],*cp;
	int i, j, k, r, src;

	if(debug)
		ast_log(LOG_NOTICE, "[%s] param=%s digits=%s source=%i link=%s \n",
			myrpt->name,param,digitbuf,command_source,mylink->name);

	if(!param)
		return DC_ERROR;

	strncpy(paramcopy, param, sizeof(paramcopy) - 1);
	paramcopy[sizeof(paramcopy) - 1] = 0;
	argc = explode_string(paramcopy, argv, 100, ',', 0);

	if(!argc)
		return DC_ERROR;

	switch(myatoi(argv[0])){
		case 1: /* System reset */
			system("killall -9 asterisk");
			return DC_COMPLETE;

		case 2:
			myrpt->p.s[myrpt->p.sysstate_cur].txdisable = 0;
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, ARB_ALPHA, (void *) "RPTENA");
			return DC_COMPLETE;

		case 3:
			myrpt->p.s[myrpt->p.sysstate_cur].txdisable = 1;
			return DC_COMPLETE;

		case 4: /* test tone on */
			if (myrpt->stopgen < 0)
			{
				myrpt->stopgen = 1;
			}
			else
			{
				myrpt->stopgen = 0;
				rpt_telemetry(myrpt, TEST_TONE, NULL);
			}
			return DC_COMPLETE;

		case 5: /* Disgorge variables to log for debug purposes */
			myrpt->disgorgetime = time(NULL) + 10; /* Do it 10 seconds later */
			return DC_COMPLETE;

		case 6: /* Simulate COR being activated (phone only) */
			if (command_source != SOURCE_PHONE) return DC_INDETERMINATE;
			return DC_DOKEY;

		case 7: /* Time out timer enable */
			myrpt->p.s[myrpt->p.sysstate_cur].totdisable = 0;
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, ARB_ALPHA, (void *) "TOTENA");
			return DC_COMPLETE;

		case 8: /* Time out timer disable */
			myrpt->p.s[myrpt->p.sysstate_cur].totdisable = 1;
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, ARB_ALPHA, (void *) "TOTDIS");
			return DC_COMPLETE;

         case 9: /* Autopatch enable */
                        myrpt->p.s[myrpt->p.sysstate_cur].autopatchdisable = 0;
			rpt_telem_select(myrpt,command_source,mylink);
                        rpt_telemetry(myrpt, ARB_ALPHA, (void *) "APENA");
                        return DC_COMPLETE;

         case 10: /* Autopatch disable */
			myrpt->p.s[myrpt->p.sysstate_cur].autopatchdisable = 1;
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, ARB_ALPHA, (void *) "APDIS");
			return DC_COMPLETE;

         case 11: /* Link Enable */
			myrpt->p.s[myrpt->p.sysstate_cur].linkfundisable = 0;
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, ARB_ALPHA, (void *) "LNKENA");
			return DC_COMPLETE;

         case 12: /* Link Disable */
                        myrpt->p.s[myrpt->p.sysstate_cur].linkfundisable = 1;
			rpt_telem_select(myrpt,command_source,mylink);
                        rpt_telemetry(myrpt, ARB_ALPHA, (void *) "LNKDIS");
                        return DC_COMPLETE;

		case 13: /* Query System State */
			string[0] = string[1] = 'S';
			string[2] = myrpt->p.sysstate_cur + '0';
			string[3] = '\0';
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, ARB_ALPHA, (void *) string);
			return DC_COMPLETE;

		case 14: /* Change System State */
			if(strlen(digitbuf) == 0)
				break;
			if((digitbuf[0] < '0') || (digitbuf[0] > '9'))
				return DC_ERROR;
			myrpt->p.sysstate_cur = digitbuf[0] - '0';
                        string[0] = string[1] = 'S';
                        string[2] = myrpt->p.sysstate_cur + '0';
                        string[3] = '\0';
			rpt_telem_select(myrpt,command_source,mylink);
                        rpt_telemetry(myrpt, ARB_ALPHA, (void *) string);
                        return DC_COMPLETE;
        case 15: /* Scheduler Enable */
                        myrpt->p.s[myrpt->p.sysstate_cur].schedulerdisable = 0;
			rpt_telem_select(myrpt,command_source,mylink);
                        rpt_telemetry(myrpt, ARB_ALPHA, (void *) "SKENA");
                        return DC_COMPLETE;

                case 16: /* Scheduler Disable */
                        myrpt->p.s[myrpt->p.sysstate_cur].schedulerdisable = 1;
			rpt_telem_select(myrpt,command_source,mylink);
                        rpt_telemetry(myrpt, ARB_ALPHA, (void *) "SKDIS");
                        return DC_COMPLETE;

                case 17: /* User functions Enable */
                        myrpt->p.s[myrpt->p.sysstate_cur].userfundisable = 0;
			rpt_telem_select(myrpt,command_source,mylink);
                        rpt_telemetry(myrpt, ARB_ALPHA, (void *) "UFENA");
                        return DC_COMPLETE;

                case 18: /* User Functions Disable */
                        myrpt->p.s[myrpt->p.sysstate_cur].userfundisable = 1;
			rpt_telem_select(myrpt,command_source,mylink);
                        rpt_telemetry(myrpt, ARB_ALPHA, (void *) "UFDIS");
                        return DC_COMPLETE;

                case 19: /* Alternate Tail Enable */
					myrpt->p.s[myrpt->p.sysstate_cur].alternatetail = 1;
					rpt_telem_select(myrpt,command_source,mylink);
					rpt_telemetry(myrpt, ARB_ALPHA, (void *) "ATENA");
					return DC_COMPLETE;

                case 20: /* Alternate Tail Disable */
					myrpt->p.s[myrpt->p.sysstate_cur].alternatetail = 0;
					rpt_telem_select(myrpt,command_source,mylink);
					rpt_telemetry(myrpt, ARB_ALPHA, (void *) "ATDIS");
					return DC_COMPLETE;

		case 21: /* Parrot Mode Enable */
			birdbath(myrpt);
			if (myrpt->p.parrotmode < 2)
			{
				myrpt->parrotonce = 0;
				myrpt->p.parrotmode = 1;
				rpt_telem_select(myrpt,command_source,mylink);
				rpt_telemetry(myrpt,COMPLETE,NULL);
				return DC_COMPLETE;
			}
			break;
			
		case 22: /* Parrot Mode Disable */
			birdbath(myrpt);
			if (myrpt->p.parrotmode < 2)
			{
				myrpt->p.parrotmode = 0;
				rpt_telem_select(myrpt,command_source,mylink);
				rpt_telemetry(myrpt,COMPLETE,NULL);
				return DC_COMPLETE;
			}
			break;
			
		case 23: /* flush parrot in progress */
			birdbath(myrpt);
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt,COMPLETE,NULL);
			return DC_COMPLETE;
			
		case 24: /* flush all telemetry */
			flush_telem(myrpt);
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt,COMPLETE,NULL);
			return DC_COMPLETE;
		case 25: /* request keying info (brief) */
			send_link_keyquery(myrpt);
			myrpt->topkeylong = 0;
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt,COMPLETE,NULL);
			return DC_COMPLETE;
		case 26: /* request keying info (full) */
			send_link_keyquery(myrpt);
			myrpt->topkeylong = 1;
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt,COMPLETE,NULL);
			return DC_COMPLETE;

		case 30: /* recall memory location on programmable radio */
		  	if(strlen(digitbuf) < 2) /* needs 2 digits */
				break;

			for(i = 0 ; i < 2 ; i++){
				if((digitbuf[i] < '0') || (digitbuf[i] > '9'))
					return DC_ERROR;
			}

			r = retreive_memory(myrpt, digitbuf);
			if (r < 0){
				rpt_telemetry(myrpt,MEMNOTFOUND,NULL);
				return DC_COMPLETE;
			}
			if (r > 0){
				return DC_ERROR;
			}
			if (setrem(myrpt) == -1) return DC_ERROR;
			return DC_COMPLETE;

		case 31:
		    /* set channel. note that it's going to change channel
		       then send a telemetry confirmation on the new channel. */
		  	if(strlen(digitbuf) < 2) /* needs 2 digits */
				break;

			for(i = 0 ; i < 2 ; i++){
				if((digitbuf[i] < '0') || (digitbuf[i] > '9'))
					return DC_ERROR;
			}
			if(command_source!=SOURCE_RPT)myrpt->localteleminhibit=1;
			channel_steer(myrpt,digitbuf);
			return DC_COMPLETE;

		case 32: /* Touch Tone Pad Test */
			i = strlen(digitbuf);
			if(!i){
				if(debug > 3)
				ast_log(LOG_NOTICE,"Padtest entered");
				myrpt->inpadtest = 1;
				break;
			}
			else{
				if(debug > 3)
					ast_log(LOG_NOTICE,"Padtest len= %d digits=%s",i,digitbuf);
				if(digitbuf[i-1] != myrpt->p.endchar)
					break;
				rpt_telemetry(myrpt, ARB_ALPHA, digitbuf);
				myrpt->inpadtest = 0;
				if(debug > 3)
					ast_log(LOG_NOTICE,"Padtest exited");
				return DC_COMPLETE;
			}
                case 33: /* Local Telem mode Enable */
			if (myrpt->p.telemdynamic)
			{
				myrpt->telemmode = 0x7fffffff;
				rpt_telemetry(myrpt,COMPLETE,NULL);
				return DC_COMPLETE;
			}
			break;
                case 34: /* Local Telem mode Disable */
			if (myrpt->p.telemdynamic)
			{
				myrpt->telemmode = 0;
				rpt_telemetry(myrpt,COMPLETE,NULL);
				return DC_COMPLETE;
			}
			break;
        case 35: /* Local Telem mode Normal */
			if (myrpt->p.telemdynamic)
			{
				myrpt->telemmode = 1;
				rpt_telem_select(myrpt,command_source,mylink);
				rpt_telemetry(myrpt,COMPLETE,NULL);
				return DC_COMPLETE;
			}
			break;
		case 36: /* Link Output Enable */
			if (!mylink) return DC_ERROR;
			src = 0;
			if (mylink->name[0] == '0') src = LINKMODE_GUI;
			if (mylink->phonemode) src = LINKMODE_PHONE;
			else if (!strncasecmp(mylink->chan->name,"echolink",8)) src = LINKMODE_ECHOLINK;
			else if (!strncasecmp(mylink->chan->name,"irlp",4)) src = LINKMODE_IRLP;
			if (src && myrpt->p.linkmodedynamic[src])
			{
				set_linkmode(mylink,LINKMODE_ON);
				rpt_telem_select(myrpt,command_source,mylink);
				rpt_telemetry(myrpt,COMPLETE,NULL);
				return DC_COMPLETE;
			}
			break;
		case 37: /* Link Output Disable */
			if (!mylink) return DC_ERROR;
			src = 0;
			if (mylink->name[0] == '0') src = LINKMODE_GUI;
			if (mylink->phonemode) src = LINKMODE_PHONE;
			else if (!strncasecmp(mylink->chan->name,"echolink",8)) src = LINKMODE_ECHOLINK;
			else if (!strncasecmp(mylink->chan->name,"irlp",4)) src = LINKMODE_IRLP;
			if (src && myrpt->p.linkmodedynamic[src])
			{
				set_linkmode(mylink,LINKMODE_OFF);
				rpt_telem_select(myrpt,command_source,mylink);
				rpt_telemetry(myrpt,COMPLETE,NULL);
				return DC_COMPLETE;
			}
			break;
		case 38: /* Gui Link Output Follow */
			if (!mylink) return DC_ERROR;
			src = 0;
			if (mylink->name[0] == '0') src = LINKMODE_GUI;
			if (mylink->phonemode) src = LINKMODE_PHONE;
			else if (!strncasecmp(mylink->chan->name,"echolink",8)) src = LINKMODE_ECHOLINK;
			else if (!strncasecmp(mylink->chan->name,"irlp",4)) src = LINKMODE_IRLP;
			if (src && myrpt->p.linkmodedynamic[src])
			{
				set_linkmode(mylink,LINKMODE_FOLLOW);
				rpt_telem_select(myrpt,command_source,mylink);
				rpt_telemetry(myrpt,COMPLETE,NULL);
				return DC_COMPLETE;
			}
			break;
		case 39: /* Link Output Demand*/
			if (!mylink) return DC_ERROR;
			src = 0;
			if (mylink->name[0] == '0') src = LINKMODE_GUI;
			if (mylink->phonemode) src = LINKMODE_PHONE;
			else if (!strncasecmp(mylink->chan->name,"echolink",8)) src = LINKMODE_ECHOLINK;
			else if (!strncasecmp(mylink->chan->name,"irlp",4)) src = LINKMODE_IRLP;
			if (src && myrpt->p.linkmodedynamic[src])
			{
				set_linkmode(mylink,LINKMODE_DEMAND);
				rpt_telem_select(myrpt,command_source,mylink);
				rpt_telemetry(myrpt,COMPLETE,NULL);
				return DC_COMPLETE;
			}
			break;

		case 40: /* IRLP announce Enable */
			myrpt->p.irlpann = 1;
			rpt_telemetry(myrpt,COMPLETE,NULL);
			return DC_COMPLETE;
		case 41: /* IRLP announce Disable */
			myrpt->p.irlpann = 0;
			rpt_telemetry(myrpt,COMPLETE,NULL);
			return DC_COMPLETE;
		case 42: /* Echolink announce node # only */
			myrpt->p.eannmode = 1;
			rpt_telemetry(myrpt,COMPLETE,NULL);
			return DC_COMPLETE;
		case 43: /* Echolink announce node Callsign only */
			myrpt->p.eannmode = 2;
			rpt_telemetry(myrpt,COMPLETE,NULL);
			return DC_COMPLETE;
		case 44: /* Echolink announce node # & Callsign */
			myrpt->p.eannmode = 3;
			rpt_telemetry(myrpt,COMPLETE,NULL);
			return DC_COMPLETE;
		case 45: /* Link activity timer enable */
			if(myrpt->p.lnkacttime && myrpt->p.lnkactmacro){
				myrpt->linkactivitytimer = 0;
				myrpt->linkactivityflag = 0;
				myrpt->p.lnkactenable = 1;
				rpt_telem_select(myrpt,command_source,mylink);
				rpt_telemetry(myrpt, ARB_ALPHA, (void *) "LATENA");
			}
			return DC_COMPLETE;

		case 46: /* Link activity timer disable */
			if(myrpt->p.lnkacttime && myrpt->p.lnkactmacro){
				myrpt->linkactivitytimer = 0;
				myrpt->linkactivityflag = 0;
				myrpt->p.lnkactenable = 0;
				rpt_telem_select(myrpt,command_source,mylink);
				rpt_telemetry(myrpt, ARB_ALPHA, (void *) "LATDIS");
			}
			return DC_COMPLETE;


		case 47: /* Link activity flag kill */
			myrpt->linkactivitytimer = 0;
			myrpt->linkactivityflag = 0;
			return DC_COMPLETE; /* Silent for a reason (only used in macros) */

		case 48: /* play page sequence */
			j = 0;
			for(i = 1; i < argc; i++)
			{
				k = strlen(argv[i]);
				if (k != 1)
				{
					j += k + 1;
					continue;
				}
				if (*argv[i] >= '0' && *argv[i] <='9')
					argv[i] = dtmf_tones[*argv[i]-'0'];
				else if (*argv[i] >= 'A' && (*argv[i]) <= 'D')
					argv[i] = dtmf_tones[*argv[i]-'A'+10];
				else if (*argv[i] == '*')
					argv[i] = dtmf_tones[14];
				else if (*argv[i] == '#')
					argv[i] = dtmf_tones[15];
				j += strlen(argv[i]);
			}
			cp = malloc(j + 100);
			if (!cp)
			{
				ast_log(LOG_NOTICE,"cannot malloc");
				return DC_ERROR;
			}
			memset(cp,0,j + 100);
			for(i = 1; i < argc; i++)
			{
				if (i != 1) strcat(cp,",");
				strcat(cp,argv[i]);
			}
			rpt_telemetry(myrpt,PAGE,cp);
			return DC_COMPLETE;

        case 49: /* Disable Incoming connections */
			myrpt->p.s[myrpt->p.sysstate_cur].noincomingconns = 1;
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, ARB_ALPHA, (void *) "NOICE");
			return DC_COMPLETE;

        case 50: /*Enable Incoming connections */
			myrpt->p.s[myrpt->p.sysstate_cur].noincomingconns = 0;
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, ARB_ALPHA, (void *) "NOICD");
			return DC_COMPLETE;

		case 55: /* Parrot Once if parrot mode is disabled */
			if(!myrpt->p.parrotmode)
				myrpt->parrotonce = 1;
			return DC_COMPLETE;					
			
		case 56: /* RX CTCSS Enable */
			if (strncasecmp(myrpt->rxchannel->name,"radio/", 6) == 0)
			{
				ast_sendtext(myrpt->rxchannel,"RXCTCSS 1");
			}
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, ARB_ALPHA, (void *) "RXPLENA");
			return DC_COMPLETE;

		case 57: /* RX CTCSS Disable */
			if (strncasecmp(myrpt->rxchannel->name,"radio/", 6) == 0)
			{
				ast_sendtext(myrpt->rxchannel,"RXCTCSS 0");
			}
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, ARB_ALPHA, (void *) "RXPLDIS");
			return DC_COMPLETE;

		case 58: /* TX CTCSS on input only Enable */
			myrpt->p.itxctcss = 1;
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, ARB_ALPHA, (void *) "TXIPLENA");
			return DC_COMPLETE;

		case 59: /* TX CTCSS on input only Disable */
			myrpt->p.itxctcss = 0;
			rpt_telem_select(myrpt,command_source,mylink);
			rpt_telemetry(myrpt, ARB_ALPHA, (void *) "TXIPLDIS");
			return DC_COMPLETE;

		case 80: /* Change Default Squelch Code */
			if(debug > 3)
				ast_log(LOG_NOTICE,"[%s] Change Default Squelch Code=%s\n",myrpt->name,digitbuf);

			/* e.g. 00670  01072  90023  91023  */
			//myrpt->txpl

		  	if(strlen(digitbuf) < 5) /* needs x digits after the command start digit */
				break;

			for(i = 0 ; i < 5 ; i++){
				if((digitbuf[i] < '0') || (digitbuf[i] > '9'))
					return DC_ERROR;
			}
	    	r=0;
			// r = retreive_memory(myrpt, digitbuf);
			if (r < 0){
				//rpt_telemetry(myrpt,MEMNOTFOUND,NULL);
				return DC_COMPLETE;
			}
			if (r > 0){
				return DC_ERROR;
			}
			//if (setrem(myrpt) == -1) return DC_ERROR;

			if(debug > 3)
				ast_log(LOG_NOTICE,"node=%s Changed Default Squelch Code=%s\n",myrpt->name,digitbuf);

			return DC_COMPLETE;


	}
	return DC_INDETERMINATE;
}
/*
* Collect digits one by one until something matches
* Note match immediate behavior.
*/
static int collect_function_digits(struct rpt *myrpt, char *digits,
	int command_source, struct rpt_link *mylink)
{
	int i,rv;
	char *stringp,*action,*param,*functiondigits;
	char function_table_name[30] = "";
	char workstring[200];

	struct ast_variable *vp;

	if(debug)ast_log(LOG_NOTICE,"[%s] start digits=%s  source=%d\n",myrpt->name, digits, command_source);

	//if(debug)
	//	printf("@@@@ Digits collected: %s, source: %d\n", digits, command_source);

	if (command_source == SOURCE_DPHONE) {
		if (!myrpt->p.dphone_functions) return DC_INDETERMINATE;
		strncpy(function_table_name, myrpt->p.dphone_functions, sizeof(function_table_name) - 1);
		}
	else if (command_source == SOURCE_ALT) {
		if (!myrpt->p.alt_functions) return DC_INDETERMINATE;
		strncpy(function_table_name, myrpt->p.alt_functions, sizeof(function_table_name) - 1);
		}
	else if (command_source == SOURCE_PHONE) {
		if (!myrpt->p.phone_functions) return DC_INDETERMINATE;
		strncpy(function_table_name, myrpt->p.phone_functions, sizeof(function_table_name) - 1);
		}
	else if (command_source == SOURCE_LNK)
		strncpy(function_table_name, myrpt->p.link_functions, sizeof(function_table_name) - 1);
	else
		strncpy(function_table_name, myrpt->p.functions, sizeof(function_table_name) - 1);
    /* find context for function table in rpt.conf file */
	vp = ast_variable_browse(myrpt->cfg, function_table_name);
	while(vp) {
		if(!strncasecmp(vp->name, digits, strlen(vp->name)))
			break;
		vp = vp->next;
	}
	/* if function context not found */
	if(!vp) {
		int n;

		n = myrpt->longestfunc;
		if (command_source == SOURCE_LNK) n = myrpt->link_longestfunc;
		else
		if (command_source == SOURCE_PHONE) n = myrpt->phone_longestfunc;
		else
		if (command_source == SOURCE_ALT) n = myrpt->alt_longestfunc;
		else
		if (command_source == SOURCE_DPHONE) n = myrpt->dphone_longestfunc;

		if(strlen(digits) >= n)
			return DC_ERROR;
		else
			return DC_INDETERMINATE;
	}
	/* Found a match, retrieve value part and parse */
	strncpy(workstring, vp->value, sizeof(workstring) - 1 );
	stringp = workstring;
	action = strsep(&stringp, ",");
	param = stringp;
	if(debug)
		ast_log(LOG_NOTICE,"action: %s, param = %s\n",action, (param) ? param : "(null)");
	/* Look up the action */
	for(i = 0 ; i < (sizeof(function_table)/sizeof(struct function_table_tag)); i++){
		if(!strncasecmp(action, function_table[i].action, strlen(action)))
			break;
	}
	if(debug)
		ast_log(LOG_NOTICE,"table index i = %d\n",i);
	if(i == (sizeof(function_table)/sizeof(struct function_table_tag))){
		/* Error, action not in table */
		return DC_ERROR;
	}
	if(function_table[i].function == NULL){
		/* Error, function undefined */
		if(debug)
			ast_log(LOG_NOTICE,"[%s] NULL for action: %s\n",myrpt->name,action);
		return DC_ERROR;
	}
	functiondigits = digits + strlen(vp->name);
	rv=(*function_table[i].function)(myrpt, param, functiondigits, command_source, mylink);
	if(debug)ast_log(LOG_NOTICE,"return found function rv=%i\n",rv);
	return(rv);
}

/*
	Handle Text Frames from other Links
*/
static void handle_link_data(struct rpt *myrpt, struct rpt_link *mylink,
	char *str)
{
char	tmp[512],tmp1[512],cmd[300] = "",dest[300],src[300],c;
int	i,seq, res, ts;
struct rpt_link *l;
struct	ast_frame wf;

	if(debug)ast_log(LOG_NOTICE,
				"[%s] start link=%s %s\n",
				myrpt->name,mylink->name,str);

	wf.frametype = AST_FRAME_TEXT;
	wf.subclass = 0;
	wf.offset = 0;
	wf.mallocd = 0;
	wf.datalen = strlen(str) + 1;
	wf.samples = 0;
	wf.src = "handle_link_data";
 	/* put string in our buffer */
	strncpy(tmp,str,sizeof(tmp) - 1);

    if (!strcmp(tmp,discstr))
    {
	    mylink->disced = 1;
	    mylink->retries = mylink->max_retries + 1;
	    ast_softhangup(mylink->chan,AST_SOFTHANGUP_DEV);
	    return;
    }
    if (!strcmp(tmp,newkeystr))
    {
		if ((!mylink->newkey) || mylink->newkeytimer)
		{
			if (debug > 1)
				ast_log(LOG_NOTICE,
					"[%s] link=%s noted %s\n",
					myrpt->name,mylink->name,newkeystr);
			mylink->newkeytimer = 0;
			mylink->newkey = 1;
			send_old_newkey(mylink->chan);
		}
        return;
    }
    if (!strcmp(tmp,newkey1str))
    {
		if (debug > 1)
			ast_log(LOG_NOTICE,
				"[%s] link=%s/%s noted %s\n",
				myrpt->name,mylink->name,mylink->chan->name,newkey1str);
		mylink->newkeytimer = 0;
		mylink->newkey = 2;
		check_linkmodemaster(myrpt,mylink);
		return;
    }

    if (!strncmp(tmp,iaxkeystr,8))
    {
		int i0,i1,i2,i3;
		mylink->iaxkey = 0;
		mylink->iaxrpt = 0;
		mylink->csmonitor=0;
		if(strlen(tmp)>9){
			sscanf(tmp+9,"%i %i %i %i", &i0, &i1, &i2, &i3);
			mylink->iaxkey=i0;
			mylink->iaxrpt=i1;
			mylink->csmonitor=i2;
			if (debug > 1)
				ast_log(LOG_NOTICE,"node=%s link=%s/%s noted iaxkey=%i iaxrpt=%i csmonitor=%i future=%i\n",
					myrpt->name,mylink->name,mylink->chan->name,mylink->iaxkey,mylink->iaxrpt,mylink->csmonitor,i3);

		    if(mylink->outbound)send_iaxkey(mylink->chan);
		}
		return;
    }

	if (tmp[0] == 'L')
	{
		rpt_mutex_lock(&myrpt->lock);
		strcpy(mylink->linklist,tmp + 2);
		time(&mylink->linklistreceived);
		rpt_mutex_unlock(&myrpt->lock);
		if (debug > 6) ast_log(LOG_NOTICE,"node=%s received node list %s from node=%s\n",
			myrpt->name,tmp,mylink->name);
		return;
	}

	if (tmp[0] == 'R' && (wf.datalen<8))
	{
		rpt_mutex_lock(&myrpt->lock);
		mylink->rssi=atoi(&(tmp[2]));
		rpt_mutex_unlock(&myrpt->lock);
		if (debug>5) ast_log(LOG_NOTICE,"[%s] from [%s] rxrssi=%i\n",
			myrpt->name,mylink->name,mylink->rssi);
		return;
	}


	if (tmp[0] == 'T')
	{
		if (sscanf(tmp,"%s %s %s",cmd,src,dest) != 3)
		{
			ast_log(LOG_WARNING, "Unable to parse telem string %s\n",str);
			return;
		}
		l = myrpt->links.next;
		/* otherwise, send it to all of em */
		while(l != &myrpt->links)
		{
			if (l->name[0] == '0')
			{
				l = l->next;
				continue;
			}
			/* dont send back from where it came */
			if ((l == mylink) || (!strcmp(l->name,mylink->name)))
			{
				l = l->next;
				continue;
			}
			/* send, but not to src */
			if (strcmp(l->name,src)) {
				wf.data = str;
				if (l->chan) rpt_qwrite(l,&wf);
			}
			l = l->next;
		}
		/* if is from me, ignore */
		if (!strcmp(src,myrpt->name)) return;

		/* set 'got T message' flag */
		mylink->gott = 1;

		rpt_telemetry(myrpt,VARCMD,dest);
		return;
	}

	if (tmp[0] == 'K')
	{
		if (sscanf(tmp,"%s %s %s %d %d",cmd,dest,src,&seq,&ts) != 5)
		{
			ast_log(LOG_WARNING, "Unable to parse keying string %s\n",str);
			return;
		}
		if (dest[0] == '0')
		{
			strcpy(dest,myrpt->name);
		}
		/* if not for me, redistribute to all links */
		if (strcmp(dest,myrpt->name))
		{
			l = myrpt->links.next;
			/* see if this is one in list */
			while(l != &myrpt->links)
			{
				if (l->name[0] == '0')
				{
					l = l->next;
					continue;
				}
				/* dont send back from where it came */
				if ((l == mylink) || (!strcmp(l->name,mylink->name)))
				{
					l = l->next;
					continue;
				}
				/* if it is, send it and we're done */
				if (!strcmp(l->name,dest))
				{
					/* send, but not to src */
					if (strcmp(l->name,src)) {
						wf.data = str;
						if (l->chan) rpt_qwrite(l,&wf);
					}
					return;
				}
				l = l->next;
			}
		}
		/* if not for me, or is broadcast, redistribute to all links */
		if ((strcmp(dest,myrpt->name)) || (dest[0] == '*'))
		{
			l = myrpt->links.next;
			/* otherwise, send it to all of em */
			while(l != &myrpt->links)
			{
				if (l->name[0] == '0')
				{
					l = l->next;
					continue;
				}
				/* dont send back from where it came */
				if ((l == mylink) || (!strcmp(l->name,mylink->name)))
				{
					l = l->next;
					continue;
				}
				/* send, but not to src */
				if (strcmp(l->name,src)) {
					wf.data = str;
					if (l->chan) rpt_qwrite(l,&wf);
				}
				l = l->next;
			}
		}
		/* if not for me, end here */
		if (strcmp(dest,myrpt->name) && (dest[0] != '*')) return;
		if (cmd[1] == '?')
		{
			time_t now;
			int n = 0;

			time(&now);
			if (myrpt->lastkeyedtime)
			{
				n = (int)(now - myrpt->lastkeyedtime);
			}
			sprintf(tmp1,"K %s %s %d %d",src,myrpt->name,myrpt->keyed,n);
			wf.data = tmp1;
			wf.datalen = strlen(tmp1) + 1;
			if (mylink->chan) rpt_qwrite(mylink,&wf);
			return;
		}
		if (myrpt->topkeystate != 1) return;
		rpt_mutex_lock(&myrpt->lock);
		for(i = 0; i < TOPKEYN; i++)
		{
			if (!strcmp(myrpt->topkey[i].node,src)) break;
		}
		if (i >= TOPKEYN)
		{
			for(i = 0; i < TOPKEYN; i++)
			{
				if (!myrpt->topkey[i].node[0]) break;
			}
		}
		if (i < TOPKEYN)
		{
			strncpy(myrpt->topkey[i].node,src,TOPKEYMAXSTR - 1);
			myrpt->topkey[i].timesince = ts;
			myrpt->topkey[i].keyed = seq;
		}
		rpt_mutex_unlock(&myrpt->lock);
		return;
	}
	if (tmp[0] == 'I')
	{
		if (sscanf(tmp,"%s %s %x",cmd,src,&seq) != 3)
		{
			ast_log(LOG_WARNING, "Unable to parse ident string %s\n",str);
			return;
		}
		mdc1200_notify(myrpt,src,seq);
		strcpy(dest,"*");
	}
	else
	{
		if (sscanf(tmp,"%s %s %s %d %c",cmd,dest,src,&seq,&c) != 5)
		{
			ast_log(LOG_WARNING, "Unable to parse link string %s\n",str);
			return;
		}
		if (strcmp(cmd,"D"))
		{
			ast_log(LOG_WARNING, "Unable to parse link string %s\n",str);
			return;
		}
	}
	if (dest[0] == '0')
	{
		strcpy(dest,myrpt->name);
	}

	/* if not for me, redistribute to all links */
	if (strcmp(dest,myrpt->name))
	{
		l = myrpt->links.next;
		/* see if this is one in list */
		while(l != &myrpt->links)
		{
			if (l->name[0] == '0')
			{
				l = l->next;
				continue;
			}
			/* dont send back from where it came */
			if ((l == mylink) || (!strcmp(l->name,mylink->name)))
			{
				l = l->next;
				continue;
			}
			/* if it is, send it and we're done */
			if (!strcmp(l->name,dest))
			{
				/* send, but not to src */
				if (strcmp(l->name,src)) {
					wf.data = str;
					if (l->chan) rpt_qwrite(l,&wf);
				}
				return;
			}
			l = l->next;
		}
		l = myrpt->links.next;
		/* otherwise, send it to all of them */
		while(l != &myrpt->links)
		{
			if (l->name[0] == '0')
			{
				l = l->next;
				continue;
			}
			/* dont send back from where it came */
			if ((l == mylink) || (!strcmp(l->name,mylink->name)))
			{
				l = l->next;
				continue;
			}
			/* send, but not to src */
			if (strcmp(l->name,src)) {
				wf.data = str;
				if (l->chan) rpt_qwrite(l,&wf);
			}
			l = l->next;
		}
		return;
	}
	if (myrpt->p.archivedir)
	{
		char str[100];

		sprintf(str,"DTMF,%s,%c",mylink->name,c);
		donodelog(myrpt,str);
	}
	c = func_xlat(myrpt,c,&myrpt->p.outxlat);
	if (!c) return;
	rpt_mutex_lock(&myrpt->lock);
	if (c == myrpt->p.endchar) myrpt->stopgen = 1;
	if (myrpt->callmode == 1)
	{
		myrpt->exten[myrpt->cidx++] = c;
		myrpt->exten[myrpt->cidx] = 0;
		/* if this exists */
		if (ast_exists_extension(myrpt->pchannel,myrpt->patchcontext,myrpt->exten,1,NULL))
		{
			/* if this really it, end now */
			if (!ast_matchmore_extension(myrpt->pchannel,myrpt->patchcontext,
				myrpt->exten,1,NULL))
			{
			    if(debug)ast_log(LOG_NOTICE,"end call. callmode now 2.\n");
				myrpt->callmode = 2;
				if(!myrpt->patchquiet)
				{
					rpt_mutex_unlock(&myrpt->lock);
					rpt_telemetry(myrpt,PROC,NULL);
					rpt_mutex_lock(&myrpt->lock);
				}
			}
			else /* othewise, reset timer */
			{
				myrpt->calldigittimer = 1;
			}
		}
		/* if can continue, do so */
		if (!ast_canmatch_extension(myrpt->pchannel,myrpt->patchcontext,myrpt->exten,1,NULL))
		{
			/* call has failed, inform user */
			myrpt->callmode = 4;
		}
	}
	if ((!myrpt->inpadtest) &&(c == myrpt->p.funcchar))
	{
		myrpt->rem_dtmfidx = 0;
		myrpt->rem_dtmfbuf[myrpt->rem_dtmfidx] = 0;
		time(&myrpt->rem_dtmf_time);
		rpt_mutex_unlock(&myrpt->lock);
		return;
	}
	else if (myrpt->rem_dtmfidx < 0)
	{
		if ((myrpt->callmode == 2) || (myrpt->callmode == 3))
		{
			myrpt->mydtmf = c;
		}
		if (myrpt->p.propagate_dtmf) do_dtmf_local(myrpt,c);
		if (myrpt->p.propagate_phonedtmf) do_dtmf_phone(myrpt,mylink,c);
		rpt_mutex_unlock(&myrpt->lock);
		return;
	}
	else if (((myrpt->inpadtest) || (c != myrpt->p.endchar)) && (myrpt->rem_dtmfidx >= 0))
	{
		time(&myrpt->rem_dtmf_time);
		if (myrpt->rem_dtmfidx < MAXDTMF)
		{
			myrpt->rem_dtmfbuf[myrpt->rem_dtmfidx++] = c;
			myrpt->rem_dtmfbuf[myrpt->rem_dtmfidx] = 0;

			rpt_mutex_unlock(&myrpt->lock);
			strncpy(cmd, myrpt->rem_dtmfbuf, sizeof(cmd) - 1);
			res = collect_function_digits(myrpt, cmd, SOURCE_LNK, mylink);
			rpt_mutex_lock(&myrpt->lock);

			switch(res){

				case DC_INDETERMINATE:
					break;

				case DC_REQ_FLUSH:
					myrpt->rem_dtmfidx = 0;
					myrpt->rem_dtmfbuf[0] = 0;
					break;

				case DC_COMPLETE:
				case DC_COMPLETEQUIET:
					myrpt->totalexecdcommands++;
					myrpt->dailyexecdcommands++;
					strncpy(myrpt->lastdtmfcommand, cmd, MAXDTMF-1);
					myrpt->lastdtmfcommand[MAXDTMF-1] = '\0';
					myrpt->rem_dtmfbuf[0] = 0;
					myrpt->rem_dtmfidx = -1;
					myrpt->rem_dtmf_time = 0;
					break;

				case DC_ERROR:
				default:
					myrpt->rem_dtmfbuf[0] = 0;
					myrpt->rem_dtmfidx = -1;
					myrpt->rem_dtmf_time = 0;
					break;
			}
		}
	}
	rpt_mutex_unlock(&myrpt->lock);
	return;
}

static void handle_link_phone_dtmf(struct rpt *myrpt, struct rpt_link *mylink,
	char c)
{

char	cmd[300];
int	res;

	if(debug)ast_log(LOG_NOTICE,
				"[%s] start link=%s dtfm=%c\n",
				myrpt->name,mylink->name,c);

	if (myrpt->p.archivedir)
	{
		char str[100];
		sprintf(str,"DTMF(P),%s,%c",mylink->name,c);
		donodelog(myrpt,str);
	}
	if (mylink->phonemonitor) return;

	rpt_mutex_lock(&myrpt->lock);

	if (mylink->phonemode == 3) /*If in simplex dumb phone mode */
	{
		if(c == myrpt->p.endchar) /* If end char */
		{
			mylink->lastrealrx = 0; /* Keying state = off */
			rpt_mutex_unlock(&myrpt->lock);
			return;
		}

		if(c == myrpt->p.funcchar) /* If lead-in char */
		{
			mylink->lastrealrx = !mylink->lastrealrx; /* Toggle keying state */
			rpt_mutex_unlock(&myrpt->lock);
			return;
		}
	}
	else
	{
		if (c == myrpt->p.endchar)
		{
			if (mylink->lastrx &&
			    strncasecmp(mylink->chan->name,"echolink",8) &&
				strncasecmp(mylink->chan->name,"irlp",4))
			{
				mylink->lastrealrx = 0;
				rpt_mutex_unlock(&myrpt->lock);
				return;
			}
			myrpt->stopgen = 1;
			if (myrpt->cmdnode[0])
			{
				myrpt->cmdnode[0] = 0;
				myrpt->dtmfidx = -1;
				myrpt->dtmfbuf[0] = 0;
				rpt_mutex_unlock(&myrpt->lock);
				rpt_telemetry(myrpt,COMPLETE,NULL);
				return;
			}
#if 0
			if ((myrpt->rem_dtmfidx < 0) &&
			    ((myrpt->callmode == 2) || (myrpt->callmode == 3)))
			{
				myrpt->mydtmf = c;
			}
#endif
		}
	}
	if (myrpt->cmdnode[0])
	{
		rpt_mutex_unlock(&myrpt->lock);
		send_link_dtmf(myrpt,c);
		return;
	}
	if (myrpt->callmode == 1)
	{
		myrpt->exten[myrpt->cidx++] = c;
		myrpt->exten[myrpt->cidx] = 0;
		/* if this exists */
		if (ast_exists_extension(myrpt->pchannel,myrpt->patchcontext,myrpt->exten,1,NULL))
		{
			/* if this really it, end now */
			if (!ast_matchmore_extension(myrpt->pchannel,myrpt->patchcontext,
				myrpt->exten,1,NULL))
			{
				myrpt->callmode = 2;
				if(!myrpt->patchquiet)
				{
					rpt_mutex_unlock(&myrpt->lock);
					rpt_telemetry(myrpt,PROC,NULL);
					rpt_mutex_lock(&myrpt->lock);
				}
			}
			else /* othewise, reset timer */
			{
				myrpt->calldigittimer = 1;
			}
		}
		/* if can continue, do so */
		if (!ast_canmatch_extension(myrpt->pchannel,myrpt->patchcontext,myrpt->exten,1,NULL))
		{
			/* call has failed, inform user */
			myrpt->callmode = 4;
		}
	}
	if ((c != myrpt->p.funcchar) && (myrpt->rem_dtmfidx < 0) &&
	  (!myrpt->inpadtest) &&
	    ((myrpt->callmode == 2) || (myrpt->callmode == 3)))
	{
		myrpt->mydtmf = c;
	}
	if ((!myrpt->inpadtest) && (c == myrpt->p.funcchar))
	{
		myrpt->rem_dtmfidx = 0;
		myrpt->rem_dtmfbuf[myrpt->rem_dtmfidx] = 0;
		time(&myrpt->rem_dtmf_time);
		rpt_mutex_unlock(&myrpt->lock);
		return;
	}
	else if (((myrpt->inpadtest) || (c != myrpt->p.endchar)) && (myrpt->rem_dtmfidx >= 0))
	{
		time(&myrpt->rem_dtmf_time);
		if (myrpt->rem_dtmfidx < MAXDTMF)
		{
			myrpt->rem_dtmfbuf[myrpt->rem_dtmfidx++] = c;
			myrpt->rem_dtmfbuf[myrpt->rem_dtmfidx] = 0;

			rpt_mutex_unlock(&myrpt->lock);
			strncpy(cmd, myrpt->rem_dtmfbuf, sizeof(cmd) - 1);
			switch(mylink->phonemode)
			{
			    case 1:
				res = collect_function_digits(myrpt, cmd,
					SOURCE_PHONE, mylink);
				break;
			    case 2:
				res = collect_function_digits(myrpt, cmd,
					SOURCE_DPHONE,mylink);
				break;
			    case 4:
				res = collect_function_digits(myrpt, cmd,
					SOURCE_ALT,mylink);
				break;
			    default:
				res = collect_function_digits(myrpt, cmd,
					SOURCE_LNK, mylink);
				break;
			}

			rpt_mutex_lock(&myrpt->lock);

			switch(res){

				case DC_INDETERMINATE:
					break;

				case DC_DOKEY:
					mylink->lastrealrx = 1;
					break;

				case DC_REQ_FLUSH:
					myrpt->rem_dtmfidx = 0;
					myrpt->rem_dtmfbuf[0] = 0;
					break;

				case DC_COMPLETE:
				case DC_COMPLETEQUIET:
					myrpt->totalexecdcommands++;
					myrpt->dailyexecdcommands++;
					strncpy(myrpt->lastdtmfcommand, cmd, MAXDTMF-1);
					myrpt->lastdtmfcommand[MAXDTMF-1] = '\0';
					myrpt->rem_dtmfbuf[0] = 0;
					myrpt->rem_dtmfidx = -1;
					myrpt->rem_dtmf_time = 0;
					break;

				case DC_ERROR:
				default:
					myrpt->rem_dtmfbuf[0] = 0;
					myrpt->rem_dtmfidx = -1;
					myrpt->rem_dtmf_time = 0;
					break;
			}
		}
	}
	else if (myrpt->p.propagate_phonedtmf) do_dtmf_local(myrpt,c);
	rpt_mutex_unlock(&myrpt->lock);
	return;
}

/* Doug Hall RBI-1 serial data definitions:
 *
 * Byte 0: Expansion external outputs
 * Byte 1:
 *	Bits 0-3 are BAND as follows:
 *	Bits 4-5 are POWER bits as follows:
 *		00 - Low Power
 *		01 - Hi Power
 *		02 - Med Power
 *	Bits 6-7 are always set
 * Byte 2:
 *	Bits 0-3 MHZ in BCD format
 *	Bits 4-5 are offset as follows:
 *		00 - minus
 *		01 - plus
 *		02 - simplex
 *		03 - minus minus (whatever that is)
 *	Bit 6 is the 0/5 KHZ bit
 *	Bit 7 is always set
 * Byte 3:
 *	Bits 0-3 are 10 KHZ in BCD format
 *	Bits 4-7 are 100 KHZ in BCD format
 * Byte 4: PL Tone code and encode/decode enable bits
 *	Bits 0-5 are PL tone code (comspec binary codes)
 *	Bit 6 is encode enable/disable
 *	Bit 7 is decode enable/disable
 */

/* take the frequency from the 10 mhz digits (and up) and convert it
   to a band number */

static int rbi_mhztoband(char *str)
{
int	i;

	i = atoi(str) / 10; /* get the 10's of mhz */
	switch(i)
	{
	    case 2:
		return 10;
	    case 5:
		return 11;
	    case 14:
		return 2;
	    case 22:
		return 3;
	    case 44:
		return 4;
	    case 124:
		return 0;
	    case 125:
		return 1;
	    case 126:
		return 8;
	    case 127:
		return 5;
	    case 128:
		return 6;
	    case 129:
		return 7;
	    default:
		break;
	}
	return -1;
}

/* take a PL frequency and turn it into a code */
static int rbi_pltocode(char *str)
{
int i;
char *s;

	s = strchr(str,'.');
	i = 0;
	if (s) i = atoi(s + 1);
	i += atoi(str) * 10;
	switch(i)
	{
	    case 670:
		return 0;
	    case 719:
		return 1;
	    case 744:
		return 2;
	    case 770:
		return 3;
	    case 797:
		return 4;
	    case 825:
		return 5;
	    case 854:
		return 6;
	    case 885:
		return 7;
	    case 915:
		return 8;
	    case 948:
		return 9;
	    case 974:
		return 10;
	    case 1000:
		return 11;
	    case 1035:
		return 12;
	    case 1072:
		return 13;
	    case 1109:
		return 14;
	    case 1148:
		return 15;
	    case 1188:
		return 16;
	    case 1230:
		return 17;
	    case 1273:
		return 18;
	    case 1318:
		return 19;
	    case 1365:
		return 20;
	    case 1413:
		return 21;
	    case 1462:
		return 22;
	    case 1514:
		return 23;
	    case 1567:
		return 24;
	    case 1622:
		return 25;
	    case 1679:
		return 26;
	    case 1738:
		return 27;
	    case 1799:
		return 28;
	    case 1862:
		return 29;
	    case 1928:
		return 30;
	    case 2035:
		return 31;
	    case 2107:
		return 32;
	    case 2181:
		return 33;
	    case 2257:
		return 34;
	    case 2336:
		return 35;
	    case 2418:
		return 36;
	    case 2503:
		return 37;
	}
	return -1;
}

/*
* Shift out a formatted serial bit stream
*/

static void rbi_out_parallel(struct rpt *myrpt,unsigned char *data)
    {
#ifdef __i386__
    int i,j;
    unsigned char od,d;
    static volatile long long delayvar;

    for(i = 0 ; i < 5 ; i++){
        od = *data++;
        for(j = 0 ; j < 8 ; j++){
            d = od & 1;
            outb(d,myrpt->p.iobase);
	    /* >= 15 us */
	    for(delayvar = 1; delayvar < 15000; delayvar++);
            od >>= 1;
            outb(d | 2,myrpt->p.iobase);
	    /* >= 30 us */
	    for(delayvar = 1; delayvar < 30000; delayvar++);
            outb(d,myrpt->p.iobase);
	    /* >= 10 us */
	    for(delayvar = 1; delayvar < 10000; delayvar++);
            }
        }
	/* >= 50 us */
        for(delayvar = 1; delayvar < 50000; delayvar++);
#endif
    }

static void rbi_out(struct rpt *myrpt,unsigned char *data)
{
struct dahdi_radio_param r;

	memset(&r,0,sizeof(struct dahdi_radio_param));
	r.radpar = DAHDI_RADPAR_REMMODE;
	r.data = DAHDI_RADPAR_REM_RBI1;
	/* if setparam ioctl fails, its probably not a pciradio card */
	if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_RADIO_SETPARAM,&r) == -1)
	{
		rbi_out_parallel(myrpt,data);
		return;
	}
	r.radpar = DAHDI_RADPAR_REMCOMMAND;
	memcpy(&r.data,data,5);
	if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_RADIO_SETPARAM,&r) == -1)
	{
		ast_log(LOG_WARNING,"Cannot send RBI command for channel %s\n",myrpt->zaprxchannel->name);
		return;
	}
}

static int serial_remote_io(struct rpt *myrpt, unsigned char *txbuf, int txbytes,
	unsigned char *rxbuf, int rxmaxbytes, int asciiflag)
{
	int i,j,index,oldmode,olddata;
	struct dahdi_radio_param prm;
	char c;

	 if(debug) {
	    ast_log(LOG_NOTICE, "ioport=%s  iofd=0x%x\n",myrpt->p.ioport,myrpt->iofd);
		printf("String output was:\n");
		for(i = 0; i < txbytes; i++)
			printf("%02X ", (unsigned char ) txbuf[i]);
		printf("\n");
	}

	if (myrpt->iofd >= 0)  /* if to do out a serial port */
	{
		if(!strcmp(myrpt->remoterig, remote_rig_tm271))
		{
			/* pause otherwise radio may not recognize the serial data */
			for(i = 0; i < txbytes; i++)
			{
				if (write(myrpt->iofd,&txbuf[i],1) != 1) return -1;
				usleep(6666);
			}
		}
		else
		{
			if (write(myrpt->iofd,txbuf,txbytes) != txbytes)
			{
				return -1;
			}
		}
		if ((!rxmaxbytes) || (rxbuf == NULL))
		{
			return(0);
		}
		memset(rxbuf,0,rxmaxbytes);
		for(i = 0; i < rxmaxbytes; i++)
		{
			j = read(myrpt->iofd,&c,1);
			if (j < 1)
			{
				return(i);
			}
			rxbuf[i] = c;
			if (asciiflag & 1)
			{
				rxbuf[i + 1] = 0;
				if (c == '\r') break;
			}
		}
		if(debug) {
			printf("String returned was:\n");
			for(j = 0; j < i; j++)
				printf("%02X ", (unsigned char ) rxbuf[j]);
			printf("\n");
		}
		return(i);
	}

	/* if not a dahdi channel, cant use pciradio stuff */
	if (myrpt->rxchannel != myrpt->zaprxchannel) return -1;

	prm.radpar = DAHDI_RADPAR_UIOMODE;
	if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_RADIO_GETPARAM,&prm) == -1) return -1;
	oldmode = prm.data;
	prm.radpar = DAHDI_RADPAR_UIODATA;
	if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_RADIO_GETPARAM,&prm) == -1) return -1;
	olddata = prm.data;
        prm.radpar = DAHDI_RADPAR_REMMODE;
        if (asciiflag & 1)  prm.data = DAHDI_RADPAR_REM_SERIAL_ASCII;
        else prm.data = DAHDI_RADPAR_REM_SERIAL;
	if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_RADIO_SETPARAM,&prm) == -1) return -1;
	if (asciiflag & 2)
	{
		i = DAHDI_ONHOOK;
		if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_HOOK,&i) == -1) return -1;
		usleep(100000);
	}
        prm.radpar = DAHDI_RADPAR_REMCOMMAND;
        prm.data = rxmaxbytes;
        memcpy(prm.buf,txbuf,txbytes);
        prm.index = txbytes;
	if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_RADIO_SETPARAM,&prm) == -1) return -1;
        if (rxbuf)
        {
                *rxbuf = 0;
                memcpy(rxbuf,prm.buf,prm.index);
        }
	index = prm.index;
        prm.radpar = DAHDI_RADPAR_REMMODE;
        prm.data = DAHDI_RADPAR_REM_NONE;
	if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_RADIO_SETPARAM,&prm) == -1) return -1;
	if (asciiflag & 2)
	{
		i = DAHDI_OFFHOOK;
		if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_HOOK,&i) == -1) return -1;
	}
	prm.radpar = DAHDI_RADPAR_UIOMODE;
	prm.data = oldmode;
	if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_RADIO_SETPARAM,&prm) == -1) return -1;
	prm.radpar = DAHDI_RADPAR_UIODATA;
	prm.data = olddata;
	if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_RADIO_SETPARAM,&prm) == -1) return -1;
        return(index);
}

static int civ_cmd(struct rpt *myrpt,unsigned char *cmd, int cmdlen)
{
unsigned char rxbuf[100];
int	i,rv ;

	rv = serial_remote_io(myrpt,cmd,cmdlen,rxbuf,cmdlen + 6,0);
	if (rv == -1) return(-1);
	if (rv != (cmdlen + 6)) return(1);
	for(i = 0; i < 6; i++)
		if (rxbuf[i] != cmd[i]) return(1);
	if (rxbuf[cmdlen] != 0xfe) return(1);
	if (rxbuf[cmdlen + 1] != 0xfe) return(1);
	if (rxbuf[cmdlen + 4] != 0xfb) return(1);
	if (rxbuf[cmdlen + 5] != 0xfd) return(1);
	return(0);
}

static int sendkenwood(struct rpt *myrpt,char *txstr, char *rxstr)
{
int	i;

	if (debug) ast_log(LOG_NOTICE,"[%s] Send to kenwood: %s\n",myrpt->name,txstr);
	i = serial_remote_io(myrpt, (unsigned char *)txstr, strlen(txstr),
		(unsigned char *)rxstr,RAD_SERIAL_BUFLEN - 1,3);
	if (i < 0) return -1;
	if ((i > 0) && (rxstr[i - 1] == '\r'))
		rxstr[i-- - 1] = 0;
	if (debug) ast_log(LOG_NOTICE,"[%s] Got from kenwood: %s\n",myrpt->name,rxstr);
	return(i);
}

/* take a PL frequency and turn it into a code */
static int kenwood_pltocode(char *str)
{
int i;
char *s;

	s = strchr(str,'.');
	i = 0;
	if (s) i = atoi(s + 1);
	i += atoi(str) * 10;
	switch(i)
	{
	    case 670:
		return 1;
	    case 719:
		return 3;
	    case 744:
		return 4;
	    case 770:
		return 5;
	    case 797:
		return 6;
	    case 825:
		return 7;
	    case 854:
		return 8;
	    case 885:
		return 9;
	    case 915:
		return 10;
	    case 948:
		return 11;
	    case 974:
		return 12;
	    case 1000:
		return 13;
	    case 1035:
		return 14;
	    case 1072:
		return 15;
	    case 1109:
		return 16;
	    case 1148:
		return 17;
	    case 1188:
		return 18;
	    case 1230:
		return 19;
	    case 1273:
		return 20;
	    case 1318:
		return 21;
	    case 1365:
		return 22;
	    case 1413:
		return 23;
	    case 1462:
		return 24;
	    case 1514:
		return 25;
	    case 1567:
		return 26;
	    case 1622:
		return 27;
	    case 1679:
		return 28;
	    case 1738:
		return 29;
	    case 1799:
		return 30;
	    case 1862:
		return 31;
	    case 1928:
		return 32;
	    case 2035:
		return 33;
	    case 2107:
		return 34;
	    case 2181:
		return 35;
	    case 2257:
		return 36;
	    case 2336:
		return 37;
	    case 2418:
		return 38;
	    case 2503:
		return 39;
	}
	return -1;
}

/* take a PL frequency and turn it into a code */
static int tm271_pltocode(char *str)
{
int i;
char *s;

	s = strchr(str,'.');
	i = 0;
	if (s) i = atoi(s + 1);
	i += atoi(str) * 10;
	switch(i)
	{
	    case 670:
		return 0;
	    case 693:
		return 1;
	    case 719:
		return 2;
	    case 744:
		return 3;
	    case 770:
		return 4;
	    case 797:
		return 5;
	    case 825:
		return 6;
	    case 854:
		return 7;
	    case 885:
		return 8;
	    case 915:
		return 9;
	    case 948:
		return 10;
	    case 974:
		return 11;
	    case 1000:
		return 12;
	    case 1035:
		return 13;
	    case 1072:
		return 14;
	    case 1109:
		return 15;
	    case 1148:
		return 16;
	    case 1188:
		return 17;
	    case 1230:
		return 18;
	    case 1273:
		return 19;
	    case 1318:
		return 20;
	    case 1365:
		return 21;
	    case 1413:
		return 22;
	    case 1462:
		return 23;
	    case 1514:
		return 24;
	    case 1567:
		return 25;
	    case 1622:
		return 26;
	    case 1679:
		return 27;
	    case 1738:
		return 28;
	    case 1799:
		return 29;
	    case 1862:
		return 30;
	    case 1928:
		return 31;
	    case 2035:
		return 32;
	    case 2065:
		return 33;
	    case 2107:
		return 34;
	    case 2181:
		return 35;
	    case 2257:
		return 36;
	    case 2291:
		return 37;
	    case 2336:
		return 38;
	    case 2418:
		return 39;
	    case 2503:
		return 40;
	}
	return -1;
}

/* take a PL frequency and turn it into a code */
static int ft950_pltocode(char *str)
{
int i;
char *s;

	s = strchr(str,'.');
	i = 0;
	if (s) i = atoi(s + 1);
	i += atoi(str) * 10;
	switch(i)
	{
	    case 670:
		return 0;
	    case 693:
		return 1;
	    case 719:
		return 2;
	    case 744:
		return 3;
	    case 770:
		return 4;
	    case 797:
		return 5;
	    case 825:
		return 6;
	    case 854:
		return 7;
	    case 885:
		return 8;
	    case 915:
		return 9;
	    case 948:
		return 10;
	    case 974:
		return 11;
	    case 1000:
		return 12;
	    case 1035:
		return 13;
	    case 1072:
		return 14;
	    case 1109:
		return 15;
	    case 1148:
		return 16;
	    case 1188:
		return 17;
	    case 1230:
		return 18;
	    case 1273:
		return 19;
	    case 1318:
		return 20;
	    case 1365:
		return 21;
	    case 1413:
		return 22;
	    case 1462:
		return 23;
	    case 1514:
		return 24;
	    case 1567:
		return 25;
	    case 1622:
		return 26;
	    case 1679:
		return 27;
	    case 1738:
		return 28;
	    case 1799:
		return 29;
	    case 1862:
		return 30;
	    case 1928:
		return 31;
	    case 2035:
		return 32;
	    case 2065:
		return 33;
	    case 2107:
		return 34;
	    case 2181:
		return 35;
	    case 2257:
		return 36;
	    case 2291:
		return 37;
	    case 2336:
		return 38;
	    case 2418:
		return 39;
	    case 2503:
		return 40;
	}
	return -1;
}


static int sendrxkenwood(struct rpt *myrpt, char *txstr, char *rxstr,
	char *cmpstr)
{
int	i,j;

	for(i = 0;i < KENWOOD_RETRIES;i++)
	{
		j = sendkenwood(myrpt,txstr,rxstr);
		if (j < 0) return(j);
		if (j == 0) continue;
		if (!strncmp(rxstr,cmpstr,strlen(cmpstr))) return(0);
	}
	return(-1);
}

static int setkenwood(struct rpt *myrpt)
{
char rxstr[RAD_SERIAL_BUFLEN],txstr[RAD_SERIAL_BUFLEN],freq[20];
char mhz[MAXREMSTR],offset[20],band,decimals[MAXREMSTR],band1,band2;
int myrxpl;

int offsets[] = {0,2,1};
int powers[] = {2,1,0};

	if (sendrxkenwood(myrpt,"VMC 0,0\r",rxstr,"VMC") < 0) return -1;
	split_freq(mhz, decimals, myrpt->freq);
	if (atoi(mhz) > 400)
	{
		band = '6';
		band1 = '1';
		band2 = '5';
		strcpy(offset,"005000000");
	}
	else
	{
		band = '2';
		band1 = '0';
		band2 = '2';
		strcpy(offset,"000600000");
	}
	strcpy(freq,"000000");
	strncpy(freq,decimals,strlen(decimals));
	myrxpl = myrpt->rxplon;
	if (IS_XPMR(myrpt)) myrxpl = 0;
	sprintf(txstr,"VW %c,%05d%s,0,%d,0,%d,%d,,%02d,,%02d,%s\r",
		band,atoi(mhz),freq,offsets[(int)myrpt->offset],
		(myrpt->txplon != 0),myrxpl,
		kenwood_pltocode(myrpt->txpl),kenwood_pltocode(myrpt->rxpl),
		offset);
	if (sendrxkenwood(myrpt,txstr,rxstr,"VW") < 0) return -1;
	sprintf(txstr,"RBN %c\r",band2);
	if (sendrxkenwood(myrpt,txstr,rxstr,"RBN") < 0) return -1;
	sprintf(txstr,"PC %c,%d\r",band1,powers[(int)myrpt->powerlevel]);
	if (sendrxkenwood(myrpt,txstr,rxstr,"PC") < 0) return -1;
	return 0;
}

static int set_tm271(struct rpt *myrpt)
{
char rxstr[RAD_SERIAL_BUFLEN],txstr[RAD_SERIAL_BUFLEN],freq[20];
char mhz[MAXREMSTR],decimals[MAXREMSTR];

int offsets[] = {0,2,1};
int powers[] = {2,1,0};

	split_freq(mhz, decimals, myrpt->freq);
	strcpy(freq,"000000");
	strncpy(freq,decimals,strlen(decimals));

	sprintf(txstr,"VF %04d%s,0,%d,0,%d,0,0,%02d,00,000,00600000,0,0\r",
		atoi(mhz),freq,offsets[(int)myrpt->offset],
		(myrpt->txplon != 0),tm271_pltocode(myrpt->txpl));

	if (sendrxkenwood(myrpt,txstr,rxstr,"VF") < 0) return -1;
	if (sendrxkenwood(myrpt,"VM 0\r",rxstr,"VM") < 0) return -1;
	sprintf(txstr,"PC %d\r",powers[(int)myrpt->powerlevel]);
	if (sendrxkenwood(myrpt,txstr,rxstr,"PC") < 0) return -1;
	return 0;
}

static int setrbi(struct rpt *myrpt)
{
char tmp[MAXREMSTR] = "",*s;
unsigned char rbicmd[5];
int	band,txoffset = 0,txpower = 0,rxpl;

	/* must be a remote system */
	if (!myrpt->remoterig) return(0);
	if (!myrpt->remoterig[0]) return(0);
	/* must have rbi hardware */
	if (strncmp(myrpt->remoterig,remote_rig_rbi,3)) return(0);
	if (setrbi_check(myrpt) == -1) return(-1);
	strncpy(tmp, myrpt->freq, sizeof(tmp) - 1);
	s = strchr(tmp,'.');
	/* if no decimal, is invalid */

	if (s == NULL){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Frequency needs a decimal\n",myrpt->name);
		return -1;
	}

	*s++ = 0;
	if (strlen(tmp) < 2){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad MHz digits: %s\n",myrpt->name,tmp);
	 	return -1;
	}

	if (strlen(s) < 3){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad KHz digits: %s\n",myrpt->name, s);
	 	return -1;
	}

	if ((s[2] != '0') && (s[2] != '5')){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] KHz must end in 0 or 5: %c\n",myrpt->name, s[2]);
	 	return -1;
	}

	band = rbi_mhztoband(tmp);
	if (band == -1){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad Band: %s\n",myrpt->name, tmp);
	 	return -1;
	}

	rxpl = rbi_pltocode(myrpt->rxpl);

	if (rxpl == -1){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad TX PL: %s\n",myrpt->name, myrpt->rxpl);
	 	return -1;
	}

	switch(myrpt->offset)
	{
	    case REM_MINUS:
		txoffset = 0;
		break;
	    case REM_PLUS:
		txoffset = 0x10;
		break;
	    case REM_SIMPLEX:
		txoffset = 0x20;
		break;
	}
	switch(myrpt->powerlevel)
	{
	    case REM_LOWPWR:
		txpower = 0;
		break;
	    case REM_MEDPWR:
		txpower = 0x20;
		break;
	    case REM_HIPWR:
		txpower = 0x10;
		break;
	}
	rbicmd[0] = 0;
	rbicmd[1] = band | txpower | 0xc0;
	rbicmd[2] = (*(s - 2) - '0') | txoffset | 0x80;
	if (s[2] == '5') rbicmd[2] |= 0x40;
	rbicmd[3] = ((*s - '0') << 4) + (s[1] - '0');
	rbicmd[4] = rxpl;
	if (myrpt->txplon) rbicmd[4] |= 0x40;
	if (myrpt->rxplon) rbicmd[4] |= 0x80;
	rbi_out(myrpt,rbicmd);
	return 0;
}

static int setrtx(struct rpt *myrpt)
{
char tmp[MAXREMSTR] = "",*s,rigstr[200],pwr,res = 0;
int	band,txoffset = 0,txpower = 0,rxpl,txpl;
float ofac;
double txfreq;

	/* must be a remote system */
	if (!myrpt->remoterig) return(0);
	if (!myrpt->remoterig[0]) return(0);
	/* must have rtx hardware */
	if (!ISRIG_RTX(myrpt->remoterig)) return(0);
	/* must be a usb radio interface type */
	if (!IS_XPMR(myrpt)) return(0);
	strncpy(tmp, myrpt->freq, sizeof(tmp) - 1);
	s = strchr(tmp,'.');
	/* if no decimal, is invalid */

	if(debug)ast_log(LOG_NOTICE,"[%s] setrtx() %s\n",myrpt->name,myrpt->remoterig);

	if (s == NULL){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Frequency needs a decimal\n",myrpt->name);
		return -1;
	}
	*s++ = 0;
	if (strlen(tmp) < 2){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad MHz digits: %s\n",myrpt->name, tmp);
	 	return -1;
	}

	if (strlen(s) < 3){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad KHz digits: %s\n",myrpt->name, s);
	 	return -1;
	}

	if ((s[2] != '0') && (s[2] != '5')){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] KHz must end in 0 or 5: %c\n",myrpt->name, s[2]);
	 	return -1;
	}

	if(!myrpt->p.remote_mars)
	{
	band = rbi_mhztoband(tmp);
	if (band == -1){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad Band: %s\n",myrpt->name, tmp);
	 	return -1;
	}

	rxpl = rbi_pltocode(myrpt->rxpl);

	if (rxpl == -1){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad RX PL: %s\n",myrpt->name, myrpt->rxpl);
	 	return -1;
	}

	txpl = rbi_pltocode(myrpt->txpl);

	if (txpl == -1){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad TX PL: %s\n",myrpt->name, myrpt->txpl);
	 	return -1;
	}
	}

	switch(myrpt->offset)
	{
	    case REM_MINUS:
		txoffset = 0;
		break;
	    case REM_PLUS:
		txoffset = 0x10;
		break;
	    case REM_SIMPLEX:
		txoffset = 0x20;
		break;
	    case REM_WILD:
		txoffset = 0x30;
		break;
	}
	switch(myrpt->powerlevel)
	{
	    case REM_LOWPWR:
		txpower = 0;
		break;
	    case REM_MEDPWR:
		txpower = 0x20;
		break;
	    case REM_HIPWR:
		txpower = 0x10;
		break;
	}

	res = setrtx_check(myrpt);
	if (res < 0) return res;
	ofac = 0.0;

	if (myrpt->offset == REM_WILD)
	{
		txfreq = atof(myrpt->freqtx);
	}
	else
	{
	if (myrpt->offset == REM_MINUS) ofac = -1.0;
	if (myrpt->offset == REM_PLUS) ofac = 1.0;

	if (!strcmp(myrpt->remoterig,remote_rig_rtx450))
		txfreq = atof(myrpt->freq) +  (ofac * 5.0);
	else
		txfreq = atof(myrpt->freq) +  (ofac * 0.6);
	}

	pwr = 'L';
	if (myrpt->powerlevel == REM_HIPWR) pwr = 'H';
	if (!res)
	{
		sprintf(rigstr,"SETFREQ %s %f %s %s %c",myrpt->freq,txfreq,
			(myrpt->rxplon) ? myrpt->rxpl : "0.0",
			(myrpt->txplon) ? myrpt->txpl : "0.0",pwr);

		send_usb_txt(myrpt,rigstr);
		rpt_telemetry(myrpt,COMPLETE,NULL);
		res = 0;
	}
	return 0;
}

static int setxpmr(struct rpt *myrpt, int dotx)
{
	char rigstr[200];
	int rxpl,txpl;

	/* must be a remote system */
	if (!myrpt->remoterig) return(0);
	if (!myrpt->remoterig[0]) return(0);
	/* must not have rtx hardware */
	if (ISRIG_RTX(myrpt->remoterig)) return(0);
	/* must be a usb radio interface type */
	if (!IS_XPMR(myrpt)) return(0);

	if(debug)ast_log(LOG_NOTICE,"[%s] setxpmr() %s\n",myrpt->name,myrpt->remoterig );

	rxpl = rbi_pltocode(myrpt->rxpl);

	if (rxpl == -1){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad RX PL: %s\n",myrpt->name, myrpt->rxpl);
	 	return -1;
	}

	if (dotx)
	{
		txpl = rbi_pltocode(myrpt->txpl);
		if (txpl == -1){
			if(debug)
				ast_log(LOG_NOTICE,"[%s] Bad TX PL: %s\n",myrpt->name, myrpt->txpl);
		 	return -1;
		}
		sprintf(rigstr,"SETFREQ 0.0 0.0 %s %s L",
			(myrpt->rxplon) ? myrpt->rxpl : "0.0",
			(myrpt->txplon) ? myrpt->txpl : "0.0");
	}
	else
	{
		sprintf(rigstr,"SETFREQ 0.0 0.0 %s 0.0 L",
			(myrpt->rxplon) ? myrpt->rxpl : "0.0");

	}
	send_usb_txt(myrpt,rigstr);
	return 0;
}


static int setrbi_check(struct rpt *myrpt)
{
char tmp[MAXREMSTR] = "",*s;
int	band,txpl;

	/* must be a remote system */
	if (!myrpt->remote) return(0);
	/* must have rbi hardware */
	if (strncmp(myrpt->remoterig,remote_rig_rbi,3)) return(0);
	strncpy(tmp, myrpt->freq, sizeof(tmp) - 1);
	s = strchr(tmp,'.');
	/* if no decimal, is invalid */

	if (s == NULL){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Frequency needs a decimal\n",myrpt->name);
		return -1;
	}

	*s++ = 0;
	if (strlen(tmp) < 2){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad MHz digits: %s\n",myrpt->name, tmp);
	 	return -1;
	}

	if (strlen(s) < 3){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad KHz digits: %s\n",myrpt->name, s);
	 	return -1;
	}

	if ((s[2] != '0') && (s[2] != '5')){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] KHz must end in 0 or 5: %c\n",myrpt->name, s[2]);
	 	return -1;
	}

	band = rbi_mhztoband(tmp);
	if (band == -1){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad Band: %s\n",myrpt->name, tmp);
	 	return -1;
	}

	txpl = rbi_pltocode(myrpt->txpl);

	if (txpl == -1){
		if(debug)
			ast_log(LOG_NOTICE,"[%s] Bad TX PL: %s\n",myrpt->name, myrpt->txpl);
	 	return -1;
	}
	return 0;
}

static int setrtx_check(struct rpt *myrpt)
{
char tmp[MAXREMSTR] = "",*s;
int	band,txpl,rxpl;

	/* must be a remote system */
	if (!myrpt->remote) return(0);
	/* must have rbi hardware */
	if (strncmp(myrpt->remoterig,remote_rig_rbi,3)) return(0);
	strncpy(tmp, myrpt->freq, sizeof(tmp) - 1);
	s = strchr(tmp,'.');
	/* if no decimal, is invalid */

	if (s == NULL){
		if(debug)
			ast_log(LOG_NOTICE,"Frequency needs a decimal\n");
		return -1;
	}

	*s++ = 0;
	if (strlen(tmp) < 2){
		if(debug)
			ast_log(LOG_NOTICE,"Bad MHz digits: %s\n", tmp);
	 	return -1;
	}

	if (strlen(s) < 3){
		if(debug)
			ast_log(LOG_NOTICE,"Bad KHz digits: %s\n", s);
	 	return -1;
	}

	if ((s[2] != '0') && (s[2] != '5')){
		if(debug)
			ast_log(LOG_NOTICE,"KHz must end in 0 or 5: %c\n", s[2]);
	 	return -1;
	}

	band = rbi_mhztoband(tmp);
	if (band == -1){
		if(debug)
			ast_log(LOG_NOTICE,"Bad Band: %s\n", tmp);
	 	return -1;
	}

	txpl = rbi_pltocode(myrpt->txpl);

	if (txpl == -1){
		if(debug)
			ast_log(LOG_NOTICE,"Bad TX PL: %s\n", myrpt->txpl);
	 	return -1;
	}

	rxpl = rbi_pltocode(myrpt->rxpl);

	if (rxpl == -1){
		if(debug)
			ast_log(LOG_NOTICE,"Bad RX PL: %s\n", myrpt->rxpl);
	 	return -1;
	}
	return 0;
}

static int check_freq_kenwood(int m, int d, int *defmode)
{
	int dflmd = REM_MODE_FM;

	if (m == 144){ /* 2 meters */
		if(d < 10100)
			return -1;
	}
	else if((m >= 145) && (m < 148)){
		;
	}
	else if((m >= 430) && (m < 450)){ /* 70 centimeters */
		;
	}
	else
		return -1;

	if(defmode)
		*defmode = dflmd;


	return 0;
}


static int check_freq_tm271(int m, int d, int *defmode)
{
	int dflmd = REM_MODE_FM;

	if (m == 144){ /* 2 meters */
		if(d < 10100)
			return -1;
	}
	else if((m >= 145) && (m < 148)){
		;
	}
	else	return -1;

	if(defmode)
		*defmode = dflmd;


	return 0;
}


/* Check for valid rbi frequency */
/* Hard coded limits now, configurable later, maybe? */

static int check_freq_rbi(int m, int d, int *defmode)
{
	int dflmd = REM_MODE_FM;

	if(m == 50){ /* 6 meters */
		if(d < 10100)
			return -1;
	}
	else if((m >= 51) && ( m < 54)){
                ;
	}
	else if(m == 144){ /* 2 meters */
		if(d < 10100)
			return -1;
	}
	else if((m >= 145) && (m < 148)){
		;
	}
 	else if((m >= 222) && (m < 225)){ /* 1.25 meters */
		;
	}
	else if((m >= 430) && (m < 450)){ /* 70 centimeters */
		;
	}
	else if((m >= 1240) && (m < 1300)){ /* 23 centimeters */
		;
	}
	else
		return -1;

	if(defmode)
		*defmode = dflmd;


	return 0;
}

/* Check for valid rtx frequency */
/* Hard coded limits now, configurable later, maybe? */

static int check_freq_rtx(int m, int d, int *defmode, struct rpt *myrpt)
{
	int dflmd = REM_MODE_FM;

	if (!strcmp(myrpt->remoterig,remote_rig_rtx150))
	{

		if(m == 144){ /* 2 meters */
			if(d < 10100)
				return -1;
		}
		else if((m >= 145) && (m < 148)){
			;
		}
		else
			return -1;
	}
	else
	{
		if((m >= 430) && (m < 450)){ /* 70 centimeters */
			;
		}
		else
			return -1;
	}
	if(defmode)
		*defmode = dflmd;


	return 0;
}

/*
 * Convert decimals of frequency to int
 */

static int decimals2int(char *fraction)
{
	int i;
	char len = strlen(fraction);
	int multiplier = 100000;
	int res = 0;

	if(!len)
		return 0;
	for( i = 0 ; i < len ; i++, multiplier /= 10)
		res += (fraction[i] - '0') * multiplier;
	return res;
}


/*
* Split frequency into mhz and decimals
*/

static int split_freq(char *mhz, char *decimals, char *freq)
{
	char freq_copy[MAXREMSTR];
	char *decp;

	decp = strchr(strncpy(freq_copy, freq, MAXREMSTR),'.');
	if(decp){
		*decp++ = 0;
		strncpy(mhz, freq_copy, MAXREMSTR);
		strcpy(decimals, "00000");
		strncpy(decimals, decp, strlen(decp));
		decimals[5] = 0;
		return 0;
	}
	else
		return -1;

}

/*
* Split ctcss frequency into hertz and decimal
*/

static int split_ctcss_freq(char *hertz, char *decimal, char *freq)
{
	char freq_copy[MAXREMSTR];
	char *decp;

	decp = strchr(strncpy(freq_copy, freq, MAXREMSTR),'.');
	if(decp){
		*decp++ = 0;
		strncpy(hertz, freq_copy, MAXREMSTR);
		strncpy(decimal, decp, strlen(decp));
		decimal[strlen(decp)] = '\0';
		return 0;
	}
	else
		return -1;
}



/*
* FT-897 I/O handlers
*/

/* Check to see that the frequency is valid */
/* Hard coded limits now, configurable later, maybe? */


static int check_freq_ft897(int m, int d, int *defmode)
{
	int dflmd = REM_MODE_FM;

	if(m == 1){ /* 160 meters */
		dflmd =	REM_MODE_LSB;
		if(d < 80000)
			return -1;
	}
	else if(m == 3){ /* 80 meters */
		dflmd = REM_MODE_LSB;
		if(d < 50000)
			return -1;
	}
	else if(m == 7){ /* 40 meters */
		dflmd = REM_MODE_LSB;
		if(d > 30000)
			return -1;
	}
	else if(m == 14){ /* 20 meters */
		dflmd = REM_MODE_USB;
		if(d > 35000)
			return -1;
	}
	else if(m == 18){ /* 17 meters */
		dflmd = REM_MODE_USB;
		if((d < 6800) || (d > 16800))
			return -1;
	}
	else if(m == 21){ /* 15 meters */
		dflmd = REM_MODE_USB;
		if((d < 20000) || (d > 45000))
			return -1;
	}
	else if(m == 24){ /* 12 meters */
		dflmd = REM_MODE_USB;
		if((d < 89000) || (d > 99000))
			return -1;
	}
	else if(m == 28){ /* 10 meters */
		dflmd = REM_MODE_USB;
	}
	else if(m == 29){
		if(d >= 51000)
			dflmd = REM_MODE_FM;
		else
			dflmd = REM_MODE_USB;
		if(d > 70000)
			return -1;
	}
	else if(m == 50){ /* 6 meters */
		if(d >= 30000)
			dflmd = REM_MODE_FM;
		else
			dflmd = REM_MODE_USB;

	}
	else if((m >= 51) && ( m < 54)){
		dflmd = REM_MODE_FM;
	}
	else if(m == 144){ /* 2 meters */
		if(d >= 30000)
			dflmd = REM_MODE_FM;
		else
			dflmd = REM_MODE_USB;
	}
	else if((m >= 145) && (m < 148)){
		dflmd = REM_MODE_FM;
	}
	else if((m >= 430) && (m < 450)){ /* 70 centimeters */
		if(m  < 438)
			dflmd = REM_MODE_USB;
		else
			dflmd = REM_MODE_FM;
		;
	}
	else
		return -1;

	if(defmode)
		*defmode = dflmd;

	return 0;
}

/*
* Set a new frequency for the FT897
*/

static int set_freq_ft897(struct rpt *myrpt, char *newfreq)
{
	unsigned char cmdstr[5];
	int fd,m,d;
	char mhz[MAXREMSTR];
	char decimals[MAXREMSTR];

	fd = 0;
	if(debug)
		ast_log(LOG_NOTICE,"New frequency: %s\n",newfreq);

	if(split_freq(mhz, decimals, newfreq))
		return -1;

	m = atoi(mhz);
	d = atoi(decimals);

	/* The FT-897 likes packed BCD frequencies */

	cmdstr[0] = ((m / 100) << 4) + ((m % 100)/10);			/* 100MHz 10Mhz */
	cmdstr[1] = ((m % 10) << 4) + (d / 10000);			/* 1MHz 100KHz */
	cmdstr[2] = (((d % 10000)/1000) << 4) + ((d % 1000)/ 100);	/* 10KHz 1KHz */
	cmdstr[3] = (((d % 100)/10) << 4) + (d % 10);			/* 100Hz 10Hz */
	cmdstr[4] = 0x01;						/* command */

	return serial_remote_io(myrpt, cmdstr, 5, NULL, 0, 0);

}

/* ft-897 simple commands */

static int simple_command_ft897(struct rpt *myrpt, char command)
{
	unsigned char cmdstr[5];

	memset(cmdstr, 0, 5);

	cmdstr[4] = command;

	return serial_remote_io(myrpt, cmdstr, 5, NULL, 0, 0);

}

/* ft-897 offset */

static int set_offset_ft897(struct rpt *myrpt, char offset)
{
	unsigned char cmdstr[5];

	memset(cmdstr, 0, 5);

	switch(offset){
		case	REM_SIMPLEX:
			cmdstr[0] = 0x89;
			break;

		case	REM_MINUS:
			cmdstr[0] = 0x09;
			break;

		case	REM_PLUS:
			cmdstr[0] = 0x49;
			break;

		default:
			return -1;
	}

	cmdstr[4] = 0x09;

	return serial_remote_io(myrpt, cmdstr, 5, NULL, 0, 0);
}

/* ft-897 mode */

static int set_mode_ft897(struct rpt *myrpt, char newmode)
{
	unsigned char cmdstr[5];

	memset(cmdstr, 0, 5);

	switch(newmode){
		case	REM_MODE_FM:
			cmdstr[0] = 0x08;
			break;

		case	REM_MODE_USB:
			cmdstr[0] = 0x01;
			break;

		case	REM_MODE_LSB:
			cmdstr[0] = 0x00;
			break;

		case	REM_MODE_AM:
			cmdstr[0] = 0x04;
			break;

		default:
			return -1;
	}
	cmdstr[4] = 0x07;

	return serial_remote_io(myrpt, cmdstr, 5, NULL, 0, 0);
}

/* Set tone encode and decode modes */

static int set_ctcss_mode_ft897(struct rpt *myrpt, char txplon, char rxplon)
{
	unsigned char cmdstr[5];

	memset(cmdstr, 0, 5);

	if(rxplon && txplon)
		cmdstr[0] = 0x2A; /* Encode and Decode */
	else if (!rxplon && txplon)
		cmdstr[0] = 0x4A; /* Encode only */
	else if (rxplon && !txplon)
		cmdstr[0] = 0x3A; /* Encode only */
	else
		cmdstr[0] = 0x8A; /* OFF */

	cmdstr[4] = 0x0A;

	return serial_remote_io(myrpt, cmdstr, 5, NULL, 0, 0);
}


/* Set transmit and receive ctcss tone frequencies */

static int set_ctcss_freq_ft897(struct rpt *myrpt, char *txtone, char *rxtone)
{
	unsigned char cmdstr[5];
	char hertz[MAXREMSTR],decimal[MAXREMSTR];
	int h,d;

	memset(cmdstr, 0, 5);

	if(split_ctcss_freq(hertz, decimal, txtone))
		return -1;

	h = atoi(hertz);
	d = atoi(decimal);

	cmdstr[0] = ((h / 100) << 4) + (h % 100)/ 10;
	cmdstr[1] = ((h % 10) << 4) + (d % 10);

	if(rxtone){

		if(split_ctcss_freq(hertz, decimal, rxtone))
			return -1;

		h = atoi(hertz);
		d = atoi(decimal);

		cmdstr[2] = ((h / 100) << 4) + (h % 100)/ 10;
		cmdstr[3] = ((h % 10) << 4) + (d % 10);
	}
	cmdstr[4] = 0x0B;

	return serial_remote_io(myrpt, cmdstr, 5, NULL, 0, 0);
}



static int set_ft897(struct rpt *myrpt)
{
	int res;

	if(debug > 2)
		ast_log(LOG_NOTICE,"lock on\n");

	res = simple_command_ft897(myrpt, 0x00);	/* LOCK on */

	if(debug > 2)
		ast_log(LOG_NOTICE,"ptt off\n");

	if(!res)
		res = simple_command_ft897(myrpt, 0x88);		/* PTT off */

	if(debug > 2)
		ast_log(LOG_NOTICE,"Modulation mode\n");

	if(!res)
		res = set_mode_ft897(myrpt, myrpt->remmode);		/* Modulation mode */

	if(debug > 2)
		ast_log(LOG_NOTICE,"Split off\n");

	if(!res)
		simple_command_ft897(myrpt, 0x82);			/* Split off */

	if(debug > 2)
		ast_log(LOG_NOTICE,"Frequency\n");

	if(!res){
		res = set_freq_ft897(myrpt, myrpt->freq);		/* Frequency */
		usleep(FT897_SERIAL_DELAY*2);
	}
	if((myrpt->remmode == REM_MODE_FM)){
		if(debug > 2)
			ast_log(LOG_NOTICE,"Offset\n");
		if(!res){
			res = set_offset_ft897(myrpt, myrpt->offset);	/* Offset if FM */
			usleep(FT897_SERIAL_DELAY);
	    }
		if((!res)&&(myrpt->rxplon || myrpt->txplon)){
			if(debug > 2)
				ast_log(LOG_NOTICE,"CTCSS tone freqs.\n");
			usleep(FT897_SERIAL_DELAY);
			res = set_ctcss_freq_ft897(myrpt, myrpt->txpl, myrpt->rxpl); /* CTCSS freqs if CTCSS is enabled */
			usleep(FT897_SERIAL_DELAY);
		}
		if(!res){
			if(debug > 2)
				ast_log(LOG_NOTICE,"CTCSS mode\n");
			res = set_ctcss_mode_ft897(myrpt, myrpt->txplon, myrpt->rxplon); /* CTCSS mode */
			usleep(FT897_SERIAL_DELAY);
		}
	}
	if((myrpt->remmode == REM_MODE_USB)||(myrpt->remmode == REM_MODE_LSB)){
		if(debug > 2)
			ast_log(LOG_NOTICE,"Clarifier off\n");
		simple_command_ft897(myrpt, 0x85);			/* Clarifier off if LSB or USB */
	}
	return res;
}

static int closerem_ft897(struct rpt *myrpt)
{
	simple_command_ft897(myrpt, 0x88); /* PTT off */
	return 0;
}

/*
* Bump frequency up or down by a small amount
* Return 0 if the new frequnecy is valid, or -1 if invalid
* Interval is in Hz, resolution is 10Hz
*/

static int multimode_bump_freq_ft897(struct rpt *myrpt, int interval)
{
	int m,d;
	char mhz[MAXREMSTR], decimals[MAXREMSTR];

	if(debug)
		ast_log(LOG_NOTICE,"Before bump: %s\n", myrpt->freq);

	if(split_freq(mhz, decimals, myrpt->freq))
		return -1;

	m = atoi(mhz);
	d = atoi(decimals);

	d += (interval / 10); /* 10Hz resolution */
	if(d < 0){
		m--;
		d += 100000;
	}
	else if(d >= 100000){
		m++;
		d -= 100000;
	}

	if(check_freq_ft897(m, d, NULL)){
		if(debug)
			ast_log(LOG_NOTICE,"Bump freq invalid\n");
		return -1;
	}

	snprintf(myrpt->freq, MAXREMSTR, "%d.%05d", m, d);

	if(debug)
		ast_log(LOG_NOTICE,"After bump: %s\n", myrpt->freq);

	return set_freq_ft897(myrpt, myrpt->freq);
}

/*
* FT-950 I/O handlers
*/

/* Check to see that the frequency is valid */
/* Hard coded limits now, configurable later, maybe? */


static int check_freq_ft950(int m, int d, int *defmode)
{
	int dflmd = REM_MODE_FM;

	if(m == 1){ /* 160 meters */
		dflmd =	REM_MODE_LSB;
		if(d < 80000)
			return -1;
	}
	else if(m == 3){ /* 80 meters */
		dflmd = REM_MODE_LSB;
		if(d < 50000)
			return -1;
	}
	else if(m == 7){ /* 40 meters */
		dflmd = REM_MODE_LSB;
		if(d > 30000)
			return -1;
	}
	else if(m == 14){ /* 20 meters */
		dflmd = REM_MODE_USB;
		if(d > 35000)
			return -1;
	}
	else if(m == 18){ /* 17 meters */
		dflmd = REM_MODE_USB;
		if((d < 6800) || (d > 16800))
			return -1;
	}
	else if(m == 21){ /* 15 meters */
		dflmd = REM_MODE_USB;
		if((d < 20000) || (d > 45000))
			return -1;
	}
	else if(m == 24){ /* 12 meters */
		dflmd = REM_MODE_USB;
		if((d < 89000) || (d > 99000))
			return -1;
	}
	else if(m == 28){ /* 10 meters */
		dflmd = REM_MODE_USB;
	}
	else if(m == 29){
		if(d >= 51000)
			dflmd = REM_MODE_FM;
		else
			dflmd = REM_MODE_USB;
		if(d > 70000)
			return -1;
	}
	else if(m == 50){ /* 6 meters */
		if(d >= 30000)
			dflmd = REM_MODE_FM;
		else
			dflmd = REM_MODE_USB;

	}
	else if((m >= 51) && ( m < 54)){
		dflmd = REM_MODE_FM;
	}
	else
		return -1;

	if(defmode)
		*defmode = dflmd;

	return 0;
}

/*
* Set a new frequency for the ft950
*/

static int set_freq_ft950(struct rpt *myrpt, char *newfreq)
{
	char cmdstr[20];
	int fd,m,d;
	char mhz[MAXREMSTR];
	char decimals[MAXREMSTR];

	fd = 0;
	if(debug)
		ast_log(LOG_NOTICE,"New frequency: %s\n",newfreq);

	if(split_freq(mhz, decimals, newfreq))
		return -1;

	m = atoi(mhz);
	d = atoi(decimals);


	sprintf(cmdstr,"FA%d%06d;",m,d * 10);
	return serial_remote_io(myrpt, (unsigned char *)cmdstr, strlen(cmdstr), NULL, 0, 0);

}

/* ft-950 offset */

static int set_offset_ft950(struct rpt *myrpt, char offset)
{
	char *cmdstr;

	switch(offset){
		case	REM_SIMPLEX:
			cmdstr = "OS00;";
			break;

		case	REM_MINUS:
			cmdstr = "OS02;";
			break;

		case	REM_PLUS:
			cmdstr = "OS01;";
			break;

		default:
			return -1;
	}

	return serial_remote_io(myrpt, (unsigned char *)cmdstr, strlen(cmdstr), NULL, 0, 0);
}

/* ft-950 mode */

static int set_mode_ft950(struct rpt *myrpt, char newmode)
{
	char *cmdstr;

	switch(newmode){
		case	REM_MODE_FM:
			cmdstr = "MD04;";
			break;

		case	REM_MODE_USB:
			cmdstr = "MD02;";
			break;

		case	REM_MODE_LSB:
			cmdstr = "MD01;";
			break;

		case	REM_MODE_AM:
			cmdstr = "MD05;";
			break;

		default:
			return -1;
	}

	return serial_remote_io(myrpt, (unsigned char *)cmdstr, strlen(cmdstr), NULL, 0, 0);
}

/* Set tone encode and decode modes */

static int set_ctcss_mode_ft950(struct rpt *myrpt, char txplon, char rxplon)
{
	char *cmdstr;


	if(rxplon && txplon)
		cmdstr = "CT01;";
	else if (!rxplon && txplon)
		cmdstr = "CT02;"; /* Encode only */
	else if (rxplon && !txplon)
		cmdstr = "CT02;"; /* Encode only */
	else
		cmdstr = "CT00;"; /* OFF */

	return serial_remote_io(myrpt, (unsigned char *)cmdstr, strlen(cmdstr), NULL, 0, 0);
}


/* Set transmit and receive ctcss tone frequencies */

static int set_ctcss_freq_ft950(struct rpt *myrpt, char *txtone, char *rxtone)
{
	char cmdstr[10];
	int c;

	c = ft950_pltocode(txtone);
	if (c < 0) return(-1);

	sprintf(cmdstr,"CN0%02d;",c);

	return serial_remote_io(myrpt, (unsigned char *)cmdstr, 5, NULL, 0, 0);
}



static int set_ft950(struct rpt *myrpt)
{
	int res;
	char *cmdstr;

	if(debug)
		ast_log(LOG_NOTICE,"ptt off\n");

	cmdstr = "MX0;";
	res = serial_remote_io(myrpt, (unsigned char *)cmdstr, strlen(cmdstr), NULL, 0, 0); /* MOX off */

	if(debug)
		printf("select ant. 1\n");

	cmdstr = "AN01;";
	res = serial_remote_io(myrpt, (unsigned char *)cmdstr, strlen(cmdstr), NULL, 0, 0); /* MOX off */

	if(debug)
		printf("Modulation mode\n");

	if(!res)
		res = set_mode_ft950(myrpt, myrpt->remmode);		/* Modulation mode */

	if(debug)
		printf("Split off\n");

	cmdstr = "OS00;";
	if(!res)
		res = serial_remote_io(myrpt, (unsigned char *)cmdstr, strlen(cmdstr), NULL, 0, 0); /* Split off */

	if(debug)
		printf("VFO Modes\n");

	if (!res)
		res = serial_remote_io(myrpt, (unsigned char *)"FR0;", 4, NULL, 0, 0);
	if (!res)
		res = serial_remote_io(myrpt, (unsigned char *)"FT2;", 4, NULL, 0, 0);

	if(debug)
		printf("Frequency\n");

	if(!res)
		res = set_freq_ft950(myrpt, myrpt->freq);		/* Frequency */
	if((myrpt->remmode == REM_MODE_FM)){
		if(debug)
			printf("Offset\n");
		if(!res)
			res = set_offset_ft950(myrpt, myrpt->offset);	/* Offset if FM */
		if((!res)&&(myrpt->rxplon || myrpt->txplon)){
			if(debug)
				printf("CTCSS tone freqs.\n");
			res = set_ctcss_freq_ft950(myrpt, myrpt->txpl, myrpt->rxpl); /* CTCSS freqs if CTCSS is enabled */
		}
		if(!res){
			if(debug)
				printf("CTCSS mode\n");
			res = set_ctcss_mode_ft950(myrpt, myrpt->txplon, myrpt->rxplon); /* CTCSS mode */
		}
	}
	if((myrpt->remmode == REM_MODE_USB)||(myrpt->remmode == REM_MODE_LSB)){
		if(debug)
			printf("Clarifier off\n");
		cmdstr = "RT0;";
		serial_remote_io(myrpt, (unsigned char *)cmdstr, strlen(cmdstr), NULL, 0, 0); /* Clarifier off if LSB or USB */
	}
	return res;
}

/*
* Bump frequency up or down by a small amount
* Return 0 if the new frequnecy is valid, or -1 if invalid
* Interval is in Hz, resolution is 10Hz
*/

static int multimode_bump_freq_ft950(struct rpt *myrpt, int interval)
{
	int m,d;
	char mhz[MAXREMSTR], decimals[MAXREMSTR];

	if(debug)
		ast_log(LOG_NOTICE,"Before bump: %s\n", myrpt->freq);

	if(split_freq(mhz, decimals, myrpt->freq))
		return -1;

	m = atoi(mhz);
	d = atoi(decimals);

	d += (interval / 10); /* 10Hz resolution */
	if(d < 0){
		m--;
		d += 100000;
	}
	else if(d >= 100000){
		m++;
		d -= 100000;
	}

	if(check_freq_ft950(m, d, NULL)){
		if(debug)
			ast_log(LOG_NOTICE,"Bump freq invalid\n");
		return -1;
	}

	snprintf(myrpt->freq, MAXREMSTR, "%d.%05d", m, d);

	if(debug)
		printf("After bump: %s\n", myrpt->freq);

	return set_freq_ft950(myrpt, myrpt->freq);
}


/*
* IC-706 I/O handlers
*/

/* Check to see that the frequency is valid */
/* returns 0 if frequency is valid          */

static int check_freq_ic706(int m, int d, int *defmode, char mars)
{
	int dflmd = REM_MODE_FM;
	int rv=0;

	if(debug > 6)
		ast_log(LOG_NOTICE,"(%i,%i,%i,%i)\n",m,d,*defmode,mars);

	/* first test for standard amateur radio bands */

	if(m == 1){ 					/* 160 meters */
		dflmd =	REM_MODE_LSB;
		if(d < 80000)rv=-1;
	}
	else if(m == 3){ 				/* 80 meters */
		dflmd = REM_MODE_LSB;
		if(d < 50000)rv=-1;
	}
	else if(m == 7){ 				/* 40 meters */
		dflmd = REM_MODE_LSB;
		if(d > 30000)rv=-1;
	}
	else if(m == 14){ 				/* 20 meters */
		dflmd = REM_MODE_USB;
		if(d > 35000)rv=-1;
	}
	else if(m == 18){ 							/* 17 meters */
		dflmd = REM_MODE_USB;
		if((d < 6800) || (d > 16800))rv=-1;
	}
	else if(m == 21){ /* 15 meters */
		dflmd = REM_MODE_USB;
		if((d < 20000) || (d > 45000))rv=-1;
	}
	else if(m == 24){ /* 12 meters */
		dflmd = REM_MODE_USB;
		if((d < 89000) || (d > 99000))rv=-1;
	}
	else if(m == 28){ 							/* 10 meters */
		dflmd = REM_MODE_USB;
	}
	else if(m == 29){
		if(d >= 51000)
			dflmd = REM_MODE_FM;
		else
			dflmd = REM_MODE_USB;
		if(d > 70000)rv=-1;
	}
	else if(m == 50){ 							/* 6 meters */
		if(d >= 30000)
			dflmd = REM_MODE_FM;
		else
			dflmd = REM_MODE_USB;
	}
	else if((m >= 51) && ( m < 54)){
		dflmd = REM_MODE_FM;
	}
	else if(m == 144){ /* 2 meters */
		if(d >= 30000)
			dflmd = REM_MODE_FM;
		else
			dflmd = REM_MODE_USB;
	}
	else if((m >= 145) && (m < 148)){
		dflmd = REM_MODE_FM;
	}
	else if((m >= 430) && (m < 450)){ 			/* 70 centimeters */
		if(m  < 438)
			dflmd = REM_MODE_USB;
		else
			dflmd = REM_MODE_FM;
	}

	/* check expanded coverage */
	if(mars && rv<0){
		if((m >= 450) && (m < 470)){ 			/* LMR */
			dflmd = REM_MODE_FM;
			rv=0;
		}
		else if((m >= 148) && (m < 174)){ 		/* LMR */
			dflmd = REM_MODE_FM;
			rv=0;
		}
		else if((m >= 138) && (m < 144)){ 		/* VHF-AM AIRCRAFT */
			dflmd = REM_MODE_AM;
			rv=0;
		}
		else if((m >= 108) && (m < 138)){ 		/* VHF-AM AIRCRAFT */
			dflmd = REM_MODE_AM;
			rv=0;
		}
		else if( (m==0 && d>=55000) || (m==1 && d<=75000) ){ 	/* AM BCB*/
			dflmd = REM_MODE_AM;
			rv=0;
		}
  		else if( (m == 1 && d>75000) || (m>1 && m<30) ){ 		/* HF SWL*/
			dflmd = REM_MODE_AM;
			rv=0;
		}
	}

	if(defmode)
		*defmode = dflmd;

	if(debug > 1)
		ast_log(LOG_NOTICE,"(%i,%i,%i,%i) returning %i\n",m,d,*defmode,mars,rv);

	return rv;
}

/* take a PL frequency and turn it into a code */
static int ic706_pltocode(char *str)
{
	int i;
	char *s;
	int rv=-1;

	s = strchr(str,'.');
	i = 0;
	if (s) i = atoi(s + 1);
	i += atoi(str) * 10;
	switch(i)
	{
	    case 670:
			rv=0;
			break;
	    case 693:
			rv=1;
			break;
	    case 719:
			rv=2;
			break;
	    case 744:
			rv=3;
			break;
	    case 770:
			rv=4;
			break;
	    case 797:
			rv=5;
			break;
	    case 825:
			rv=6;
			break;
	    case 854:
			rv=7;
			break;
	    case 885:
			rv=8;
			break;
	    case 915:
			rv=9;
			break;
	    case 948:
			rv=10;
			break;
	    case 974:
			rv=11;
			break;
	    case 1000:
			rv=12;
			break;
	    case 1035:
			rv=13;
			break;
	    case 1072:
			rv=14;
			break;
	    case 1109:
			rv=15;
			break;
	    case 1148:
			rv=16;
			break;
	    case 1188:
			rv=17;
			break;
	    case 1230:
			rv=18;
			break;
	    case 1273:
			rv=19;
			break;
	    case 1318:
			rv=20;
			break;
	    case 1365:
			rv=21;
			break;
	    case 1413:
			rv=22;
			break;
	    case 1462:
			rv=23;
			break;
	    case 1514:
			rv=24;
			break;
	    case 1567:
			rv=25;
			break;
	    case 1598:
			rv=26;
			break;
	    case 1622:
			rv=27;
			break;
	    case 1655:
			rv=28;
			break;
	    case 1679:
			rv=29;
			break;
	    case 1713:
			rv=30;
			break;
	    case 1738:
			rv=31;
			break;
	    case 1773:
			rv=32;
			break;
	    case 1799:
			rv=33;
			break;
	    case 1835:
			rv=34;
			break;
	    case 1862:
			rv=35;
			break;
	    case 1899:
			rv=36;
			break;
	    case 1928:
			rv=37;
			break;
	    case 1966:
			rv=38;
			break;
	    case 1995:
			rv=39;
			break;
	    case 2035:
			rv=40;
			break;
	    case 2065:
			rv=41;
			break;
	    case 2107:
			rv=42;
			break;
	    case 2181:
			rv=43;
			break;
	    case 2257:
			rv=44;
			break;
	    case 2291:
			rv=45;
			break;
	    case 2336:
			rv=46;
			break;
	    case 2418:
			rv=47;
			break;
	    case 2503:
			rv=48;
			break;
	    case 2541:
			rv=49;
			break;
	}
	if(debug > 1)
		ast_log(LOG_NOTICE,"%i  rv=%i\n",i, rv);

	return rv;
}

/* ic-706 simple commands */

static int simple_command_ic706(struct rpt *myrpt, char command, char subcommand)
{
	unsigned char cmdstr[10];

	cmdstr[0] = cmdstr[1] = 0xfe;
	cmdstr[2] = myrpt->p.civaddr;
	cmdstr[3] = 0xe0;
	cmdstr[4] = command;
	cmdstr[5] = subcommand;
	cmdstr[6] = 0xfd;

	return(civ_cmd(myrpt,cmdstr,7));
}

/*
* Set a new frequency for the ic706
*/

static int set_freq_ic706(struct rpt *myrpt, char *newfreq)
{
	unsigned char cmdstr[20];
	char mhz[MAXREMSTR], decimals[MAXREMSTR];
	int fd,m,d;

	fd = 0;
	if(debug)
		ast_log(LOG_NOTICE,"newfreq:%s\n",newfreq);

	if(split_freq(mhz, decimals, newfreq))
		return -1;

	m = atoi(mhz);
	d = atoi(decimals);

	/* The ic-706 likes packed BCD frequencies */

	cmdstr[0] = cmdstr[1] = 0xfe;
	cmdstr[2] = myrpt->p.civaddr;
	cmdstr[3] = 0xe0;
	cmdstr[4] = 5;
	cmdstr[5] = ((d % 10) << 4);
	cmdstr[6] = (((d % 1000)/ 100) << 4) + ((d % 100)/10);
	cmdstr[7] = ((d / 10000) << 4) + ((d % 10000)/1000);
	cmdstr[8] = (((m % 100)/10) << 4) + (m % 10);
	cmdstr[9] = (m / 100);
	cmdstr[10] = 0xfd;

	return(civ_cmd(myrpt,cmdstr,11));
}

/* ic-706 offset */

static int set_offset_ic706(struct rpt *myrpt, char offset)
{
	unsigned char c;

	if(debug > 6)
		ast_log(LOG_NOTICE,"offset=%i\n",offset);

	switch(offset){
		case	REM_SIMPLEX:
			c = 0x10;
			break;

		case	REM_MINUS:
			c = 0x11;
			break;

		case	REM_PLUS:
			c = 0x12;
			break;

		default:
			return -1;
	}

	return simple_command_ic706(myrpt,0x0f,c);

}

/* ic-706 mode */

static int set_mode_ic706(struct rpt *myrpt, char newmode)
{
	unsigned char c;

	if(debug > 6)
		ast_log(LOG_NOTICE,"newmode=%i\n",newmode);

	switch(newmode){
		case	REM_MODE_FM:
			c = 5;
			break;

		case	REM_MODE_USB:
			c = 1;
			break;

		case	REM_MODE_LSB:
			c = 0;
			break;

		case	REM_MODE_AM:
			c = 2;
			break;

		default:
			return -1;
	}
	return simple_command_ic706(myrpt,6,c);
}

/* Set tone encode and decode modes */

static int set_ctcss_mode_ic706(struct rpt *myrpt, char txplon, char rxplon)
{
	unsigned char cmdstr[10];
	int rv;

	if(debug > 6)
		ast_log(LOG_NOTICE,"txplon=%i  rxplon=%i \n",txplon,rxplon);

	cmdstr[0] = cmdstr[1] = 0xfe;
	cmdstr[2] = myrpt->p.civaddr;
	cmdstr[3] = 0xe0;
	cmdstr[4] = 0x16;
	cmdstr[5] = 0x42;
	cmdstr[6] = (txplon != 0);
	cmdstr[7] = 0xfd;

	rv = civ_cmd(myrpt,cmdstr,8);
	if (rv) return(-1);

	cmdstr[0] = cmdstr[1] = 0xfe;
	cmdstr[2] = myrpt->p.civaddr;
	cmdstr[3] = 0xe0;
	cmdstr[4] = 0x16;
	cmdstr[5] = 0x43;
	cmdstr[6] = (rxplon != 0);
	cmdstr[7] = 0xfd;

	return(civ_cmd(myrpt,cmdstr,8));
}

#if 0
/* Set transmit and receive ctcss tone frequencies */

static int set_ctcss_freq_ic706(struct rpt *myrpt, char *txtone, char *rxtone)
{
	unsigned char cmdstr[10];
	char hertz[MAXREMSTR],decimal[MAXREMSTR];
	int h,d,rv;

	memset(cmdstr, 0, 5);

	if(debug > 6)
		ast_log(LOG_NOTICE,"txtone=%s  rxtone=%s \n",txtone,rxtone);

	if(split_ctcss_freq(hertz, decimal, txtone))
		return -1;

	h = atoi(hertz);
	d = atoi(decimal);

	cmdstr[0] = cmdstr[1] = 0xfe;
	cmdstr[2] = myrpt->p.civaddr;
	cmdstr[3] = 0xe0;
	cmdstr[4] = 0x1b;
	cmdstr[5] = 0;
	cmdstr[6] = ((h / 100) << 4) + (h % 100)/ 10;
	cmdstr[7] = ((h % 10) << 4) + (d % 10);
	cmdstr[8] = 0xfd;

	rv = civ_cmd(myrpt,cmdstr,9);
	if (rv) return(-1);

	if (!rxtone) return(0);

	if(split_ctcss_freq(hertz, decimal, rxtone))
		return -1;

	h = atoi(hertz);
	d = atoi(decimal);

	cmdstr[0] = cmdstr[1] = 0xfe;
	cmdstr[2] = myrpt->p.civaddr;
	cmdstr[3] = 0xe0;
	cmdstr[4] = 0x1b;
	cmdstr[5] = 1;
	cmdstr[6] = ((h / 100) << 4) + (h % 100)/ 10;
	cmdstr[7] = ((h % 10) << 4) + (d % 10);
	cmdstr[8] = 0xfd;
	return(civ_cmd(myrpt,cmdstr,9));
}
#endif

static int vfo_ic706(struct rpt *myrpt)
{
	unsigned char cmdstr[10];

	cmdstr[0] = cmdstr[1] = 0xfe;
	cmdstr[2] = myrpt->p.civaddr;
	cmdstr[3] = 0xe0;
	cmdstr[4] = 7;
	cmdstr[5] = 0xfd;

	return(civ_cmd(myrpt,cmdstr,6));
}

static int mem2vfo_ic706(struct rpt *myrpt)
{
	unsigned char cmdstr[10];

	cmdstr[0] = cmdstr[1] = 0xfe;
	cmdstr[2] = myrpt->p.civaddr;
	cmdstr[3] = 0xe0;
	cmdstr[4] = 0x0a;
	cmdstr[5] = 0xfd;

	return(civ_cmd(myrpt,cmdstr,6));
}

static int select_mem_ic706(struct rpt *myrpt, int slot)
{
	unsigned char cmdstr[10];

	cmdstr[0] = cmdstr[1] = 0xfe;
	cmdstr[2] = myrpt->p.civaddr;
	cmdstr[3] = 0xe0;
	cmdstr[4] = 8;
	cmdstr[5] = 0;
	cmdstr[6] = ((slot / 10) << 4) + (slot % 10);
	cmdstr[7] = 0xfd;

	return(civ_cmd(myrpt,cmdstr,8));
}

static int set_ic706(struct rpt *myrpt)
{
	int res = 0,i;

	if(debug)ast_log(LOG_NOTICE, "Set to VFO A iobase=%i\n",myrpt->p.iobase);

	if (!res)
		res = simple_command_ic706(myrpt,7,0);

	if((myrpt->remmode == REM_MODE_FM))
	{
		i = ic706_pltocode(myrpt->rxpl);
		if (i == -1) return -1;
		if(debug)
			ast_log(LOG_NOTICE,"Select memory number\n");
		if (!res)
			res = select_mem_ic706(myrpt,i + IC706_PL_MEMORY_OFFSET);
		if(debug)
			ast_log(LOG_NOTICE,"Transfer memory to VFO\n");
		if (!res)
			res = mem2vfo_ic706(myrpt);
	}

	if(debug)
		ast_log(LOG_NOTICE,"Set to VFO\n");

	if (!res)
		res = vfo_ic706(myrpt);

	if(debug)
		printf("Modulation mode\n");

	if (!res)
		res = set_mode_ic706(myrpt, myrpt->remmode);		/* Modulation mode */

	if(debug)
		printf("Split off\n");

	if(!res)
		simple_command_ic706(myrpt, 0x82,0);			/* Split off */

	if(debug)
		printf("Frequency\n");

	if(!res)
		res = set_freq_ic706(myrpt, myrpt->freq);		/* Frequency */
	if((myrpt->remmode == REM_MODE_FM)){
		if(debug)
			printf("Offset\n");
		if(!res)
			res = set_offset_ic706(myrpt, myrpt->offset);	/* Offset if FM */
		if(!res){
			if(debug)
				printf("CTCSS mode\n");
			res = set_ctcss_mode_ic706(myrpt, myrpt->txplon, myrpt->rxplon); /* CTCSS mode */
		}
	}
	return res;
}

/*
* Bump frequency up or down by a small amount
* Return 0 if the new frequnecy is valid, or -1 if invalid
* Interval is in Hz, resolution is 10Hz
*/

static int multimode_bump_freq_ic706(struct rpt *myrpt, int interval)
{
	int m,d;
	char mhz[MAXREMSTR], decimals[MAXREMSTR];
	unsigned char cmdstr[20];

	if(debug)
		ast_log(LOG_NOTICE,"Before bump: %s\n", myrpt->freq);

	if(split_freq(mhz, decimals, myrpt->freq))
		return -1;

	m = atoi(mhz);
	d = atoi(decimals);

	d += (interval / 10); /* 10Hz resolution */
	if(d < 0){
		m--;
		d += 100000;
	}
	else if(d >= 100000){
		m++;
		d -= 100000;
	}

	if(check_freq_ic706(m, d, NULL,myrpt->p.remote_mars)){
		if(debug)
			ast_log(LOG_NOTICE,"Bump freq invalid\n");
		return -1;
	}

	snprintf(myrpt->freq, MAXREMSTR, "%d.%05d", m, d);

	if(debug)
		printf("After bump: %s\n", myrpt->freq);

	/* The ic-706 likes packed BCD frequencies */

	cmdstr[0] = cmdstr[1] = 0xfe;
	cmdstr[2] = myrpt->p.civaddr;
	cmdstr[3] = 0xe0;
	cmdstr[4] = 0;
	cmdstr[5] = ((d % 10) << 4);
	cmdstr[6] = (((d % 1000)/ 100) << 4) + ((d % 100)/10);
	cmdstr[7] = ((d / 10000) << 4) + ((d % 10000)/1000);
	cmdstr[8] = (((m % 100)/10) << 4) + (m % 10);
	cmdstr[9] = (m / 100);
	cmdstr[10] = 0xfd;

	return(serial_remote_io(myrpt,cmdstr,11,NULL,0,0));
}



/*
* Dispatch to correct I/O handler
*/
static int setrem(struct rpt *myrpt)
{
char	str[300];
char	*offsets[] = {"SIMPLEX","MINUS","PLUS"};
char	*powerlevels[] = {"LOW","MEDIUM","HIGH"};
char	*modes[] = {"FM","USB","LSB","AM"};
int	res = -1;

if(debug)ast_log(LOG_NOTICE,"[%s] start %s\n",myrpt->name,myrpt->freq);

#if	0
ast_log(LOG_NOTICE,"FREQ,%s,%s,%s,%s,%s,%s,%d,%d\n",myrpt->freq,
	modes[(int)myrpt->remmode],
	myrpt->txpl,myrpt->rxpl,offsets[(int)myrpt->offset],
	powerlevels[(int)myrpt->powerlevel],myrpt->txplon,
	myrpt->rxplon);
#endif
	if (myrpt->p.archivedir)
	{
		sprintf(str,"FREQ,%s,%s,%s,%s,%s,%s,%d,%d",myrpt->freq,
			modes[(int)myrpt->remmode],
			myrpt->txpl,myrpt->rxpl,offsets[(int)myrpt->offset],
			powerlevels[(int)myrpt->powerlevel],myrpt->txplon,
			myrpt->rxplon);
		donodelog(myrpt,str);
	}

	if(!strcmp(myrpt->remoterig, remote_rig_ft897))
	{
		rpt_telemetry(myrpt,SETREMOTE,NULL);
		res = 0;
	}
	if(!strcmp(myrpt->remoterig, remote_rig_ft950))
	{
		rpt_telemetry(myrpt,SETREMOTE,NULL);
		res = 0;
	}
	if(!strcmp(myrpt->remoterig, remote_rig_ic706))
	{
		rpt_telemetry(myrpt,SETREMOTE,NULL);
		res = 0;
	}
	if(!strcmp(myrpt->remoterig, remote_rig_tm271))
	{
		rpt_telemetry(myrpt,SETREMOTE,NULL);
		res = 0;
	}
	else if(!strcmp(myrpt->remoterig, remote_rig_rbi))
	{
		res = setrbi_check(myrpt);
		if (!res)
		{
			rpt_telemetry(myrpt,SETREMOTE,NULL);
			res = 0;
		}
	}
	else if(ISRIG_RTX(myrpt->remoterig))
	{
		setrtx(myrpt);
		res = 0;
	}
	else if(!strcmp(myrpt->remoterig, remote_rig_kenwood)) {
		rpt_telemetry(myrpt,SETREMOTE,NULL);
		res = 0;
	}
	else
		res = 0;

	if (res < 0) ast_log(LOG_ERROR,"Unable to send remote command on node %s\n",myrpt->name);

	return res;
}

static int closerem(struct rpt *myrpt)
{
	if(!strcmp(myrpt->remoterig, remote_rig_ft897))
		return closerem_ft897(myrpt);
	else
		return 0;
}

/*
* Dispatch to correct RX frequency checker
*/
static int check_freq(struct rpt *myrpt, int m, int d, int *defmode)
{
	if(!strcmp(myrpt->remoterig, remote_rig_ft897))
		return check_freq_ft897(m, d, defmode);
	else if(!strcmp(myrpt->remoterig, remote_rig_ft950))
		return check_freq_ft950(m, d, defmode);
	else if(!strcmp(myrpt->remoterig, remote_rig_ic706))
		return check_freq_ic706(m, d, defmode,myrpt->p.remote_mars);
	else if(!strcmp(myrpt->remoterig, remote_rig_rbi))
		return check_freq_rbi(m, d, defmode);
	else if(!strcmp(myrpt->remoterig, remote_rig_kenwood))
		return check_freq_kenwood(m, d, defmode);
	else if(!strcmp(myrpt->remoterig, remote_rig_tm271))
		return check_freq_tm271(m, d, defmode);
	else if(ISRIG_RTX(myrpt->remoterig))
		return check_freq_rtx(m, d, defmode, myrpt);
	else
		return -1;
}

/*
 * Check TX frequency before transmitting
   rv=1 if tx frequency in ok.
*/

static char check_tx_freq(struct rpt *myrpt)
{
	int i,rv=0;
	int radio_mhz, radio_decimals, ulimit_mhz, ulimit_decimals, llimit_mhz, llimit_decimals;
	char radio_mhz_char[MAXREMSTR];
	char radio_decimals_char[MAXREMSTR];
	char limit_mhz_char[MAXREMSTR];
	char limit_decimals_char[MAXREMSTR];
	char limits[256];
	char *limit_ranges[40];
	struct ast_variable *limitlist;

	if(debug > 3){
		ast_log(LOG_NOTICE, "myrpt->freq = %s\n", myrpt->freq);
	}

	/* Must have user logged in and tx_limits defined */

	if(!myrpt->p.txlimitsstanzaname || !myrpt->loginuser[0] || !myrpt->loginlevel[0]){
		if(debug > 3){
			ast_log(LOG_NOTICE, "No tx band table defined, or no user logged in. rv=1\n");
		}
		rv=1;
		return 1; /* Assume it's ok otherwise */
	}

	/* Retrieve the band table for the loginlevel */
	limitlist = ast_variable_browse(myrpt->cfg, myrpt->p.txlimitsstanzaname);

	if(!limitlist){
		ast_log(LOG_WARNING, "No entries in %s band table stanza. rv=0\n", myrpt->p.txlimitsstanzaname);
		rv=0;
		return 0;
	}

	split_freq(radio_mhz_char, radio_decimals_char, myrpt->freq);
	radio_mhz = atoi(radio_mhz_char);
	radio_decimals = decimals2int(radio_decimals_char);

	if(debug > 3){
		ast_log(LOG_NOTICE, "Login User = %s, login level = %s\n", myrpt->loginuser, myrpt->loginlevel);
	}

	/* Find our entry */

	for(;limitlist; limitlist=limitlist->next){
		if(!strcmp(limitlist->name, myrpt->loginlevel))
			break;
	}

	if(!limitlist){
		ast_log(LOG_WARNING, "Can't find %s entry in band table stanza %s. rv=0\n", myrpt->loginlevel, myrpt->p.txlimitsstanzaname);
		rv=0;
	    return 0;
	}

	if(debug > 3){
		ast_log(LOG_NOTICE, "Auth: %s = %s\n", limitlist->name, limitlist->value);
	}

	/* Parse the limits */

	strncpy(limits, limitlist->value, 256);
	limits[255] = 0;
	finddelim(limits, limit_ranges, 40);
	for(i = 0; i < 40 && limit_ranges[i] ; i++){
		char range[40];
		char *r,*s;
		strncpy(range, limit_ranges[i], 40);
		range[39] = 0;
        if(debug > 3)
        	ast_log(LOG_NOTICE, "Check %s within %s\n", myrpt->freq, range);

		r = strchr(range, '-');
		if(!r){
			ast_log(LOG_WARNING, "Malformed range in %s tx band table entry. rv=0\n", limitlist->name);
			rv=0;
			break;
		}
		*r++ = 0;
		s = eatwhite(range);
		r = eatwhite(r);
		split_freq(limit_mhz_char, limit_decimals_char, s);
		llimit_mhz = atoi(limit_mhz_char);
		llimit_decimals = decimals2int(limit_decimals_char);
		split_freq(limit_mhz_char, limit_decimals_char, r);
		ulimit_mhz = atoi(limit_mhz_char);
		ulimit_decimals = decimals2int(limit_decimals_char);

		if((radio_mhz >= llimit_mhz) && (radio_mhz <= ulimit_mhz)){
			if(radio_mhz == llimit_mhz){ /* CASE 1: TX freq is in llimit mhz portion of band */
				if(radio_decimals >= llimit_decimals){ /* Cannot be below llimit decimals */
					if(llimit_mhz == ulimit_mhz){ /* If bandwidth < 1Mhz, check ulimit decimals */
						if(radio_decimals <= ulimit_decimals){
							rv=1;
							break;
						}
						else{
							if(debug > 3)
								ast_log(LOG_NOTICE, "Invalid TX frequency, debug msg 1\n");
							rv=0;
							break;
						}
					}
					else{
						rv=1;
						break;
					}
				}
				else{ /* Is below llimit decimals */
					if(debug > 3)
						ast_log(LOG_NOTICE, "Invalid TX frequency, debug msg 2\n");
					rv=0;
					break;
				}
			}
			else if(radio_mhz == ulimit_mhz){ /* CASE 2: TX freq not in llimit mhz portion of band */
				if(radio_decimals <= ulimit_decimals){
					if(debug > 3)
						ast_log(LOG_NOTICE, "radio_decimals <= ulimit_decimals\n");
					rv=1;
					break;
				}
				else{ /* Is above ulimit decimals */
					if(debug > 3)
						ast_log(LOG_NOTICE, "Invalid TX frequency, debug msg 3\n");
					rv=0;
					break;
				}
			}
			else /* CASE 3: TX freq within a multi-Mhz band and ok */
				if(debug > 3)
						ast_log(LOG_NOTICE, "Valid TX freq within a multi-Mhz band and ok.\n");
				rv=1;
				break;
		}
	}
	if(debug > 3)
		ast_log(LOG_NOTICE, "rv=%i\n",rv);

	return rv;
}


/*
* Dispatch to correct frequency bumping function
*/

static int multimode_bump_freq(struct rpt *myrpt, int interval)
{
	if(!strcmp(myrpt->remoterig, remote_rig_ft897))
		return multimode_bump_freq_ft897(myrpt, interval);
	else if(!strcmp(myrpt->remoterig, remote_rig_ft950))
		return multimode_bump_freq_ft950(myrpt, interval);
	else if(!strcmp(myrpt->remoterig, remote_rig_ic706))
		return multimode_bump_freq_ic706(myrpt, interval);
	else
		return -1;
}


/*
* Queue announcment that scan has been stopped
*/

static void stop_scan(struct rpt *myrpt)
{
	myrpt->hfscanstop = 1;
	rpt_telemetry(myrpt,SCAN,0);
}

/*
* This is called periodically when in scan mode
*/
static int service_scan(struct rpt *myrpt)
{
	int res, interval;
	char mhz[MAXREMSTR], decimals[MAXREMSTR], k10=0i, k100=0;

	switch(myrpt->hfscanmode){

		case HF_SCAN_DOWN_SLOW:
			interval = -10; /* 100Hz /sec */
			break;

		case HF_SCAN_DOWN_QUICK:
			interval = -50; /* 500Hz /sec */
			break;

		case HF_SCAN_DOWN_FAST:
			interval = -200; /* 2KHz /sec */
			break;

		case HF_SCAN_UP_SLOW:
			interval = 10; /* 100Hz /sec */
			break;

		case HF_SCAN_UP_QUICK:
			interval = 50; /* 500 Hz/sec */
			break;

		case HF_SCAN_UP_FAST:
			interval = 200; /* 2KHz /sec */
			break;

		default:
			myrpt->hfscanmode = 0; /* Huh? */
			return -1;
	}

	res = split_freq(mhz, decimals, myrpt->freq);

	if(!res){
		k100 =decimals[0];
		k10 = decimals[1];
		res = multimode_bump_freq(myrpt, interval);
	}

	if(!res)
		res = split_freq(mhz, decimals, myrpt->freq);


	if(res){
		myrpt->hfscanmode = 0;
		myrpt->hfscanstatus = -2;
		return -1;
	}

	/* Announce 10KHz boundaries */
	if(k10 != decimals[1]){
		int myhund = (interval < 0) ? k100 : decimals[0];
		int myten = (interval < 0) ? k10 : decimals[1];
		myrpt->hfscanstatus = (myten == '0') ? (myhund - '0') * 100 : (myten - '0') * 10;
	} else myrpt->hfscanstatus = 0;
	return res;
}
/*
	retrieve memory setting and set radio
*/
static int get_mem_set(struct rpt *myrpt, char *digitbuf)
{
	int res=0;
	if(debug)ast_log(LOG_NOTICE,"[%s] start digitbuf=%s\n",myrpt->name,digitbuf);
	res = retreive_memory(myrpt, digitbuf);
	if(!res)res=setrem(myrpt);
	if(debug)ast_log(LOG_NOTICE,"end freq=%s  res=%i\n", myrpt->freq, res);
	return res;
}
/*
	steer the radio selected channel to either one programmed into the radio
	or if the radio is VFO agile, to an rpt.conf memory location.
*/
static int channel_steer(struct rpt *myrpt, char *data)
{
	int res=0;

	if(debug)ast_log(LOG_NOTICE,"[%s] start remoterig=%s, data=%s\n",myrpt->name,myrpt->remoterig,data);
	if (!myrpt->remoterig) return(0);
	if(data<=0)
	{
		res=-1;
	}
	else
	{
		myrpt->nowchan=strtod(data,NULL);
		if(!strcmp(myrpt->remoterig, remote_rig_ppp16))
		{
			char string[16];
			sprintf(string,"SETCHAN %d ",myrpt->nowchan);
			send_usb_txt(myrpt,string);
		}
		else
		{
			if(get_mem_set(myrpt, data))res=-1;
		}
	}
	if(debug)ast_log(LOG_NOTICE,"end nowchan=%i  res=%i\n",myrpt->nowchan, res);
	return res;
}
/*
*/
static int channel_revert(struct rpt *myrpt)
{
	int res=0;
	if(debug)ast_log(LOG_NOTICE,"remoterig=%s, nowchan=%02d, waschan=%02d\n",myrpt->remoterig,myrpt->nowchan,myrpt->waschan);
	if (!myrpt->remoterig) return(0);
	if(myrpt->nowchan!=myrpt->waschan)
	{
		char data[8];
        if(debug)ast_log(LOG_NOTICE,"reverting.\n");
		sprintf(data,"%02d",myrpt->waschan);
		myrpt->nowchan=myrpt->waschan;
		channel_steer(myrpt,data);
		res=1;
	}
	return(res);
}
/*
* Remote base function
*/

static int function_remote(struct rpt *myrpt, char *param, char *digitbuf, int command_source, struct rpt_link *mylink)
{
	char *s,*s1,*s2;
	int i,j,p,r,ht,k,l,ls2,m,d,offset,offsave, modesave, defmode;
	char multimode = 0;
	char oc,*cp,*cp1,*cp2;
	char tmp[20], freq[20] = "", savestr[20] = "";
	char mhz[MAXREMSTR], decimals[MAXREMSTR];

    if(debug > 6) {
    	ast_log(LOG_NOTICE,"%s param=%s digitbuf=%s source=%i\n",myrpt->name,param,digitbuf,command_source);
	}

	if((!param) || (command_source == SOURCE_RPT) || (command_source == SOURCE_LNK))
		return DC_ERROR;

	p = myatoi(param);

	if ((p != 99) && (p != 5) && (p != 140) && myrpt->p.authlevel &&
		(!myrpt->loginlevel[0])) return DC_ERROR;
	multimode = multimode_capable(myrpt);

	switch(p){

		case 1:  /* retrieve memory */
			if(strlen(digitbuf) < 2) /* needs 2 digits */
				break;

			for(i = 0 ; i < 2 ; i++){
				if((digitbuf[i] < '0') || (digitbuf[i] > '9'))
					return DC_ERROR;
			}
		    	r=get_mem_set(myrpt, digitbuf);
			if (r < 0){
				rpt_telemetry(myrpt,MEMNOTFOUND,NULL);
				return DC_COMPLETE;
			}
			else if (r > 0){
				return DC_ERROR;
			}
			return DC_COMPLETE;

		case 2:  /* set freq and offset */
	    		for(i = 0, j = 0, k = 0, l = 0 ; digitbuf[i] ; i++){ /* look for M+*K+*O or M+*H+* depending on mode */
				if(digitbuf[i] == '*'){
					j++;
					continue;
				}
				if((digitbuf[i] < '0') || (digitbuf[i] > '9'))
					goto invalid_freq;
				else{
					if(j == 0)
						l++; /* # of digits before first * */
					if(j == 1)
						k++; /* # of digits after first * */
				}
			}

			i = strlen(digitbuf) - 1;
			if(multimode){
				if((j > 2) || (l > 3) || (k > 6))
					goto invalid_freq; /* &^@#! */
 			}
			else{
				if((j > 2) || (l > 4) || (k > 3))
					goto invalid_freq; /* &^@#! */
			}

			/* Wait for M+*K+* */

			if(j < 2)
				break; /* Not yet */

			/* We have a frequency */

			strncpy(tmp, digitbuf ,sizeof(tmp) - 1);

			s = tmp;
			s1 = strsep(&s, "*"); /* Pick off MHz */
			s2 = strsep(&s,"*"); /* Pick off KHz and Hz */
			ls2 = strlen(s2);

			switch(ls2){ /* Allow partial entry of khz and hz digits for laziness support */
				case 1:
					ht = 0;
					k = 100 * atoi(s2);
					break;

				case 2:
					ht = 0;
					k = 10 * atoi(s2);
					break;

				case 3:
					if(!multimode){
						if((s2[2] != '0')&&(s2[2] != '5'))
							goto invalid_freq;
					}
					ht = 0;
					k = atoi(s2);
						break;
				case 4:
					k = atoi(s2)/10;
					ht = 10 * (atoi(s2+(ls2-1)));
					break;

				case 5:
					k = atoi(s2)/100;
					ht = (atoi(s2+(ls2-2)));
					break;

				default:
					goto invalid_freq;
			}

			/* Check frequency for validity and establish a default mode */

			snprintf(freq, sizeof(freq), "%s.%03d%02d",s1, k, ht);

 			if(debug)
				ast_log(LOG_NOTICE, "New frequency: %s\n", freq);

			split_freq(mhz, decimals, freq);
			m = atoi(mhz);
			d = atoi(decimals);

			if(check_freq(myrpt, m, d, &defmode)) /* Check to see if frequency entered is legit */
			        goto invalid_freq;

 			if((defmode == REM_MODE_FM) && (digitbuf[i] == '*')) /* If FM, user must enter and additional offset digit */
				break; /* Not yet */


			offset = REM_SIMPLEX; /* Assume simplex */

			if(defmode == REM_MODE_FM){
				oc = *s; /* Pick off offset */

				if (oc){
					switch(oc){
						case '1':
							offset = REM_MINUS;
							break;

						case '2':
							offset = REM_SIMPLEX;
						break;

						case '3':
							offset = REM_PLUS;
							break;

						default:
							goto invalid_freq;
					}
				}
			}
			offsave = myrpt->offset;
			modesave = myrpt->remmode;
			strncpy(savestr, myrpt->freq, sizeof(savestr) - 1);
			strncpy(myrpt->freq, freq, sizeof(myrpt->freq) - 1);
			myrpt->offset = offset;
			myrpt->remmode = defmode;

			if (setrem(myrpt) == -1){
				myrpt->offset = offsave;
				myrpt->remmode = modesave;
				strncpy(myrpt->freq, savestr, sizeof(myrpt->freq) - 1);
				goto invalid_freq;
			}

			return DC_COMPLETE;

invalid_freq:
			rpt_telemetry(myrpt,INVFREQ,NULL);
			return DC_ERROR;

		case 3: /* set rx PL tone */
	    		for(i = 0, j = 0, k = 0, l = 0 ; digitbuf[i] ; i++){ /* look for N+*N */
				if(digitbuf[i] == '*'){
					j++;
					continue;
				}
				if((digitbuf[i] < '0') || (digitbuf[i] > '9'))
					return DC_ERROR;
				else{
					if(j)
						l++;
					else
						k++;
				}
			}
			if((j > 1) || (k > 3) || (l > 1))
				return DC_ERROR; /* &$@^! */
			i = strlen(digitbuf) - 1;
			if((j != 1) || (k < 2)|| (l != 1))
				break; /* Not yet */
			if(debug)
				ast_log(LOG_NOTICE,"PL digits entered %s\n", digitbuf);

			strncpy(tmp, digitbuf, sizeof(tmp) - 1);
			/* see if we have at least 1 */
			s = strchr(tmp,'*');
			if(s)
				*s = '.';
			strncpy(savestr, myrpt->rxpl, sizeof(savestr) - 1);
			strncpy(myrpt->rxpl, tmp, sizeof(myrpt->rxpl) - 1);
			if(!strcmp(myrpt->remoterig, remote_rig_rbi))
			{
				strncpy(myrpt->txpl, tmp, sizeof(myrpt->txpl) - 1);
			}
			if (setrem(myrpt) == -1){
				strncpy(myrpt->rxpl, savestr, sizeof(myrpt->rxpl) - 1);
				return DC_ERROR;
			}
			return DC_COMPLETE;

		case 4: /* set tx PL tone */
			/* cant set tx tone on RBI (rx tone does both) */
			if(!strcmp(myrpt->remoterig, remote_rig_rbi))
				return DC_ERROR;
			/*  eventually for the ic706 instead of just throwing the exception
				we can check if we are in encode only mode and allow the tx
				ctcss code to be changed. but at least the warning message is
				issued for now.
			*/
			if(!strcmp(myrpt->remoterig, remote_rig_ic706))
			{
				if(debug)
					ast_log(LOG_WARNING,"Setting IC706 Tx CTCSS Code Not Supported. Set Rx Code for both.\n");
				return DC_ERROR;
			}
	    	for(i = 0, j = 0, k = 0, l = 0 ; digitbuf[i] ; i++){ /* look for N+*N */
				if(digitbuf[i] == '*'){
					j++;
					continue;
				}
				if((digitbuf[i] < '0') || (digitbuf[i] > '9'))
					return DC_ERROR;
				else{
					if(j)
						l++;
					else
						k++;
				}
			}
			if((j > 1) || (k > 3) || (l > 1))
				return DC_ERROR; /* &$@^! */
			i = strlen(digitbuf) - 1;
			if((j != 1) || (k < 2)|| (l != 1))
				break; /* Not yet */
			if(debug)
				ast_log(LOG_NOTICE,"PL digits entered %s\n", digitbuf);

			strncpy(tmp, digitbuf, sizeof(tmp) - 1);
			/* see if we have at least 1 */
			s = strchr(tmp,'*');
			if(s)
				*s = '.';
			strncpy(savestr, myrpt->txpl, sizeof(savestr) - 1);
			strncpy(myrpt->txpl, tmp, sizeof(myrpt->txpl) - 1);

			if (setrem(myrpt) == -1){
				strncpy(myrpt->txpl, savestr, sizeof(myrpt->txpl) - 1);
				return DC_ERROR;
			}
			return DC_COMPLETE;


		case 6: /* MODE (FM,USB,LSB,AM) */
			if(strlen(digitbuf) < 1)
				break;

			if(!multimode)
				return DC_ERROR; /* Multimode radios only */

			switch(*digitbuf){
				case '1':
					split_freq(mhz, decimals, myrpt->freq);
					m=atoi(mhz);
					if(m < 29) /* No FM allowed below 29MHz! */
						return DC_ERROR;
					myrpt->remmode = REM_MODE_FM;

					rpt_telemetry(myrpt,REMMODE,NULL);
					break;

				case '2':
					myrpt->remmode = REM_MODE_USB;
					rpt_telemetry(myrpt,REMMODE,NULL);
					break;

				case '3':
					myrpt->remmode = REM_MODE_LSB;
					rpt_telemetry(myrpt,REMMODE,NULL);
					break;

				case '4':
					myrpt->remmode = REM_MODE_AM;
					rpt_telemetry(myrpt,REMMODE,NULL);
					break;

				default:
					return DC_ERROR;
			}

			if(setrem(myrpt))
				return DC_ERROR;
			return DC_COMPLETEQUIET;
		case 99:
			/* cant log in when logged in */
			if (myrpt->loginlevel[0])
				return DC_ERROR;
			*myrpt->loginuser = 0;
			myrpt->loginlevel[0] = 0;
			cp = ast_strdup(param);
			cp1 = strchr(cp,',');
			ast_mutex_lock(&myrpt->lock);
			if (cp1)
			{
				*cp1 = 0;
				cp2 = strchr(cp1 + 1,',');
				if (cp2)
				{
					*cp2 = 0;
					strncpy(myrpt->loginlevel,cp2 + 1,
						sizeof(myrpt->loginlevel) - 1);
				}
				strncpy(myrpt->loginuser,cp1 + 1,sizeof(myrpt->loginuser));
				ast_mutex_unlock(&myrpt->lock);
				if (myrpt->p.archivedir)
				{
					char str[100];

					sprintf(str,"LOGIN,%s,%s",
					    myrpt->loginuser,myrpt->loginlevel);
					donodelog(myrpt,str);
				}
				if (debug)
					ast_log(LOG_NOTICE,"loginuser %s level %s\n",myrpt->loginuser,myrpt->loginlevel);
				rpt_telemetry(myrpt,REMLOGIN,NULL);
			}
			ast_free(cp);
			return DC_COMPLETEQUIET;
		case 100: /* RX PL Off */
			myrpt->rxplon = 0;
			setrem(myrpt);
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 101: /* RX PL On */
			myrpt->rxplon = 1;
			setrem(myrpt);
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 102: /* TX PL Off */
			myrpt->txplon = 0;
			setrem(myrpt);
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 103: /* TX PL On */
			myrpt->txplon = 1;
			setrem(myrpt);
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 104: /* Low Power */
			if(!strcmp(myrpt->remoterig, remote_rig_ic706))
				return DC_ERROR;
			myrpt->powerlevel = REM_LOWPWR;
			setrem(myrpt);
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 105: /* Medium Power */
			if(!strcmp(myrpt->remoterig, remote_rig_ic706))
				return DC_ERROR;
			if (ISRIG_RTX(myrpt->remoterig)) return DC_ERROR;
			myrpt->powerlevel = REM_MEDPWR;
			setrem(myrpt);
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 106: /* Hi Power */
			if(!strcmp(myrpt->remoterig, remote_rig_ic706))
				return DC_ERROR;
			myrpt->powerlevel = REM_HIPWR;
			setrem(myrpt);
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 107: /* Bump down 20Hz */
			multimode_bump_freq(myrpt, -20);
			return DC_COMPLETE;
		case 108: /* Bump down 100Hz */
			multimode_bump_freq(myrpt, -100);
			return DC_COMPLETE;
		case 109: /* Bump down 500Hz */
			multimode_bump_freq(myrpt, -500);
			return DC_COMPLETE;
		case 110: /* Bump up 20Hz */
			multimode_bump_freq(myrpt, 20);
			return DC_COMPLETE;
		case 111: /* Bump up 100Hz */
			multimode_bump_freq(myrpt, 100);
			return DC_COMPLETE;
		case 112: /* Bump up 500Hz */
			multimode_bump_freq(myrpt, 500);
			return DC_COMPLETE;
		case 113: /* Scan down slow */
			myrpt->scantimer = REM_SCANTIME;
			myrpt->hfscanmode = HF_SCAN_DOWN_SLOW;
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 114: /* Scan down quick */
			myrpt->scantimer = REM_SCANTIME;
			myrpt->hfscanmode = HF_SCAN_DOWN_QUICK;
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 115: /* Scan down fast */
			myrpt->scantimer = REM_SCANTIME;
			myrpt->hfscanmode = HF_SCAN_DOWN_FAST;
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 116: /* Scan up slow */
			myrpt->scantimer = REM_SCANTIME;
			myrpt->hfscanmode = HF_SCAN_UP_SLOW;
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 117: /* Scan up quick */
			myrpt->scantimer = REM_SCANTIME;
			myrpt->hfscanmode = HF_SCAN_UP_QUICK;
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 118: /* Scan up fast */
			myrpt->scantimer = REM_SCANTIME;
			myrpt->hfscanmode = HF_SCAN_UP_FAST;
			rpt_telemetry(myrpt,REMXXX,(void *)p);
			return DC_COMPLETEQUIET;
		case 119: /* Tune Request */
			if(debug > 3)
				ast_log(LOG_NOTICE,"TUNE REQUEST\n");
			/* if not currently going, and valid to do */
			if((!myrpt->tunerequest) &&
			    ((!strcmp(myrpt->remoterig, remote_rig_ft897) ||
				(!strcmp(myrpt->remoterig, remote_rig_ft950)) ||
				!strcmp(myrpt->remoterig, remote_rig_ic706)) )) {
				myrpt->remotetx = 0;
				if (strncasecmp(myrpt->txchannel->name,
					"dahdi/pseudo",10))
				{
					ast_indicate(myrpt->txchannel,
						AST_CONTROL_RADIO_UNKEY);
				}
				myrpt->tunerequest = 1;
				rpt_telemetry(myrpt,TUNE,NULL);
				return DC_COMPLETEQUIET;
			}
			return DC_ERROR;
		case 5: /* Long Status */
			rpt_telemetry(myrpt,REMLONGSTATUS,NULL);
			return DC_COMPLETEQUIET;
		case 140: /* Short Status */
			rpt_telemetry(myrpt,REMSHORTSTATUS,NULL);
			return DC_COMPLETEQUIET;
		case 200:
		case 201:
		case 202:
		case 203:
		case 204:
		case 205:
		case 206:
		case 207:
		case 208:
		case 209:
		case 210:
		case 211:
		case 212:
		case 213:
		case 214:
		case 215:
			do_dtmf_local(myrpt,remdtmfstr[p - 200]);
			return DC_COMPLETEQUIET;
		default:
			break;
	}
	return DC_INDETERMINATE;
}


static int handle_remote_dtmf_digit(struct rpt *myrpt,char c, char *keyed, int phonemode)
{
time_t	now;
int	ret,res = 0,src;

	if(debug)
		ast_log(LOG_NOTICE,"start c=%c  phonemode=%i  dtmfidx=%i\n",
			c,phonemode,myrpt->dtmfidx);

	time(&myrpt->last_activity_time);
	/* Stop scan mode if in scan mode */
	if(myrpt->hfscanmode){
		stop_scan(myrpt);
		return 0;
	}

	time(&now);
	/* if timed-out */
	if ((myrpt->dtmf_time_rem + DTMF_TIMEOUT) < now)
	{
		myrpt->dtmfidx = -1;
		myrpt->dtmfbuf[0] = 0;
		myrpt->dtmf_time_rem = 0;
	}
	/* if decode not active */
	if (myrpt->dtmfidx == -1)
	{
		/* if not lead-in digit, dont worry */
		if (c != myrpt->p.funcchar)
		{
			if (!myrpt->p.propagate_dtmf)
			{
				rpt_mutex_lock(&myrpt->lock);
				do_dtmf_local(myrpt,c);
				rpt_mutex_unlock(&myrpt->lock);
			}
			return 0;
		}
		myrpt->dtmfidx = 0;
		myrpt->dtmfbuf[0] = 0;
		myrpt->dtmf_time_rem = now;
		return 0;
	}
	/* if too many in buffer, start over */
	if (myrpt->dtmfidx >= MAXDTMF)
	{
		myrpt->dtmfidx = 0;
		myrpt->dtmfbuf[0] = 0;
		myrpt->dtmf_time_rem = now;
	}
	if (c == myrpt->p.funcchar)
	{
		/* if star at beginning, or 2 together, erase buffer */
		if ((myrpt->dtmfidx < 1) ||
			(myrpt->dtmfbuf[myrpt->dtmfidx - 1] == myrpt->p.funcchar))
		{
			myrpt->dtmfidx = 0;
			myrpt->dtmfbuf[0] = 0;
			myrpt->dtmf_time_rem = now;
			return 0;
		}
	}
	myrpt->dtmfbuf[myrpt->dtmfidx++] = c;
	myrpt->dtmfbuf[myrpt->dtmfidx] = 0;
	myrpt->dtmf_time_rem = now;


	src = SOURCE_RMT;
	if (phonemode == 2) src = SOURCE_DPHONE;
	else if (phonemode) src = SOURCE_PHONE;
	else if (phonemode == 4) src = SOURCE_ALT;
	ret = collect_function_digits(myrpt, myrpt->dtmfbuf, src, NULL);

	switch(ret){

		case DC_INDETERMINATE:
			res = 0;
			break;

		case DC_DOKEY:
			if (keyed) *keyed = 1;
			res = 0;
			break;

		case DC_REQ_FLUSH:
			myrpt->dtmfidx = 0;
			myrpt->dtmfbuf[0] = 0;
			res = 0;
			break;


		case DC_COMPLETE:
			res = 1;
		case DC_COMPLETEQUIET:
			myrpt->totalexecdcommands++;
			myrpt->dailyexecdcommands++;
			strncpy(myrpt->lastdtmfcommand, myrpt->dtmfbuf, MAXDTMF-1);
			myrpt->lastdtmfcommand[MAXDTMF-1] = '\0';
			myrpt->dtmfbuf[0] = 0;
			myrpt->dtmfidx = -1;
			myrpt->dtmf_time_rem = 0;
			break;

		case DC_ERROR:
		default:
			myrpt->dtmfbuf[0] = 0;
			myrpt->dtmfidx = -1;
			myrpt->dtmf_time_rem = 0;
			res = 0;
			break;
	}

	return res;
}

static int handle_remote_data(struct rpt *myrpt, char *str)
{
char	tmp[300],cmd[300],dest[300],src[300],c;
int	seq,res;

 	/* put string in our buffer */
	strncpy(tmp,str,sizeof(tmp) - 1);
	if (!strcmp(tmp,discstr)) return 0;
    if (!strcmp(tmp,newkeystr))
    {
		if (!myrpt->newkey)
		{
		    if (debug) ast_log(LOG_NOTICE,"node=%s set myrpt->newkey = 1\n",myrpt->name);
			send_old_newkey(myrpt->rxchannel);
			myrpt->newkey = 1;
		}
	    return 0;
    }
    if (!strcmp(tmp,newkey1str))
    {
		if (debug) ast_log(LOG_NOTICE,"node=%s set myrpt->newkey = 2\n",myrpt->name);
		myrpt->newkey = 2;
		return 0;
    }
	if (tmp[0] == 'T') return 0;

#ifndef	DO_NOT_NOTIFY_MDC1200_ON_REMOTE_BASES
	if (tmp[0] == 'I')
	{
		if (sscanf(tmp,"%s %s %x",cmd,src,&seq) != 3)
		{
			ast_log(LOG_WARNING, "Unable to parse ident string %s\n",str);
			return 0;
		}
		mdc1200_notify(myrpt,src,seq);
		return 0;
	}
#endif
	if (tmp[0] == 'L') return 0;
	if (sscanf(tmp,"%s %s %s %d %c",cmd,dest,src,&seq,&c) != 5)
	{
		ast_log(LOG_WARNING, "Unable to parse link string %s\n",str);
		return 0;
	}
	if (strcmp(cmd,"D"))
	{
		ast_log(LOG_WARNING, "Unable to parse link string %s\n",str);
		return 0;
	}
	/* if not for me, ignore */
	if (strcmp(dest,myrpt->name)) return 0;
	if (myrpt->p.archivedir)
	{
		char str[100];

		sprintf(str,"DTMF,%c",c);
		donodelog(myrpt,str);
	}
	c = func_xlat(myrpt,c,&myrpt->p.outxlat);
	if (!c) return(0);
	res = handle_remote_dtmf_digit(myrpt,c, NULL, 0);
	if (res != 1)
		return res;
	rpt_telemetry(myrpt,COMPLETE,NULL);
	return 0;
}

static int handle_remote_phone_dtmf(struct rpt *myrpt, char c, char *keyed, int phonemode)
{
int	res;


	if(phonemode == 3) /* simplex phonemode, funcchar key/unkey toggle */
	{
		if (keyed && *keyed && ((c == myrpt->p.funcchar) || (c == myrpt->p.endchar)))
		{
			*keyed = 0; /* UNKEY */
			return 0;
		}
		else if (keyed && !*keyed && (c = myrpt->p.funcchar))
		{
			*keyed = 1; /* KEY */
			return 0;
		}
	}
	else /* endchar unkey */
	{

		if (keyed && *keyed && (c == myrpt->p.endchar))
		{
			*keyed = 0;
			return DC_INDETERMINATE;
		}
	}
	if (myrpt->p.archivedir)
	{
		char str[100];

		sprintf(str,"DTMF(P),%c",c);
		donodelog(myrpt,str);
	}
	res = handle_remote_dtmf_digit(myrpt,c,keyed, phonemode);
	if (res != 1)
		return res;
	rpt_telemetry(myrpt,COMPLETE,NULL);
	return 0;
}


static int attempt_reconnect(struct rpt *myrpt, struct rpt_link *l)
{
	char *val, *s, *s1, *s2, *tele;
	char tmp[300], deststr[300] = "";
	char sx[320],*sy;
	struct ast_frame *f1;


	val = node_lookup(myrpt,l->name);
	if (!val)
	{
		fprintf(stderr,"attempt_reconnect: cannot find node %s\n",l->name);
		return -1;
	}

	/* cannot apply to echolink */
	if (!strncasecmp(val,"echolink",8)) return 0;
	/* nor to irlp */
	if (!strncasecmp(val,"irlp",4)) return 0;
	rpt_mutex_lock(&myrpt->lock);
	/* remove from queue */
	remque((struct qelem *) l);
	rpt_mutex_unlock(&myrpt->lock);
	strncpy(tmp,val,sizeof(tmp) - 1);
	s = tmp;
	s1 = strsep(&s,",");
	if (!strchr(s1,':') && strchr(s1,'/') && strncasecmp(s1, "local/", 6))
	{
		sy = strchr(s1,'/');
		*sy = 0;
		sprintf(sx,"%s:4569/%s",s1,sy + 1);
		s1 = sx;
	}
	s2 = strsep(&s,",");
	snprintf(deststr, sizeof(deststr), "IAX2/%s", s1);
	tele = strchr(deststr, '/');
	if (!tele) {
		fprintf(stderr,"attempt_reconnect:Dial number (%s) must be in format tech/number\n",deststr);
		return -1;
	}
	*tele++ = 0;
	l->elaptime = 0;
	l->connecttime = 0;
	l->thisconnected = 0;
	l->newkey = 0;
	l->chan = ast_request(deststr, AST_FORMAT_SLINEAR, tele,NULL);
	l->linkmode = 0;
	l->lastrx1 = 0;
	l->lastrealrx = 0;
	l->rxlingertimer = RX_LINGER_TIME;
	l->newkeytimer = NEWKEYTIME;
	l->newkey = 2;
	while((f1 = AST_LIST_REMOVE_HEAD(&l->textq,frame_list))) ast_frfree(f1);
	if (l->chan){
		ast_set_read_format(l->chan, AST_FORMAT_SLINEAR);
		ast_set_write_format(l->chan, AST_FORMAT_SLINEAR);
#ifndef	NEW_ASTERISK
		l->chan->whentohangup = 0;
#endif
		l->chan->appl = "Apprpt";
		l->chan->data = "(Remote Rx)";
		if (option_verbose > 2)
			ast_verbose(VERBOSE_PREFIX_3 "rpt (attempt_reconnect) initiating call to %s/%s on %s\n",
				deststr, tele, l->chan->name);
		if(l->chan->cid.cid_num)
			ast_free(l->chan->cid.cid_num);
		l->chan->cid.cid_num = ast_strdup(myrpt->name);
                ast_call(l->chan,tele,999);

	}
	else
	{
		if (option_verbose > 2)
			ast_verbose(VERBOSE_PREFIX_3 "Unable to place call to %s/%s\n",
				deststr,tele);
		return -1;
	}

	rpt_mutex_lock(&myrpt->lock);
	/* put back in queue */
	insque((struct qelem *)l,(struct qelem *)myrpt->links.next);
	rpt_mutex_unlock(&myrpt->lock);
	ast_log(LOG_WARNING,"Reconnect Attempt to %s in process\n",l->name);
	return 0;
}

/* 0 return=continue, 1 return = break, -1 return = error */
static void local_dtmf_helper(struct rpt *myrpt,char c_in)
{
int	res;
pthread_attr_t	attr;
char	cmd[MAXDTMF+1] = "",c;


	c = c_in & 0x7f;
	if (myrpt->p.archivedir)
	{
		char str[100];

		sprintf(str,"DTMF,MAIN,%c",c);
		donodelog(myrpt,str);
	}
	if (c == myrpt->p.endchar)
	{
	/* if in simple mode, kill autopatch */
		if (myrpt->p.simple && myrpt->callmode)
		{
			if(debug)
				ast_log(LOG_WARNING, "simple mode autopatch kill\n");
			rpt_mutex_lock(&myrpt->lock);
			myrpt->callmode = 0;
			myrpt->macropatch=0;
			channel_revert(myrpt);
			rpt_mutex_unlock(&myrpt->lock);
			rpt_telemetry(myrpt,TERM,NULL);
			return;
		}
		rpt_mutex_lock(&myrpt->lock);
		myrpt->stopgen = 1;
		if (myrpt->cmdnode[0])
		{
			myrpt->cmdnode[0] = 0;
			myrpt->dtmfidx = -1;
			myrpt->dtmfbuf[0] = 0;
			rpt_mutex_unlock(&myrpt->lock);
			rpt_telemetry(myrpt,COMPLETE,NULL);
			return;
		}
		else if(!myrpt->inpadtest)
                {
                        rpt_mutex_unlock(&myrpt->lock);
                        if (myrpt->p.propagate_phonedtmf)
                               do_dtmf_phone(myrpt,NULL,c);
			if ((myrpt->dtmfidx == -1) &&
			   ((myrpt->callmode == 2) || (myrpt->callmode == 3)))
			{
				myrpt->mydtmf = c;
			}
			return;
                }
		else
		{
			rpt_mutex_unlock(&myrpt->lock);
		}
	}
	rpt_mutex_lock(&myrpt->lock);
	if (myrpt->cmdnode[0])
	{
		rpt_mutex_unlock(&myrpt->lock);
		send_link_dtmf(myrpt,c);
		return;
	}
	if (!myrpt->p.simple)
	{
		if ((!myrpt->inpadtest)&&(c == myrpt->p.funcchar))
		{
			myrpt->dtmfidx = 0;
			myrpt->dtmfbuf[myrpt->dtmfidx] = 0;
			rpt_mutex_unlock(&myrpt->lock);
			time(&myrpt->dtmf_time);
			return;
		}
		else if (((myrpt->inpadtest)||(c != myrpt->p.endchar)) && (myrpt->dtmfidx >= 0))
		{
			time(&myrpt->dtmf_time);

			if (myrpt->dtmfidx < MAXDTMF)
			{
				int src;

				myrpt->dtmfbuf[myrpt->dtmfidx++] = c;
				myrpt->dtmfbuf[myrpt->dtmfidx] = 0;

				strncpy(cmd, myrpt->dtmfbuf, sizeof(cmd) - 1);

				rpt_mutex_unlock(&myrpt->lock);
				src = SOURCE_RPT;
				if (c_in & 0x80) src = SOURCE_ALT;
				res = collect_function_digits(myrpt, cmd, src, NULL);
				rpt_mutex_lock(&myrpt->lock);
				switch(res){
				    case DC_INDETERMINATE:
					break;
				    case DC_REQ_FLUSH:
					myrpt->dtmfidx = 0;
					myrpt->dtmfbuf[0] = 0;
					break;
				    case DC_COMPLETE:
				    case DC_COMPLETEQUIET:
					myrpt->totalexecdcommands++;
					myrpt->dailyexecdcommands++;
					strncpy(myrpt->lastdtmfcommand, cmd, MAXDTMF-1);
					myrpt->lastdtmfcommand[MAXDTMF-1] = '\0';
					myrpt->dtmfbuf[0] = 0;
					myrpt->dtmfidx = -1;
					myrpt->dtmf_time = 0;
					break;

				    case DC_ERROR:
				    default:
					myrpt->dtmfbuf[0] = 0;
					myrpt->dtmfidx = -1;
					myrpt->dtmf_time = 0;
					break;
				}
				if(res != DC_INDETERMINATE) {
					rpt_mutex_unlock(&myrpt->lock);
					return;
				}
			}
		}
	}
	else /* if simple */
	{
		if ((!myrpt->callmode) && (c == myrpt->p.funcchar))
		{
			myrpt->callmode = 1;
			myrpt->patchnoct = 0;
			myrpt->patchquiet = 0;
			myrpt->patchfarenddisconnect = 0;
			myrpt->patchdialtime = 0;
			strncpy(myrpt->patchcontext, myrpt->p.ourcontext, MAXPATCHCONTEXT);
			myrpt->cidx = 0;
			myrpt->exten[myrpt->cidx] = 0;
			rpt_mutex_unlock(&myrpt->lock);
		        pthread_attr_init(&attr);
		        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			ast_pthread_create(&myrpt->rpt_call_thread,&attr,rpt_call,(void *)myrpt);
			return;
		}
	}
	if (myrpt->callmode == 1)
	{
		myrpt->exten[myrpt->cidx++] = c;
		myrpt->exten[myrpt->cidx] = 0;
		/* if this exists */
		if (ast_exists_extension(myrpt->pchannel,myrpt->patchcontext,myrpt->exten,1,NULL))
		{
			/* if this really it, end now */
			if (!ast_matchmore_extension(myrpt->pchannel,myrpt->patchcontext,
				myrpt->exten,1,NULL))
			{
				myrpt->callmode = 2;
				rpt_mutex_unlock(&myrpt->lock);
				if(!myrpt->patchquiet)
					rpt_telemetry(myrpt,PROC,NULL);
				return;
			}
			else /* othewise, reset timer */
			{
				myrpt->calldigittimer = 1;
			}
		}
		/* if can continue, do so */
		if (!ast_canmatch_extension(myrpt->pchannel,myrpt->patchcontext,myrpt->exten,1,NULL))
		{
			/* call has failed, inform user */
			myrpt->callmode = 4;
		}
		rpt_mutex_unlock(&myrpt->lock);
		return;
	}
	if ((myrpt->callmode == 2) || (myrpt->callmode == 3))
	{
		myrpt->mydtmf = c;
	}
	rpt_mutex_unlock(&myrpt->lock);
	if ((myrpt->dtmfidx < 0) && myrpt->p.propagate_phonedtmf)
		do_dtmf_phone(myrpt,NULL,c);
	return;
}


/* place an ID event in the telemetry queue */

static void queue_id(struct rpt *myrpt)
{
	if(myrpt->p.idtime){ /* ID time must be non-zero */
		myrpt->mustid = myrpt->tailid = 0;
		myrpt->idtimer = myrpt->p.idtime; /* Reset our ID timer */
		rpt_mutex_unlock(&myrpt->lock);
		rpt_telemetry(myrpt,ID,NULL);
		rpt_mutex_lock(&myrpt->lock);
	}
}

/* Scheduler */
/* must be called locked */

static void do_scheduler(struct rpt *myrpt)
{
	int i,res;

#ifdef	NEW_ASTERISK
	struct ast_tm tmnow;
#else
	struct tm tmnow;
#endif
	struct ast_variable *skedlist;
	char *strs[5],*vp,*val,value[100];

	memcpy(&myrpt->lasttv, &myrpt->curtv, sizeof(struct timeval));

	if( (res = gettimeofday(&myrpt->curtv, NULL)) < 0)
		ast_log(LOG_NOTICE, "Scheduler gettime of day returned: %s\n", strerror(res));

	/* Try to get close to a 1 second resolution */

	if(myrpt->lasttv.tv_sec == myrpt->curtv.tv_sec)
		return;

	/* Service activity timer */
	if(myrpt->p.lnkactmacro && myrpt->p.lnkacttime && myrpt->p.lnkactenable && myrpt->linkactivityflag){
		myrpt->linkactivitytimer++;
		/* 30 second warn */
		if ((myrpt->p.lnkacttime - myrpt->linkactivitytimer == 30) && myrpt->p.lnkacttimerwarn){
			if(debug > 4)
				ast_log(LOG_NOTICE, "Warning user of activity timeout\n");
			rpt_telemetry(myrpt,LOCALPLAY, myrpt->p.lnkacttimerwarn);
		}
		if(myrpt->linkactivitytimer >= myrpt->p.lnkacttime){
			/* Execute lnkactmacro */
			if ((MAXMACRO - strlen(myrpt->macrobuf)) < strlen(myrpt->p.lnkactmacro)){
				ast_log(LOG_WARNING, "Link Activity timer could not execute macro %s: Macro buffer full\n",
					myrpt->p.lnkactmacro);
			}
			else{
				if(debug > 4)
					ast_log(LOG_NOTICE, "Executing activity timer macro %s\n", myrpt->p.lnkactmacro);
				myrpt->macrotimer = MACROTIME;
				strncat(myrpt->macrobuf,myrpt->p.lnkactmacro, MAXMACRO - 1);
			}
			myrpt->linkactivitytimer = 0;
			myrpt->linkactivityflag = 0;
		}
	}

	rpt_localtime(&myrpt->curtv.tv_sec, &tmnow);

	/* If midnight, then reset all daily statistics */

	if((tmnow.tm_hour == 0)&&(tmnow.tm_min == 0)&&(tmnow.tm_sec == 0)){
		myrpt->dailykeyups = 0;
		myrpt->dailytxtime = 0;
		myrpt->dailykerchunks = 0;
		myrpt->dailyexecdcommands = 0;
	}

	if(tmnow.tm_sec != 0)
		return;

	/* Code below only executes once per minute */


	/* Don't schedule if remote */

        if (myrpt->remote)
                return;

	/* Don't schedule if disabled */

        if(myrpt->p.s[myrpt->p.sysstate_cur].schedulerdisable){
		if(debug > 6)
			ast_log(LOG_NOTICE, "Scheduler disabled\n");
		return;
	}

	if(!myrpt->p.skedstanzaname){ /* No stanza means we do nothing */
		if(debug > 7)
			ast_log(LOG_NOTICE,"No stanza for scheduler in rpt.conf\n");
		return;
	}

    /* get pointer to linked list of scheduler entries */
    skedlist = ast_variable_browse(myrpt->cfg, myrpt->p.skedstanzaname);

	if(debug > 6){
		ast_log(LOG_NOTICE, "Time now: %02d:%02d %02d %02d %02d\n",
			tmnow.tm_hour,tmnow.tm_min,tmnow.tm_mday,tmnow.tm_mon + 1, tmnow.tm_wday);
	}
	/* walk the list */
	for(; skedlist; skedlist = skedlist->next){
		if(debug > 6)
			ast_log(LOG_NOTICE, "Scheduler entry %s = %s being considered\n",skedlist->name, skedlist->value);
		strncpy(value,skedlist->value,99);
		value[99] = 0;
		/* point to the substrings for minute, hour, dom, month, and dow */
		for( i = 0, vp = value ; i < 5; i++){
			if(!*vp)
				break;
			while((*vp == ' ') || (*vp == 0x09)) /* get rid of any leading white space */
				vp++;
			strs[i] = vp; /* save pointer to beginning of substring */
			while((*vp != ' ') && (*vp != 0x09) && (*vp != 0)) /* skip over substring */
				vp++;
			if(*vp)
				*vp++ = 0; /* mark end of substring */
		}
		if(debug > 6)
			ast_log(LOG_NOTICE, "i = %d, min = %s, hour = %s, mday=%s, mon=%s, wday=%s\n",i,
				strs[0], strs[1], strs[2], strs[3], strs[4]);
 		if(i == 5){
			if((*strs[0] != '*')&&(atoi(strs[0]) != tmnow.tm_min))
				continue;
			if((*strs[1] != '*')&&(atoi(strs[1]) != tmnow.tm_hour))
				continue;
			if((*strs[2] != '*')&&(atoi(strs[2]) != tmnow.tm_mday))
				continue;
			if((*strs[3] != '*')&&(atoi(strs[3]) != tmnow.tm_mon + 1))
				continue;
			if(atoi(strs[4]) == 7)
				strs[4] = "0";
			if((*strs[4] != '*')&&(atoi(strs[4]) != tmnow.tm_wday))
				continue;
			if(debug)
				ast_log(LOG_NOTICE, "Executing scheduler entry %s = %s\n", skedlist->name, skedlist->value);
			if(atoi(skedlist->name) == 0)
				return; /* Zero is reserved for the startup macro */
			val = (char *) ast_variable_retrieve(myrpt->cfg, myrpt->p.macro, skedlist->name);
			if (!val){
				ast_log(LOG_WARNING,"Scheduler could not find macro %s\n",skedlist->name);
				return; /* Macro not found */
			}
			if ((MAXMACRO - strlen(myrpt->macrobuf)) < strlen(val)){
				ast_log(LOG_WARNING, "Scheduler could not execute macro %s: Macro buffer full\n",
					skedlist->name);
				return; /* Macro buffer full */
			}
			myrpt->macrotimer = MACROTIME;
			strncat(myrpt->macrobuf,val,MAXMACRO - 1);
		}
		else{
			ast_log(LOG_WARNING,"Malformed scheduler entry in rpt.conf: %s = %s\n",
				skedlist->name, skedlist->value);
		}
	}

}

/*
	get a file handle for audio logging
*/
static int get_handle_audiolog(struct rpt *myrpt, int type)
{
	if (myrpt->monstream) ast_closestream(myrpt->monstream);
	myrpt->monstream = NULL;
	if (myrpt->p.archivedir)
	{
		long blocksleft;
		time_t myt;
		char mydate[100],myfname[100];

		time(&myt);
        if(!type)
		  strftime(mydate,sizeof(mydate) - 1,"%Y%m%d%H%M%Srx",
			localtime(&myt));
		else
		  strftime(mydate,sizeof(mydate) - 1,"%Y%m%d%H%M%S",
			localtime(&myt));
		sprintf(myfname,"%s/%s/%s",myrpt->p.archivedir,
			myrpt->name,mydate);
		myrpt->monstream = ast_writefile(myfname,"wav49",
			"app_rpt Air Archive",O_CREAT | O_APPEND,0,0660);
		if (myrpt->p.monminblocks)
		{
			blocksleft = diskavail(myrpt);
			if (blocksleft >= myrpt->p.monminblocks)
				donodelog(myrpt,"TXKEY,MAIN");
		} else donodelog(myrpt,"TXKEY,MAIN");
	}
	return 0;
}

/* single thread with one file (request) to dial */
/*

*/
static void *rpt(void *this)
{
struct	rpt *myrpt = (struct rpt *)this;
char *tele,*idtalkover,c,myfirst,*p;
int ms = MSWAIT,i,lasttx=0,val,identqueued,othertelemqueued;
int tailmessagequeued,ctqueued,dtmfed,lastmyrx,localmsgqueued;
struct ast_channel *who;
struct dahdi_confinfo ci;  /* conference info */
time_t	t;
struct rpt_link *l,*m;
struct rpt_tele *telem;
char tmpstr[300],lstr[MAXLINKLIST];

	ast_log(LOG_NOTICE,"[%s] start \n",myrpt->name);
	if (myrpt->p.archivedir) mkdir(myrpt->p.archivedir,0777);
	sprintf(tmpstr,"%s/%s",myrpt->p.archivedir,myrpt->name);
	mkdir(tmpstr,0777);
	rpt_mutex_lock(&myrpt->lock);
	myrpt->remrx = 0;
	myrpt->monstream = 0;
	telem = myrpt->tele.next;
	while(telem != &myrpt->tele)
	{
		ast_softhangup(telem->chan,AST_SOFTHANGUP_DEV);
		telem = telem->next;
	}
	rpt_mutex_unlock(&myrpt->lock);
	/* find our index, and load the vars initially */
	for(i = 0; i < nrpts; i++)
	{
		if (&rpt_vars[i] == myrpt)
		{
			load_rpt_vars(i,0);
			break;
		}
	}

	rpt_mutex_lock(&myrpt->lock);
	while(myrpt->xlink)
	{
		myrpt->xlink = 3;
		rpt_mutex_unlock(&myrpt->lock);
		usleep(100000);
		rpt_mutex_lock(&myrpt->lock);
	}
	if ((!strcmp(myrpt->remoterig, remote_rig_rbi)) &&
	  (ioperm(myrpt->p.iobase,1,1) == -1))
	{
		rpt_mutex_unlock(&myrpt->lock);
		ast_log(LOG_WARNING, "Cant get io permission on IO port %x hex\n",myrpt->p.iobase);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
	strncpy(tmpstr,myrpt->rxchanname,sizeof(tmpstr) - 1);
	tele = strchr(tmpstr,'/');
	if (!tele)
	{
		fprintf(stderr,"rpt:Rxchannel Dial number (%s) must be in format tech/number\n",myrpt->rxchanname);
		rpt_mutex_unlock(&myrpt->lock);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
	*tele++ = 0;
	myrpt->rxchannel = ast_request(tmpstr,AST_FORMAT_SLINEAR,tele,NULL);
	myrpt->zaprxchannel = NULL;
	myrpt->hasnoradio=0;
	if (!strcasecmp(tmpstr,"dahdi"))
		myrpt->zaprxchannel = myrpt->rxchannel;
	if (myrpt->rxchannel)
	{
		if (myrpt->rxchannel->_state == AST_STATE_BUSY)
		{
			fprintf(stderr,"%i rpt:Sorry unable to obtain Rx channel (AST_STATE_BUSY)\n",__LINE__);
			rpt_mutex_unlock(&myrpt->lock);
			ast_hangup(myrpt->rxchannel);
			myrpt->rpt_thread = AST_PTHREADT_STOP;
			pthread_exit(NULL);
		}
		ast_set_read_format(myrpt->rxchannel,AST_FORMAT_SLINEAR);
		ast_set_write_format(myrpt->rxchannel,AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
		if (myrpt->rxchannel->cdr)
			ast_set_flag(myrpt->rxchannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
#ifndef	NEW_ASTERISK
		myrpt->rxchannel->whentohangup = 0;
#endif
		myrpt->rxchannel->appl = "Apprpt";
		myrpt->rxchannel->data = "(Repeater Rx)";
		if (option_verbose > 2)
			ast_verbose(VERBOSE_PREFIX_3 "rpt (Rx) initiating call to %s/%s on %s\n",
				tmpstr,tele,myrpt->rxchannel->name);
		ast_call(myrpt->rxchannel,tele,999);
		if (myrpt->rxchannel->_state != AST_STATE_UP)
		{
			rpt_mutex_unlock(&myrpt->lock);
			ast_hangup(myrpt->rxchannel);
			myrpt->rpt_thread = AST_PTHREADT_STOP;
			pthread_exit(NULL);
		}
	}
	else
	{
		fprintf(stderr,"%i rpt:Sorry unable to obtain Rx channel (ast_request)\n",__LINE__);
		rpt_mutex_unlock(&myrpt->lock);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
	myrpt->zaptxchannel = NULL;
	if (myrpt->txchanname)
	{
		strncpy(tmpstr,myrpt->txchanname,sizeof(tmpstr) - 1);
		tele = strchr(tmpstr,'/');
		if (!tele)
		{
			fprintf(stderr,"rpt:Txchannel Dial number (%s) must be in format tech/number\n",myrpt->txchanname);
			rpt_mutex_unlock(&myrpt->lock);
			ast_hangup(myrpt->rxchannel);
			myrpt->rpt_thread = AST_PTHREADT_STOP;
			pthread_exit(NULL);
		}
		*tele++ = 0;
		myrpt->txchannel = ast_request(tmpstr,AST_FORMAT_SLINEAR,tele,NULL);
		if ((!strcasecmp(tmpstr,"dahdi")) && strcasecmp(tele,"pseudo"))
			myrpt->zaptxchannel = myrpt->txchannel;
		if (myrpt->txchannel)
		{
			if (myrpt->txchannel->_state == AST_STATE_BUSY)
			{
				fprintf(stderr,"rpt:Sorry unable to obtain Tx channel\n");
				rpt_mutex_unlock(&myrpt->lock);
				ast_hangup(myrpt->txchannel);
				ast_hangup(myrpt->rxchannel);
				myrpt->rpt_thread = AST_PTHREADT_STOP;
				pthread_exit(NULL);
			}
			ast_set_read_format(myrpt->txchannel,AST_FORMAT_SLINEAR);
			ast_set_write_format(myrpt->txchannel,AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
			if (myrpt->txchannel->cdr)
				ast_set_flag(myrpt->txchannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
#ifndef	NEW_ASTERISK
			myrpt->txchannel->whentohangup = 0;
#endif
			myrpt->txchannel->appl = "Apprpt";
			myrpt->txchannel->data = "(Repeater Tx)";
			if (option_verbose > 2)
				ast_verbose(VERBOSE_PREFIX_3 "rpt (Tx) initiating call to %s/%s on %s\n",
					tmpstr,tele,myrpt->txchannel->name);
			ast_call(myrpt->txchannel,tele,999);
			if (myrpt->rxchannel->_state != AST_STATE_UP)
			{
				rpt_mutex_unlock(&myrpt->lock);
				ast_hangup(myrpt->rxchannel);
				ast_hangup(myrpt->txchannel);
				myrpt->rpt_thread = AST_PTHREADT_STOP;
				pthread_exit(NULL);
			}
		}
		else
		{
			fprintf(stderr,"rpt:Sorry unable to obtain Tx channel\n");
			rpt_mutex_unlock(&myrpt->lock);
			ast_hangup(myrpt->rxchannel);
			myrpt->rpt_thread = AST_PTHREADT_STOP;
			pthread_exit(NULL);
		}
	}
	else
	{
		myrpt->txchannel = myrpt->rxchannel;
		if ((!strncasecmp(myrpt->rxchanname,"dahdi",3)) &&
		    strcasecmp(myrpt->rxchanname,"dahdi/pseudo"))
		{
			myrpt->zaptxchannel = myrpt->txchannel;
		}
	}

	if ( !strcasecmp(myrpt->rxchanname,"dahdi/pseudo") )
	{
		myrpt->hasnoradio=1;
	}
	if (strncasecmp(myrpt->txchannel->name,"dahdi/pseudo",10))
	{
		ast_indicate(myrpt->txchannel,AST_CONTROL_RADIO_KEY);
		ast_indicate(myrpt->txchannel,AST_CONTROL_RADIO_UNKEY);
	}
	/* Rx Conference Channel Resources */
	/* allocate a pseudo-channel thru asterisk */
	myrpt->pchannel = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
	if (!myrpt->pchannel)
	{
		fprintf(stderr,"rpt:Sorry unable to obtain pseudo channel\n");
		rpt_mutex_unlock(&myrpt->lock);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
#ifdef	AST_CDR_FLAG_POST_DISABLED
	if (myrpt->pchannel->cdr)
		ast_set_flag(myrpt->pchannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
	ast_set_read_format(myrpt->pchannel,AST_FORMAT_SLINEAR);
	ast_set_write_format(myrpt->pchannel,AST_FORMAT_SLINEAR);
	if (!myrpt->zaprxchannel) myrpt->zaprxchannel = myrpt->pchannel;
	if (!myrpt->zaptxchannel)
	{
		/* allocate a pseudo-channel thru asterisk */
		myrpt->zaptxchannel = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
		if (!myrpt->zaptxchannel)
		{
			fprintf(stderr,"rpt:Sorry unable to obtain pseudo channel\n");
			rpt_mutex_unlock(&myrpt->lock);
			if (myrpt->txchannel != myrpt->rxchannel)
				ast_hangup(myrpt->txchannel);
			ast_hangup(myrpt->rxchannel);
			myrpt->rpt_thread = AST_PTHREADT_STOP;
			pthread_exit(NULL);
		}
		ast_set_read_format(myrpt->zaptxchannel,AST_FORMAT_SLINEAR);
		ast_set_write_format(myrpt->zaptxchannel,AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
		if (myrpt->zaptxchannel->cdr)
			ast_set_flag(myrpt->zaptxchannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
	}
	/* Monitor Channel */
	/* allocate a pseudo-channel thru asterisk */
	myrpt->monchannel = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
	if (!myrpt->monchannel)
	{
		fprintf(stderr,"rpt:Sorry unable to obtain pseudo channel\n");
		rpt_mutex_unlock(&myrpt->lock);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
	ast_set_read_format(myrpt->monchannel,AST_FORMAT_SLINEAR);
	ast_set_write_format(myrpt->monchannel,AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
	if (myrpt->monchannel->cdr)
		ast_set_flag(myrpt->monchannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
	/* make a conference for the tx */
	ci.chan = 0;
	ci.confno = -1; /* make a new conf */
	ci.confmode = DAHDI_CONF_CONF | DAHDI_CONF_LISTENER;
	/* first put the channel on the conference in proper mode */
	if (ioctl(myrpt->zaptxchannel->fds[0],DAHDI_SETCONF,&ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
		rpt_mutex_unlock(&myrpt->lock);
		ast_hangup(myrpt->pchannel);
		ast_hangup(myrpt->monchannel);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
	/* save tx conference number */
	myrpt->txconf = ci.confno;
	/* make a conference for the pseudo */
	ci.chan = 0;
	ci.confno = -1; /* make a new conf */
	ci.confmode = ((myrpt->p.duplex == 2) || (myrpt->p.duplex == 4)) ? DAHDI_CONF_CONFANNMON :
		(DAHDI_CONF_CONF | DAHDI_CONF_LISTENER | DAHDI_CONF_TALKER);
	/* first put the channel on the conference in announce mode */
	if (ioctl(myrpt->pchannel->fds[0],DAHDI_SETCONF,&ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
		rpt_mutex_unlock(&myrpt->lock);
		ast_hangup(myrpt->pchannel);
		ast_hangup(myrpt->monchannel);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
	/* save pseudo channel conference number */
	myrpt->conf = ci.confno;
	/* make a conference for the pseudo */
	ci.chan = 0;
	if ((strstr(myrpt->txchannel->name,"pseudo") == NULL) &&
		(myrpt->zaptxchannel == myrpt->txchannel))
	{
		/* get tx channel's port number */
		if (ioctl(myrpt->txchannel->fds[0],DAHDI_CHANNO,&ci.confno) == -1)
		{
			ast_log(LOG_WARNING, "Unable to set tx channel's chan number\n");
			rpt_mutex_unlock(&myrpt->lock);
			ast_hangup(myrpt->pchannel);
			ast_hangup(myrpt->monchannel);
			if (myrpt->txchannel != myrpt->rxchannel)
				ast_hangup(myrpt->txchannel);
			ast_hangup(myrpt->rxchannel);
			myrpt->rpt_thread = AST_PTHREADT_STOP;
			pthread_exit(NULL);
		}
		ci.confmode = DAHDI_CONF_MONITORTX;
	}
	else
	{
		ci.confno = myrpt->txconf;
		ci.confmode = DAHDI_CONF_CONFANNMON;
	}
	/* first put the channel on the conference in announce mode */
	if (ioctl(myrpt->monchannel->fds[0],DAHDI_SETCONF,&ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode for monitor\n");
		rpt_mutex_unlock(&myrpt->lock);
		ast_hangup(myrpt->pchannel);
		ast_hangup(myrpt->monchannel);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}

	/* Parrot Channel Resources */
	/* allocate a pseudo-channel thru asterisk */
	myrpt->parrotchannel = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
	if (!myrpt->parrotchannel)
	{
		fprintf(stderr,"rpt:Sorry unable to obtain pseudo channel\n");
		rpt_mutex_unlock(&myrpt->lock);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
	ast_set_read_format(myrpt->parrotchannel,AST_FORMAT_SLINEAR);
	ast_set_write_format(myrpt->parrotchannel,AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
	if (myrpt->parrotchannel->cdr)
		ast_set_flag(myrpt->parrotchannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif


	/* Telemetry Channel Resources */
	/* allocate a pseudo-channel thru asterisk */
	myrpt->telechannel = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
	if (!myrpt->telechannel)
	{
		fprintf(stderr,"rpt:Sorry unable to obtain pseudo channel\n");
		rpt_mutex_unlock(&myrpt->lock);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
	ast_set_read_format(myrpt->telechannel,AST_FORMAT_SLINEAR);
	ast_set_write_format(myrpt->telechannel,AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
	if (myrpt->telechannel->cdr)
		ast_set_flag(myrpt->telechannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
 	/* make a conference for the voice/tone telemetry */
	ci.chan = 0;
	ci.confno = -1; // make a new conference
	ci.confmode = DAHDI_CONF_CONF | DAHDI_CONF_TALKER | DAHDI_CONF_LISTENER;
 	/* put the channel on the conference in proper mode */
	if (ioctl(myrpt->telechannel->fds[0],DAHDI_SETCONF,&ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
		rpt_mutex_unlock(&myrpt->lock);
		ast_hangup(myrpt->txpchannel);
		ast_hangup(myrpt->monchannel);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
	myrpt->teleconf=ci.confno;

	/* make a channel to connect between the telemetry conference process
	   and the main tx audio conference. */
	myrpt->btelechannel = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
	if (!myrpt->btelechannel)
	{
		fprintf(stderr,"rtp:Failed to obtain pseudo channel for btelechannel\n");
		rpt_mutex_unlock(&myrpt->lock);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
	ast_set_read_format(myrpt->btelechannel,AST_FORMAT_SLINEAR);
	ast_set_write_format(myrpt->btelechannel,AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
	if (myrpt->btelechannel->cdr)
		ast_set_flag(myrpt->btelechannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
	/* make a conference linked to the main tx conference */
	ci.chan = 0;
	ci.confno = myrpt->txconf;
	ci.confmode = DAHDI_CONF_CONF | DAHDI_CONF_LISTENER | DAHDI_CONF_TALKER;
	/* first put the channel on the conference in proper mode */
	if (ioctl(myrpt->btelechannel->fds[0], DAHDI_SETCONF, &ci) == -1)
	{
		ast_log(LOG_ERROR, "Failed to create btelechannel.\n");
		ast_hangup(myrpt->btelechannel);
		ast_hangup(myrpt->btelechannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}

	/* VOX Channel Resources */
	/* allocate a pseudo-channel thru asterisk */
	myrpt->voxchannel = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
	if (!myrpt->voxchannel)
	{
		fprintf(stderr,"rpt:Sorry unable to obtain pseudo channel\n");
		rpt_mutex_unlock(&myrpt->lock);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
	ast_set_read_format(myrpt->voxchannel,AST_FORMAT_SLINEAR);
	ast_set_write_format(myrpt->voxchannel,AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
	if (myrpt->voxchannel->cdr)
		ast_set_flag(myrpt->voxchannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif

	/* Tx Conference Resources */
	/* allocate a pseudo-channel thru asterisk */
	myrpt->txpchannel = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
	if (!myrpt->txpchannel)
	{
		fprintf(stderr,"rpt:Sorry unable to obtain pseudo channel\n");
		rpt_mutex_unlock(&myrpt->lock);
		ast_hangup(myrpt->pchannel);
		ast_hangup(myrpt->monchannel);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}
#ifdef	AST_CDR_FLAG_POST_DISABLED
	if (myrpt->txpchannel->cdr)
		ast_set_flag(myrpt->txpchannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
	/* make a conference for the tx */
	ci.chan = 0;
	ci.confno = myrpt->txconf;
	ci.confmode = DAHDI_CONF_CONF | DAHDI_CONF_TALKER ;
 	/* first put the channel on the conference in proper mode */
	if (ioctl(myrpt->txpchannel->fds[0],DAHDI_SETCONF,&ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
		rpt_mutex_unlock(&myrpt->lock);
		ast_hangup(myrpt->txpchannel);
		ast_hangup(myrpt->monchannel);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		myrpt->rpt_thread = AST_PTHREADT_STOP;
		pthread_exit(NULL);
	}

	/* if serial io port, open it */
	myrpt->iofd = -1;
	if (myrpt->p.ioport && ((myrpt->iofd = openserial(myrpt,myrpt->p.ioport)) == -1))
	{
		ast_log(LOG_ERROR, "Unable to open %s\n",myrpt->p.ioport);
		rpt_mutex_unlock(&myrpt->lock);
		ast_hangup(myrpt->pchannel);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		pthread_exit(NULL);
	}

	/* if radio is parallel port channel steering type, steer to channel 1 */
	if (!strcmp(myrpt->remoterig, remote_rig_ppp16))
	{
		sprintf(tmpstr,"%d",1);
		channel_steer(myrpt, tmpstr);
	}

	/* Now, the idea here is to copy from the physical rx channel buffer
	   into the pseudo tx buffer, and from the pseudo rx buffer into the
	   tx channel buffer */
	myrpt->links.next = &myrpt->links;
	myrpt->links.prev = &myrpt->links;
	myrpt->tailtimer = 0;
	myrpt->totimer = myrpt->p.totime;
	myrpt->tmsgtimer = myrpt->p.tailmessagetime;
	myrpt->idtimer = myrpt->p.politeid;
	myrpt->mustid = myrpt->tailid = 0;
	myrpt->callmode = 0;
	myrpt->tounkeyed = 0;
	myrpt->tonotify = 0;
	myrpt->retxtimer = 0;
	myrpt->rerxtimer = 0;
	myrpt->skedtimer = 0;
	myrpt->tailevent = 0;
	lasttx = 0;
	myrpt->keyed = 0;
	myrpt->rxchankeyed = 0;
	myrpt->txkeyed = 0;
	time(&myrpt->lastkeyedtime);
	myrpt->lastkeyedtime -= RPT_LOCKOUT_SECS;
	time(&myrpt->lasttxkeyedtime);
	myrpt->lasttxkeyedtime -= RPT_LOCKOUT_SECS;
	idtalkover = (char *) ast_variable_retrieve(myrpt->cfg, myrpt->name, "idtalkover");
	myrpt->dtmfidx = -1;
	myrpt->dtmfbuf[0] = 0;
	myrpt->rem_dtmfidx = -1;
	myrpt->rem_dtmfbuf[0] = 0;
	myrpt->dtmf_time = 0;
	myrpt->rem_dtmf_time = 0;
	myrpt->inpadtest = 0;
	myrpt->disgorgetime = 0;
	myrpt->lastnodewhichkeyedusup[0] = '\0';
	myrpt->dailytxtime = 0;
	myrpt->totaltxtime = 0;
	myrpt->dailykeyups = 0;
	myrpt->totalkeyups = 0;
	myrpt->dailykerchunks = 0;
	myrpt->totalkerchunks = 0;
	myrpt->dailyexecdcommands = 0;
	myrpt->totalexecdcommands = 0;
	myrpt->timeouts = 0;
	myrpt->exten[0] = '\0';
	myrpt->lastdtmfcommand[0] = '\0';
	voxinit_rpt(myrpt,1);
	myrpt->wasvox = 0;
	myrpt->linkactivityflag = 0;
	myrpt->linkactivitytimer = 0;
	myrpt->vote_counter=10;

	if (myrpt->p.rxburstfreq)
	{
		tone_detect_init(&myrpt->burst_tone_state,myrpt->p.rxburstfreq,
			myrpt->p.rxbursttime,myrpt->p.rxburstthreshold);
	}
	if (myrpt->p.startupmacro)
	{
		snprintf(myrpt->macrobuf,MAXMACRO - 1,"PPPP%s",myrpt->p.startupmacro);
	}
	rpt_mutex_unlock(&myrpt->lock);
	val = 1;
	ast_channel_setoption(myrpt->rxchannel,AST_OPTION_RELAXDTMF,&val,sizeof(char),0);
	val = 1;
	ast_channel_setoption(myrpt->rxchannel,AST_OPTION_TONE_VERIFY,&val,sizeof(char),0);
	if (myrpt->p.archivedir) donodelog(myrpt,"STARTUP");
	dtmfed = 0;
	if (myrpt->remoterig && !ISRIG_RTX(myrpt->remoterig)) setrem(myrpt);
	lastmyrx = 0;
	myfirst = 0;

	/* loop forever and service the node */
	while (ms >= 0)
	{
		struct ast_frame *f,*f1,*f2;
		struct ast_channel *cs[300],*cs1[300];
		int totx=0,elap=0,n,x,toexit=0;

		/* DEBUG Dump */
		if((myrpt->disgorgetime) && (time(NULL) >= myrpt->disgorgetime)){
			struct rpt_link *zl;
			struct rpt_tele *zt;

			myrpt->disgorgetime = 0;
			ast_log(LOG_NOTICE,"******** Variable Dump Start node %s ********\n",myrpt->name);
			ast_log(LOG_NOTICE,"totx = %d\n",totx);
			ast_log(LOG_NOTICE,"myrpt->remrx = %d\n",myrpt->remrx);
			ast_log(LOG_NOTICE,"lasttx = %d\n",lasttx);
			ast_log(LOG_NOTICE,"elap = %d\n",elap);
			ast_log(LOG_NOTICE,"toexit = %d\n",toexit);

			ast_log(LOG_NOTICE,"myrpt->p.duplex = %i\n",myrpt->p.duplex);
			ast_log(LOG_NOTICE,"myrpt->p.startupmacro = %s\n",myrpt->p.startupmacro);
			ast_log(LOG_NOTICE,"myrpt->p.propagate_dtmf = %d\n",myrpt->p.propagate_dtmf);
			ast_log(LOG_NOTICE,"myrpt->p.nonodelist = %d\n",myrpt->p.nonodelist);
			ast_log(LOG_NOTICE,"myrpt->p.itxctcss = %i\n",myrpt->p.itxctcss);
			ast_log(LOG_NOTICE,"myrpt->p.ncodemasters = %i\n",myrpt->p.ncodemasters);
			ast_log(LOG_NOTICE,"myrpt->p.ncomnum = %s\n",myrpt->p.ncomnum);

			ast_log(LOG_NOTICE,"myrpt->innox = %d\n",myrpt->innox);
			ast_log(LOG_NOTICE,"myrpt->radiofail = %d\n",myrpt->radiofail);

			ast_log(LOG_NOTICE,"myrpt->hasnoradio = %d\n",myrpt->hasnoradio);
			ast_log(LOG_NOTICE,"myrpt->radiotaken = %d\n",myrpt->radiotaken);
			ast_log(LOG_NOTICE,"myrpt->newkey = %d\n",myrpt->newkey);
			ast_log(LOG_NOTICE,"myrpt->keyed = %d\n",myrpt->keyed);
			ast_log(LOG_NOTICE,"myrpt->rxchankeyed = %d\n",myrpt->rxchankeyed);
			ast_log(LOG_NOTICE,"myrpt->localtx = %d\n",myrpt->localtx);

			ast_log(LOG_NOTICE,"myrpt->csrxnow = %s\n",myrpt->csrxnow);
			ast_log(LOG_NOTICE,"myrpt->cstxnow = %s\n",myrpt->cstxnow);
			ast_log(LOG_NOTICE,"myrpt->csrxcode = %s\n",myrpt->csrxcode);
			ast_log(LOG_NOTICE,"myrpt->cstxcode = %s\n",myrpt->cstxcode);

			ast_log(LOG_NOTICE,"myrpt->callmode = %d\n",myrpt->callmode);
			ast_log(LOG_NOTICE,"myrpt->mustid = %d\n",myrpt->mustid);
			ast_log(LOG_NOTICE,"myrpt->tounkeyed = %d\n",myrpt->tounkeyed);
			ast_log(LOG_NOTICE,"myrpt->tonotify = %d\n",myrpt->tonotify);
			ast_log(LOG_NOTICE,"myrpt->retxtimer = %ld\n",myrpt->retxtimer);
			ast_log(LOG_NOTICE,"myrpt->totimer = %d\n",myrpt->totimer);
			ast_log(LOG_NOTICE,"myrpt->tailtimer = %d\n",myrpt->tailtimer);
			ast_log(LOG_NOTICE,"myrpt->tailevent = %d\n",myrpt->tailevent);
			ast_log(LOG_NOTICE,"myrpt->linkactivitytimer = %d\n",myrpt->linkactivitytimer);
			ast_log(LOG_NOTICE,"myrpt->linkactivityflag = %d\n",(int) myrpt->linkactivityflag);
			ast_log(LOG_NOTICE,"myrpt->p.parrotmode = %d\n",(int) myrpt->p.parrotmode);
			ast_log(LOG_NOTICE,"myrpt->parrotonce = %d\n",(int) myrpt->parrotonce);

			zl = myrpt->links.next;
            while(zl != &myrpt->links){
				ast_log(LOG_NOTICE,"*** Link Name: %s ***\n",zl->name);

				ast_log(LOG_NOTICE,"        link->mode = %d\n",zl->mode);
				ast_log(LOG_NOTICE,"        link->isremote = %d\n",zl->isremote);
				ast_log(LOG_NOTICE,"        link->phonemode = %d\n",zl->phonemode);
				ast_log(LOG_NOTICE,"        link->phonemode = %d\n",zl->phonemode);
				ast_log(LOG_NOTICE,"        link->phonemonitor = %d\n",zl->phonemonitor);
				ast_log(LOG_NOTICE,"        link->lasttx %d\n",zl->lasttx);
				ast_log(LOG_NOTICE,"        link->lastrx %d\n",zl->lastrx);
				ast_log(LOG_NOTICE,"        link->connected %d\n",zl->connected);
				ast_log(LOG_NOTICE,"        link->hasconnected %d\n",zl->hasconnected);
				ast_log(LOG_NOTICE,"        link->outbound %d\n",zl->outbound);
				ast_log(LOG_NOTICE,"        link->disced %d\n",zl->disced);
				ast_log(LOG_NOTICE,"        link->killme %d\n",zl->killme);
				ast_log(LOG_NOTICE,"        link->disctime %ld\n",zl->disctime);
				ast_log(LOG_NOTICE,"        link->retrytimer %ld\n",zl->retrytimer);
				ast_log(LOG_NOTICE,"        link->retries = %d\n",zl->retries);
				ast_log(LOG_NOTICE,"        link->reconnects = %d\n",zl->reconnects);
				ast_log(LOG_NOTICE,"        link->newkey = %d\n",zl->newkey);
				ast_log(LOG_NOTICE,"        link->iaxkey = %d\n",zl->iaxkey);
				ast_log(LOG_NOTICE,"        link->iaxrpt = %d\n",zl->iaxrpt);
				ast_log(LOG_NOTICE,"        link->csmonitor = %d\n",zl->csmonitor);
				ast_log(LOG_NOTICE,"        link->cstxcode = %s\n",zl->cstxcode);
				ast_log(LOG_NOTICE,"        link->csrxcode = %s\n",zl->csrxcode);
				ast_log(LOG_NOTICE,"        link->cstxnow = %s\n",zl->cstxnow);
				ast_log(LOG_NOTICE,"        link->csrxnow = %s\n",zl->csrxnow);
				ast_log(LOG_NOTICE,"        link->csm = %d\n",zl->csm);
				ast_log(LOG_NOTICE,"        link->csmm = %d\n",zl->csmm);
				ast_log(LOG_NOTICE,"        link->gott = %d\n",zl->gott);
				ast_log(LOG_NOTICE,"        link->linkmode = %d\n",zl->linkmode);

                zl = zl->next;
            }

			zt = myrpt->tele.next;
			if(zt != &myrpt->tele)
				ast_log(LOG_NOTICE,"*** Telemetry Queue ***\n");
              		while(zt != &myrpt->tele){
				ast_log(LOG_NOTICE,"        Telemetry mode: %d\n",zt->mode);
                        	zt = zt->next;
                	}
			ast_log(LOG_NOTICE,"******* Variable Dump End *******\n");

		}


		if (myrpt->reload)
		{
			struct rpt_tele *telem;

			rpt_mutex_lock(&myrpt->lock);
			telem = myrpt->tele.next;
			while(telem != &myrpt->tele)
			{
				ast_softhangup(telem->chan,AST_SOFTHANGUP_DEV);
				telem = telem->next;
			}
			myrpt->reload = 0;
			rpt_mutex_unlock(&myrpt->lock);
			usleep(10000);
			/* find our index, and load the vars */
			for(i = 0; i < nrpts; i++)
			{
				if (&rpt_vars[i] == myrpt)
				{
					load_rpt_vars(i,0);
					break;
				}
			}
		}

		/* check termination conditions */
		rpt_mutex_lock(&myrpt->lock);
		if (ast_check_hangup(myrpt->rxchannel)) break;
		if (ast_check_hangup(myrpt->txchannel)) break;
		if (ast_check_hangup(myrpt->pchannel)) break;
		if (ast_check_hangup(myrpt->monchannel)) break;
		if (myrpt->parrotchannel &&
			ast_check_hangup(myrpt->parrotchannel)) break;
		if (myrpt->voxchannel &&
			ast_check_hangup(myrpt->voxchannel)) break;
		if (ast_check_hangup(myrpt->txpchannel)) break;
		if (myrpt->zaptxchannel && ast_check_hangup(myrpt->zaptxchannel)) break;

		/* Set local tx with keyed */
		myrpt->localtx = myrpt->keyed;
		/* If someone's connected, and they're transmitting from their end to us, set myrpt->remrx true */
		l = myrpt->links.next;
		myrpt->remrx = 0;  			/* no remotes keyed */
		myrpt->voteremrx = 0;		/* no voter remotes keyed */
		while(l != &myrpt->links)
		{
			/* get code from a coded squelch master */
			if (l->lastrx)
			{
				if ( strlen(l->cstxcode) && !(l->cstxcode[0]==' ') && !strlen(myrpt->cstxnow) )
				{
					ast_copy_string(myrpt->cstxcode,l->cstxcode,sizeof(myrpt->cstxnow));
					// ast_log(LOG_NOTICE,"10 cstxcode=%s\n",myrpt->cstxcode);
				}

				if ( l->csmm && !strlen(myrpt->cstxnow) )
				{
					ast_copy_string(myrpt->cstxcode,l->csrxnow,sizeof(myrpt->cstxcode));
					// ast_log(LOG_NOTICE,"20 cstxcode=%s\n",myrpt->cstxcode);
				}
			}

			if (l->lastrx){
				myrpt->remrx = 1;
				if(l->voterlink)myrpt->voteremrx=1;
				/* Ignore '0' nodes which can be iaxrpt clients */
				if(l->name[0] != '0' && strcmp(myrpt->lastnodewhichkeyedusup,l->name) )
				{
					#if 0
				    ast_log(LOG_NOTICE,"[%s] lnwku_update\n",myrpt->name);
					myrpt->lnwku_update = 1;
					#endif
					strcpy(myrpt->lastnodewhichkeyedusup, l->name); /* Note the node which is doing the key up */
				}
			}
			l = l->next;
		}
		/* Create a "must_id" flag for the cleanup ID */
		if(myrpt->p.idtime) /* ID time must be non-zero */
			myrpt->mustid |= (myrpt->idtimer) && (myrpt->keyed || myrpt->remrx) ;
		/* Build a fresh totx from myrpt->keyed and autopatch activated */
		/* If full duplex, add local tx to totx */
		if (myrpt->p.duplex > 1)
		{
			totx = myrpt->callmode;
			totx = totx || myrpt->localtx;
		}
		else
		{
			int myrx = myrpt->localtx || myrpt->remrx || (!myrpt->callmode);

			if (lastmyrx != myrx)
			{
				voxinit_rpt(myrpt,!myrx);
				lastmyrx = myrx;
			}
			totx = 0;
			if (myrpt->callmode && (myrpt->voxtotimer <= 0))
			{
				if (myrpt->voxtostate)
				{
					myrpt->voxtotimer = myrpt->p.voxtimeout_ms;
					myrpt->voxtostate = 0;
				}
				else
				{
					myrpt->voxtotimer = myrpt->p.voxrecover_ms;
					myrpt->voxtostate = 1;
				}
			}
			if (!myrpt->voxtostate)
				totx = myrpt->callmode && myrpt->wasvox;
		}
		/* Traverse the telemetry list to see what's queued */
		identqueued = 0;
		localmsgqueued = 0;
		othertelemqueued = 0;
		tailmessagequeued = 0;
		ctqueued = 0;
		telem = myrpt->tele.next;
		while(telem != &myrpt->tele)
		{
			if((telem->mode == ID) || (telem->mode == IDTALKOVER)){
				identqueued = 1; /* Identification telemetry */
			}
			else if(telem->mode == TAILMSG)
			{
				tailmessagequeued = 1; /* Tail message telemetry */
			}
			else if((telem->mode == STATS_TIME_LOCAL) || (telem->mode == LOCALPLAY))
			{
				localmsgqueued = 1; /* Local message */
			}
			else
			{
				if ((telem->mode != UNKEY) && (telem->mode != LINKUNKEY))
					othertelemqueued = 1;  /* Other telemetry */
				else
					ctqueued = 1; /* Courtesy tone telemetry */
			}
			telem = telem->next;
		}

		/* Add in any "other" telemetry, unless specified otherwise */
		if (!myrpt->p.notelemtx) totx = totx || othertelemqueued;
		/* Update external (to links) transmitter PTT state with everything but */
		/* ID, CT, local messages, and tailmessage telemetry */
		myrpt->exttx = totx;
		if (myrpt->localoverride) totx = 1;
		totx = totx || myrpt->dtmf_local_timer;
		/* If half or 3/4 duplex, add localtx to external link tx */
		if (myrpt->p.duplex < 2) myrpt->exttx = myrpt->exttx || myrpt->localtx;
		/* Add in ID telemetry to local transmitter */
		totx = totx || myrpt->remrx;
		/* If 3/4 or full duplex, add in ident, CT telemetry, and local messages */
		if (myrpt->p.duplex > 0)
			totx = totx || identqueued || ctqueued;
		/* If 3/4 or full duplex, or half-duplex with link to link, add in localmsg ptt */
		if (myrpt->p.duplex > 0 || myrpt->p.linktolink)
			totx = totx || localmsgqueued;
		/* If full duplex, add local dtmf stuff active */
		if (myrpt->p.duplex > 1)
		{
			totx = totx || (myrpt->dtmfidx > -1) ||
				myrpt->cmdnode[0];
		}
		/* add in parrot stuff */
		totx = totx || (myrpt->parrotstate > 1);
		/* Reset time out timer variables if there is no activity */
		if (!totx)
		{
			myrpt->totimer = myrpt->p.totime;
			myrpt->tounkeyed = 0;
			myrpt->tonotify = 0;
		}
		else{
			myrpt->tailtimer = myrpt->p.s[myrpt->p.sysstate_cur].alternatetail ?
				myrpt->p.althangtime : /* Initialize tail timer */
				myrpt->p.hangtime;
		}
		/* if in 1/2 or 3/4 duplex, give rx priority */
		if ((myrpt->p.duplex < 2) && (myrpt->keyed) && (!myrpt->p.linktolink)) totx = 0;
		/* Disable the local transmitter if we are timed out */
		totx = totx && myrpt->totimer;
		/* if timed-out and not said already, say it */
		if ((!myrpt->totimer) && (!myrpt->tonotify))
		{
			myrpt->tonotify = 1;
			myrpt->timeouts++;
			rpt_mutex_unlock(&myrpt->lock);
			rpt_telemetry(myrpt,TIMEOUT,NULL);
			rpt_mutex_lock(&myrpt->lock);
		}

		/* If unkey and re-key, reset time out timer */
		if ((!totx) && (!myrpt->totimer) && (!myrpt->tounkeyed) && (!myrpt->keyed))
		{
			myrpt->tounkeyed = 1;
		}
		if ((!totx) && (!myrpt->totimer) && myrpt->tounkeyed && myrpt->keyed)
		{
			myrpt->totimer = myrpt->p.totime;
			myrpt->tounkeyed = 0;
			myrpt->tonotify = 0;
			rpt_mutex_unlock(&myrpt->lock);
			continue;
		}
		/* if timed-out and in circuit busy after call */
		if ((!totx) && (!myrpt->totimer) && (myrpt->callmode == 4))
		{
		    if(debug)
				ast_log(LOG_NOTICE, "timed-out and in circuit busy after call\n");
			myrpt->callmode = 0;
			myrpt->macropatch=0;
			channel_revert(myrpt);
		}
		/* get rid of tail if timed out */
		if (!myrpt->totimer) myrpt->tailtimer = 0;
		/* if not timed-out, add in tail */
		if (myrpt->totimer) totx = totx || myrpt->tailtimer;
		/* If user or links key up or are keyed up over standard ID, switch to talkover ID, if one is defined */
		/* If tail message, kill the message if someone keys up over it */
		if ((myrpt->keyed || myrpt->remrx || myrpt->localoverride) && ((identqueued && idtalkover) || (tailmessagequeued))) {
			int hasid = 0,hastalkover = 0;

			telem = myrpt->tele.next;
			while(telem != &myrpt->tele){
				if(telem->mode == ID && !telem->killed){
					if (telem->chan) ast_softhangup(telem->chan, AST_SOFTHANGUP_DEV); /* Whoosh! */
					telem->killed = 1;
					hasid = 1;
				}
				if(telem->mode == TAILMSG && !telem->killed){
                                        if (telem->chan) ast_softhangup(telem->chan, AST_SOFTHANGUP_DEV); /* Whoosh! */
					telem->killed = 1;
                                }
				if (telem->mode == IDTALKOVER) hastalkover = 1;
				telem = telem->next;
			}
			if(hasid && (!hastalkover)){
				if(debug >= 6)
					ast_log(LOG_NOTICE,"IDTALKOVER: node=%s\n",myrpt->name);
				rpt_mutex_unlock(&myrpt->lock);
				rpt_telemetry(myrpt, IDTALKOVER, NULL); /* Start Talkover ID */
				rpt_mutex_lock(&myrpt->lock);
			}
		}
		/* Try to be polite */
		/* If the repeater has been inactive for longer than the ID time, do an initial ID in the tail*/
		/* If within 30 seconds of the time to ID, try do it in the tail */
		/* else if at ID time limit, do it right over the top of them */
		/* If beaconing is enabled, always id when the timer expires */
		/* Lastly, if the repeater has been keyed, and the ID timer is expired, do a clean up ID */
		if(((myrpt->mustid)||(myrpt->p.beaconing)) && (!myrpt->idtimer))
			queue_id(myrpt);

		if ((myrpt->p.idtime && totx && (!myrpt->exttx) &&
			 (myrpt->idtimer <= myrpt->p.politeid) && myrpt->tailtimer)) /* ID time must be non-zero */
			{
				myrpt->tailid = 1;
			}

		/* If tail timer expires, then check for tail messages */

		if(myrpt->tailevent){
			myrpt->tailevent = 0;
			if(myrpt->tailid){
				totx = 1;
				queue_id(myrpt);
			}
			else if ((myrpt->p.tailmessages[0]) &&
				(myrpt->p.tailmessagetime) && (myrpt->tmsgtimer == 0)){
					totx = 1;
					myrpt->tmsgtimer = myrpt->p.tailmessagetime;
					rpt_mutex_unlock(&myrpt->lock);
					rpt_telemetry(myrpt, TAILMSG, NULL);
					rpt_mutex_lock(&myrpt->lock);
			}
		}

		/* Main TX control */

		/* let telemetry transmit anyway (regardless of timeout) */
		if (myrpt->p.duplex > 0) totx = totx || (myrpt->tele.next != &myrpt->tele);
		totx = totx && !myrpt->p.s[myrpt->p.sysstate_cur].txdisable;
		myrpt->txrealkeyed = totx;
		totx = totx || (!AST_LIST_EMPTY(&myrpt->txq));
		/* if in 1/2 or 3/4 duplex, give rx priority */
		if ((myrpt->p.duplex < 2) && (!myrpt->p.linktolink) && (myrpt->keyed)) totx = 0;
		/*
			if transmit state changed since the last time we serviced it,
			handle the change.
		*/
		if (totx && (!lasttx))
		{
			if(debug>=5)
			    	ast_log(LOG_NOTICE,
			    		"[%s] TKD totx=%d lasttx=%d tailevent=%d remrx=%d\n",
			    		myrpt->name,totx,lasttx,myrpt->tailevent,myrpt->remrx);

			get_handle_audiolog(myrpt,1);

			lasttx = 1;
			myrpt->txkeyed = 1;
			time(&myrpt->lasttxkeyedtime);
			myrpt->dailykeyups++;
			myrpt->totalkeyups++;
			rpt_mutex_unlock(&myrpt->lock);

			if (strncasecmp(myrpt->txchannel->name,"dahdi/pseudo",10))
			{
				/*	key the channel's local transmitter	*/
			    if(debug>=6)
			    	ast_log(LOG_NOTICE,
			    		"[%s] on txchannel %s indicate ACRK data=%s\n",
			    		myrpt->name,myrpt->txchannel->name,myrpt->cstxcode);

				NETMARK((2003));  	/* Network Timing Event */

				if(!ast_strlen_zero(myrpt->cstxcode))
					ast_indicate_data(myrpt->txchannel,AST_CONTROL_RADIO_KEY,myrpt->cstxcode,strlen(myrpt->cstxcode)+1);
				else
					ast_indicate(myrpt->txchannel,AST_CONTROL_RADIO_KEY);
			}
			else
			{
				/*	hasnoradio so handshake on the code immediately */
				if(debug>=6)
			    	ast_log(LOG_NOTICE,
			    		"[%s] txchannel/dahdi/pseudo has no radio set cstxnow=%s\n",
			    		myrpt->name,myrpt->cstxcode);

				ast_copy_string(myrpt->cstxnow,myrpt->cstxcode,sizeof(myrpt->cstxnow));
			}
			rpt_mutex_lock(&myrpt->lock);
		}
		if ((!totx) && lasttx)
		{
			if (myrpt->monstream){
				ast_closestream(myrpt->monstream);
				myrpt->monstream = 0;
			}

			lasttx = 0;
			myrpt->txkeyed = 0;
			time(&myrpt->lasttxkeyedtime);
			rpt_mutex_unlock(&myrpt->lock);
			if (strncasecmp(myrpt->txchannel->name,"dahdi/pseudo",10)){
				if(debug>6)
			    	ast_log(LOG_NOTICE,
			    		"[%s] txchannel/dahdi/pseudo ast_indicate ACRUK totx\n",myrpt->name);
				ast_indicate(myrpt->txchannel,AST_CONTROL_RADIO_UNKEY);
			}
			else
			{
				/*	hasnoradio so handshake and clear the code immediately */
				memset(myrpt->cstxnow,0,sizeof(myrpt->cstxnow));
			}
			rpt_mutex_lock(&myrpt->lock);
			donodelog(myrpt,"TXUNKEY,MAIN");
		}

		time(&t);
		/* if DTMF timeout */
		if ((!myrpt->cmdnode[0]) && (myrpt->dtmfidx >= 0) && ((myrpt->dtmf_time + DTMF_TIMEOUT) < t))
		{
			myrpt->inpadtest = 0;
			myrpt->dtmfidx = -1;
			myrpt->dtmfbuf[0] = 0;
		}
		/* if remote DTMF timeout */
		if ((myrpt->rem_dtmfidx >= 0) && ((myrpt->rem_dtmf_time + DTMF_TIMEOUT) < t))
		{
			myrpt->inpadtest = 0;
			myrpt->rem_dtmfidx = -1;
			myrpt->rem_dtmfbuf[0] = 0;
		}

		if (myrpt->exttx && myrpt->parrotchannel &&
			(myrpt->p.parrotmode || myrpt->parrotonce) && (!myrpt->parrotstate))
		{
			char myfname[300];

			ci.confno = myrpt->conf;
			ci.confmode = DAHDI_CONF_CONFANNMON;
			ci.chan = 0;

			/* first put the channel on the conference in announce mode */
			if (ioctl(myrpt->parrotchannel->fds[0],DAHDI_SETCONF,&ci) == -1)
			{
				ast_log(LOG_WARNING, "Unable to set conference mode for parrot\n");
				break;
			}

			sprintf(myfname,PARROTFILE,myrpt->name,myrpt->parrotcnt);
			strcat(myfname,".wav");
			unlink(myfname);
			sprintf(myfname,PARROTFILE,myrpt->name,myrpt->parrotcnt);
			myrpt->parrotstate = 1;
			myrpt->parrottimer = myrpt->p.parrottime;
			if (myrpt->parrotstream)
				ast_closestream(myrpt->parrotstream);
			myrpt->parrotstream = NULL;
			myrpt->parrotstream = ast_writefile(myfname,"wav",
				"app_rpt Parrot",O_CREAT | O_TRUNC,0,0660);
		}

		/* Traverse all links, flag update, Reconnect or Disconnect as necessary */

		l = myrpt->links.next;
		while(l != &myrpt->links)
		{
			if (l->killme)
			{
				/* remove from queue */
				remque((struct qelem *) l);
				if (!strcmp(myrpt->cmdnode,l->name))
					myrpt->cmdnode[0] = 0;
				rpt_mutex_unlock(&myrpt->lock);
				/* hang-up on call to device */
				if (l->chan) ast_hangup(l->chan);
				ast_hangup(l->pchan);
				ast_free(l);
				rpt_mutex_lock(&myrpt->lock);
				/* re-start link traversal */
				l = myrpt->links.next;
				continue;
			}
			#if 0
			if(myrpt->lnwku_update && l->iaxrpt )l->lnwku_update=1;
			#endif
			l = l->next;
		}
		#if 0
		myrpt->lnwku_update=0;
		#endif

		/* check state of last inhibit tx ctcss flag and take action on change. */
		x = myrpt->remrx || myrpt->localtx || myrpt->callmode || myrpt->parrotstate;
		if (x != myrpt->lastitx)
		{
			char str[16];
			myrpt->lastitx = x;
			if (myrpt->p.itxctcss)
			{
				if (strncasecmp(myrpt->rxchannel->name,"radio/", 6) == 0)
				{
					sprintf(str,"TXCTCSS %d",!(!x));
					ast_sendtext(myrpt->rxchannel,str);
				}
			}
		}

		n = 0;	/* number of ast channels for this rpt */
		cs[n++] = myrpt->rxchannel;
		cs[n++] = myrpt->pchannel;
		cs[n++] = myrpt->monchannel;
		cs[n++] = myrpt->telechannel;
		cs[n++] = myrpt->btelechannel;

		if (myrpt->parrotchannel) cs[n++] = myrpt->parrotchannel;
		if (myrpt->voxchannel) cs[n++] = myrpt->voxchannel;
		cs[n++] = myrpt->txpchannel;
		if (myrpt->txchannel != myrpt->rxchannel) cs[n++] = myrpt->txchannel;
		if (myrpt->zaptxchannel != myrpt->txchannel)
			cs[n++] = myrpt->zaptxchannel;
		l = myrpt->links.next;
		while(l != &myrpt->links)
		{
			if ((!l->killme) && (!l->disctime) && l->chan)
			{
				cs[n++] = l->chan;		/* Tx stream to Link from psuedo */
				cs[n++] = l->pchan;		/* Rx stream from Link to psuedo */
			}
			l = l->next;
		}
		if ((myrpt->topkeystate == 1) &&
		    ((t - myrpt->topkeytime) > TOPKEYWAIT))
		{
			myrpt->topkeystate = 2;
			qsort(myrpt->topkey,TOPKEYN,sizeof(struct rpt_topkey),
				topcompar);
		}
		rpt_mutex_unlock(&myrpt->lock);

		if (myrpt->topkeystate == 2)
		{
			rpt_telemetry(myrpt,TOPKEY,NULL);
			myrpt->topkeystate = 3;
		}
		ms = MSWAIT;
		for(x = 0; x < n; x++)
		{
			int s = -(-x - myrpt->scram - 1) % n;
			cs1[x] = cs[s];
		}
		myrpt->scram++;
		/*
			Wait for an event on the scrambled list of channels.
			Called with ms set with time time to wait for and returns with
			ms set to how long the wait was. MSWAIT is the time between frames.
			Therefore elap is
			which is the time
		*/
		who = ast_waitfor_n(cs1,n,&ms);
		if (who == NULL) ms = 0;
		elap = MSWAIT - ms;
		/*
			We now have input ready on channel 'who'
			First handle timing events on the links and write any necessary
			non-voice messages to them.
		*/
		rpt_mutex_lock(&myrpt->lock);
		l = myrpt->links.next;
		while(l != &myrpt->links)
		{
			int myrx,mymaxct;

			if (l->chan && l->thisconnected && (!AST_LIST_EMPTY(&l->textq)))
			{
				f = AST_LIST_REMOVE_HEAD(&l->textq,frame_list);
				ast_write(l->chan,f);
				ast_frfree(f);
			}

			if (l->rxlingertimer!=0)
			{
				l->rxlingertimer -= elap;
				if (l->rxlingertimer <= 0)
				{
					l->rxlingertimer = 0;
					if (debug > 5) ast_log(LOG_NOTICE,
							"[%s] link=%s/%s TIMEOUT rxlingertimer\n",
							myrpt->name,l->name,l->chan->name);
				}
			}

			x = l->newkeytimer;
			if (l->newkeytimer) l->newkeytimer -= elap;
			if (l->newkeytimer < 0) l->newkeytimer = 0;

			if ((x > 0) && (!l->newkeytimer))
			{
				if (l->thisconnected)
				{
					if (l->newkey == 2) l->newkey = 0;
				}
				else
				{
					l->newkeytimer = NEWKEYTIME;
				}
			}
			if ((l->linkmode > 1) && (l->linkmode < 0x7ffffffe))
			{
				l->linkmode -= elap;
				if (l->linkmode < 1) l->linkmode = 1;
			}
			/*
				if this link is of type newkey1 and keyed and the lingertimer
				has expired (meaning no voice packets have arrived lately) then
				clear the keying line.
			*/
			if ((l->newkey == 2) && !l->iaxkey && l->lastrealrx && (!l->rxlingertimer) )
			{
				l->lastrealrx = 0;
				l->rerxtimer = 0;
				if (l->lastrx1)
				{
					if (myrpt->p.archivedir)
					{
						char str[100];
							sprintf(str,"RXUNKEY,%s",l->name);
						donodelog(myrpt,str);
					}
					l->lastrx1 = 0;
					time(&l->lastunkeytime);
					if(myrpt->p.duplex)
						rpt_telemetry(myrpt,LINKUNKEY,l);
				}
				if (debug > 5) ast_log(LOG_NOTICE,
						"[%s] link=%s/%s rxlingertimer unkey \n",
							myrpt->name,l->name,l->chan->name);
			}

			if (l->voxtotimer) l->voxtotimer -= elap;
			if (l->voxtotimer < 0) l->voxtotimer = 0;

			if (l->lasttx != l->lasttx1)
			{
				voxinit_link(l,!l->lasttx);
				l->lasttx1 = l->lasttx;
			}
			myrx = l->lastrealrx;
			if ((l->phonemode) && (l->phonevox))
			{
				myrx = myrx || (!AST_LIST_EMPTY(&l->rxq));
				if (l->voxtotimer <= 0)
				{
					if (l->voxtostate)
					{
						l->voxtotimer = myrpt->p.voxtimeout_ms;
						l->voxtostate = 0;
					}
					else
					{
						l->voxtotimer = myrpt->p.voxrecover_ms;
						l->voxtostate = 1;
					}
				}
				if (!l->voxtostate)
					myrx = myrx || l->wasvox ;
			}
			l->lastrx = myrx;

			#if 0
			/* report channel rssi */
			if(l->iaxrpt)
			{
				l->rssi_report_timer-=20;

				if(l->rssi_report_timer<0)
				{
					l->rssi_report_timer=10000;
					if (debug > 5||1) ast_log(LOG_NOTICE,
								"[%s] TIMEOUT rssi_report_timer\n",
								myrpt->name );
				}
			}
			#endif

			/* Timer and Handler to send out list of connected nodes */
			if (l->linklisttimer)
			{
				l->linklisttimer -= elap;
				if (l->linklisttimer < 0) l->linklisttimer = 0;
			}

			if (!myrpt->p.nonodelist && (!l->linklisttimer) && (l->name[0] != '0' || l->iaxrpt) && (!l->isremote))
			{
				struct	ast_frame lf;

				memset(&lf,0,sizeof(lf));
				lf.frametype = AST_FRAME_TEXT;
				lf.subclass = 0;
				lf.offset = 0;
				lf.mallocd = 0;
				lf.samples = 0;
				l->linklisttimer = LINKLISTTIME;
				strcpy(lstr,"L ");

				if(!l->iaxrpt)
					__mklinklist(myrpt,l,lstr + 2);
				else
					__mklinklist2(myrpt,l,lstr + 2);

				if (l->chan)
				{
					lf.datalen = strlen(lstr) + 1;
					lf.data = lstr;
					rpt_qwrite(l,&lf);
					if (debug > 6) ast_log(LOG_NOTICE,
						"[%s] to node=%s sent node string %s \n",
							myrpt->name,l->name,lstr);
				}
			}

			if (l->newkey == 1)
			{
				if ( ((l->retxtimer += elap) >= REDUNDANT_TX_TIME) )
				{
					l->retxtimer = 0;
					if (l->chan && l->phonemode == 0)
					{
						if (l->lasttx)
						{
							if(debug>5)
								ast_log(LOG_NOTICE,
									"[%s] link=%s/%s ast_indicate ACRK  cstxnow=%s  remrx=%i\n",
									myrpt->name,l->name,l->chan->name,myrpt->cstxnow[0]?myrpt->cstxnow:"",myrpt->remrx);

						    if(isalnum(myrpt->cstxnow[0]))
								ast_indicate_data(l->chan,AST_CONTROL_RADIO_KEY,myrpt->cstxnow,strlen(myrpt->cstxnow)+1);
							else
								ast_indicate(l->chan,AST_CONTROL_RADIO_KEY);
						}
						else
						{
							if(debug>5)
								ast_log(LOG_NOTICE,
									"[%s] link=%s/%s ast_indicate ACRUK\n",
									myrpt->name,l->name,l->chan->name);
							ast_indicate(l->chan,AST_CONTROL_RADIO_UNKEY);
						}
					}
				}
			}

			if (l->newkey == 1 )
			{
				/*
					if no tx control messages received after 5 time periods,
					assume the link is lost.
				*/
				if ((l->rerxtimer += elap) >= (REDUNDANT_TX_TIME * 5))
				{
					if (debug > 5)
						ast_log(LOG_NOTICE,"[%s] link=%s ACRK Timeout LOS\n",
							myrpt->name,l->name);
					l->lastrealrx = 0;
					l->rerxtimer = 0;
					if (l->lastrx1)
					{
						if (myrpt->p.archivedir)
						{
							char str[100];
							sprintf(str,"RXUNKEY(T),%s",l->name);
							donodelog(myrpt,str);
						}
						if(myrpt->p.duplex)
							rpt_telemetry(myrpt,LINKUNKEY,l);
						l->lastrx1 = 0;
						l->csrxnow[0] = l->csm = 0;
					}
				}
			}

			if (l->disctime) /* Disconnect timer active on a channel ? */
			{
				l->disctime -= elap;
				if (l->disctime <= 0) /* Disconnect timer expired on inbound channel ? */
					l->disctime = 0; /* Yep */
			}

			if (l->retrytimer)
			{
				l->retrytimer -= elap;
				if (l->retrytimer < 0) l->retrytimer = 0;
			}

			/* Tally connect time */
			l->connecttime += elap;

			/* ignore non-timing channels */
			if (l->elaptime < 0)
			{
				l = l->next;
				continue;
			}
			l->elaptime += elap;
			mymaxct = MAXCONNECTTIME;
			if (l->chan && (!strncasecmp(l->chan->name,"irlp",4)))
				mymaxct = MAXCONNECTTIME_IRLP;
			/* if IRLP connection attempt has taken too long */
			if ((l->elaptime > mymaxct) &&
			   ((!l->chan) || (l->chan->_state != AST_STATE_UP)))
			{
				l->elaptime = 0;
				rpt_mutex_unlock(&myrpt->lock);
				if (l->chan) ast_softhangup(l->chan,AST_SOFTHANGUP_DEV);
				rpt_mutex_lock(&myrpt->lock);
				break;
			}
			if ((!l->chan) && (!l->retrytimer) && l->outbound &&
				(l->retries++ < l->max_retries) && (l->hasconnected))
			{
				if (l->chan) ast_hangup(l->chan);
				l->chan = 0;
				rpt_mutex_unlock(&myrpt->lock);
				if ((l->name[0] != '0') && (!l->isremote))
				{
					if (attempt_reconnect(myrpt,l) == -1)
					{
						l->retrytimer = RETRY_TIMER_MS;
					}
				}
				else
				{
					l->retries = l->max_retries + 1;
				}

				rpt_mutex_lock(&myrpt->lock);
				break;
			}
			if ((!l->chan) && (!l->retrytimer) && l->outbound &&
				(l->retries >= l->max_retries))
			{
				/* remove from queue */
				remque((struct qelem *) l);
				if (!strcmp(myrpt->cmdnode,l->name))
					myrpt->cmdnode[0] = 0;
				rpt_mutex_unlock(&myrpt->lock);
				if (l->name[0] != '0')
				{
					if (!l->hasconnected)
						rpt_telemetry(myrpt,CONNFAIL,l);
					else rpt_telemetry(myrpt,REMDISC,l);
				}
				if (myrpt->p.archivedir)
				{
					char str[100];

					if (!l->hasconnected)
						sprintf(str,"LINKFAIL,%s",l->name);
					else
						sprintf(str,"LINKDISC,%s",l->name);
					donodelog(myrpt,str);
				}
				/* hang-up on call to device */
				ast_hangup(l->pchan);
				ast_free(l);
                rpt_mutex_lock(&myrpt->lock);
				break;
			}
            if ((!l->chan) && (!l->disctime) && (!l->outbound))
            {
				if(debug) ast_log(LOG_NOTICE, "[%s] LINKDISC AA \n",myrpt->name);
				/* remove from queue */
				remque((struct qelem *) l);
				if (myrpt->links.next==&myrpt->links) channel_revert(myrpt);
				if (!strcmp(myrpt->cmdnode,l->name)) myrpt->cmdnode[0] = 0;
				rpt_mutex_unlock(&myrpt->lock);
				if (l->name[0] != '0')
				{
					rpt_telemetry(myrpt,REMDISC,l);
				}
				if (myrpt->p.archivedir)
				{
					char str[100];
					sprintf(str,"LINKDISC,%s",l->name);
					donodelog(myrpt,str);
				}
				dodispgm(myrpt,l->name);
                /* hang-up on call to device */
                ast_hangup(l->pchan);
                ast_free(l);
                rpt_mutex_lock(&myrpt->lock);
                break;
            }
			l = l->next;
		}
		/*	all links time and event control handler	*/

		if (myrpt->linkposttimer)
		{
			myrpt->linkposttimer -= elap;
			if (myrpt->linkposttimer < 0) myrpt->linkposttimer = 0;
		}
		if (myrpt->linkposttimer <= 0)
		{
			int nstr;
			char lst,*str;
			time_t now;

			myrpt->linkposttimer = LINKPOSTTIME;
			nstr = 0;
			for(l = myrpt->links.next; l != &myrpt->links; l = l->next)
			{
				/* if is not a real link, ignore it */
				if (l->name[0] == '0') continue;
				nstr += strlen(l->name) + 1;
			}
			str = ast_malloc(nstr + 256);
			if (!str)
			{
				ast_log(LOG_ERROR,"Cannot ast_malloc()\n");
				break;
			}
			nstr = 0;
			strcpy(str,"nodes=");
			for(l = myrpt->links.next; l != &myrpt->links; l = l->next)
			{
				/* if is not a real link, ignore it */
				if (l->name[0] == '0') continue;
				lst = 'T';
				if (!l->mode) lst = 'R';
				if (!l->thisconnected) lst = 'C';
				if (nstr) strcat(str,",");
				sprintf(str + strlen(str),"%c%s",lst,l->name);
				nstr = 1;
			}
			p = strstr(tdesc, "version");
			if(p){
				int vmajor,vminor;
				if(sscanf(p, "version %d.%d", &vmajor, &vminor) == 2)
				sprintf(str + strlen(str),"&apprptvers=%d.%d",vmajor,vminor);
			}
			time(&now);
			sprintf(str + strlen(str),"&apprptuptime=%d",(int)(now-starttime));
			sprintf(str + strlen(str),
			"&totalkerchunks=%d&totalkeyups=%d&totaltxtime=%d&timeouts=%d&totalexecdcommands=%d",
			myrpt->totalkerchunks,myrpt->totalkeyups,(int) myrpt->totaltxtime/1000,
			myrpt->timeouts,myrpt->totalexecdcommands);
			rpt_mutex_unlock(&myrpt->lock);
			statpost(myrpt,str);
			rpt_mutex_lock(&myrpt->lock);
			ast_free(str);
		}
		if (myrpt->keyposttimer)
		{
			myrpt->keyposttimer -= elap;
			if (myrpt->keyposttimer < 0) myrpt->keyposttimer = 0;
		}
		if (myrpt->keyposttimer <= 0)
		{
			char str[100];
			int n = 0;
			time_t now;

			myrpt->keyposttimer = KEYPOSTTIME;
			time(&now);
			if (myrpt->lastkeyedtime)
			{
				n = (int)(now - myrpt->lastkeyedtime);
			}
			sprintf(str,"keyed=%d&keytime=%d",myrpt->keyed,n);
			rpt_mutex_unlock(&myrpt->lock);
			statpost(myrpt,str);
			rpt_mutex_lock(&myrpt->lock);
		}
		if(totx){
			myrpt->dailytxtime += elap;
			myrpt->totaltxtime += elap;
		}
		i = myrpt->tailtimer;
		if (myrpt->tailtimer) myrpt->tailtimer -= elap;
		if (myrpt->tailtimer < 0) myrpt->tailtimer = 0;
		if((i) && (myrpt->tailtimer == 0))
			myrpt->tailevent = 1;
		if ((!myrpt->p.s[myrpt->p.sysstate_cur].totdisable) && myrpt->totimer) myrpt->totimer -= elap;
		if (myrpt->totimer < 0) myrpt->totimer = 0;
		if (myrpt->idtimer) myrpt->idtimer -= elap;
		if (myrpt->idtimer < 0) myrpt->idtimer = 0;
		if (myrpt->tmsgtimer) myrpt->tmsgtimer -= elap;
		if (myrpt->tmsgtimer < 0) myrpt->tmsgtimer = 0;
		if (myrpt->voxtotimer) myrpt->voxtotimer -= elap;
		if (myrpt->voxtotimer < 0) myrpt->voxtotimer = 0;
		if ((myrpt->telemmode != 0x7fffffff) && (myrpt->telemmode > 1))
		{
			myrpt->telemmode -= elap;
			if (myrpt->telemmode < 1) myrpt->telemmode = 1;
		}
		if (myrpt->exttx)
		{
			myrpt->parrottimer = myrpt->p.parrottime;
		}
		else
		{
			if (myrpt->parrottimer) myrpt->parrottimer -= elap;
			if (myrpt->parrottimer < 0) myrpt->parrottimer = 0;
		}
		/* do macro timers */
		if (myrpt->macrotimer) myrpt->macrotimer -= elap;
		if (myrpt->macrotimer < 0) myrpt->macrotimer = 0;
		/* do local dtmf timer */
		if (myrpt->dtmf_local_timer)
		{
			if (myrpt->dtmf_local_timer > 1) myrpt->dtmf_local_timer -= elap;
			if (myrpt->dtmf_local_timer < 1) myrpt->dtmf_local_timer = 1;
		}
		do_dtmf_local(myrpt,0);
		/* Execute scheduler appx. every 2 tenths of a second */
		if (myrpt->skedtimer <= 0){
			myrpt->skedtimer = 200;
			do_scheduler(myrpt);
		}
		else
			myrpt->skedtimer -=elap;
		if (!ms)
		{
			rpt_mutex_unlock(&myrpt->lock);
			continue;
		}
		if ((myrpt->p.parrotmode || myrpt->parrotonce) && (myrpt->parrotstate == 1) &&
			(myrpt->parrottimer <= 0))
		{
			ci.confno = 0;
			ci.confmode = 0;
			ci.chan = 0;

			/* first put the channel on the conference in announce mode */
			if (ioctl(myrpt->parrotchannel->fds[0],DAHDI_SETCONF,&ci) == -1)
			{
				ast_log(LOG_ERROR, "Unable to set conference mode for parrot\n");
				break;
			}
			if (myrpt->parrotstream)
				ast_closestream(myrpt->parrotstream);
			myrpt->parrotstream = NULL;
			myrpt->parrotstate = 2;
			rpt_telemetry(myrpt,PARROT,(struct rpt_link *) myrpt->parrotcnt++);
		}
		if (myrpt->cmdAction.state == CMD_STATE_READY)
		{ /* there is a command waiting to be processed */
			int status;
			myrpt->cmdAction.state = CMD_STATE_EXECUTING;
			// lose the lock
			rpt_mutex_unlock(&myrpt->lock);
			// do the function
			status = (*function_table[myrpt->cmdAction.functionNumber].function)(myrpt,myrpt->cmdAction.param, myrpt->cmdAction.digits, myrpt->cmdAction.command_source, NULL);
			// get the lock again
			rpt_mutex_lock(&myrpt->lock);
			myrpt->cmdAction.state = CMD_STATE_IDLE;
		} /* if myrpt->cmdAction.state == CMD_STATE_READY */

		c = myrpt->macrobuf[0];
		time(&t);
		if (c && (!myrpt->macrotimer) &&
			starttime && (t > (starttime + START_DELAY)))
		{
			char cin = c & 0x7f;

			memmove(myrpt->macrobuf,myrpt->macrobuf + 1,MAXMACRO - 1);

			if ((cin == 'p') || (cin == 'P'))
				myrpt->macrotimer = MACROPTIME;
			else if(myrpt->innox)
				myrpt->macrotimer = MACRONTIME;
			else
			 	myrpt->macrotimer = MACROTIME;

			rpt_mutex_unlock(&myrpt->lock);
			if (myrpt->p.archivedir)
			{
				char str[100];

				sprintf(str,"DTMF(M),MAIN,%c",cin);
				donodelog(myrpt,str);
			}
			local_dtmf_helper(myrpt,c);
		}
		else rpt_mutex_unlock(&myrpt->lock);


		if (who == myrpt->rxchannel)
		{
			/* if it was a read from local radio device rxchannel */
			int ismuted;

			f = ast_read(myrpt->rxchannel);
			if (!f)
			{
				if (debug) ast_log(LOG_NOTICE,"node=%s rxchannel Hang Up implied\n",myrpt->name);
				break;
			}

			if(f->frametype == AST_FRAME_TEXT && myrpt->rxchankeyed)
			{
				char myrxrssi[32];

				if(sscanf((char *)f->data, "R %s", myrxrssi) == 1)
				{
					myrpt->rxrssi=atoi(myrxrssi);
					if(debug>7)ast_log(LOG_NOTICE,"[%s] rxchannel rssi=%i\n",
						myrpt->name,myrpt->rxrssi);
					if(myrpt->p.votertype==2)
						rssi_send(myrpt);
				}
			}

			/* if out voted drop DTMF frames */
			if(  myrpt->p.votermode &&
			    !myrpt->votewinner &&

			    ( f->frametype == AST_FRAME_DTMF_BEGIN ||
                  f->frametype == AST_FRAME_DTMF_END
                )
			  )
			{
				rpt_mutex_unlock(&myrpt->lock);
				ast_frfree(f);
				continue;
			}

			if (f->frametype == AST_FRAME_VOICE)
			{
			    if (myrpt->monstream && !myrpt->txkeyed)
			    	ast_writestream(myrpt->monstream,f);
#ifdef	_MDC_DECODE_H_
				unsigned char ubuf[2560];
				short *sp;
				int n;
#endif
				/*	tone burst decoder and logic */
				if (myrpt->p.rxburstfreq)
				{
					if ((!myrpt->reallykeyed) || myrpt->keyed)
					{
						myrpt->lastrxburst = 0;
						goertzel_reset(&myrpt->burst_tone_state.tone);
						myrpt->burst_tone_state.last_hit = 0;
						myrpt->burst_tone_state.hit_count = 0;
						myrpt->burst_tone_state.energy = 0.0;
					}
					else
					{
						i = tone_detect(&myrpt->burst_tone_state,f->data,f->samples);
						ast_log(LOG_DEBUG,"Node %s got %d Hz Rx Burst\n",
							myrpt->name,myrpt->p.rxburstfreq);
						if ((!i) && myrpt->lastrxburst)
						{
							ast_log(LOG_DEBUG,"Node %s now keyed after Rx Burst\n",myrpt->name);
							myrpt->linkactivitytimer = 0;
							myrpt->keyed = 1;
							time(&myrpt->lastkeyedtime);
							myrpt->keyposttimer = KEYPOSTSHORTTIME;
						}
						myrpt->lastrxburst = i;
					}
				}
				if ((!myrpt->localtx) && /* (!myrpt->p.linktolink) && */
				    (!myrpt->localoverride))
				{
					memset(f->data,0,f->datalen);
				}
#ifdef	_MDC_DECODE_H_
				sp = (short *) f->data;
				/* convert block to unsigned char */
				for(n = 0; n < f->datalen / 2; n++)
				{
					ubuf[n] = (*sp++ >> 8) + 128;
				}
				n = mdc_decoder_process_samples(myrpt->mdc,ubuf,f->datalen / 2);
				if (n == 1)
				{
						unsigned char op,arg;
						unsigned short unitID;

						mdc_decoder_get_packet(myrpt->mdc,&op,&arg,&unitID);
						if (debug > 2)
						{
							ast_log(LOG_NOTICE,"Got (single-length) packet:\n");
							ast_log(LOG_NOTICE,"op: %02x, arg: %02x, UnitID: %04x\n",
								op & 255,arg & 255,unitID);
						}
						if ((op == 1) && (arg == 0))
						{
							myrpt->lastunit = unitID;
							mdc1200_notify(myrpt,NULL,myrpt->lastunit);
							mdc1200_send(myrpt,myrpt->lastunit);
						}
				}
				if ((debug > 2) && (i == 2))
				{
					unsigned char op,arg,ex1,ex2,ex3,ex4;
					unsigned short unitID;

					mdc_decoder_get_double_packet(myrpt->mdc,&op,&arg,&unitID,
						&ex1,&ex2,&ex3,&ex4);
					ast_log(LOG_NOTICE,"Got (double-length) packet:\n");
					ast_log(LOG_NOTICE,"op: %02x, arg: %02x, UnitID: %04x\n",
						op & 255,arg & 255,unitID);
					ast_log(LOG_NOTICE,"ex1: %02x, ex2: %02x, ex3: %02x, ex4: %02x\n",
						ex1 & 255, ex2 & 255, ex3 & 255, ex4 & 255);
				}
#endif
#ifdef	__RPT_NOTCH
				/* apply inbound filters, if any */
				rpt_filter(myrpt,f->data,f->datalen / 2);
#endif
				if (ioctl(myrpt->zaprxchannel->fds[0], DAHDI_GETCONFMUTE, &ismuted) == -1)
				{
					ismuted = 0;
				}
				if (dtmfed) ismuted = 1;
				dtmfed = 0;

				if(myrpt->p.votertype==1)
				{
                    if(!myrpt->rxchankeyed)
                        myrpt->votewinner=0;

                    if(!myrpt->voteremrx)
                        myrpt->voted_link=NULL;

                    if(!myrpt->rxchankeyed&&!myrpt->voteremrx)
                    {
					    myrpt->voter_oneshot=0;
					    myrpt->voted_rssi=0;
					}
				}

				if( myrpt->p.votertype==1 && myrpt->vote_counter && (myrpt->rxchankeyed||myrpt->voteremrx)
				    && (myrpt->p.votermode==2||(myrpt->p.votermode==1&&!myrpt->voter_oneshot))
				  )
				{
					if(--myrpt->vote_counter<=0)
					{
						myrpt->vote_counter=10;
						if(debug>6)ast_log(LOG_NOTICE,"[%s] vote rxrssi=%i\n",
							myrpt->name,myrpt->rxrssi);
						FindBestRssi(myrpt);
						myrpt->voter_oneshot=1;
					}
				}
				/* if a voting rx and not the winner, mute audio */
				if(myrpt->p.votertype==1 && myrpt->voted_link!=NULL)
				{
					ismuted=1;
				}

    			if (ismuted)
				{
					memset(f->data,0,f->datalen);
					if (myrpt->lastf1)
						memset(myrpt->lastf1->data,0,myrpt->lastf1->datalen);
					if (myrpt->lastf2)
						memset(myrpt->lastf2->data,0,myrpt->lastf2->datalen);
				}
				if (f) f2 = ast_frdup(f);
				else f2 = NULL;
				f1 = myrpt->lastf2;
				myrpt->lastf2 = myrpt->lastf1;
				myrpt->lastf1 = f2;
				if (ismuted)
				{
					if (myrpt->lastf1)
						memset(myrpt->lastf1->data,0,myrpt->lastf1->datalen);
					if (myrpt->lastf2)
						memset(myrpt->lastf2->data,0,myrpt->lastf2->datalen);
				}
				if (f1)
				{
	  				if (myrpt->localoverride)
						ast_write(myrpt->txpchannel,f1);
					else
						ast_write(myrpt->pchannel,f1);
	   				ast_frfree(f1);
				}
			}
#ifndef	OLD_ASTERISK
			else if (f->frametype == AST_FRAME_DTMF_BEGIN)
			{
				if (myrpt->lastf1)
					memset(myrpt->lastf1->data,0,myrpt->lastf1->datalen);
				if (myrpt->lastf2)
					memset(myrpt->lastf2->data,0,myrpt->lastf2->datalen);
				dtmfed = 1;
			}
#endif
			else if (f->frametype == AST_FRAME_DTMF)
			{
				c = (char) f->subclass; /* get DTMF char */
				ast_frfree(f);
				if (myrpt->lastf1)
					memset(myrpt->lastf1->data,0,myrpt->lastf1->datalen);
				if (myrpt->lastf2)
					memset(myrpt->lastf2->data,0,myrpt->lastf2->datalen);
				dtmfed = 1;
				if ((!myrpt->keyed) && (!myrpt->localoverride)) continue;
				c = func_xlat(myrpt,c,&myrpt->p.inxlat);
				if (c) local_dtmf_helper(myrpt,c);
				continue;
			}
			else if (f->frametype == AST_FRAME_TEXT)
			{
				char myrxctcssfreq[64];
				char myrxcodelock[64];

				if(debug)
					ast_log(LOG_NOTICE,"[%s] rxchannel text: %s\n",
						myrpt->name,f->datalen?(char *)f->data:"");

				if(sscanf((char *)f->data, "rxctcssfreq=%s rxcodelock=%s", myrxctcssfreq, myrxcodelock) == 2)
				{
					if(debug)ast_log(LOG_NOTICE,"[%s] read rxchannel got myrxctcssfreq=%s myrxcodelock=%s\n",myrpt->name,myrxctcssfreq,myrxcodelock);

					if(myrxcodelock[0] && strcmp(myrxctcssfreq,myrxcodelock))
					{
						int busy=0;
						char *val;

						val = (char *) ast_variable_retrieve(myrpt->cfg, myrpt->p.tonemacro, myrxctcssfreq);
						if (val)
						{
							if(debug)ast_log(LOG_NOTICE,"[%s] RxCode %s doing macro %s\n",myrpt->name,myrxctcssfreq,val);
							rpt_mutex_lock(&myrpt->lock);
							if ((MAXMACRO - strlen(myrpt->macrobuf)) < strlen(val)){
								rpt_mutex_unlock(&myrpt->lock);
								busy=1;
							}
							if(!busy){
								myrpt->macrotimer = MACROTIME;
								strncat(myrpt->macrobuf,val,MAXMACRO - 1);
								strcpy(myrpt->lasttone,myrxctcssfreq);
							}
							rpt_mutex_unlock(&myrpt->lock);
						}
					}
				}

			    if(f->datalen>4 && !strncasecmp(f->data,"cstx=",5))
				{
			    	ast_copy_string(myrpt->cstxnow,f->datalen?(char *)f->data+5:"",sizeof(myrpt->cstxnow));
					if(debug>1)
						ast_log(LOG_NOTICE,"[%s] read rxchannel set cstxnow=%s\n",
							myrpt->name,myrpt->cstxnow);

					if( strlen(myrpt->cstxcode) &&
					    strcmp(myrpt->cstxcode,myrpt->cstxnow) &&
					    myrpt->txkeyed
					  )
						if(debug>1)
							ast_log(LOG_WARNING,"[%s] txcode failure\n",
								myrpt->name);
				}
			}
			else if (f->frametype == AST_FRAME_CONTROL)
			{
				if(debug>1)
					ast_log(LOG_NOTICE,"[%s] read rxchannel got AFC=%i data=%s\n",
						myrpt->name,f->subclass,f->datalen?(char *)f->data:"");

				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if (debug) ast_log(LOG_NOTICE,"[%s] Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
				/* if RX key */
				if (f->subclass == AST_CONTROL_RADIO_KEY)
				{
					myrpt->rxchankeyed = 1;
					if ((!lasttx) || (myrpt->p.duplex > 1) || (myrpt->p.linktolink))
					{
						if (debug >= 7)
							ast_log(LOG_NOTICE,"[%s] rxchannel got ACRK data=%s\n",
								myrpt->name,f->datalen?(char *)f->data:"");
						myrpt->reallykeyed = 1;
						if (!myrpt->p.rxburstfreq)
						{
							myrpt->linkactivitytimer = 0;
							myrpt->keyed = 1;
							time(&myrpt->lastkeyedtime);
							myrpt->keyposttimer = KEYPOSTSHORTTIME;
						}
					}

					if (myrpt->p.archivedir)
					{
						donodelog(myrpt,"RXKEY,MAIN");
					    if(!myrpt->p.duplex)
							get_handle_audiolog(myrpt,0);
					}

					myrpt->localoverride = 0; // from local list flag

					if (myrpt->p.ncomnum!=0 && !myrpt->txkeyed )
					{
						static char sphunk=0;
						char busy = 0;
					    char val[16];

						NETMARK((1003));

					    if(debug)
					    	ast_log(LOG_NOTICE,"NCOM ARCK  %s  %i  %i\n",
					    	myrpt->p.ncomnum,myrpt->txkeyed,myrpt->macropatch);

						memset(val,0,16);
						strcat(val,"*6");
						strcat(val,myrpt->p.ncomnum);
						strcat(val,"#");

						if(sphunk==0)
						{
							sphunk=1;
							busy=1;
						}

						rpt_mutex_lock(&myrpt->lock);
						if ((MAXMACRO - strlen(myrpt->macrobuf)) < strlen(val)){
							rpt_mutex_unlock(&myrpt->lock);
							busy=1;
						}
						if(!busy){
							myrpt->macrotimer = MACRONTIME;
							strncat(myrpt->macrobuf,val,MAXMACRO - 1);
							myrpt->innox=1;
						}
						rpt_mutex_unlock(&myrpt->lock);
					}

					memset(myrpt->csrxnow,0,sizeof(myrpt->csrxnow));	// clear last received squelch code
					if (f->datalen && f->data)
					{
						char *val, busy = 0;
						strncpy(myrpt->csrxnow, f->data,f->datalen);
						if (myrpt->p.nlocallist)
						{
							for(x = 0; x < myrpt->p.nlocallist; x++)
							{
								if (!strcasecmp(f->data,myrpt->p.locallist[x]))
								{
									myrpt->localoverride = 1;
									myrpt->keyed = 0;
									break;
								}
							}
						}
						if (debug) ast_log(LOG_NOTICE,"[%s] rxchannel process ACRK data=%s\n",myrpt->name,(char *)f->data);
						// coded squelch autopatch initiate
						if (strstr((char *)f->data,"/M/")&& !myrpt->macropatch)
						{
						    char val[16];
							strcat(val,"*6");
							myrpt->macropatch=1;
							rpt_mutex_lock(&myrpt->lock);
							if ((MAXMACRO - strlen(myrpt->macrobuf)) < strlen(val)){
								rpt_mutex_unlock(&myrpt->lock);
								busy=1;
							}
							if(!busy){
								myrpt->macrotimer = MACROTIME;
								strncat(myrpt->macrobuf,val,MAXMACRO - 1);
								if (!busy) strcpy(myrpt->lasttone,(char*)f->data);
							}
							rpt_mutex_unlock(&myrpt->lock);
						}
						else if (strcmp((char *)f->data,myrpt->lasttone))
						{
							// do tonemacro on string
							val = (char *) ast_variable_retrieve(myrpt->cfg, myrpt->p.tonemacro, (char *)f->data);
							if (val)
							{
								if (debug) ast_log(LOG_NOTICE,"[%s] RxCode %s doing macro %s\n",myrpt->name,(char *) f->data,val);
								rpt_mutex_lock(&myrpt->lock);
								if ((MAXMACRO - strlen(myrpt->macrobuf)) < strlen(val)){
									rpt_mutex_unlock(&myrpt->lock);
									busy=1;
								}
								if(!busy){
									myrpt->macrotimer = MACROTIME;
									strncat(myrpt->macrobuf,val,MAXMACRO - 1);
								}
								rpt_mutex_unlock(&myrpt->lock);
							}
						 	if (!busy) strcpy(myrpt->lasttone,(char*)f->data);
						}
					} else myrpt->lasttone[0] = 0;
				}
				/* if RX un-key */
				if (f->subclass == AST_CONTROL_RADIO_UNKEY)
				{
					myrpt->rxchankeyed = 0;
				    if (debug == 7)
				    	ast_log(LOG_NOTICE,"[%s] rxchannel ACRUK data=%s\n",
							myrpt->name,f->datalen?(char *)f->data:"");

					if(f->datalen)
					{
						char *pc=(char *)f->data;
						if(pc[0]=='F')
						{
							myrpt->radiofail=1;
							if (debug)
								ast_log(LOG_NOTICE,"[%s] radio fail\n",myrpt->name);
						}
					}

					/* clear rx channel rssi	 */
					myrpt->rxrssi=0;

				    /* clear last received squelch code	 */
				    memset(myrpt->csrxnow,0,sizeof(myrpt->csrxnow));
					if ((!lasttx) || (myrpt->p.duplex > 1) || (myrpt->p.linktolink))
					{
						if(myrpt->p.duplex && myrpt->keyed) {
							rpt_telemetry(myrpt,UNKEY,NULL);
						}
					}
					myrpt->reallykeyed = 0;
					myrpt->keyed = 0;
					if ((myrpt->p.duplex > 1) && myrpt->localoverride)
					{
						rpt_telemetry(myrpt,LOCUNKEY,NULL);
					}
					myrpt->localoverride = 0;
					time(&myrpt->lastkeyedtime);
					myrpt->keyposttimer = KEYPOSTSHORTTIME;
					if (myrpt->p.archivedir)
					{
						donodelog(myrpt,"RXUNKEY,MAIN");
						if (myrpt->monstream && !myrpt->p.duplex)
						{
							ast_closestream(myrpt->monstream);
							myrpt->monstream = 0;
						}
					}

					if(myrpt->innox)
					{
						char val[16];
						char busy = 0;

						if(debug)
							ast_log(LOG_NOTICE,"NCOM ACRUK  %s  %i  %i\n",
							myrpt->p.ncomnum,myrpt->txkeyed,myrpt->macropatch);

						memset(val,0,16);
						strcat(val,"*0");

						rpt_mutex_lock(&myrpt->lock);
						if ((MAXMACRO - strlen(myrpt->macrobuf)) < strlen(val)){
							rpt_mutex_unlock(&myrpt->lock);
							busy=1;
						}
						if(!busy){
							myrpt->macrotimer = MACRONTIME;
							strncat(myrpt->macrobuf,val,MAXMACRO - 1);
						}
						rpt_mutex_unlock(&myrpt->lock);
				    }
				}
			}
			ast_frfree(f);
			continue;
		}
		if (who == myrpt->pchannel) /* if it was a read from the pseudo channel */
		{
			f = ast_read(myrpt->pchannel);
			if (!f)
			{
 			    if(debug)
			    	ast_log(LOG_NOTICE, "node=%s pchannel Hang Up Implied\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_VOICE)
			{
				if (!myrpt->localoverride)
					ast_write(myrpt->txpchannel,f);
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
				if (debug)
					ast_log(LOG_NOTICE, "node=%s pchannel got AFC %i \n",myrpt->name,f->subclass);

				if ( f->subclass == AST_CONTROL_HANGUP )
				{
					if (debug)
						ast_log(LOG_NOTICE, "node=%s pchannel got AST_CONTROL_HANGUP\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}
		if (who == myrpt->txchannel)
		{
			/* if it was a read from the tx conf which we usually just write to */
			f = ast_read(myrpt->txchannel);
			if (!f)
			{
				if (debug>4)
					ast_log(LOG_NOTICE, "node=%s txchannel implied Hang Up\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
				if ( debug>4 )
					ast_log(LOG_NOTICE, "[%s] txchannel got AST_CONTROL %i \n",myrpt->name,f->subclass);

				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug>4 )
						ast_log(LOG_NOTICE, "[%s] txchannel got AST_CONTROL_HANGUP\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}
		if (who == myrpt->zaptxchannel) /* if it was a read from a zaptel telephone channel */
		{
			f = ast_read(myrpt->zaptxchannel);
			if (!f)
			{
				if (debug) ast_log(LOG_NOTICE,"node=%s Hung Up zaptxchannel implied.\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_VOICE)
			{
				struct ast_frame *f1;

				if (myrpt->p.duplex < 2)
				{
					if (myrpt->txrealkeyed)
					{
						if ((!myfirst) && myrpt->callmode)
						{
						    x = 0;
						    AST_LIST_TRAVERSE(&myrpt->txq, f1,
							frame_list) x++;
						    for(;x < myrpt->p.simplexpatchdelay; x++)
						    {
								f1 = ast_frdup(f);
								memset(f1->data,0,f1->datalen);
								memset(&f1->frame_list,0,sizeof(f1->frame_list));
								AST_LIST_INSERT_TAIL(&myrpt->txq,f1,frame_list);
						    }
						    myfirst = 1;
						}
						f1 = ast_frdup(f);
						memset(&f1->frame_list,0,sizeof(f1->frame_list));
						AST_LIST_INSERT_TAIL(&myrpt->txq,
							f1,frame_list);
					} else myfirst = 0;
					x = 0;
					AST_LIST_TRAVERSE(&myrpt->txq, f1,
						frame_list) x++;
					if (!x)
					{
						memset(f->data,0,f->datalen);
					}
					else
					{
						ast_frfree(f);
						f = AST_LIST_REMOVE_HEAD(&myrpt->txq,
							frame_list);
					}
				}
				else
				{
					while((f1 = AST_LIST_REMOVE_HEAD(&myrpt->txq,
						frame_list))) ast_frfree(f1);
				}
				ast_write(myrpt->txchannel,f);
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
				if ( debug>4 )
					ast_log(LOG_NOTICE,"node=%s from zaptxchannel got AFC=%i data=%s\n",
						myrpt->name,f->subclass,f->datalen?(char *)f->data:"");

				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug>4 )
						ast_log(LOG_NOTICE, "node=%s zaptxchannel got AST_CONTROL_HANGUP\n",
							myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}
		/*  ***************************************************************
				Still handling a read from some channel.
				Traverse the links again, control and write to them.
			*************************************************************** */
		toexit = 0;
		rpt_mutex_lock(&myrpt->lock);
		l = myrpt->links.next;
		while(l != &myrpt->links)
		{
			int remnomute = 0;
			int remrx = 0;
			struct timeval now;

			if (l->disctime)
			{
				l = l->next;
				continue;
			}

			remrx = 0;

			/* see if any other links are receiving */
			m = myrpt->links.next;
			while(m != &myrpt->links)
			{
				/* if not us, count it */
				if ((m != l) && (m->lastrx)) remrx = 1;
				m = m->next;
			}
			rpt_mutex_unlock(&myrpt->lock);
			now = ast_tvnow();
			if ((who == l->chan) || (!l->lastlinktv.tv_sec) ||
				(ast_tvdiff_ms(now,l->lastlinktv) >= 19))
			{
				l->lastlinktv = now;
				remnomute = myrpt->localtx &&
				    (!(myrpt->cmdnode[0] ||
					(myrpt->dtmfidx > -1)));
				totx = (((l->isremote) ? (remnomute) :
					myrpt->localtx || myrpt->callmode) || remrx) && l->mode;

				if ( (!l->lastrx) && altlink(myrpt,l)){
					totx = myrpt->txkeyed;
					// ast_log(LOG_NOTICE, "[%s] link=%s totx=%i\n",myrpt->name,l->name,totx);
				}
				if (altlink1(myrpt,l)){
					totx = 1;
					// ast_log(LOG_NOTICE, "[%s] link=%s totx\n",myrpt->name,l->name);
				}

				/* if link has a squelch code, only tx if codes match */
				totx = totx && ( !strlen(l->cstxcode) || !strcmp(l->cstxcode,myrpt->cstxnow) );

				if (l->phonemode == 0 && l->chan && (l->lasttxhandled != totx))
				{
				    /* check for squelch code match for this link */
				    int lasttxwas;
					// asdf maw sph scm=rpt_check_channel(myrpt,l,0);
					lasttxwas=l->lasttx;
					if ( totx && !l->voterlink)
					{
						/* 	radio channel may be busy ending another squelch code so
							hold off a bit */
						if( l->csmm && !myrpt->p.duplex && !myrpt->txkeyed && strlen(myrpt->csrxnow) )
						{
							if(debug>1)
						    	ast_log(LOG_NOTICE,
						    		"[%s] link=%s/%s l->csmm=%i simplex sent ACRK data=%s\n",
						    		myrpt->name,l->name,l->chan->name,l->csmm,myrpt->csrxnow);
							ast_indicate_data(l->chan,AST_CONTROL_RADIO_KEY,
								myrpt->csrxnow,strlen(myrpt->csrxnow)+1);

							l->lasttx = 1;
							l->lasttxhandled=1;
						}
						else if( l->csmm && !strlen(myrpt->cstxnow) )
						{
					    	if(debug>1)
					    		ast_log(LOG_NOTICE,
					    			"[%s] link=%s/%s l->csmm=%i HOLDOFF\n",
					    			myrpt->name,l->name,l->chan->name,l->csmm);
						}
						else
						{
							char *pchar;
							int i=0;

							if(debug>1)
						    	ast_log(LOG_NOTICE,
						    		"[%s] link=%s/%s l->csmm=%i remrx=%i lastnode=%s sent ACRK data=%s\n",
						    		myrpt->name,l->name,l->chan->name,
									l->csmm,
									myrpt->remrx,
									myrpt->remrx?myrpt->lastnodewhichkeyedusup:NULL,
						    		l->csmm?myrpt->cstxnow:NULL);

							tmpstr[0]=0;
							pchar=tmpstr;
							if( l->csmm && isalnum(myrpt->cstxnow[0]) )
							{
							    i=snprintf(pchar,256,"%s",myrpt->cstxnow);
								pchar+=i;
							}

							if(l->iaxrpt && myrpt->remrx && myrpt->lastnodewhichkeyedusup)
							{
							    snprintf(pchar,256-i," LNWKU=%s",myrpt->lastnodewhichkeyedusup);
							}
							else if(l->iaxrpt)
							{
								snprintf(pchar,256-i," LNWKU=%s",myrpt->name);
							}

							#if 0
							ast_log(LOG_NOTICE,"ACRK data=%s\n",tmpstr);
							#endif

							ast_indicate_data(l->chan,AST_CONTROL_RADIO_KEY,
								tmpstr,
								strlen(tmpstr)+1);

							l->lasttx = 1;
							l->lasttxhandled=1;
						}
					}
					else
					{
						if(debug>1)
					    	ast_log(LOG_NOTICE,
					    		"[%s] link=%s/%s sent ACRUK\n",
					    		myrpt->name,l->name,l->chan->name);
						ast_indicate(l->chan,AST_CONTROL_RADIO_UNKEY);
						l->lasttx = 0;
						l->lasttxhandled=0;
					}
					/* log Tx/Rx transitions */
					if (lasttxwas!=l->lasttx && myrpt->p.archivedir)
					{
						char str[100];

						if (l->lasttx)
							sprintf(str,"TXKEY,%s",l->name);
						else
							sprintf(str,"TXUNKEY,%s",l->name);
						donodelog(myrpt,str);
					}
				}

				#if 0
				if(l->lnwku_update && myrpt->lastnodewhichkeyedusup)
				{
					char *pchar;
					int i=0;
					tmpstr[0]=0;
					pchar=tmpstr;

					ast_log(LOG_NOTICE,"[%s] sent %s AST_CONTROL_RADIO_KEY w/LNWKU=%s\n",myrpt->name,l->name,myrpt->lastnodewhichkeyedusup);
					snprintf(pchar,256-i," LNWKU=%s",myrpt->lastnodewhichkeyedusup);
					ast_indicate_data(l->chan,AST_CONTROL_RADIO_KEY,
								tmpstr,
								strlen(tmpstr)+1);
				}
				l->lnwku_update=0;
				#endif
			}
			rpt_mutex_lock(&myrpt->lock);
			if (who == l->chan)
			{
				/* if trigger was a frame read from the link */
				rpt_mutex_unlock(&myrpt->lock);
				f = ast_read(l->chan);
				if (!f)
				{
					rpt_mutex_lock(&myrpt->lock);
					__kickshort(myrpt);
					rpt_mutex_unlock(&myrpt->lock);
					if ((strncasecmp(l->chan->name,"echolink",8)) &&
					    (strncasecmp(l->chan->name,"irlp",4)))
					{
						if ((!l->disced) && (!l->outbound))
						{
							if ((l->name[0] == '0') || l->isremote)
								l->disctime = 1;
							else
								l->disctime = DISC_TIME;
							rpt_mutex_lock(&myrpt->lock);
							ast_hangup(l->chan);
							l->chan = 0;
							break;
						}

						if (l->retrytimer)
						{
							ast_hangup(l->chan);
							l->chan = 0;
							rpt_mutex_lock(&myrpt->lock);
							break;
						}
						if (l->outbound && (l->retries++ < l->max_retries) && (l->hasconnected))
						{
							rpt_mutex_lock(&myrpt->lock);
							if (l->chan) ast_hangup(l->chan);
							l->chan = 0;
							l->hasconnected = 1;
							l->retrytimer = RETRY_TIMER_MS;
							l->elaptime = 0;
							l->connecttime = 0;
							l->thisconnected = 0;
							break;
						}
					}
					rpt_mutex_lock(&myrpt->lock);
					/* remove from queue */
					remque((struct qelem *) l);
					if (!strcmp(myrpt->cmdnode,l->name))
						myrpt->cmdnode[0] = 0;
					__kickshort(myrpt);
					rpt_mutex_unlock(&myrpt->lock);
					if (!l->hasconnected)
						rpt_telemetry(myrpt,CONNFAIL,l);
					else if (l->disced != 2) rpt_telemetry(myrpt,REMDISC,l);
					if (myrpt->p.archivedir)
					{
						char str[100];

						if (!l->hasconnected)
							sprintf(str,"LINKFAIL,%s",l->name);
						else
							sprintf(str,"LINKDISC,%s",l->name);
						donodelog(myrpt,str);
					}
					if (l->lastf1) ast_frfree(l->lastf1);
					l->lastf1 = NULL;
					if (l->lastf2) ast_frfree(l->lastf2);
					l->lastf2 = NULL;
					/* hang-up on call to device */
					ast_hangup(l->chan);
					ast_hangup(l->pchan);
					ast_free(l);
					rpt_mutex_lock(&myrpt->lock);
					break;
				}
				if (f->frametype == AST_FRAME_VOICE)
				{
					int ismuted,n1;
					float fac,fsamp;

                    /* 20100907 1013 <sph@xelatec.com>
                    	Based on WB6NIL's patch of 2010/09/03.
					    It makes sure the dahdi buffer is large enough to accept the frame data.
						This method doesn't create a buffer and do ioctl's on each frame.
                       	Someday this might be done more elegantly or perhaps not even be needed.
                    */
                    if(l->pchblen < f->samples)
                    {
 	                   struct dahdi_bufferinfo bi;
 	                   memset(&bi,0,sizeof(bi));
 	                   if (ioctl(l->pchan->fds[0],DAHDI_GET_BUFINFO,&bi) != -1)
 	                   {
 	                      l->pchblen=bi.bufsize*bi.numbufs;
 	                      if(l->pchblen < f->samples)
 	                      {
	                          bi.numbufs = (f->samples / bi.bufsize) + 1;
	                          l->pchblen=bi.bufsize*bi.numbufs;
	                          ioctl(l->pchan->fds[0],DAHDI_SET_BUFINFO,&bi);
 	                      }
 	                   }
                    }

					fac = 1.0;
					if (l->chan && (!strncasecmp(l->chan->name,"irlp",4)))
						fac = myrpt->p.irxgain;
					else if (l->chan && (!strncasecmp(l->chan->name,"echolink",8)))
						fac = myrpt->p.erxgain;
					if (fac != 1.0)
					{
						int x1;
						short *sp;

						sp = (short *)f->data;
						for(x1 = 0; x1 < f->datalen / 2; x1++)
						{
							fsamp = (float) sp[x1] * fac;
							if (fsamp > 32765.0) fsamp = 32765.0;
							if (fsamp < -32765.0) fsamp = -32765.0;
							sp[x1] = (int) fsamp;
						}
					}

					l->rxlingertimer = RX_LINGER_TIME;

					/* no voice packet keying from iaxrpt or iaxkey links */
					if ( (l->newkey == 2) &&
						 (!l->lastrealrx) &&
						 (!l->iaxrpt && !l->iaxkey)
					    )
					{
						l->lastrealrx = 1;
						l->rerxtimer = 0;
						if (!l->lastrx1)
						{
							if (debug)
								ast_log(LOG_NOTICE,
									"[%s] link=%s/%s keyed lasttrx1 on Voice Frame \n",
									myrpt->name,l->name,l->chan->name);

							memset(l->csrxnow,0,sizeof(l->csrxnow));
							if (myrpt->p.archivedir)
							{
								char str[100];

								sprintf(str,"RXKEY,%s",l->name);
								donodelog(myrpt,str);
							}
							l->lastrx1 = 1;
							time(&l->lastkeytime);
						}
					}

					/*	voice packet keying is also associated with VOX phone
					    connections or links to Echolink and IRLP stations
					*/
					if (((l->phonemode) && (l->phonevox)) ||
					    (!strncasecmp(l->chan->name,"echolink",8)) ||
						(!strncasecmp(l->chan->name,"irlp",4)))
					{
						if (l->phonevox)
						{
							n1 = dovox(&l->vox,
								f->data,f->datalen / 2);
							if (n1 != l->wasvox)
							{
								if (debug)ast_log(LOG_DEBUG,
									"[%s] link=%s/%s phone VOX to %d\n",
									myrpt->name,l->name,l->chan->name,n1);
								l->wasvox = n1;
								l->voxtostate = 0;
								if (n1) l->voxtotimer = myrpt->p.voxtimeout_ms;
								else l->voxtotimer = 0;
							}
							if (l->lastrealrx || n1)
							{
								if (!myfirst)
								{
								    x = 0;
								    AST_LIST_TRAVERSE(&l->rxq, f1,
									frame_list) x++;
								    for(;x < myrpt->p.simplexphonedelay; x++)
									{
										f1 = ast_frdup(f);
										memset(f1->data,0,f1->datalen);
										memset(&f1->frame_list,0,sizeof(f1->frame_list));
										AST_LIST_INSERT_TAIL(&l->rxq,
											f1,frame_list);
								    }
								    myfirst = 1;
								}
								f1 = ast_frdup(f);
								memset(&f1->frame_list,0,sizeof(f1->frame_list));
								AST_LIST_INSERT_TAIL(&l->rxq,f1,frame_list);
							} else myfirst = 0;
							x = 0;
							AST_LIST_TRAVERSE(&l->rxq, f1,frame_list) x++;
							if (!x)
							{
								memset(f->data,0,f->datalen);
							}
							else
							{
								ast_frfree(f);
								f = AST_LIST_REMOVE_HEAD(&l->rxq,frame_list);
							}
						}
						if (ioctl(l->chan->fds[0], DAHDI_GETCONFMUTE, &ismuted) == -1)
						{
							ismuted = 0;
						}
						/* if not receiving, zero-out audio */
						ismuted |= (!l->lastrx);

						/* if another txcode has the channel mute audio out on this link */
						ismuted |= ( strlen(l->cstxcode) &&
									 strlen(myrpt->cstxnow) &&
									strcmp(l->cstxcode,myrpt->cstxnow) );

						if (l->dtmfed &&
							(l->phonemode ||
							    (!strncasecmp(l->chan->name,"echolink",8)) ||
								(!strncasecmp(l->chan->name,"irlp",4)))) ismuted = 1;
						l->dtmfed = 0;

						/* if a voting rx link and not the winner, mute audio */
						if(myrpt->p.votertype==1 && l->voterlink && myrpt->voted_link!=l)
						{
							ismuted=1;
						}

						if (ismuted)
						{
							memset(f->data,0,f->datalen);
							if (l->lastf1)
								memset(l->lastf1->data,0,l->lastf1->datalen);
							if (l->lastf2)
								memset(l->lastf2->data,0,l->lastf2->datalen);
						}
						if (f) f2 = ast_frdup(f);
						else f2 = NULL;
						f1 = l->lastf2;
						l->lastf2 = l->lastf1;
						l->lastf1 = f2;
						if (ismuted)
						{
							if (l->lastf1)
								memset(l->lastf1->data,0,l->lastf1->datalen);
							if (l->lastf2)
								memset(l->lastf2->data,0,l->lastf2->datalen);
						}
						if (f1)
						{
							ast_write(l->pchan,f1);
							ast_frfree(f1);
						}
					}
					else
					{
						/* if a voting rx link and not the winner, mute audio */
						if(myrpt->p.votertype==1 && l->voterlink && myrpt->voted_link!=l)
							ismuted=1;
						else
							ismuted=0;

						if ( !l->lastrx || ismuted )
							memset(f->data,0,f->datalen);
						ast_write(l->pchan,f);
					}
				}
#ifndef	OLD_ASTERISK
				else if (f->frametype == AST_FRAME_DTMF_BEGIN)
				{
				    /* mute DTMF voiceband audio	*/
					if (l->lastf1)
						memset(l->lastf1->data,0,l->lastf1->datalen);
					if (l->lastf2)
						memset(l->lastf2->data,0,l->lastf2->datalen);
					l->dtmfed = 1;
				}
#endif
				if (f->frametype == AST_FRAME_TEXT)
				{
					if(debug>5)
						ast_log(LOG_NOTICE,
							"node=%s link=%s/%s got TEXT=%s\n",
							myrpt->name,l->name,l->chan->name,f->datalen?(char *)f->data:"");
					handle_link_data(myrpt,l,f->data);
				}
				if (f->frametype == AST_FRAME_DTMF)
				{
					/* mute DTMF voiceband audio	*/
					if (l->lastf1)
						memset(l->lastf1->data,0,l->lastf1->datalen);
					if (l->lastf2)
						memset(l->lastf2->data,0,l->lastf2->datalen);
					l->dtmfed = 1;
					handle_link_phone_dtmf(myrpt,l,f->subclass);
				}
				if (f->frametype == AST_FRAME_CONTROL)
				{
					if(debug>5)
						ast_log(LOG_NOTICE,
							"node=%s hnr=%d link=%s/%s got AFC=%i data=%s\n",
							myrpt->name,myrpt->hasnoradio,l->name,l->chan->name,f->subclass,f->datalen?(char *)f->data:"");

					if (f->subclass == AST_CONTROL_ANSWER)
					{
						char lconnected = l->connected;

						if(debug>5)
							ast_log(LOG_NOTICE,
								"node=%s link=%s/%s got AST_CONTROL_ANSWER\n",
								myrpt->name,l->name,l->chan->name);

						__kickshort(myrpt);
						myrpt->rxlingertimer = RX_LINGER_TIME;
						l->connected = 1;
						l->hasconnected = 1;
						l->thisconnected = 1;
						l->elaptime = -1;
						if (!l->phonemode) send_newkey(l->chan);
						if (!l->isremote) l->retries = 0;
						if (!lconnected)
						{
							rpt_telemetry(myrpt,CONNECTED,l);
							if (myrpt->p.archivedir)
							{
								char str[100];

								if (l->mode)
									sprintf(str,"LINKTRX,%s",l->name);
								else
									sprintf(str,"LINKMONITOR,%s",l->name);
								donodelog(myrpt,str);
							}
							doconpgm(myrpt,l->name);
						}
						else
							l->reconnects++;
					}
					/* if link sent rpt a radio key */
					if (f->subclass == AST_CONTROL_RADIO_KEY)
					{
						if (debug > 5)
							ast_log(LOG_NOTICE,
								"node=%s link=%s/%s ACRK data=%s \n",
								myrpt->name,l->name,l->chan->name,(char *)f->data);

						/* if radio channel is busy transmitting another squelch code, ignore */
						if ( strlen(l->cstxcode) && strlen(myrpt->cstxnow) && strcmp(l->cstxcode,myrpt->cstxnow) )
						{
							l->rerxtimer = 0;
							ast_sendtext(l->chan,"DENY/INHIBIT");
							if (debug > 5)
								ast_log(LOG_NOTICE,
									"node=%s link=%s/%s ACRK data=%s was sent TEXT DENY/INHIBIT \n",
									myrpt->name,l->name,l->chan->name,(char *)f->data);
							break;
						}

						memset(l->csrxnow,0,sizeof(l->csrxnow));
						if ( f->datalen && f->data ) {
							strncpy(l->csrxnow, f->data,f->datalen);
						    if ( l->csrxcode[0]!=0 ) {
								if ( !strcmp(l->csrxcode,f->data) ) l->csm = 1;
								else l->csm = 0;
							}
						}

						// if ( l->csrxcode[0]==0 || l->csm==1 )
						{
							if (debug > 5)
								ast_log(LOG_NOTICE,
									"[%s] link=%s/%s ACRK data=%s set l->lastrealrx \n",
									myrpt->name,l->name,l->chan->name,l->csrxnow);

							l->rxlingertimer = RX_LINGER_TIME;
							l->lastrealrx = 1;
							l->rerxtimer = 0;
							if (!l->lastrx1)
							{
								if (myrpt->p.archivedir)
								{
									char str[100];
									sprintf(str,"RXKEY on %s ACRK",l->name);
									donodelog(myrpt,str);
								}
								l->lastrx1 = 1;
								time(&l->lastkeytime);
							}
						}
					}
					/* if link sent rpt a radio un-key */
					if ( f->subclass == AST_CONTROL_RADIO_UNKEY )
					{
						if (debug >= 6 && l->lastrx1)
							ast_log(LOG_NOTICE,"[%s] link=%s/%s got fresh ACRUK\n",
								myrpt->name,l->name,l->chan->name);

						l->csm=0;
						l->lastrealrx = 0;
						l->rerxtimer = 0;
						memset(myrpt->csrxnow,0,sizeof(myrpt->csrxnow));
						if (l->lastrx1)
						{
							if (myrpt->p.archivedir)
							{
								char str[100];

								sprintf(str,"RXUNKEY,%s",l->name);
								donodelog(myrpt,str);
							}
							l->lastrx1 = 0;
							if(myrpt->p.duplex)
								rpt_telemetry(myrpt,LINKUNKEY,l);
							time(&l->lastunkeytime);
						}
					}
					if (f->subclass == AST_CONTROL_HANGUP)
					{
						if (debug>5) ast_log(LOG_NOTICE,"[%s] link=%s/%s got AST_CONTROL_HANGUP\n",myrpt->name,l->name,l->chan->name);
						ast_frfree(f);
						rpt_mutex_lock(&myrpt->lock);
						__kickshort(myrpt);
						rpt_mutex_unlock(&myrpt->lock);
						if ((strncasecmp(l->chan->name,"echolink",8)) &&
						    (strncasecmp(l->chan->name,"irlp",4)))
						{
							if ((!l->outbound) && (!l->disced))
							{
								if ((l->name[0] == '0') || l->isremote)
									l->disctime = 1;
								else
									l->disctime = DISC_TIME;
								rpt_mutex_lock(&myrpt->lock);
								ast_hangup(l->chan);
								l->chan = 0;
								break;
							}
							if (l->retrytimer)
							{
								if (l->chan) ast_hangup(l->chan);
								l->chan = 0;
								rpt_mutex_lock(&myrpt->lock);
								break;
							}
							if (l->outbound && (l->retries++ < l->max_retries) && (l->hasconnected))
							{
								rpt_mutex_lock(&myrpt->lock);
								if (l->chan) ast_hangup(l->chan);
								l->chan = 0;
								l->hasconnected = 1;
								l->elaptime = 0;
								l->retrytimer = RETRY_TIMER_MS;
								l->connecttime = 0;
								l->thisconnected = 0;
								break;
							}
						}
						rpt_mutex_lock(&myrpt->lock);
						/* remove from queue */
						remque((struct qelem *) l);
						if (!strcmp(myrpt->cmdnode,l->name))
							myrpt->cmdnode[0] = 0;
						__kickshort(myrpt);
						rpt_mutex_unlock(&myrpt->lock);
						if (!l->hasconnected)
							rpt_telemetry(myrpt,CONNFAIL,l);
						else if (l->disced != 2) rpt_telemetry(myrpt,REMDISC,l);
						if (myrpt->p.archivedir)
						{
							char str[100];

							if (!l->hasconnected)
								sprintf(str,"LINKFAIL,%s",l->name);
							else
								sprintf(str,"LINKDISC,%s",l->name);
							donodelog(myrpt,str);
						}
						if (l->hasconnected) dodispgm(myrpt,l->name);
						if (l->lastf1) ast_frfree(l->lastf1);
						l->lastf1 = NULL;
						if (l->lastf2) ast_frfree(l->lastf2);
						l->lastf2 = NULL;
						/* hang-up on call to device */
						ast_hangup(l->chan);
						ast_hangup(l->pchan);
						ast_free(l);
						rpt_mutex_lock(&myrpt->lock);
						break;
					}
				}
				ast_frfree(f);
				rpt_mutex_lock(&myrpt->lock);
				break;
			}
			if (who == l->pchan)
			{
				/* we got a frame from the rpt psuedo channel to the link */
				rpt_mutex_unlock(&myrpt->lock);
				f = ast_read(l->pchan);
				if (!f)
				{
					if (debug) ast_log(LOG_NOTICE,"node=%s link=%s/%s Hung Up pchan implied\n",myrpt->name,l->name,l->chan->name);
					toexit = 1;
					rpt_mutex_lock(&myrpt->lock);
					break;
				}
				if (f->frametype == AST_FRAME_VOICE)
				{
				    /* adjust amplitude for these channel types */
					float fac,fsamp;

					fac = 1.0;
					if (l->chan && (!strncasecmp(l->chan->name,"irlp",4)))
						fac = myrpt->p.itxgain;
					else if (l->chan && (!strncasecmp(l->chan->name,"echolink",8)))
						fac = myrpt->p.etxgain;

					if (fac != 1.0)
					{
						int x1;
						short *sp;

						sp = (short *) f->data;
						for(x1 = 0; x1 < f->datalen / 2; x1++)
						{
							fsamp = (float) sp[x1] * fac;
							if (fsamp > 32765.0) fsamp = 32765.0;
							if (fsamp < -32765.0) fsamp = -32765.0;
							sp[x1] = (int) fsamp;
						}
					}
					/* write voice frame to the link's channel	*/
					if (l->chan && (l->lastrx || (!altlink(myrpt,l))) &&
					    ((l->newkey < 2) || l->lasttx ||
						strncasecmp(l->chan->name,"IAX",3)))
							ast_write(l->chan,f);
				}
				if (f->frametype == AST_FRAME_CONTROL)
				{
				    /* KEY/UNKEY control messages are handled elsewhere */
					if(debug>5)
						ast_log(LOG_NOTICE,
							"node=%s link=%s/%s got AFC=%i data=%s\n",
							myrpt->name,l->name,l->chan->name,f->subclass,f->datalen?(char *)f->data:"");

					if (f->subclass == AST_CONTROL_HANGUP)
					{
						if (debug>6) ast_log(LOG_NOTICE,"node=%s link=%s/%s Hung Up from pchan\n",myrpt->name,l->name,l->chan->name);
						ast_frfree(f);
						toexit = 1;
						rpt_mutex_lock(&myrpt->lock);
						break;
					}
				}
				ast_frfree(f);
				rpt_mutex_lock(&myrpt->lock);
				break;
			}
			l = l->next;
		}
		/*	end of traverse all links	*/

		rpt_mutex_unlock(&myrpt->lock);
		if (toexit) break;
		if (who == myrpt->monchannel)
		{
			f = ast_read(myrpt->monchannel);
			if (!f)
			{
				if (debug) ast_log(LOG_NOTICE,"node=%s Hung Up from monchannel implied\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_VOICE)
			{
				/*	this is audio out to the link */
				struct ast_frame *fs;
				short *sp;
				int x1;
				float fac,fsamp;

				if (myrpt->monstream && (myrpt->p.duplex || myrpt->txkeyed ))
					ast_writestream(myrpt->monstream,f);

				fs = ast_frdup(f);
				fac = 1.0;
				if (l->chan && (!strncasecmp(l->chan->name,"irlp",4)))
					fac = myrpt->p.itxgain;
				else if (l->chan && (!strncasecmp(l->chan->name,"echolink",8)))
					fac = myrpt->p.etxgain;
				if (fac != 1.0)
				{
					sp = (short *)fs->data;
					for(x1 = 0; x1 < fs->datalen / 2; x1++)
					{
						fsamp = (float) sp[x1] * fac;
						if (fsamp > 32765.0) fsamp = 32765.0;
						if (fsamp < -32765.0) fsamp = -32765.0;
						sp[x1] = (int) fsamp;
					}
				}
				l = myrpt->links.next;
				/* go thru all the links */
				while(l != &myrpt->links)
				{
					if (l->chan && altlink(myrpt,l) && (!l->lastrx) && ((l->newkey < 2) || l->lasttx ||
					    strncasecmp(l->chan->name,"IAX",3)))
				    {
						if (l->chan &&
						    ((!strncasecmp(l->chan->name,"irlp",4)) ||
							(!strncasecmp(l->chan->name,"echolink",8))))
						{
							ast_write(l->chan,fs);
						}
						else
						{
							ast_write(l->chan,f);
						}
				    }
					l = l->next;
				}
				ast_frfree(fs);
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug>5 ) ast_log(LOG_NOTICE,"node=%s Hung Up from monchannel\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}
		if (myrpt->parrotchannel && (who == myrpt->parrotchannel))
		{
			f = ast_read(myrpt->parrotchannel);
			if (!f)
			{
				if (debug) ast_log(LOG_NOTICE,"node=%s Hung Up from parrotchannel implied\n",myrpt->name);
				break;
			}
			if (!(myrpt->p.parrotmode || myrpt->parrotonce))
			{
				char myfname[300];

				if (myrpt->parrotstream)
				{
					ast_closestream(myrpt->parrotstream);
					myrpt->parrotstream = 0;
				}
				sprintf(myfname,PARROTFILE,myrpt->name,myrpt->parrotcnt);
				strcat(myfname,".wav");
				unlink(myfname);
			} else if (f->frametype == AST_FRAME_VOICE)
			{
				if (myrpt->parrotstream)
					ast_writestream(myrpt->parrotstream,f);
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug>5 ) ast_log(LOG_NOTICE,"node=%s parrotchannel Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}
		if (myrpt->voxchannel && (who == myrpt->voxchannel))
		{
			f = ast_read(myrpt->voxchannel);
			if (!f)
			{
				if (debug) ast_log(LOG_NOTICE,"node=%s voxchannel Hang Up implied\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_VOICE)
			{
				n = dovox(&myrpt->vox,f->data,f->datalen / 2);
				if (n != myrpt->wasvox)
				{
					if (debug) ast_log(LOG_DEBUG,"Node=%s, voxchannel VOX to %d\n",myrpt->name,n);
					myrpt->wasvox = n;
					myrpt->voxtostate = 0;
					if (n) myrpt->voxtotimer = myrpt->p.voxtimeout_ms;
					else myrpt->voxtotimer = 0;
				}
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
			    if ( debug>5 ) ast_log(LOG_NOTICE,"node=%s voxchannel AFC %i\n",myrpt->name,f->subclass);
				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug ) ast_log(LOG_NOTICE,"node=%s voxchannel Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}
		if (who == myrpt->txpchannel) /* if it was a read from remote tx */
		{
			f = ast_read(myrpt->txpchannel);
			if (!f)
			{
				if ( debug>4 ) ast_log(LOG_NOTICE,"node=%s txchannel Hang Up implied\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug>5 ) ast_log(LOG_NOTICE,"node=%s txchannel Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}

	    /* Handle telemetry conference output */
		if (who == myrpt->telechannel) /* if is telemetry conference output */
		{
			//if(debug)ast_log(LOG_NOTICE,"node=%s %p %p %d %d %d\n",myrpt->name,who,myrpt->telechannel,myrpt->rxchankeyed,myrpt->remrx,myrpt->noduck);
			f = ast_read(myrpt->telechannel);
			if (!f)
			{
				if (debug) ast_log(LOG_NOTICE,"node=%s telechannel Hung Up implied\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_VOICE)
			{
				float gain;

				if(!myrpt->noduck&&(myrpt->rxchankeyed||myrpt->remrx))
					gain = myrpt->p.telemduckgain;
				else
					gain = myrpt->p.telemnomgain;

				//ast_log(LOG_NOTICE,"node=%s %i %i telem gain set %d %d %d\n",myrpt->name,who,myrpt->telechannel,myrpt->rxchankeyed,myrpt->noduck);

				if(gain!=0)
				{
					int n,k;
					short *sp = (short *) f->data;
					for(n=0; n<f->datalen/2; n++)
					{
						k=sp[n]*gain;
						if (k > 32767) k = 32767;
						else if (k < -32767) k = -32767;
						sp[n]=k;
					}
				}
				ast_write(myrpt->btelechannel,f);
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug>5 ) ast_log(LOG_NOTICE,"node=%s telechannel Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}
		/* if is btelemetry conference output */
		if (who == myrpt->btelechannel)
		{
			f = ast_read(myrpt->btelechannel);
			if (!f)
			{
				if (debug) ast_log(LOG_NOTICE,"node=%s btelechannel Hung Up implied\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug>5 ) ast_log(LOG_NOTICE,"node=%s btelechannel Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}
	}
	/*
	terminate and cleanup app_rpt node instance
	*/
	usleep(100000);
	ast_hangup(myrpt->pchannel);
	ast_hangup(myrpt->monchannel);
	if (myrpt->parrotchannel) ast_hangup(myrpt->parrotchannel);
	myrpt->parrotstate = 0;
	if (myrpt->voxchannel) ast_hangup(myrpt->voxchannel);
	ast_hangup(myrpt->btelechannel);
	ast_hangup(myrpt->telechannel);
	ast_hangup(myrpt->txpchannel);
	if (myrpt->txchannel != myrpt->rxchannel) ast_hangup(myrpt->txchannel);
	if (myrpt->zaptxchannel != myrpt->txchannel) ast_hangup(myrpt->zaptxchannel);
	if (myrpt->lastf1) ast_frfree(myrpt->lastf1);
	myrpt->lastf1 = NULL;
	if (myrpt->lastf2) ast_frfree(myrpt->lastf2);
	myrpt->lastf2 = NULL;
	ast_hangup(myrpt->rxchannel);
	rpt_mutex_lock(&myrpt->lock);
	l = myrpt->links.next;
	while(l != &myrpt->links)
	{
		struct rpt_link *ll = l;
		/* remove from queue */
		remque((struct qelem *) l);
		/* hang-up on call to device */
		if (l->chan) ast_hangup(l->chan);
		ast_hangup(l->pchan);
		l = l->next;
		ast_free(ll);
	}
	if (myrpt->xlink  == 1) myrpt->xlink = 2;
	rpt_mutex_unlock(&myrpt->lock);
	if (debug) ast_log(LOG_NOTICE,"node=%s Hung Up Channel\n",myrpt->name);
	myrpt->rpt_thread = AST_PTHREADT_STOP;
	pthread_exit(NULL);
	return NULL;
}
/* end of rpt()	*/


/*! \brief Read rpt.conf, create and start radio nodes. */
/*
*/
static void *rpt_master(void *ignore)
{
int	i,n;
pthread_attr_t attr;
struct ast_config *cfg;
char *this,*val;

	/* init nodelog queue */
	nodelog.next = nodelog.prev = &nodelog;
	/* go thru all the specified repeaters */
	this = NULL;
	n = 0;
#ifndef OLD_ASTERISK
	/* wait until asterisk starts */
    while(!ast_test_flag(&ast_options,AST_OPT_FLAG_FULLY_BOOTED))
            usleep(250000);
#endif
#ifdef	NEW_ASTERISK
	rpt_vars[n].cfg = ast_config_load("rpt.conf",config_flags);
#else
	rpt_vars[n].cfg = ast_config_load("rpt.conf");
#endif
	cfg = rpt_vars[n].cfg;
	if (!cfg) {
		ast_log(LOG_NOTICE, "Unable to open radio repeater configuration rpt.conf.  Radio Repeater disabled.\n");
		pthread_exit(NULL);
	}
	while((this = ast_category_browse(cfg,this)) != NULL)
	{
		for(i = 0 ; i < strlen(this) ; i++){
			if((this[i] < '0') || (this[i] > '9'))
				break;
		}
		if(i != strlen(this)) continue; /* Not a node defn */
		memset(&rpt_vars[n],0,sizeof(rpt_vars[n]));
		rpt_vars[n].name = ast_strdup(this);
		val = (char *) ast_variable_retrieve(cfg,this,"rxchannel");
		if (val) rpt_vars[n].rxchanname = ast_strdup(val);
		val = (char *) ast_variable_retrieve(cfg,this,"txchannel");
		if (val) rpt_vars[n].txchanname = ast_strdup(val);
		rpt_vars[n].remote = 0;
		rpt_vars[n].remoterig = "";
		val = (char *) ast_variable_retrieve(cfg,this,"remote");
		if (val)
		{
			rpt_vars[n].remoterig = ast_strdup(val);
			rpt_vars[n].remote = 1;
		}
		val = (char *) ast_variable_retrieve(cfg,this,"radiotype");
		if (val) rpt_vars[n].remoterig = ast_strdup(val);
		ast_mutex_init(&rpt_vars[n].lock);
		ast_mutex_init(&rpt_vars[n].remlock);
		ast_mutex_init(&rpt_vars[n].statpost_lock);
		rpt_vars[n].tele.next = &rpt_vars[n].tele;
		rpt_vars[n].tele.prev = &rpt_vars[n].tele;
		rpt_vars[n].rpt_thread = AST_PTHREADT_NULL;
		rpt_vars[n].tailmessagen = 0;
#ifdef	_MDC_DECODE_H_
		rpt_vars[n].mdc = mdc_decoder_new(8000);
#endif
		n++;
	}
	nrpts = n;
	ast_config_destroy(cfg);

	/* start em all */
	for(i = 0; i < n; i++)
	{
		load_rpt_vars(i,1);

		/* if is a remote, dont start one for it */
		if (rpt_vars[i].remote)
		{
			if(retreive_memory(&rpt_vars[i],"init")){ /* Try to retreive initial memory channel */
				if (!strcmp(rpt_vars[i].remoterig,remote_rig_rtx450))
					strncpy(rpt_vars[i].freq, "446.500", sizeof(rpt_vars[i].freq) - 1);
				else
					strncpy(rpt_vars[i].freq, "146.580", sizeof(rpt_vars[i].freq) - 1);
				strncpy(rpt_vars[i].rxpl, "100.0", sizeof(rpt_vars[i].rxpl) - 1);

				strncpy(rpt_vars[i].txpl, "100.0", sizeof(rpt_vars[i].txpl) - 1);
				rpt_vars[i].remmode = REM_MODE_FM;
				rpt_vars[i].offset = REM_SIMPLEX;
				rpt_vars[i].powerlevel = REM_LOWPWR;
			}
			continue;
		}
		else /* is a normal repeater */
		{
		    rpt_vars[i].p.memory = rpt_vars[i].name;
			if(retreive_memory(&rpt_vars[i],"radiofreq")){ /* Try to retreive initial memory channel */
				if (!strcmp(rpt_vars[i].remoterig,remote_rig_rtx450))
					strncpy(rpt_vars[i].freq, "446.500", sizeof(rpt_vars[i].freq) - 1);
				else if (!strcmp(rpt_vars[i].remoterig,remote_rig_rtx150))
					strncpy(rpt_vars[i].freq, "146.580", sizeof(rpt_vars[i].freq) - 1);
				strncpy(rpt_vars[i].rxpl, "100.0", sizeof(rpt_vars[i].rxpl) - 1);

				strncpy(rpt_vars[i].txpl, "100.0", sizeof(rpt_vars[i].txpl) - 1);
				rpt_vars[i].remmode = REM_MODE_FM;
				rpt_vars[i].offset = REM_SIMPLEX;
				rpt_vars[i].powerlevel = REM_LOWPWR;
			}
			ast_log(LOG_NOTICE,"Normal Repeater Init  %s  %s  %s\n",rpt_vars[i].name, rpt_vars[i].remoterig, rpt_vars[i].freq);
		}
		if (rpt_vars[i].p.ident && (!*rpt_vars[i].p.ident))
		{
			ast_log(LOG_WARNING,"Did not specify ident for node %s\n",rpt_vars[i].name);
			ast_config_destroy(cfg);
			pthread_exit(NULL);
		}
	    pthread_attr_init(&attr);
	    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		ast_pthread_create(&rpt_vars[i].rpt_thread,&attr,rpt,(void *) &rpt_vars[i]);
	}
	usleep(500000);
	time(&starttime);
	for(;;)
	{
		/* Now monitor each thread, and restart it if necessary */
		for(i = 0; i < n; i++)
		{
			int rv;
			if (rpt_vars[i].remote || rpt_vars[i].radiotaken) continue;
			if (rpt_vars[i].rpt_thread == AST_PTHREADT_STOP)
				rv = -1;
			else
				rv = pthread_kill(rpt_vars[i].rpt_thread,0);
			if (rv)
			{
				if(time(NULL) - rpt_vars[i].lastthreadrestarttime <= 15)
				{
					if(rpt_vars[i].threadrestarts >= 5)
					{
						ast_log(LOG_ERROR,"Continual RPT thread restarts, killing Asterisk\n");
						exit(1); /* Stuck in a restart loop, kill Asterisk and start over */
					}
					else
					{
						ast_log(LOG_NOTICE,"RPT thread restarted on %s\n",rpt_vars[i].name);
						rpt_vars[i].threadrestarts++;
					}
				}
				else
					rpt_vars[i].threadrestarts = 0;

				rpt_vars[i].lastthreadrestarttime = time(NULL);
			        pthread_attr_init(&attr);
	 		        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
				ast_pthread_create(&rpt_vars[i].rpt_thread,&attr,rpt,(void *) &rpt_vars[i]);
				/* if (!rpt_vars[i].xlink) */
					ast_log(LOG_WARNING, "rpt_thread restarted on node %s\n", rpt_vars[i].name);
			}

		}
		for(;;)
		{
			struct nodelog *nodep;
			char *space,datestr[100],fname[300];
			int fd;

			ast_mutex_lock(&nodeloglock);
			nodep = nodelog.next;
			if(nodep == &nodelog) /* if nothing in queue */
			{
				ast_mutex_unlock(&nodeloglock);
				break;
			}
			remque((struct qelem *)nodep);
			ast_mutex_unlock(&nodeloglock);
			space = strchr(nodep->str,' ');
			if (!space)
			{
				ast_free(nodep);
				continue;
			}
			*space = 0;
			strftime(datestr,sizeof(datestr) - 1,"%Y%m%d",
				localtime(&nodep->timestamp));
			sprintf(fname,"%s/%s/%s.txt",nodep->archivedir,
				nodep->str,datestr);
			fd = open(fname,O_WRONLY | O_CREAT | O_APPEND,0664);
			if (fd == -1)
			{
				ast_log(LOG_ERROR,"Cannot open node log file %s for write.\n",fname);
				ast_free(nodep);
				continue;
			}
			if (write(fd,space + 1,strlen(space + 1)) !=
				strlen(space + 1))
			{
				ast_log(LOG_ERROR,"Cannot write node log file %s for write.\n",fname);
				ast_free(nodep);
				continue;
			}
			close(fd);
			ast_free(nodep);
		}
		usleep(2000000);
	}
	ast_config_destroy(cfg);
	pthread_exit(NULL);
}

/*! \brief Handle dialplan rpt() app for incoming calls and remotes. */

/*  Called from the dialplan.
	For remotes it initiates, loops and controls the radio channel.
	For calls to a repeater node it sets up a new link attached to the
	repeater, returns and then rpt() services the link.
*/
static int rpt_exec(struct ast_channel *chan, void *data)
{
	int res=-1,i,x,rem_totx,rem_rx,remkeyed,n,phone_mode = 0;
	int iskenwood_pci4,authtold,authreq,setting,notremming,reming;
	int ismuted,dtmfed,phone_vox = 0, phone_monitor = 0, phone_nox=0,bargechan=0;
#ifdef	OLD_ASTERISK
	struct localuser *u;
#endif
	char tmp[256], keyed = 0,keyed1 = 0;
	char *options,*stringp,*tele,c,*altp,*memp,*codep,*forcep=NULL;
	char sx[320],*sy,myfirst;
	struct	rpt *myrpt;
	struct ast_frame *f,*f1,*f2;
	struct ast_channel *who;
	struct ast_channel *cs[20];
	struct	rpt_link *l;
	struct dahdi_confinfo ci;  /* conference info */
	struct dahdi_params par;
	int ms,elap,n1,myrx;
	time_t t,last_timeout_warning;
	struct	dahdi_radio_param z;
	struct rpt_tele *telem;
	int	numlinks;
	int nullfd=-1;

	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING, "Rpt requires an argument (system node)\n");
		return -1;
	}

	strncpy(tmp, (char *)data, sizeof(tmp)-1);
	time(&t);
	/* if time has externally shifted negative, screw it */
	if (t < starttime) t = starttime + START_DELAY;
	if ((!starttime) || (t < (starttime + START_DELAY)))
	{
		ast_log(LOG_NOTICE,"[%s] rejecting call: too soon!\n",tmp);
		ast_safe_sleep(chan,3000);
		return -1;
	}

	ast_set_read_format(chan,AST_FORMAT_SLINEAR);
	ast_set_write_format(chan,AST_FORMAT_SLINEAR);

	ast_log(LOG_NOTICE,"parsing argument=%s\n",tmp);

	altp=strstr(tmp, "|*");
	if(altp){
		altp[0]=0;
		altp++;
    }

	memp=strstr(tmp, "|M");
	if(memp){
		memp[0]=0;
		memp+=2;
    }

	forcep=strstr(tmp, "|F");
	if(forcep){
		forcep[0]=0;
		forcep+=2;
    }

	/*	read squelch code option for this call/link */
	codep=strstr(tmp, "|c");
	if(codep){
		codep[0]=0;
		codep+=2;
		if(debug>0)ast_log(LOG_NOTICE,"Node %s call code %s\n",tmp,codep);
    }

	stringp=tmp;
	strsep(&stringp, "|");
	options = stringp;

//	ast_log(LOG_NOTICE,"options=%s \n",options);
//	if(memp>0)ast_log(LOG_NOTICE,"memp=%s \n",memp);
//	if(altp>0)ast_log(LOG_NOTICE,"altp=%s \n",altp);

	myrpt = NULL;
	/* see if we can find our specified one */
	for(i = 0; i < nrpts; i++)
	{
		/* if name matches, assign it and exit loop */
		if (!strcmp(tmp,rpt_vars[i].name))
		{
			myrpt = &rpt_vars[i];
			break;
		}
	}

	pbx_builtin_setvar_helper(chan, "RPT_STAT_ERR", "");

	if (myrpt == NULL)
	{
		pbx_builtin_setvar_helper(chan, "RPT_STAT_ERR", "NODE_NOT_FOUND");
		ast_log(LOG_WARNING, "Cannot find specified system node %s\n",tmp);
		return (priority_jump(NULL,chan));
	}

	numlinks=linkcount(myrpt);

	if(options && *options == 'q')
	{
		int res=0;
		int numlinks;
	 	char buf2[128];

		numlinks=linkcount(myrpt);
		if(myrpt->keyed)
			pbx_builtin_setvar_helper(chan, "RPT_STAT_RXKEYED", "1");
		else
			pbx_builtin_setvar_helper(chan, "RPT_STAT_RXKEYED", "0");

		if(myrpt->txkeyed)
			pbx_builtin_setvar_helper(chan, "RPT_STAT_TXKEYED", "1");
		else
			pbx_builtin_setvar_helper(chan, "RPT_STAT_TXKEYED", "0");

		snprintf(buf2,sizeof(buf2),"%s=%i", "RPT_STAT_XLINK", myrpt->xlink);
		pbx_builtin_setvar(chan, buf2);
		snprintf(buf2,sizeof(buf2),"%s=%i", "RPT_STAT_LINKS", numlinks);
		pbx_builtin_setvar(chan, buf2);
		snprintf(buf2,sizeof(buf2),"%s=%d", "RPT_STAT_WASCHAN", myrpt->waschan);
		pbx_builtin_setvar(chan, buf2);
		snprintf(buf2,sizeof(buf2),"%s=%d", "RPT_STAT_NOWCHAN", myrpt->nowchan);
		pbx_builtin_setvar(chan, buf2);
		snprintf(buf2,sizeof(buf2),"%s=%d", "RPT_STAT_DUPLEX", myrpt->p.duplex);
		pbx_builtin_setvar(chan, buf2);
		snprintf(buf2,sizeof(buf2),"%s=%d", "RPT_STAT_PARROT", myrpt->p.parrotmode);
		pbx_builtin_setvar(chan, buf2);
		snprintf(buf2,sizeof(buf2),"%s=%i", "RPT_STAT_PHONELINKS", myrpt->phonelinks);
		pbx_builtin_setvar(chan, buf2);
		//snprintf(buf2,sizeof(buf2),"%s=%d", "RPT_STAT_PHONEVOX", myrpt->phonevox);
		//pbx_builtin_setvar(chan, buf2);
		//snprintf(buf2,sizeof(buf2),"%s=%d", "RPT_STAT_CONNECTED", myrpt->connected);
		//pbx_builtin_setvar(chan, buf2);
		snprintf(buf2,sizeof(buf2),"%s=%i", "RPT_STAT_CALLMODE", myrpt->callmode);
		pbx_builtin_setvar(chan, buf2);
		snprintf(buf2,sizeof(buf2),"%s=%s", "RPT_STAT_LASTTONE", myrpt->lasttone);
		pbx_builtin_setvar(chan, buf2);
		snprintf(buf2,sizeof(buf2),"%s=%d", "RPT_STAT_RADIOFAIL", myrpt->radiofail);
		pbx_builtin_setvar(chan, buf2);
		snprintf(buf2,sizeof(buf2),"%s=%d", "RPT_STAT_BUSY", (numlinks||forcep)?1:0);
		pbx_builtin_setvar(chan, buf2);

		res=priority_jump(myrpt,chan);
		return res;
	}

	if(options && *options == 'o')
	{
		return(channel_revert(myrpt));
	}

	#if 0
	if((altp)&&(*options == 'Z'))
	{
		rpt_push_alt_macro(myrpt,altp);
		return 0;
	}
	#endif

	if(forcep!=NULL) bargechan = 1;

	/* if not phone access, must be an IAX connection */
	if (options && ((*options == 'N') || (*options == 'P') ||
	   (*options == 'D') || (*options == 'R') || (*options == 'S')))
	{
		int val;

		pbx_builtin_setvar_helper(chan, "RPT_STAT_BUSY", "0");

		if(memp>0)
		{
			char radiochan;
			radiochan=strtod(data,NULL);

			if(numlinks>0 && radiochan!=myrpt->nowchan && !bargechan)
			{
				pbx_builtin_setvar_helper(chan, "RPT_STAT_BUSY", "1");
				ast_log(LOG_NOTICE, "Radio Channel Busy.\n");
				return (priority_jump(myrpt,chan));
			}
			else if(radiochan!=myrpt->nowchan || bargechan)
			{
				channel_steer(myrpt,memp);
			}
		}
		if(altp)rpt_push_alt_macro(myrpt,altp);
		phone_mode = 1;
		if (*options == 'N')
		{
			phone_mode = 2;
			phone_nox = 1;
		}
		if (*options == 'D') phone_mode = 2;
		if (*options == 'S') phone_mode = 3;
		ast_set_callerid(chan,"0","app_rpt user","0");
		val = 1;
		ast_channel_setoption(chan,AST_OPTION_TONE_VERIFY,&val,sizeof(char),0);
		if (strchr(options + 1,'v') || strchr(options + 1,'V')) phone_vox = 1;
		if (strchr(options + 1,'m') || strchr(options + 1,'M')) phone_monitor = 1;
	}
	else
	{
#ifdef ALLOW_LOCAL_CHANNELS
		/* Check to insure the connection is IAX2 or Local*/
		if ( (strncmp(chan->name,"IAX2",4)) && (strncmp(chan->name,"Local",5)) &&
		     (strncasecmp(chan->name,"echolink",8)) && (strncasecmp(chan->name,"irlp",4)) ) {
		    ast_log(LOG_WARNING, "We only accept links via IAX2, Echolink, IRLP or Local!!\n");
		    return -1;
		}
#else
		if (strncmp(chan->name,"IAX2",4) && strncasecmp(chan->name,"Echolink",8) &&
		    strncasecmp(chan->name,"irlp",4))
		{
			ast_log(LOG_WARNING, "We only accept links via IAX2 or Echolink!!\n");
			return -1;
		}
#endif
		/* Do not allow incoming radio connections if disabled or noincomingconns is set */
        if((myrpt->p.s[myrpt->p.sysstate_cur].txdisable) || myrpt->p.s[myrpt->p.sysstate_cur].noincomingconns){
      	        ast_log(LOG_NOTICE, "Connect attempt to node %s  with tx disabled or NOICE cop function active", myrpt->name);
              	return -1;
      	}
	}
	if (options && (*options == 'R'))
	{
		/* Parts of this section taken from app_parkandannounce */
		char *return_context;
		int l, m, lot, timeout = 0;
		char tmp[256],*template;
		char *working, *context, *exten, *priority;
		char *s,*orig_s;

		rpt_mutex_lock(&myrpt->lock);
		m = myrpt->callmode;
		rpt_mutex_unlock(&myrpt->lock);

		if ((!myrpt->p.nobusyout) && m)
		{
			if (chan->_state != AST_STATE_UP)
			{
				ast_indicate(chan,AST_CONTROL_BUSY);
			}
			while(ast_safe_sleep(chan,10000) != -1);
			return -1;
		}

		if (chan->_state != AST_STATE_UP)
		{
			ast_answer(chan);
			if (!phone_mode) send_newkey(chan);
		}

		l=strlen(options)+2;
		orig_s=ast_malloc(l);
		if(!orig_s) {
			ast_log(LOG_WARNING, "Out of memory\n");
			return -1;
		}
		s=orig_s;
		strncpy(s,options,l);

		template=strsep(&s,"|");
		if(!template) {
			ast_log(LOG_WARNING, "An announce template must be defined\n");
			ast_free(orig_s);
			return -1;
		}

		if(s) {
			timeout = atoi(strsep(&s, "|"));
			timeout *= 1000;
		}

		return_context = s;

		if(return_context != NULL) {
			/* set the return context. Code borrowed from the Goto builtin */
			working = return_context;
			context = strsep(&working, "|");
			exten = strsep(&working, "|");
			if(!exten) {
				/* Only a priority in this one */
				priority = context;
				exten = NULL;
				context = NULL;
			} else {
				priority = strsep(&working, "|");
				if(!priority) {
					/* Only an extension and priority in this one */
					priority = exten;
					exten = context;
					context = NULL;
			}
		}
		if(atoi(priority) < 0) {
			ast_log(LOG_WARNING, "Priority '%s' must be a number > 0\n", priority);
			ast_free(orig_s);
			return -1;
		}
		/* At this point we have a priority and maybe an extension and a context */
		chan->priority = atoi(priority);
#ifdef OLD_ASTERISK
		if(exten && strcasecmp(exten, "BYEXTENSION"))
#else
		if(exten)
#endif
			strncpy(chan->exten, exten, sizeof(chan->exten)-1);
		if(context)
			strncpy(chan->context, context, sizeof(chan->context)-1);
		} else {  /* increment the priority by default*/
			chan->priority++;
		}

		if(option_verbose > 2) {
			ast_verbose( VERBOSE_PREFIX_3 "Return Context: (%s,%s,%d) ID: %s\n", chan->context,chan->exten, chan->priority, chan->cid.cid_num);
			if(!ast_exists_extension(chan, chan->context, chan->exten, chan->priority, chan->cid.cid_num)) {
				ast_verbose( VERBOSE_PREFIX_3 "Warning: Return Context Invalid, call will return to default|s\n");
			}
		}

		/* we are using masq_park here to protect * from touching the channel once we park it.  If the channel comes out of timeout
		before we are done announcing and the channel is messed with, Kablooeee.  So we use Masq to prevent this.  */
		ast_masq_park_call(chan, NULL, timeout, &lot);

		if (option_verbose > 2)
			ast_verbose( VERBOSE_PREFIX_3 "Call Parking Called, lot: %d, timeout: %d, context: %s\n",
				lot, timeout, return_context);

		snprintf(tmp,sizeof(tmp) - 1,"%d,%s",lot,template + 1);
		rpt_telemetry(myrpt,REV_PATCH,tmp);
		ast_free(orig_s);
		return 0;
	}
	i = 0;
	if (!strncasecmp(chan->name,"echolink",8)) i = 1;
	if (!strncasecmp(chan->name,"irlp",4)) i = 1;
	if ((!options) && (!i))
	{
        struct ast_hostent ahp;
        struct hostent *hp;
        struct in_addr ia;
        char hisip[100],nodeip[100],*val, *s, *s1, *s2, *s3, *b,*b1;

		/* look at callerid to see what node this comes from */
		if (!chan->cid.cid_num) /* if doesn't have caller id */
		{
			ast_log(LOG_WARNING, "Does not have callerid on %s\n",tmp);
			return -1;
		}
		/* get his IP from IAX2 module */
		memset(hisip,0,sizeof(hisip));
#ifdef ALLOW_LOCAL_CHANNELS
        /* set IP address if this is a local connection*/
        if (strncmp(chan->name,"Local",5)==0) {
            strcpy(hisip,"127.0.0.1");
        } else {
			pbx_substitute_variables_helper(chan,"${IAXPEER(CURRENTCHANNEL)}",hisip,sizeof(hisip) - 1);
		}
#else
		pbx_substitute_variables_helper(chan,"${IAXPEER(CURRENTCHANNEL)}",hisip,sizeof(hisip) - 1);
#endif

		if (!hisip[0])
		{
			ast_log(LOG_WARNING, "Link IP address cannot be determined!!\n");
			return -1;
		}

		ast_callerid_parse(chan->cid.cid_num,&b,&b1);
		ast_shrink_phone_number(b1);
		if (!strcmp(myrpt->name,b1))
		{
			ast_log(LOG_WARNING, "Trying to link to self!!\n");
			return -1;
		}

		if (*b1 < '1')
		{
			ast_log(LOG_WARNING, "Node %s Invalid for connection here!!\n",b1);
			return -1;
		}

		/* look for his reported node string */
		val = node_lookup(myrpt,b1);
		if (!val)
		{
			ast_log(LOG_WARNING, "Reported node %s cannot be found!!\n",b1);
			return -1;
		}
		strncpy(tmp,val,sizeof(tmp) - 1);
		s = tmp;
		s1 = strsep(&s,",");
		if (!strchr(s1,':') && strchr(s1,'/') && strncasecmp(s1, "local/", 6))
		{
			sy = strchr(s1,'/');
			*sy = 0;
			sprintf(sx,"%s:4569/%s",s1,sy + 1);
			s1 = sx;
		}
		s2 = strsep(&s,",");
		if (!s2)
		{
			ast_log(LOG_WARNING, "Reported node %s not in correct format!!\n",b1);
			return -1;
		}

        if (strcmp(s2,"NONE"))
		{
			hp = ast_gethostbyname(s2, &ahp);
			if (!hp)
			{
				ast_log(LOG_WARNING, "Reported node %s, name %s cannot be found!!\n",b1,s2);
				return -1;
			}
			memcpy(&ia,hp->h_addr,sizeof(in_addr_t));
#ifdef	OLD_ASTERISK
			ast_inet_ntoa(nodeip,sizeof(nodeip) - 1,ia);
#else
			strncpy(nodeip,ast_inet_ntoa(ia),sizeof(nodeip) - 1);
#endif
			s3 = strchr(hisip,':');
			if (s3) *s3 = 0;
			if (strcmp(hisip,nodeip))
			{
				s3 = strchr(s1,'@');
				if (s3) s1 = s3 + 1;
				s3 = strchr(s1,'/');
				if (s3) *s3 = 0;
				s3 = strchr(s1,':');
				if (s3) *s3 = 0;
				hp = ast_gethostbyname(s1, &ahp);
				if (!hp)
				{
					ast_log(LOG_WARNING, "Reported node %s, name %s cannot be found!!\n",b1,s1);
					return -1;
				}
				memcpy(&ia,hp->h_addr,sizeof(in_addr_t));
#ifdef	OLD_ASTERISK
				ast_inet_ntoa(nodeip,sizeof(nodeip) - 1,ia);
#else
				strncpy(nodeip,ast_inet_ntoa(ia),sizeof(nodeip) - 1);
#endif
				if (strcmp(hisip,nodeip))
				{
					ast_log(LOG_WARNING, "Node %s IP %s does not match link IP %s!!\n",b1,nodeip,hisip);
					return -1;
				}
			}
		}
	}

	/* if is not a remote */
	if (!myrpt->remote)
	{
		char *b,*b1;
		int reconnects = 0;

		rpt_mutex_lock(&myrpt->lock);
		i = myrpt->xlink;
		rpt_mutex_unlock(&myrpt->lock);
		if (i)
		{
			ast_log(LOG_WARNING, "Cannot connect to node %s, system busy\n",myrpt->name);
			return -1;
		}
		/* look at callerid to see what node this comes from */
		if (!chan->cid.cid_num) /* if doesn't have caller id */
		{
			ast_log(LOG_WARNING, "Doesn't have callerid on %s\n",tmp);
			return -1;
		}

		ast_callerid_parse(chan->cid.cid_num,&b,&b1);
		ast_shrink_phone_number(b1);
		if (!strcmp(myrpt->name,b1))
		{
			ast_log(LOG_WARNING, "Trying to link to self!!\n");
			return -1;
		}
		rpt_mutex_lock(&myrpt->lock);
		l = myrpt->links.next;
		/* try to find this one in queue */
		while(l != &myrpt->links)
		{
			if (l->name[0] == '0')
			{
				l = l->next;
				continue;
			}
			/* if found matching string */
			if (!strcmp(l->name,b1)) break;
			l = l->next;
		}
		/* if found */
		if (l != &myrpt->links)
		{
			l->killme = 1;
			l->retries = l->max_retries + 1;
			l->disced = 2;
			reconnects = l->reconnects;
			reconnects++;
                        rpt_mutex_unlock(&myrpt->lock);
			usleep(500000);
		} else
			rpt_mutex_unlock(&myrpt->lock);
		/* establish call in tranceive mode */
		l = ast_malloc(sizeof(struct rpt_link));
		if (!l)
		{
			ast_log(LOG_WARNING, "Unable to malloc\n");
			pthread_exit(NULL);
		}
		/* zero the silly thing */
		memset((char *)l,0,sizeof(struct rpt_link));
		l->parent = (void *)myrpt;
		l->mode = 1;
		strncpy(l->name,b1,MAXNODESTR - 1);
		l->isremote = 0;
		l->chan = chan;
		l->connected = 1;
		l->thisconnected = 1;
		l->hasconnected = 1;
		l->reconnects = reconnects;
		l->phonemode = phone_mode;
		l->phonevox = phone_vox;
		l->phonemonitor = phone_monitor;
		l->lastf1 = NULL;
		l->lastf2 = NULL;
		l->dtmfed = 0;
		l->gott = 0;
		l->rxlingertimer = RX_LINGER_TIME;
		l->newkeytimer = NEWKEYTIME;
		l->newkey = 0;


		if ((!phone_mode) && (l->name[0] != '0') &&
		    strncasecmp(chan->name,"echolink",8) &&
			strncasecmp(chan->name,"irlp",4)) l->newkey = 2;
		voxinit_link(l,1);
		if (!strncasecmp(chan->name,"echolink",8))
			init_linkmode(myrpt,l,LINKMODE_ECHOLINK);
		else if (!strncasecmp(chan->name,"irlp",4))
			init_linkmode(myrpt,l,LINKMODE_IRLP);
		else if (phone_mode) init_linkmode(myrpt,l,LINKMODE_PHONE);
		else init_linkmode(myrpt,l,LINKMODE_GUI);
		ast_set_read_format(l->chan,AST_FORMAT_SLINEAR);
		ast_set_write_format(l->chan,AST_FORMAT_SLINEAR);
		/* allocate a pseudo-channel thru asterisk */
		l->pchan = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
		if (!l->pchan)
		{
			fprintf(stderr,"rpt:Sorry unable to obtain pseudo channel\n");
			pthread_exit(NULL);
		}
		ast_set_read_format(l->pchan,AST_FORMAT_SLINEAR);
		ast_set_write_format(l->pchan,AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
		if (l->pchan->cdr)
			ast_set_flag(l->pchan->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
		/* make a conference for the tx */
		ci.chan = 0;
		ci.confno = myrpt->conf;
		ci.confmode = DAHDI_CONF_CONF | DAHDI_CONF_LISTENER | DAHDI_CONF_TALKER;
		/* first put the channel on the conference in proper mode */
		if (ioctl(l->pchan->fds[0],DAHDI_SETCONF,&ci) == -1)
		{
			ast_log(LOG_WARNING, "node=%s Unable to set conference mode to Announce\n",myrpt->name);
			pthread_exit(NULL);
		}
		rpt_mutex_lock(&myrpt->lock);

		/*	set radio channel squelch codes for this call/link */
		myrpt->callcode=0;
		if (codep) {
			myrpt->callcode=1;
			strncpy(l->csrxcode,codep,strlen(codep));
			strncpy(l->cstxcode,codep,strlen(codep));
			strncpy(myrpt->lasttone,codep,strlen(codep));
			if(debug>0)ast_log(LOG_NOTICE,"Node=%s Link=%s/%s set csrxcode/cstxcode=%s\n",myrpt->name,l->name,l->chan->name,l->csrxcode);
		}

		if ((phone_mode == 2) && (!phone_vox)) l->lastrealrx = 1;
		if ((phone_mode == 2) && phone_nox ) l->lastrealrx = 1;
		l->max_retries = MAX_RETRIES;
		/* insert at end of queue */
		insque((struct qelem *)l,(struct qelem *)myrpt->links.next);
		__kickshort(myrpt);
		rpt_mutex_unlock(&myrpt->lock);
		if (chan->_state != AST_STATE_UP) {
			ast_answer(chan);
			if (!phone_mode) send_newkey(chan);
		}
		if (myrpt->p.archivedir)
		{
			char str[100];

			if (l->phonemode)
				sprintf(str,"LINK(P),%s",l->name);
			else
				sprintf(str,"LINK,%s",l->name);
			donodelog(myrpt,str);
		}
		doconpgm(myrpt,l->name);
		if (!phone_mode) send_newkey(chan);
		if ((!strncasecmp(l->chan->name,"echolink",8)) ||
		    (!strncasecmp(l->chan->name,"irlp",4)))
			rpt_telemetry(myrpt,CONNECTED,l);

		if(debug)ast_log(LOG_NOTICE, "node=%s returning AST_PBX_KEEPALIVE\n",myrpt->name);
		return AST_PBX_KEEPALIVE;
	}
	/* ******************************************************************** */
	/* well, then it is a remote, so everything after this sets it up       */
	/* ******************************************************************** */
	ast_log(LOG_NOTICE, "node=%s Handle Remote\n",myrpt->name);
	rpt_mutex_lock(&myrpt->lock);
	/* if remote, error if anyone else already linked */
	if (myrpt->remoteon)
	{
		rpt_mutex_unlock(&myrpt->lock);
		usleep(500000);
		if (myrpt->remoteon)
		{
			ast_log(LOG_WARNING, "[%s] Trying to use busy link on %s\n",myrpt->name,tmp);
#ifdef	AST_CDR_FLAG_POST_DISABLED
			if (chan->cdr)
				ast_set_flag(chan->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
			return -1;
		}
		rpt_mutex_lock(&myrpt->lock);
	}
	if (myrpt->p.rptnode)
	{
		char killedit = 0;
		time_t now;

		time(&now);
		if(debug)ast_log(LOG_NOTICE, "[%s] starting remote using device %s\n",myrpt->name,myrpt->rxchanname);
		for(i = 0; i < nrpts; i++)
		{
			if (!strcasecmp(rpt_vars[i].name,myrpt->p.rptnode))
			{
				if ((rpt_vars[i].links.next != &rpt_vars[i].links) ||
				   rpt_vars[i].keyed ||
				    ((rpt_vars[i].lastkeyedtime + RPT_LOCKOUT_SECS) > now) ||
				     rpt_vars[i].txkeyed ||
				      ((rpt_vars[i].lasttxkeyedtime + RPT_LOCKOUT_SECS) > now))
				{
					rpt_mutex_unlock(&myrpt->lock);
					ast_log(LOG_WARNING, "[%s] trying to use busy link (repeater node %s) on %s\n",myrpt->name,rpt_vars[i].name,tmp);
#ifdef	AST_CDR_FLAG_POST_DISABLED
					if (chan->cdr)
						ast_set_flag(chan->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
					return -1;
				}
				if(debug)ast_log(LOG_NOTICE, "[%s] radiotaken by remote using device %s from [%s] \n",myrpt->name,myrpt->rxchanname,rpt_vars[i].name);
				rpt_vars[i].radiotaken=1;

				while(rpt_vars[i].xlink != 3)
				{
					if (!killedit)
					{
						ast_softhangup(rpt_vars[i].rxchannel,AST_SOFTHANGUP_DEV);
						rpt_vars[i].xlink = 1;
						killedit = 1;
					}
					rpt_mutex_unlock(&myrpt->lock);
					if (ast_safe_sleep(chan,500) == -1)
					{
#ifdef	AST_CDR_FLAG_POST_DISABLED
						if (chan->cdr)
							ast_set_flag(chan->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
						return -1;
					}
					rpt_mutex_lock(&myrpt->lock);
				}
				break;
			}
		}
	}

	if ( (!strcmp(myrpt->remoterig, remote_rig_rbi)||!strcmp(myrpt->remoterig, remote_rig_ppp16)) &&
	  (ioperm(myrpt->p.iobase,1,1) == -1))
	{
		rpt_mutex_unlock(&myrpt->lock);
		ast_log(LOG_WARNING, "[%s] Can't get io permission on IO port %x hex\n",myrpt->name,myrpt->p.iobase);
		return -1;
	}
	myrpt->remoteon = 1;
#ifdef	OLD_ASTERISK
	LOCAL_USER_ADD(u);
#endif
	voxinit_rpt(myrpt,1);
	rpt_mutex_unlock(&myrpt->lock);
	/* find our index, and load the vars initially */
	for(i = 0; i < nrpts; i++)
	{
		if (&rpt_vars[i] == myrpt)
		{
			load_rpt_vars(i,0);
			break;
		}
	}
	rpt_mutex_lock(&myrpt->lock);
	tele = strchr(myrpt->rxchanname,'/');
	if (!tele)
	{
		fprintf(stderr,"rpt:Dial number must be in format tech/number\n");
		rpt_mutex_unlock(&myrpt->lock);
		pthread_exit(NULL);
	}
	*tele++ = 0;
	myrpt->rxchannel = ast_request(myrpt->rxchanname,AST_FORMAT_SLINEAR,tele,NULL);
	myrpt->zaprxchannel = NULL;
	if (!strcasecmp(myrpt->rxchanname,"dahdi"))
		myrpt->zaprxchannel = myrpt->rxchannel;
	if (myrpt->rxchannel)
	{
		ast_set_read_format(myrpt->rxchannel,AST_FORMAT_SLINEAR);
		ast_set_write_format(myrpt->rxchannel,AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
		if (myrpt->rxchannel->cdr)
			ast_set_flag(myrpt->rxchannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
#ifndef	NEW_ASTERISK
		myrpt->rxchannel->whentohangup = 0;
#endif
		myrpt->rxchannel->appl = "Apprpt";
		myrpt->rxchannel->data = "(Link Rx)";
		if (option_verbose > 2)
			ast_verbose(VERBOSE_PREFIX_3 "rpt (Rx) initiating call to %s/%s on %s\n",
				myrpt->rxchanname,tele,myrpt->rxchannel->name);
		rpt_mutex_unlock(&myrpt->lock);
		ast_call(myrpt->rxchannel,tele,999);
		rpt_mutex_lock(&myrpt->lock);
	}
	else
	{
		fprintf(stderr,"%i rpt:Sorry unable to obtain Rx channel\n",__LINE__);
		rpt_mutex_unlock(&myrpt->lock);
		pthread_exit(NULL);
	}
	*--tele = '/';
	myrpt->zaptxchannel = NULL;
	if (myrpt->txchanname)
	{
		tele = strchr(myrpt->txchanname,'/');
		if (!tele)
		{
			fprintf(stderr,"rpt:Dial number must be in format tech/number\n");
			rpt_mutex_unlock(&myrpt->lock);
			ast_hangup(myrpt->rxchannel);
			pthread_exit(NULL);
		}
		*tele++ = 0;
		myrpt->txchannel = ast_request(myrpt->txchanname,AST_FORMAT_SLINEAR,tele,NULL);
		if (!strncasecmp(myrpt->txchanname,"dahdi",3))
			myrpt->zaptxchannel = myrpt->txchannel;
		if (myrpt->txchannel)
		{
			ast_set_read_format(myrpt->txchannel,AST_FORMAT_SLINEAR);
			ast_set_write_format(myrpt->txchannel,AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
			if (myrpt->txchannel->cdr)
				ast_set_flag(myrpt->txchannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
#ifndef	NEW_ASTERISK
			myrpt->txchannel->whentohangup = 0;
#endif
			myrpt->txchannel->appl = "Apprpt";
			myrpt->txchannel->data = "(Link Tx)";
			if (option_verbose > 2)
				ast_verbose(VERBOSE_PREFIX_3 "rpt (Tx) initiating call to %s/%s on %s\n",
					myrpt->txchanname,tele,myrpt->txchannel->name);
			rpt_mutex_unlock(&myrpt->lock);
			ast_call(myrpt->txchannel,tele,999);
			rpt_mutex_lock(&myrpt->lock);
		}
		else
		{
			fprintf(stderr,"rpt:Sorry unable to obtain Tx channel\n");
			rpt_mutex_unlock(&myrpt->lock);
			ast_hangup(myrpt->rxchannel);
			pthread_exit(NULL);
		}
		*--tele = '/';
	}
	else
	{
		myrpt->txchannel = myrpt->rxchannel;
		if (!strncasecmp(myrpt->rxchanname,"dahdi",3))
			myrpt->zaptxchannel = myrpt->rxchannel;
	}
	i = 3;
	ast_channel_setoption(myrpt->rxchannel,AST_OPTION_TONE_VERIFY,&i,sizeof(char),0);
	/* allocate a pseudo-channel thru asterisk */
	myrpt->pchannel = ast_request("dahdi",AST_FORMAT_SLINEAR,"pseudo",NULL);
	if (!myrpt->pchannel)
	{
		fprintf(stderr,"rpt:Sorry unable to obtain pseudo channel\n");
		rpt_mutex_unlock(&myrpt->lock);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		pthread_exit(NULL);
	}
	ast_set_read_format(myrpt->pchannel,AST_FORMAT_SLINEAR);
	ast_set_write_format(myrpt->pchannel,AST_FORMAT_SLINEAR);
#ifdef	AST_CDR_FLAG_POST_DISABLED
	if (myrpt->pchannel->cdr)
		ast_set_flag(myrpt->pchannel->cdr,AST_CDR_FLAG_POST_DISABLED);
#endif
	if (!myrpt->zaprxchannel) myrpt->zaprxchannel = myrpt->pchannel;
	if (!myrpt->zaptxchannel) myrpt->zaptxchannel = myrpt->pchannel;
	/* make a conference for the pseudo */
	ci.chan = 0;
	ci.confno = -1; /* make a new conf */
	ci.confmode = DAHDI_CONF_CONFANNMON ;
	/* first put the channel on the conference in announce/monitor mode */
	if (ioctl(myrpt->pchannel->fds[0],DAHDI_SETCONF,&ci) == -1)
	{
		ast_log(LOG_WARNING, "Unable to set conference mode to Announce\n");
		rpt_mutex_unlock(&myrpt->lock);
		ast_hangup(myrpt->pchannel);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		pthread_exit(NULL);
	}
	/* save pseudo channel conference number */
	myrpt->conf = myrpt->txconf = ci.confno;
	/* if serial io port, open it */
	myrpt->iofd = -1;
	if (myrpt->p.ioport && ((myrpt->iofd = openserial(myrpt,myrpt->p.ioport)) == -1))
	{
		rpt_mutex_unlock(&myrpt->lock);
		ast_hangup(myrpt->pchannel);
		if (myrpt->txchannel != myrpt->rxchannel)
			ast_hangup(myrpt->txchannel);
		ast_hangup(myrpt->rxchannel);
		pthread_exit(NULL);
	}
	iskenwood_pci4 = 0;
	memset(&z,0,sizeof(z));
	if ((myrpt->iofd < 1) && (myrpt->txchannel == myrpt->zaptxchannel))
	{
		z.radpar = DAHDI_RADPAR_REMMODE;
		z.data = DAHDI_RADPAR_REM_NONE;
		res = ioctl(myrpt->zaptxchannel->fds[0],DAHDI_RADIO_SETPARAM,&z);
		/* if PCIRADIO and kenwood selected */
		if ((!res) && (!strcmp(myrpt->remoterig,remote_rig_kenwood)))
		{
			z.radpar = DAHDI_RADPAR_UIOMODE;
			z.data = 1;
			if (ioctl(myrpt->zaptxchannel->fds[0],DAHDI_RADIO_SETPARAM,&z) == -1)
			{
				ast_log(LOG_ERROR,"Cannot set UIOMODE\n");
				return -1;
			}
			z.radpar = DAHDI_RADPAR_UIODATA;
			z.data = 3;
			if (ioctl(myrpt->zaptxchannel->fds[0],DAHDI_RADIO_SETPARAM,&z) == -1)
			{
				ast_log(LOG_ERROR,"Cannot set UIODATA\n");
				return -1;
			}
			i = DAHDI_OFFHOOK;
			if (ioctl(myrpt->zaptxchannel->fds[0],DAHDI_HOOK,&i) == -1)
			{
				ast_log(LOG_ERROR,"Cannot set hook\n");
				return -1;
			}
			iskenwood_pci4 = 1;
		}
	}
	if (myrpt->txchannel == myrpt->zaptxchannel)
	{
		i = DAHDI_ONHOOK;
		ioctl(myrpt->zaptxchannel->fds[0],DAHDI_HOOK,&i);
		/* if PCIRADIO and Yaesu ft897/ICOM IC-706 selected */
		if ((myrpt->iofd < 1) && (!res) &&
		   ((!strcmp(myrpt->remoterig,remote_rig_ft897)) ||
		    (!strcmp(myrpt->remoterig,remote_rig_ft950)) ||
		      (!strcmp(myrpt->remoterig,remote_rig_ic706)) ||
		         (!strcmp(myrpt->remoterig,remote_rig_tm271))))
		{
			z.radpar = DAHDI_RADPAR_UIOMODE;
			z.data = 1;
			if (ioctl(myrpt->zaptxchannel->fds[0],DAHDI_RADIO_SETPARAM,&z) == -1)
			{
				ast_log(LOG_ERROR,"Cannot set UIOMODE\n");
				return -1;
			}
			z.radpar = DAHDI_RADPAR_UIODATA;
			z.data = 3;
			if (ioctl(myrpt->zaptxchannel->fds[0],DAHDI_RADIO_SETPARAM,&z) == -1)
			{
				ast_log(LOG_ERROR,"Cannot set UIODATA\n");
				return -1;
			}
		}
	}
	myrpt->remoterx = 0;
	myrpt->remotetx = 0;
	myrpt->retxtimer = 0;
	myrpt->rerxtimer = 0;
	myrpt->remoteon = 1;
	myrpt->dtmfidx = -1;
	myrpt->dtmfbuf[0] = 0;
	myrpt->dtmf_time_rem = 0;
	myrpt->hfscanmode = 0;
	myrpt->hfscanstatus = 0;
	if (myrpt->p.startupmacro)
	{
		snprintf(myrpt->macrobuf,MAXMACRO - 1,"PPPP%s",myrpt->p.startupmacro);
	}
	time(&myrpt->start_time);
	myrpt->last_activity_time = myrpt->start_time;
	last_timeout_warning = 0;
	myrpt->reload = 0;
	myrpt->tele.next = &myrpt->tele;
	myrpt->tele.prev = &myrpt->tele;
	myrpt->newkey = 0;
	rpt_mutex_unlock(&myrpt->lock);
	ast_set_write_format(chan, AST_FORMAT_SLINEAR);
	ast_set_read_format(chan, AST_FORMAT_SLINEAR);
	rem_rx = 0;
	remkeyed = 0;
	/* if we are on 2w loop and are a remote, turn EC on */
	if (myrpt->remote && (myrpt->rxchannel == myrpt->txchannel))
	{
		i = 128;
		ioctl(myrpt->zaprxchannel->fds[0],DAHDI_ECHOCANCEL,&i);
		if(debug) ast_log(LOG_NOTICE,
						"[%s] enabled echo canceller on %s\n",
						myrpt->name,myrpt->rxchannel->name);
	}
	if (chan->_state != AST_STATE_UP) {
		ast_answer(chan);
		if (!phone_mode) send_newkey(chan);
	}

	if (myrpt->rxchannel == myrpt->zaprxchannel)
	{
		if (ioctl(myrpt->zaprxchannel->fds[0],DAHDI_GET_PARAMS,&par) != -1)
		{
			if (par.rxisoffhook)
			{
				if(debug)
					ast_log(LOG_NOTICE,
						"[%s] indicate ACRK to zap rxchannel %s\n",
						myrpt->name,myrpt->rxchannel->name);
				ast_indicate(chan,AST_CONTROL_RADIO_KEY);
				myrpt->remoterx = 1;
				remkeyed = 1;
			}
		}
	}

	nullfd = open("/dev/null",O_RDWR);

	if (myrpt->p.archivedir)
	{
		char mycmd[100],mydate[100],*b,*b1;
		time_t myt;
		long blocksleft;

		mkdir(myrpt->p.archivedir,0777);
		sprintf(mycmd,"%s/%s",myrpt->p.archivedir,myrpt->name);
		mkdir(mycmd,0777);
		time(&myt);
		strftime(mydate,sizeof(mydate) - 1,"%Y%m%d%H%M%S",
			localtime(&myt));
		sprintf(mycmd,"mixmonitor start %s %s/%s/%s.wav49 a",chan->name,
			myrpt->p.archivedir,myrpt->name,mydate);
		if (myrpt->p.monminblocks)
		{
			blocksleft = diskavail(myrpt);
			if (myrpt->p.remotetimeout)
			{
				blocksleft -= (myrpt->p.remotetimeout *
					MONITOR_DISK_BLOCKS_PER_MINUTE) / 60;
			}
			if (blocksleft >= myrpt->p.monminblocks)
				ast_cli_command(nullfd,mycmd);
		} else ast_cli_command(nullfd,mycmd);
		/* look at callerid to see what node this comes from */
		if (!chan->cid.cid_num) /* if doesn't have caller id */
		{
			b1 = "0";
		} else {
			ast_callerid_parse(chan->cid.cid_num,&b,&b1);
			ast_shrink_phone_number(b1);
		}
		sprintf(mycmd,"CONNECT,%s",b1);
		donodelog(myrpt,mycmd);
		doconpgm(myrpt,b1);
	}
	myrpt->loginuser[0] = 0;
	myrpt->loginlevel[0] = 0;
	myrpt->authtelltimer = 0;
	myrpt->authtimer = 0;
	authtold = 0;
	authreq = 0;
	if (myrpt->p.authlevel > 1) authreq = 1;
	setrem(myrpt);
	n = 0;
	dtmfed = 0;
	cs[n++] = chan;
	cs[n++] = myrpt->rxchannel;
	cs[n++] = myrpt->pchannel;
	cs[n++] = myrpt->telechannel;
    cs[n++] = myrpt->btelechannel;
	if (myrpt->rxchannel != myrpt->txchannel)
		cs[n++] = myrpt->txchannel;
	if (!phone_mode) send_newkey(chan);
	myfirst = 0;
	/* start un-locked */
	for(;;)
	{
		if (ast_check_hangup(chan)) break;
		if (ast_check_hangup(myrpt->rxchannel)) break;
		notremming = 0;
		setting = 0;
		reming = 0;
		telem = myrpt->tele.next;
		while(telem != &myrpt->tele)
		{
			if (telem->mode == SETREMOTE) setting = 1;
			if ((telem->mode == SETREMOTE) ||
			    (telem->mode == SCAN) ||
				(telem->mode == TUNE))  reming = 1;
			else notremming = 1;
			telem = telem->next;
		}
		if (myrpt->reload)
		{
			myrpt->reload = 0;
			/* find our index, and load the vars */
			for(i = 0; i < nrpts; i++)
			{
				if (&rpt_vars[i] == myrpt)
				{
					load_rpt_vars(i,0);
					break;
				}
			}
		}
		time(&t);
		if (myrpt->p.remotetimeout)
		{
			time_t r;

			r = (t - myrpt->start_time);
			if (r >= myrpt->p.remotetimeout)
			{
				saynode(myrpt,chan,myrpt->name);
				sayfile(chan,"rpt/timeout");
				ast_safe_sleep(chan,1000);
				break;
			}
			if ((myrpt->p.remotetimeoutwarning) &&
			    (r >= (myrpt->p.remotetimeout -
				myrpt->p.remotetimeoutwarning)) &&
				    (r <= (myrpt->p.remotetimeout -
				    	myrpt->p.remotetimeoutwarningfreq)))
			{
				if (myrpt->p.remotetimeoutwarningfreq)
				{
				    if ((t - last_timeout_warning) >=
					myrpt->p.remotetimeoutwarningfreq)
				    {
					time(&last_timeout_warning);
					rpt_telemetry(myrpt,TIMEOUT_WARNING,0);
				    }
				}
				else
				{
				    if (!last_timeout_warning)
				    {
					time(&last_timeout_warning);
					rpt_telemetry(myrpt,TIMEOUT_WARNING,0);
				    }
				}
			}
		}
		if (myrpt->p.remoteinacttimeout && myrpt->last_activity_time)
		{
			time_t r;

			r = (t - myrpt->last_activity_time);
			if (r >= myrpt->p.remoteinacttimeout)
			{
				saynode(myrpt,chan,myrpt->name);
				ast_safe_sleep(chan,1000);
				break;
			}
			if ((myrpt->p.remotetimeoutwarning) &&
			    (r >= (myrpt->p.remoteinacttimeout -
				myrpt->p.remotetimeoutwarning)) &&
				    (r <= (myrpt->p.remoteinacttimeout -
				    	myrpt->p.remotetimeoutwarningfreq)))
			{
				if (myrpt->p.remotetimeoutwarningfreq)
				{
				    if ((t - last_timeout_warning) >=
					myrpt->p.remotetimeoutwarningfreq)
				    {
					time(&last_timeout_warning);
					rpt_telemetry(myrpt,ACT_TIMEOUT_WARNING,0);
				    }
				}
				else
				{
				    if (!last_timeout_warning)
				    {
					time(&last_timeout_warning);
					rpt_telemetry(myrpt,ACT_TIMEOUT_WARNING,0);
				    }
				}
			}
		}

		/*  the remote mode handler for channel events */
		/*	wait for an event from a channel in the unscrambled list */

		ms = MSWAIT;
		who = ast_waitfor_n(cs,n,&ms);
		if (who == NULL) ms = 0;
		elap = MSWAIT - ms;
		if (myrpt->macrotimer) myrpt->macrotimer -= elap;
		if (myrpt->macrotimer < 0) myrpt->macrotimer = 0;
		if (!ms) continue;
		/* do local dtmf timer */
		if (myrpt->dtmf_local_timer)
		{
			if (myrpt->dtmf_local_timer > 1) myrpt->dtmf_local_timer -= elap;
			if (myrpt->dtmf_local_timer < 1) myrpt->dtmf_local_timer = 1;
		}
		if (myrpt->voxtotimer) myrpt->voxtotimer -= elap;
		if (myrpt->voxtotimer < 0) myrpt->voxtotimer = 0;
		myrx = keyed;
		if (phone_mode && phone_vox)
		{
			myrx = (!AST_LIST_EMPTY(&myrpt->rxq));
			if (myrpt->voxtotimer <= 0)
			{
				if (myrpt->voxtostate)
				{
					myrpt->voxtotimer = myrpt->p.voxtimeout_ms;
					myrpt->voxtostate = 0;
				}
				else
				{
					myrpt->voxtotimer = myrpt->p.voxrecover_ms;
					myrpt->voxtostate = 1;
				}
			}
			if (!myrpt->voxtostate)
				myrx = myrx || myrpt->wasvox ;
		}
		keyed = myrx;

		if (myrpt->rxlingertimer!=0)
		{
			myrpt->rxlingertimer -= elap;
			if (myrpt->rxlingertimer <= 0)
			{
				myrpt->rxlingertimer = 0;
				if (debug > 5) ast_log(LOG_NOTICE,
						"node=%s TIMEOUT rxlingertimer\n",
						myrpt->name);
			}
		}

		/* if no voice packets assume unkey ptt/cor condition */
		if ((myrpt->newkey == 2) && keyed && (!myrpt->rxlingertimer))
		{
			myrpt->rerxtimer = 0;
			keyed = 0;
		}

		rpt_mutex_lock(&myrpt->lock);
		do_dtmf_local(myrpt,0);
		rpt_mutex_unlock(&myrpt->lock);
		//
		rem_totx =  myrpt->dtmf_local_timer && (!phone_mode);
		rem_totx |= keyed && (!myrpt->tunerequest);
		rem_rx = (remkeyed && (!setting)) || (myrpt->tele.next != &myrpt->tele);
		if(!strcmp(myrpt->remoterig, remote_rig_ic706))
			rem_totx |= myrpt->tunerequest;

	    if((debug > 7) && rem_totx) {
	    	ast_log(LOG_NOTICE,
	    		"Set rem_totx=%i.  dtmf_local_timer=%i phone_mode=%i keyed=%i tunerequest=%i\n",
	    		rem_totx,myrpt->dtmf_local_timer,phone_mode,keyed,myrpt->tunerequest);
		}

		if (keyed && (!keyed1))
		{
			keyed1 = 1;
		}

		if (!keyed && (keyed1))
		{
			time_t myt;

			keyed1 = 0;
			time(&myt);
			/* if login necessary, and not too soon */
			if ((myrpt->p.authlevel) &&
			    (!myrpt->loginlevel[0]) &&
				(myt > (t + 3)))
			{
				authreq = 1;
				authtold = 0;
				myrpt->authtelltimer = AUTHTELLTIME - AUTHTXTIME;
			}
		}

		if (rem_rx && (!myrpt->remoterx))
		{
			myrpt->remoterx = 1;
			if(debug>6)
				ast_log(LOG_NOTICE,"node=%s rem_rx indicate ACRK \n",myrpt->name);
			ast_indicate(chan,AST_CONTROL_RADIO_KEY);
		}
		if ((!rem_rx) && (myrpt->remoterx))
		{
			myrpt->remoterx = 0;
			if(debug>6)
				ast_log(LOG_NOTICE,"[%s] rem_rx indicate ACRUK \n",myrpt->name);
			ast_indicate(chan,AST_CONTROL_RADIO_UNKEY);
		}
		/* if auth requested, and not authed yet */
		if (authreq && (!myrpt->loginlevel[0]))
		{
			if ((!authtold) && ((myrpt->authtelltimer += elap)
				 >= AUTHTELLTIME))
			{
				authtold = 1;
				rpt_telemetry(myrpt,LOGINREQ,NULL);
			}
			if ((myrpt->authtimer += elap) >= AUTHLOGOUTTIME)
			{
				break; /* if not logged in, hang up after a time */
			}
		}
		/* Only send redundant control messages to newkey == 1 links */
		if (myrpt->newkey == 1)
		{
			if ((myrpt->retxtimer += elap) >= REDUNDANT_TX_TIME)
			{
				myrpt->retxtimer = 0;
				if ((myrpt->remoterx) && (!myrpt->remotetx))
				{
					if(debug>6) ast_log(LOG_NOTICE,"[%s] REDUNDANT ACRK sent on remote\n",myrpt->name);
					ast_indicate(chan,AST_CONTROL_RADIO_KEY);
				}
				else
				{
					if(debug>6) ast_log(LOG_NOTICE,"[%s] REDUNDANT ACRUK sent on remote\n",myrpt->name);
					ast_indicate(chan,AST_CONTROL_RADIO_UNKEY);
				}
			}

			if ((myrpt->rerxtimer += elap) >= (REDUNDANT_TX_TIME * 2))
			{
				keyed = 0;
				myrpt->rerxtimer = 0;
			}
		}
		if (rem_totx && (!myrpt->remotetx))
		{
			/* if not authed, and needed, do not transmit */
			if ((!myrpt->p.authlevel) || myrpt->loginlevel[0])
			{
				if(debug > 6)
					ast_log(LOG_NOTICE,"Handle rem_totx=%i.  dtmf_local_timer=%i  tunerequest=%i\n",rem_totx,myrpt->dtmf_local_timer,myrpt->tunerequest);

				myrpt->remotetx = 1;
				if((myrpt->remtxfreqok = check_tx_freq(myrpt)))
				{
					time(&myrpt->last_activity_time);
					if ((iskenwood_pci4) && (myrpt->txchannel == myrpt->zaptxchannel))
					{
						z.radpar = DAHDI_RADPAR_UIODATA;
						z.data = 1;
						if (ioctl(myrpt->zaptxchannel->fds[0],DAHDI_RADIO_SETPARAM,&z) == -1)
						{
							ast_log(LOG_ERROR,"Cannot set UIODATA\n");
							return -1;
						}
					}
					else
					{
						if(debug>6)ast_log(LOG_NOTICE,"ACRK node=%s\n",myrpt->name);
						ast_indicate(myrpt->txchannel,AST_CONTROL_RADIO_KEY);
					}
					if (myrpt->p.archivedir) donodelog(myrpt,"TXKEY");
				}
			}
		}
		if ((!rem_totx) && myrpt->remotetx) /* Remote base radio TX unkey */
		{
			myrpt->remotetx = 0;
			if(!myrpt->remtxfreqok){
				rpt_telemetry(myrpt,UNAUTHTX,NULL);
			}
			if ((iskenwood_pci4) && (myrpt->txchannel == myrpt->zaptxchannel))
			{
				z.radpar = DAHDI_RADPAR_UIODATA;
				z.data = 3;
				if (ioctl(myrpt->zaptxchannel->fds[0],DAHDI_RADIO_SETPARAM,&z) == -1)
				{
					ast_log(LOG_ERROR,"Cannot set UIODATA\n");
					return -1;
				}
			}
			else
			{
				ast_indicate(myrpt->txchannel,AST_CONTROL_RADIO_UNKEY);
			}
			if (myrpt->p.archivedir) donodelog(myrpt,"TXUNKEY");
		}
		if (myrpt->hfscanmode){
			myrpt->scantimer -= elap;
			if(myrpt->scantimer <= 0){
				if (!reming)
				{
					myrpt->scantimer = REM_SCANTIME;
					rpt_telemetry(myrpt,SCAN,0);
				} else myrpt->scantimer = 1;
			}
		}
		rpt_mutex_lock(&myrpt->lock);
		c = myrpt->macrobuf[0];
		if (c && (!myrpt->macrotimer))
		{
			myrpt->macrotimer = MACROTIME;
			memmove(myrpt->macrobuf,myrpt->macrobuf + 1,MAXMACRO - 1);
			if ((c == 'p') || (c == 'P'))
				myrpt->macrotimer = MACROPTIME;
			rpt_mutex_unlock(&myrpt->lock);
			if (myrpt->p.archivedir)
			{
				char str[100];
					sprintf(str,"DTMF(M),%c",c);
				donodelog(myrpt,str);
			}
			if (handle_remote_dtmf_digit(myrpt,c,&keyed,0) == -1) break;
			continue;
		} else rpt_mutex_unlock(&myrpt->lock);
		if (who == chan) /* if it was a read from incoming */
		{
			f = ast_read(chan);
			if (!f)
			{
				if (debug) ast_log(LOG_NOTICE,"node=%s Hung Up\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_VOICE)
			{
				/*
				if (!keyed)
				{
					// ??? !!! 20090511
					keyed = 1;
					myrpt->rerxtimer = 0;
					ast_log(LOG_NOTICE,"node=%s keyed on voice\n",myrpt->name);
					myrpt->rxlingertimer = RX_LINGER_TIME;
				}
				*/

				if (myrpt->newkey == 2)
				{
					myrpt->rxlingertimer = RX_LINGER_TIME;
					if (!keyed)
					{
						keyed = 1;
						myrpt->rerxtimer = 0;
						if (debug>5) ast_log(LOG_NOTICE,"node=%s keyed on voice\n",myrpt->name);
					}
				}
				if (phone_mode && phone_vox)
				{
					n1 = dovox(&myrpt->vox,
						f->data,f->datalen / 2);
					if (n1 != myrpt->wasvox)
					{
						if (debug) ast_log(LOG_DEBUG,"Remote VOX to %d\n",n1);
						myrpt->wasvox = n1;
						myrpt->voxtostate = 0;
						if (n1) myrpt->voxtotimer = myrpt->p.voxtimeout_ms;
						else myrpt->voxtotimer = 0;
					}
					if (n1)
					{
						if (!myfirst)
						{
						    x = 0;
						    AST_LIST_TRAVERSE(&myrpt->rxq, f1,
							frame_list) x++;
						    for(;x < myrpt->p.simplexphonedelay; x++)
						    {
							f1 = ast_frdup(f);
							memset(f1->data,0,f1->datalen);
							memset(&f1->frame_list,0,sizeof(f1->frame_list));
							AST_LIST_INSERT_TAIL(&myrpt->rxq,
								f1,frame_list);
						    }
						    myfirst = 1;
						}
						f1 = ast_frdup(f);
						memset(&f1->frame_list,0,sizeof(f1->frame_list));
						AST_LIST_INSERT_TAIL(&myrpt->rxq,f1,frame_list);
					} else myfirst = 0;
					x = 0;
					AST_LIST_TRAVERSE(&myrpt->rxq, f1,frame_list) x++;
					if (!x)
					{
						memset(f->data,0,f->datalen);
					}
					else
					{
						ast_frfree(f);
						f = AST_LIST_REMOVE_HEAD(&myrpt->rxq,frame_list);
					}
				}
				if (ioctl(chan->fds[0], DAHDI_GETCONFMUTE, &ismuted) == -1)
				{
					ismuted = 0;
				}
				/* if not transmitting, zero-out audio */
				ismuted |= (!myrpt->remotetx);
				if (dtmfed && phone_mode) ismuted = 1;
				dtmfed = 0;
				if (ismuted)
				{
					memset(f->data,0,f->datalen);
					if (myrpt->lastf1)
						memset(myrpt->lastf1->data,0,myrpt->lastf1->datalen);
					if (myrpt->lastf2)
						memset(myrpt->lastf2->data,0,myrpt->lastf2->datalen);
				}
				if (f) f2 = ast_frdup(f);
				else f2 = NULL;
				f1 = myrpt->lastf2;
				myrpt->lastf2 = myrpt->lastf1;
				myrpt->lastf1 = f2;
				if (ismuted)
				{
					if (myrpt->lastf1)
						memset(myrpt->lastf1->data,0,myrpt->lastf1->datalen);
					if (myrpt->lastf2)
						memset(myrpt->lastf2->data,0,myrpt->lastf2->datalen);
				}
				if (f1)
				{
					if (phone_mode)
						ast_write(myrpt->txchannel,f1);
					else
						ast_write(myrpt->txchannel,f);
					ast_frfree(f1);
				}
			}
#ifndef	OLD_ASTERISK
			else if (f->frametype == AST_FRAME_DTMF_BEGIN)
			{
				if (myrpt->lastf1)
					memset(myrpt->lastf1->data,0,myrpt->lastf1->datalen);
				if (myrpt->lastf2)
					memset(myrpt->lastf2->data,0,myrpt->lastf2->datalen);
				dtmfed = 1;
			}
#endif
			if (f->frametype == AST_FRAME_DTMF)
			{
				if (myrpt->lastf1)
					memset(myrpt->lastf1->data,0,myrpt->lastf1->datalen);
				if (myrpt->lastf2)
					memset(myrpt->lastf2->data,0,myrpt->lastf2->datalen);
				dtmfed = 1;
				if (handle_remote_phone_dtmf(myrpt,f->subclass,&keyed,phone_mode) == -1)
				{
					if (debug) ast_log(LOG_NOTICE,"node=%s Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			if (f->frametype == AST_FRAME_TEXT)
			{
				if( debug>5 )
					ast_log(LOG_NOTICE,
						"node=%s read from incoming got TEXT=%s\n",
						myrpt->name,f->datalen?(char *)f->data:"");

				if (handle_remote_data(myrpt,f->data) == -1)
				{
					if (debug) ast_log(LOG_NOTICE,"node=%s Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
				if( debug>5 )
					ast_log(LOG_NOTICE,
						"node=%s read from incoming got AFC=%i data=%s\n",
						myrpt->name,f->subclass,f->datalen?(char *)f->data:"");

				if ( f->subclass == AST_CONTROL_HANGUP )
				{
					if ( debug>5 ) ast_log(LOG_NOTICE,"node=%s Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
				/* if RX key */
				if ( f->subclass == AST_CONTROL_RADIO_KEY )
				{
					if ( debug>1 )
						ast_log(LOG_NOTICE,"[%s] read from incoming got ACRK data=%s\n",
							myrpt->name,(char *)f->data);

					keyed = 1;
					myrpt->rerxtimer = 0;
				}
				/* if RX un-key */
				if (f->subclass == AST_CONTROL_RADIO_UNKEY)
				{
					if(debug>1)
						ast_log(LOG_NOTICE,"[%s] read from incoming got ACRUK data=%s\n",
							myrpt->name,(char *)f->data);
					myrpt->rerxtimer = 0;
					keyed = 0;
				}
			}
			ast_frfree(f);
			continue;
		}
		if (who == myrpt->rxchannel) /* if it was a read from the radio channel */
		{
			f = ast_read(myrpt->rxchannel);
			if (!f)
			{
				if (debug) ast_log(LOG_NOTICE,"node=%s Hung Up\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_VOICE)
			{
				int myreming = 0;

				if(!strcmp(myrpt->remoterig, remote_rig_kenwood))
					myreming = reming;

				if (myreming || (!remkeyed) ||
				((myrpt->remote) && (myrpt->remotetx)) ||
				  ((myrpt->remmode != REM_MODE_FM) &&
				    notremming))
					memset(f->data,0,f->datalen);
				 ast_write(myrpt->pchannel,f);
			}
			else if ( f->frametype == AST_FRAME_CONTROL )
			{
				if ( debug>4 )
					ast_log(LOG_NOTICE,
						"node=%s rxchannel AFC=%i data=%s\n",
						myrpt->name,f->subclass,f->datalen?(char *)f->data:"");

				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug>4 ) ast_log(LOG_NOTICE,"node=%s Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
				/* if RX key */
				if (f->subclass == AST_CONTROL_RADIO_KEY)
				{
					if (debug)
						ast_log(LOG_NOTICE,
							"node=%s rxchannel ACRK data=%s\n",
							myrpt->name,(char *)f->data);

					if (!myrpt->remotetx)
					{
						remkeyed = 1;
					}
				}
				/* if RX un-key */
				if (f->subclass == AST_CONTROL_RADIO_UNKEY)
				{
					if (debug)
						ast_log(LOG_NOTICE,
							"[%s] rxchannel ACRUK data=%s\n",
							myrpt->name,(char *)f->data);

					if (!myrpt->remotetx)
					{
						remkeyed = 0;
					}
				}
			}
			ast_frfree(f);
			continue;
		}

	    /* Handle telemetry conference output */
		if (who == myrpt->telechannel) /* if is telemetry conference output */
		{
			f = ast_read(myrpt->telechannel);
			if (!f)
			{
				if (debug) ast_log(LOG_NOTICE,"node=%s telechannel Hung Up implied\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_VOICE)
			{
				float gain;

				if(myrpt->keyed)
					gain = myrpt->p.telemduckgain;
				else
					gain = myrpt->p.telemnomgain;

				if(gain)
				{
					int n,k;
					short *sp = (short *) f->data;
					for(n=0; n<f->datalen/2; n++)
					{
						k=sp[n]*gain;
						if (k > 32767) k = 32767;
						else if (k < -32767) k = -32767;
						sp[n]=k;
					}
				}
				ast_write(myrpt->btelechannel,f);
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug>5 ) ast_log(LOG_NOTICE,"node=%s telechannel Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}
		/* if is btelemetry conference output */
		if (who == myrpt->btelechannel)
		{
			f = ast_read(myrpt->btelechannel);
			if (!f)
			{
				if (debug) ast_log(LOG_NOTICE,"node=%s btelechannel Hung Up implied\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug>5 ) ast_log(LOG_NOTICE,"node=%s btelechannel Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}

		if (who == myrpt->pchannel) /* if is remote mix output */
		{
			f = ast_read(myrpt->pchannel);
			if (!f)
			{
				if (debug) ast_log(LOG_NOTICE,"node=%s pchannel Hung Up implied\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_VOICE)
			{
				if ((myrpt->newkey < 2) || myrpt->remoterx ||
				    strncasecmp(chan->name,"IAX",3))
					ast_write(chan,f);
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug>5 ) ast_log(LOG_NOTICE,"node=%s pchannel Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}
		if ((myrpt->rxchannel != myrpt->txchannel) &&
			(who == myrpt->txchannel)) /* do this cuz you have to */
		{
			f = ast_read(myrpt->txchannel);
			if (!f)
			{
				if ( debug>4 ) ast_log(LOG_NOTICE,"node=%s Hang Up Implied\n",myrpt->name);
				break;
			}
			if (f->frametype == AST_FRAME_CONTROL)
			{
 				if (f->subclass == AST_CONTROL_HANGUP)
				{
					if ( debug>4 ) ast_log(LOG_NOTICE,"node=%s Hung Up\n",myrpt->name);
					ast_frfree(f);
					break;
				}
			}
			ast_frfree(f);
			continue;
		}
	}
	if (debug)ast_log(LOG_NOTICE,"[%s] exit remote loop\n",myrpt->name);

	/* 	***********************************************
				end of rpt_exec() forever loop
		*********************************************** */

	if (myrpt->p.archivedir || myrpt->p.discpgm)
	{
		char mycmd[100],*b,*b1;

		/* look at callerid to see what node this comes from */
		if (!chan->cid.cid_num) /* if doesn't have caller id */
		{
			b1 = "0";
		} else {
			ast_callerid_parse(chan->cid.cid_num,&b,&b1);
			ast_shrink_phone_number(b1);
		}
		sprintf(mycmd,"DISCONNECT,%s",b1);
		if (myrpt->p.archivedir) donodelog(myrpt,mycmd);
		dodispgm(myrpt,b1);
	}
	/* wait for telem to be done */
	while(myrpt->tele.next != &myrpt->tele) usleep(100000);
	sprintf(tmp,"mixmonitor stop %s",chan->name);
    ast_cli_command(nullfd,tmp);

	if (nullfd != -1) close(nullfd);

	rpt_mutex_lock(&myrpt->lock);
	myrpt->hfscanmode = 0;
	myrpt->hfscanstatus = 0;
	myrpt->remoteon = 0;
	rpt_mutex_unlock(&myrpt->lock);
	if (myrpt->lastf1) ast_frfree(myrpt->lastf1);
	myrpt->lastf1 = NULL;
	if (myrpt->lastf2) ast_frfree(myrpt->lastf2);
	myrpt->lastf2 = NULL;
	if ((iskenwood_pci4) && (myrpt->txchannel == myrpt->zaptxchannel))
	{
		z.radpar = DAHDI_RADPAR_UIOMODE;
		z.data = 3;
		if (ioctl(myrpt->zaptxchannel->fds[0],DAHDI_RADIO_SETPARAM,&z) == -1)
		{
			ast_log(LOG_ERROR,"Cannot set UIOMODE\n");
			return -1;
		}
		z.radpar = DAHDI_RADPAR_UIODATA;
		z.data = 3;
		if (ioctl(myrpt->zaptxchannel->fds[0],DAHDI_RADIO_SETPARAM,&z) == -1)
		{
			ast_log(LOG_ERROR,"Cannot set UIODATA\n");
			return -1;
		}
		i = DAHDI_OFFHOOK;
		if (ioctl(myrpt->zaptxchannel->fds[0],DAHDI_HOOK,&i) == -1)
		{
			ast_log(LOG_ERROR,"Cannot set hook\n");
			return -1;
		}
	}
	if (myrpt->iofd) close(myrpt->iofd);
	myrpt->iofd = -1;
	ast_hangup(myrpt->pchannel);
	if (myrpt->rxchannel != myrpt->txchannel) ast_hangup(myrpt->txchannel);
	ast_hangup(myrpt->rxchannel);
	closerem(myrpt);
	if (myrpt->p.rptnode)
	{
		rpt_mutex_lock(&myrpt->lock);
		for(i = 0; i < nrpts; i++)
		{
			if (!strcasecmp(rpt_vars[i].name,myrpt->p.rptnode))
			{
				rpt_vars[i].xlink = 0;
				rpt_vars[i].radiotaken=0;
				break;
			}
		}
		rpt_mutex_unlock(&myrpt->lock);
	}
#ifdef	OLD_ASTERISK
	LOCAL_USER_REMOVE(u);
#endif
	if(debug)ast_log(LOG_NOTICE, "[%s] Remote call returning \n",myrpt->name);

	return res;
}

#ifndef OLD_ASTERISK
/*!\brief callback to display list of locally configured nodes
   \addtogroup Group_AMI
 */
static int manager_rpt_local_nodes(struct mansession *s, const struct message *m)
{
    int i;
    astman_append(s, "<?xml version=\"1.0\"?>\r\n");
    astman_append(s, "<nodes>\r\n");
    for (i=0; i< nrpts; i++)
    {
        astman_append(s, "  <node>%s</node>\r\n", rpt_vars[i].name);
    } /* for i */
    astman_append(s, "</nodes>\r\n");
    astman_append(s, "\r\n");	/* Properly terminate Manager output */
    return RESULT_SUCCESS;
} /* manager_rpt_local_nodes() */



/*
 * Append Success and ActionID to manager response message
 */

static void rpt_manager_success(struct mansession *s, const struct message *m)
{
	const char *id = astman_get_header(m, "ActionID");
	if (!ast_strlen_zero(id))
		astman_append(s, "ActionID: %s\r\n", id);
	astman_append(s, "Response: Success\r\n");
}



static int rpt_manager_do_sawstat(struct mansession *ses, const struct message *m, char *str)
{
	int i;
	struct rpt_link *l;
	const char *node = astman_get_header(m, "Node");
	time_t now;

	time(&now);
	for(i = 0; i < nrpts; i++)
	{
		if ((node)&&(!strcmp(node,rpt_vars[i].name))){
			rpt_manager_success(ses,m);
			astman_append(ses,"Node: %s\r\n",node);

			rpt_mutex_lock(&rpt_vars[i].lock); /* LOCK */

			l = rpt_vars[i].links.next;
			while(l && (l != &rpt_vars[i].links)){
				if (l->name[0] == '0'){ // Skip '0' nodes 
					l = l->next;
					continue;
				}
				astman_append(ses, "Conn: %s %d %d %d\r\n",l->name,l->lastrx1,
					(l->lastkeytime) ? (int)(now - l->lastkeytime) : -1,
					(l->lastunkeytime) ? (int)(now - l->lastunkeytime) : -1);
				l = l->next;
			}
			rpt_mutex_unlock(&rpt_vars[i].lock); // UNLOCK 
			astman_append(ses, "\r\n");
			return(0);
		}
	}
	astman_send_error(ses, m, "RptStatus unknown or missing node");
	return 0;
}


static int rpt_manager_do_xstat(struct mansession *ses, const struct message *m, char *str)
{
	int i,j;
	char ns;
	char lbuf[MAXLINKLIST],*strs[MAXLINKLIST];
	struct rpt *myrpt;
	struct ast_var_t *newvariable;
	char *connstate;
	struct rpt_link *l;
	struct rpt_lstat *s,*t;
	struct rpt_lstat s_head;
	const char *node = astman_get_header(m, "Node");

	s = NULL;
	s_head.next = &s_head;
	s_head.prev = &s_head;


	char *parrot_ena, *sys_ena, *tot_ena, *link_ena, *patch_ena, *patch_state;
	char *sch_ena, *user_funs, *tail_type, *iconns, *tot_state, *ider_state, *tel_mode; 

	for(i = 0; i < nrpts; i++)
	{
		if ((node)&&(!strcmp(node,rpt_vars[i].name))){
			rpt_manager_success(ses,m);
			astman_append(ses,"Node: %s\r\n",node);

			/* Make a copy of all stat variables while locked */
			myrpt = &rpt_vars[i];
			rpt_mutex_lock(&myrpt->lock); /* LOCK */

//### GET RPT STATUS STATES WHILE LOCKED ########################
			if(myrpt->p.parrotmode)
				parrot_ena = "1";	//"ENABLED";
			else
				parrot_ena = "0";	//"DISABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].txdisable)
				sys_ena = "0";	//"DISABLED";
			else
				sys_ena = "1";	//"ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].totdisable)
				tot_ena = "0";	//"DISABLED";
			else
				tot_ena = "1";	//"ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].linkfundisable)
				link_ena = "0";	//"DISABLED";
			else
				link_ena = "1";	//"ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].autopatchdisable)
				patch_ena = "0";	//"DISABLED";
			else
				patch_ena = "1";	//"ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].schedulerdisable)
				sch_ena = "0";	//"DISABLED";
			else
				sch_ena = "1";	//"ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].userfundisable)
				user_funs = "0";	//"DISABLED";
			else
				user_funs = "1";	//"ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].alternatetail)
				tail_type = "1";	//"ALTERNATE";
			else
				tail_type = "0";	//"STANDARD";

			if(myrpt->p.s[myrpt->p.sysstate_cur].noincomingconns)
				iconns = "0";	//"DISABLED";
			else
				iconns = "1";	//"ENABLED";

			if(!myrpt->totimer)
				tot_state = "0";	//"TIMED OUT!";
			else if(myrpt->totimer != myrpt->p.totime)
				tot_state = "1";	//"ARMED";
			else
				tot_state = "2";	//"RESET";

			if(myrpt->tailid)
				ider_state = "0";	//"QUEUED IN TAIL";
			else if(myrpt->mustid)
				ider_state = "1";	//"QUEUED FOR CLEANUP";
			else
				ider_state = "2";	//"CLEAN";


			switch(myrpt->callmode){
				case 1:
					patch_state = "0";		//"DIALING";
					break;
				case 2:
					patch_state = "1";		//"CONNECTING";
					break;
				case 3:
					patch_state = "2";		//"UP";
					break;

				case 4:
					patch_state = "3";		//"CALL FAILED";
					break;

				default:
					patch_state = "4";		//"DOWN";
			}


			if (myrpt->p.telemdynamic)
			{
				if(myrpt->telemmode == 0x7fffffff)
					tel_mode = "1";
				else if(myrpt->telemmode == 0x00)
					tel_mode = "0";
				else
					tel_mode = "2";
			}
			else{
				tel_mode= "3";
			}


//### GET CONNECTED NODE INFO ####################
			// Traverse the list of connected nodes 

			__mklinklist(myrpt,NULL,lbuf);

			j = 0;
			l = myrpt->links.next;
			while(l && (l != &myrpt->links)){
				if (l->name[0] == '0'){ // Skip '0' nodes 
					l = l->next;
					continue;
				}
				if((s = (struct rpt_lstat *) ast_malloc(sizeof(struct rpt_lstat))) == NULL){
					ast_log(LOG_ERROR, "Malloc failed in rpt_do_lstats\r\n");
					rpt_mutex_unlock(&myrpt->lock); // UNLOCK 
					return -1;
				}
				memset(s, 0, sizeof(struct rpt_lstat));
				strncpy(s->name, l->name, MAXREMSTR - 1);
				if (l->chan) pbx_substitute_variables_helper(l->chan, "${IAXPEER(CURRENTCHANNEL)}", s->peer, MAXPEERSTR - 1);
				else strcpy(s->peer,"(none)");
				s->mode = l->mode;
				s->outbound = l->outbound;
				s->reconnects = l->reconnects;
				s->connecttime = l->connecttime;
				s->thisconnected = l->thisconnected;
				memcpy(s->chan_stat,l->chan_stat,NRPTSTAT * sizeof(struct rpt_chan_stat));
				insque((struct qelem *) s, (struct qelem *) s_head.next);
				memset(l->chan_stat,0,NRPTSTAT * sizeof(struct rpt_chan_stat));
				l = l->next;
			}
			rpt_mutex_unlock(&myrpt->lock); // UNLOCK 
			for(s = s_head.next; s != &s_head; s = s->next){
				int hours, minutes, seconds;
				long long connecttime = s->connecttime;
				char conntime[21];
				hours = (int) connecttime/3600000;
				connecttime %= 3600000;
				minutes = (int) connecttime/60000;
				connecttime %= 60000;
				seconds = (int)  connecttime/1000;
				connecttime %= 1000;
				snprintf(conntime, 20, "%02d:%02d:%02d",
					hours, minutes, seconds);
				conntime[20] = 0;
				if(s->thisconnected)
					connstate  = "ESTABLISHED";
				else
					connstate = "CONNECTING";
				astman_append(ses, "Conn: %-10s%-20s%-12d%-11s%-20s%-20s\r\n",
					s->name, s->peer, s->reconnects, (s->outbound)? "OUT":"IN", conntime, connstate);
			}	
			// destroy our local link queue 
			s = s_head.next;
			while(s != &s_head){
				t = s;
				s = s->next;
				remque((struct qelem *)t);
				ast_free(t);
			}	

			astman_append(ses,"LinkedNodes: ");
//### GET ALL LINKED NODES INFO ####################
			/* parse em */
			ns = finddelim(lbuf,strs,MAXLINKLIST);
			/* sort em */
			if (ns) qsort((void *)strs,ns,sizeof(char *),mycompar);
			for(j = 0 ;; j++){
				if(!strs[j]){
					if(!j){
						astman_append(ses,"<NONE>");
					}
					break;
				}
				astman_append(ses, "%s", strs[j]);
				if(strs[j + 1])
					astman_append(ses, ", ");

			}
			astman_append(ses,"\r\n");

//### GET VARIABLES INFO ####################
			j = 0;
			ast_channel_lock(rpt_vars[i].rxchannel);
			AST_LIST_TRAVERSE (&rpt_vars[i].rxchannel->varshead, newvariable, entries) {
				j++;
				astman_append(ses,"Var: %s=%s\r\n", ast_var_name(newvariable), ast_var_value(newvariable));
			}
			ast_channel_unlock(rpt_vars[i].rxchannel);

//### OUTPUT RPT STATUS STATES ##############
			astman_append(ses, "parrot_ena: %s\r\n", parrot_ena);
			astman_append(ses, "sys_ena: %s\r\n", sys_ena);
			astman_append(ses, "tot_ena: %s\r\n", tot_ena);
			astman_append(ses, "link_ena: %s\r\n", link_ena);
			astman_append(ses, "patch_ena: %s\r\n", patch_ena);
			astman_append(ses, "patch_state: %s\r\n", patch_state);
			astman_append(ses, "sch_ena: %s\r\n", sch_ena);
			astman_append(ses, "user_funs: %s\r\n", user_funs);
			astman_append(ses, "tail_type: %s\r\n", tail_type);
			astman_append(ses, "iconns: %s\r\n", iconns);
			astman_append(ses, "tot_state: %s\r\n", tot_state);
			astman_append(ses, "ider_state: %s\r\n", ider_state);
			astman_append(ses, "tel_mode: %s\r\n\r\n", tel_mode);

			return 0;
		}
	}
	astman_send_error(ses, m, "RptStatus unknown or missing node");
	return 0;
}

/*
* Dump statistics to manager session
*/

static int rpt_manager_do_stats(struct mansession *s, const struct message *m, char *str)
{
	int i,j,numoflinks;
	int dailytxtime, dailykerchunks;
	time_t now;
	int totalkerchunks, dailykeyups, totalkeyups, timeouts;
	int totalexecdcommands, dailyexecdcommands, hours, minutes, seconds;
	long long totaltxtime;
	struct	rpt_link *l;
	char *listoflinks[MAX_STAT_LINKS];
	char *lastdtmfcommand,*parrot_ena;
	char *tot_state, *ider_state, *patch_state;
	char *reverse_patch_state, *sys_ena, *tot_ena, *link_ena, *patch_ena;
	char *sch_ena, *input_signal, *called_number, *user_funs, *tail_type;
	char *transmitterkeyed;
	const char *node = astman_get_header(m, "Node");
	struct rpt *myrpt;

	static char *not_applicable = "N/A";

	tot_state = ider_state =
	patch_state = reverse_patch_state =
	input_signal = not_applicable;
	called_number = lastdtmfcommand = transmitterkeyed = NULL;

	time(&now);
	for(i = 0; i < nrpts; i++)
	{
		if ((node)&&(!strcmp(node,rpt_vars[i].name))){
			rpt_manager_success(s,m);

			myrpt = &rpt_vars[i];

			if(myrpt->remote){ /* Remote base ? */
				char *loginuser, *loginlevel, *freq, *rxpl, *txpl, *modestr;
				char offset = 0,powerlevel = 0,rxplon = 0,txplon = 0,remoteon,remmode = 0,reportfmstuff;
				char offsetc,powerlevelc;

				loginuser = loginlevel = freq = rxpl = txpl = NULL;
				/* Make a copy of all stat variables while locked */
				rpt_mutex_lock(&myrpt->lock); /* LOCK */
				if((remoteon = myrpt->remoteon)){
					if(!ast_strlen_zero(myrpt->loginuser))
						loginuser = ast_strdup(myrpt->loginuser);
					if(!ast_strlen_zero(myrpt->loginlevel))
						loginlevel = ast_strdup(myrpt->loginlevel);
					if(!ast_strlen_zero(myrpt->freq))
						freq = ast_strdup(myrpt->freq);
					if(!ast_strlen_zero(myrpt->rxpl))
						rxpl = ast_strdup(myrpt->rxpl);
					if(!ast_strlen_zero(myrpt->txpl))
						txpl = ast_strdup(myrpt->txpl);
					remmode = myrpt->remmode;
					offset = myrpt->offset;
					powerlevel = myrpt->powerlevel;
					rxplon = myrpt->rxplon;
					txplon = myrpt->txplon;
				}
				rpt_mutex_unlock(&myrpt->lock); /* UNLOCK */
				astman_append(s, "IsRemoteBase: YES\r\n");
				astman_append(s, "RemoteOn: %s\r\n",(remoteon) ? "YES": "NO");
				if(remoteon){
					if(loginuser){
						astman_append(s, "LogInUser: %s\r\n", loginuser);
						ast_free(loginuser);
					}
					if(loginlevel){
						astman_append(s, "LogInLevel: %s\r\n", loginlevel);
						ast_free(loginlevel);
					}
					if(freq){
						astman_append(s, "Freq: %s\r\n", freq);
						ast_free(freq);
					}
					reportfmstuff = 0;
					switch(remmode){
						case REM_MODE_FM:
							modestr = "FM";
							reportfmstuff = 1;
							break;
						case REM_MODE_AM:
							modestr = "AM";
							break;
						case REM_MODE_USB:
							modestr = "USB";
							break;
						default:
							modestr = "LSB";
							break;
					}
					astman_append(s, "RemMode: %s\r\n", modestr);
					if(reportfmstuff){
						switch(offset){
							case REM_SIMPLEX:
								offsetc = 'S';
								break;
							case REM_MINUS:
								offsetc = '-';
								break;
							case REM_WILD:
								offsetc = 'w';
								break;
							default:
								offsetc = '+';
								break;
						}
						astman_append(s, "RemOffset: %c\r\n", offsetc);
						if(rxplon && rxpl){
							astman_append(s, "RxPl: %s\r\n",rxpl);
							free(rxpl);
						}
						if(txplon && txpl){
							astman_append(s, "TxPl: %s\r\n",txpl);
							free(txpl);
						}
					}
					switch(powerlevel){
						case REM_LOWPWR:
							powerlevelc = 'L';
							break;
						case REM_MEDPWR:
							powerlevelc = 'M';
							break;
						default:
							powerlevelc = 'H';
							break;
					}
					astman_append(s,"PowerLevel: %c\r\n", powerlevelc);
				}
				astman_append(s, "\r\n");
				return 0; /* End of remote base status reporting */
			}

			/* ELSE Process as a repeater node */
			/* Make a copy of all stat variables while locked */
			rpt_mutex_lock(&myrpt->lock); /* LOCK */
			dailytxtime = myrpt->dailytxtime;
			totaltxtime = myrpt->totaltxtime;
			dailykeyups = myrpt->dailykeyups;
			totalkeyups = myrpt->totalkeyups;
			dailykerchunks = myrpt->dailykerchunks;
			totalkerchunks = myrpt->totalkerchunks;
			dailyexecdcommands = myrpt->dailyexecdcommands;
			totalexecdcommands = myrpt->totalexecdcommands;
			timeouts = myrpt->timeouts;


			/* Traverse the list of connected nodes */
			reverse_patch_state = "DOWN";
			numoflinks = 0;
			l = myrpt->links.next;
			while(l && (l != &myrpt->links)){
				if(numoflinks >= MAX_STAT_LINKS){
					ast_log(LOG_NOTICE,
					"maximum number of links exceeds %d in rpt_do_stats()!",MAX_STAT_LINKS);
					break;
				}
				if (l->name[0] == '0'){ /* Skip '0' nodes */
					reverse_patch_state = "UP";
					l = l->next;
					continue;
				}
				listoflinks[numoflinks] = ast_strdup(l->name);
				if(listoflinks[numoflinks] == NULL){
					break;
				}
				else{
					numoflinks++;
				}
				l = l->next;
			}

			if(myrpt->keyed)
				input_signal = "YES";
			else
				input_signal = "NO";

			if(myrpt->txkeyed)
				transmitterkeyed = "YES";
			else
				transmitterkeyed = "NO";

			if(myrpt->p.parrotmode)
				parrot_ena = "ENABLED";
			else
				parrot_ena = "DISABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].txdisable)
				sys_ena = "DISABLED";
			else
				sys_ena = "ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].totdisable)
				tot_ena = "DISABLED";
			else
				tot_ena = "ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].linkfundisable)
				link_ena = "DISABLED";
			else
				link_ena = "ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].autopatchdisable)
				patch_ena = "DISABLED";
			else
				patch_ena = "ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].schedulerdisable)
				sch_ena = "DISABLED";
			else
				sch_ena = "ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].userfundisable)
				user_funs = "DISABLED";
			else
				user_funs = "ENABLED";

			if(myrpt->p.s[myrpt->p.sysstate_cur].alternatetail)
				tail_type = "ALTERNATE";
			else
				tail_type = "STANDARD";

			if(!myrpt->totimer)
				tot_state = "TIMED OUT!";
			else if(myrpt->totimer != myrpt->p.totime)
				tot_state = "ARMED";
			else
				tot_state = "RESET";

			if(myrpt->tailid)
				ider_state = "QUEUED IN TAIL";
			else if(myrpt->mustid)
				ider_state = "QUEUED FOR CLEANUP";
			else
				ider_state = "CLEAN";

			switch(myrpt->callmode){
				case 1:
					patch_state = "DIALING";
					break;
				case 2:
					patch_state = "CONNECTING";
					break;
				case 3:
					patch_state = "UP";
					break;

				case 4:
					patch_state = "CALL FAILED";
					break;

				default:
					patch_state = "DOWN";
			}

			if(strlen(myrpt->exten)){
				called_number = ast_strdup(myrpt->exten);
			}

			if(strlen(myrpt->lastdtmfcommand)){
				lastdtmfcommand = ast_strdup(myrpt->lastdtmfcommand);
			}
			rpt_mutex_unlock(&myrpt->lock); /* UNLOCK */

			astman_append(s, "IsRemoteBase: NO\r\n");
			astman_append(s, "NodeState: %d\r\n", myrpt->p.sysstate_cur);
			astman_append(s, "SignalOnInput: %s\r\n", input_signal);
			astman_append(s, "TransmitterKeyed: %s\r\n", transmitterkeyed);
			astman_append(s, "Transmitter: %s\r\n", sys_ena);
			astman_append(s, "Parrot: %s\r\n", parrot_ena);
			astman_append(s, "Scheduler: %s\r\n", sch_ena);
			astman_append(s, "TailLength: %s\r\n", tail_type);
			astman_append(s, "TimeOutTimer: %s\r\n", tot_ena);
			astman_append(s, "TimeOutTimerState: %s\r\n", tot_state);
			astman_append(s, "TimeOutsSinceSystemInitialization: %d\r\n", timeouts);
			astman_append(s, "IdentifierState: %s\r\n", ider_state);
			astman_append(s, "KerchunksToday: %d\r\n", dailykerchunks);
			astman_append(s, "KerchunksSinceSystemInitialization: %d\r\n", totalkerchunks);
			astman_append(s, "KeyupsToday: %d\r\n", dailykeyups);
			astman_append(s, "KeyupsSinceSystemInitialization: %d\r\n", totalkeyups);
			astman_append(s, "DtmfCommandsToday: %d\r\n", dailyexecdcommands);
			astman_append(s, "DtmfCommandsSinceSystemInitialization: %d\r\n", totalexecdcommands);
			astman_append(s, "LastDtmfCommandExecuted: %s\r\n",
			(lastdtmfcommand && strlen(lastdtmfcommand)) ? lastdtmfcommand : not_applicable);
			hours = dailytxtime/3600000;
			dailytxtime %= 3600000;
			minutes = dailytxtime/60000;
			dailytxtime %= 60000;
			seconds = dailytxtime/1000;
			dailytxtime %= 1000;

			astman_append(s, "TxTimeToday: %02d:%02d:%02d.%d\r\n",
				hours, minutes, seconds, dailytxtime);

			hours = (int) totaltxtime/3600000;
			totaltxtime %= 3600000;
			minutes = (int) totaltxtime/60000;
			totaltxtime %= 60000;
			seconds = (int)  totaltxtime/1000;
			totaltxtime %= 1000;

			astman_append(s, "TxTimeSinceSystemInitialization: %02d:%02d:%02d.%d\r\n",
				 hours, minutes, seconds, (int) totaltxtime);

  			sprintf(str, "NodesCurrentlyConnectedToUs: ");
                        if(!numoflinks){
  	                      strcat(str,"<NONE>");
                        }
			else{
				for(j = 0 ;j < numoflinks; j++){
					sprintf(str+strlen(str), "%s", listoflinks[j]);
					if(j < numoflinks - 1)
						strcat(str,",");
				}
			}
			astman_append(s,"%s\r\n", str);

			astman_append(s, "Autopatch: %s\r\n", patch_ena);
			astman_append(s, "AutopatchState: %s\r\n", patch_state);
			astman_append(s, "AutopatchCalledNumber: %s\r\n",
			(called_number && strlen(called_number)) ? called_number : not_applicable);
			astman_append(s, "ReversePatchIaxrptConnected: %s\r\n", reverse_patch_state);
			astman_append(s, "UserLinkingCommands: %s\r\n", link_ena);
			astman_append(s, "UserFunctions: %s\r\n", user_funs);

			for(j = 0; j < numoflinks; j++){ /* ast_free() all link names */
				ast_free(listoflinks[j]);
			}
			if(called_number){
				ast_free(called_number);
			}
			if(lastdtmfcommand){
				ast_free(lastdtmfcommand);
			}
			astman_append(s, "\r\n"); /* We're Done! */
		        return 0;
		}
	}
	astman_send_error(s, m, "RptStatus unknown or missing node");
	return -1;
}



/*
 * Implement the RptStatus Manager Interface
 */

static int manager_rpt_status(struct mansession *s, const struct message *m)
{
	int i,res,len,index;
	int uptime,hours,minutes;
	time_t now;
	const char *cmd = astman_get_header(m, "Command");
	char *str;
	enum {MGRCMD_RPTSTAT,MGRCMD_NODESTAT,MGRCMD_XSTAT,MGRCMD_SAWSTAT};
	struct mgrcmdtbl{
		const char *cmd;
		int index;
	};
	static struct mgrcmdtbl mct[] = {
		{"RptStat",MGRCMD_RPTSTAT},
		{"NodeStat",MGRCMD_NODESTAT},
		{"XStat",MGRCMD_XSTAT},
		{"SawStat",MGRCMD_SAWSTAT},
		{NULL,0} /* NULL marks end of command table */
	};

	time(&now);

	len = 1024; /* Allocate a working buffer */
	if(!(str = ast_malloc(len)))
		return -1;

	/* Check for Command */
	if(ast_strlen_zero(cmd)){
		astman_send_error(s, m, "RptStatus missing command");
		ast_free(str);
		return 0;
	}
	/* Try to find the command in the table */
	for(i = 0 ; mct[i].cmd ; i++){
		if(!strcmp(mct[i].cmd, cmd))
			break;
	}

	if(!mct[i].cmd){ /* Found or not found ? */
		astman_send_error(s, m, "RptStatus unknown command");
		ast_free(str);
		return 0;
	}
	else
		index = mct[i].index;

	switch(index){ /* Use the index to go to the correct command */

		case MGRCMD_RPTSTAT:
			/* Return Nodes: and a comma separated list of nodes */
			if((res = snprintf(str, len, "Nodes: ")) > -1)
				len -= res;
			else{
				ast_free(str);
				return 0;
			}
			for(i = 0; i < nrpts; i++){
				if(i < nrpts - 1){
					if((res = snprintf(str+strlen(str), len, "%s,",rpt_vars[i].name)) < 0){
						ast_free(str);
						return 0;
					}
				}
				else{
					if((res = snprintf(str+strlen(str), len, "%s",rpt_vars[i].name)) < 0){
						ast_free(str);
						return 0;
					}
				}
				len -= res;
			}

			rpt_manager_success(s,m);

			if(!nrpts)
				astman_append(s, "<NONE>\r\n");
			else
				astman_append(s, "%s\r\n", str);

			uptime = (int)(now - starttime);
                     	hours = uptime/3600;
                        uptime %= 3600;
                        minutes = uptime/60;
                        uptime %= 60;
                        astman_append(s, "RptUptime: %02d:%02d:%02d\r\n",
                                hours, minutes, uptime);
			astman_append(s, "\r\n");
			break;

		case	MGRCMD_XSTAT:
			res = rpt_manager_do_xstat(s,m,str);
			ast_free(str);
			return res;

		case	MGRCMD_SAWSTAT:
			res = rpt_manager_do_sawstat(s,m,str);
			ast_free(str);
			return res;

		case	MGRCMD_NODESTAT:
			res = rpt_manager_do_stats(s,m,str);
			ast_free(str);
			return res;

		default:
			astman_send_error(s, m, "RptStatus invalid command");
			break;
	}
	ast_free(str);
	return 0;
}

#endif

#ifdef	OLD_ASTERISK
int unload_module()
#else
static int unload_module(void)
#endif
{
	int i, res;

#ifdef	OLD_ASTERISK
	STANDARD_HANGUP_LOCALUSERS;
#endif
	for(i = 0; i < nrpts; i++) {
		if (!strcmp(rpt_vars[i].name,rpt_vars[i].p.nodes)) continue;
                ast_mutex_destroy(&rpt_vars[i].lock);
                ast_mutex_destroy(&rpt_vars[i].remlock);
	}
	res = ast_unregister_application(app);

#ifdef	NEW_ASTERISK
	ast_cli_unregister_multiple(rpt_cli,sizeof(rpt_cli) /
		sizeof(struct ast_cli_entry));
#else
	/* Unregister cli extensions */
	ast_cli_unregister(&cli_debug);
	ast_cli_unregister(&cli_dump);
	ast_cli_unregister(&cli_stats);
	ast_cli_unregister(&cli_lstats);
	ast_cli_unregister(&cli_nodes);
	ast_cli_unregister(&cli_local_nodes);
	ast_cli_unregister(&cli_reload);
	ast_cli_unregister(&cli_restart);
	ast_cli_unregister(&cli_playback);
	ast_cli_unregister(&cli_localplay);
	ast_cli_unregister(&cli_irlpplay);
	ast_cli_unregister(&cli_fun);
	ast_cli_unregister(&cli_fun1);
	res |= ast_cli_unregister(&cli_cmd);
#endif
#ifndef OLD_ASTERISK
	res |= ast_manager_unregister("RptLocalNodes");
	res |= ast_manager_unregister("RptStatus");
#endif
	return res;
}

#ifdef	OLD_ASTERISK
int load_module()
#else
static int load_module(void)
#endif
{
	int res,fd;
	struct dahdi_versioninfo dv;
	char *cp;

	fd = open("/dev/dahdi/ctl",O_RDWR);
	if (fd == -1)
	{
		ast_log(LOG_ERROR,"Cannot open DAHDI device for probe\n");
		return -1;
	}
	if (ioctl(fd,DAHDI_GETVERSION,&dv) == -1)
	{
		ast_log(LOG_ERROR,"Cannot get DAHDI version info\n");
		close(fd);
		return -1;
	}
	close(fd);
	cp = strstr(dv.version,"RPT_");
	#if 0 // check for compatible DAHDI
	if ((!cp) || (*(cp + 4) < REQUIRED_DAHDI_VERSION))
	{
		ast_log(LOG_ERROR,"DAHDI version %s must at least level RPT_%c to operate\n",
			dv.version,REQUIRED_DAHDI_VERSION);
		return -1;
	}
	#endif
	ast_pthread_create(&rpt_master_thread,NULL,rpt_master,NULL);

#ifdef	NEW_ASTERISK
	ast_cli_register_multiple(rpt_cli,sizeof(rpt_cli) /
		sizeof(struct ast_cli_entry));
	res = 0;
#else
	/* Register cli extensions */
	ast_cli_register(&cli_debug);
	ast_cli_register(&cli_dump);
	ast_cli_register(&cli_stats);
	ast_cli_register(&cli_lstats);
	ast_cli_register(&cli_nodes);
	ast_cli_register(&cli_local_nodes);
	ast_cli_register(&cli_reload);
	ast_cli_register(&cli_restart);
	ast_cli_register(&cli_playback);
	ast_cli_register(&cli_localplay);
	ast_cli_register(&cli_irlpplay);
	ast_cli_register(&cli_fun);
	ast_cli_register(&cli_fun1);
	res = ast_cli_register(&cli_cmd);
#endif
#ifndef OLD_ASTERISK
	res |= ast_manager_register("RptLocalNodes", 0, manager_rpt_local_nodes, "List local node numbers");
	res |= ast_manager_register("RptStatus", 0, manager_rpt_status, "Return Rpt Status for CGI");

#endif
	res |= ast_register_application(app, rpt_exec, synopsis, descrip);
	return res;
}

#ifdef	OLD_ASTERISK
char *description()
{
	return tdesc;
}
int usecount(void)
{
	int res;
	STANDARD_USECOUNT(res);
	return res;
}

char *key()
{
	return ASTERISK_GPL_KEY;
}
#endif

#ifdef	OLD_ASTERISK
int reload()
#else
static int reload(void)
#endif
{
int	n;

	for(n = 0; n < nrpts; n++) rpt_vars[n].reload = 1;
	return(0);
}


#ifndef	OLD_ASTERISK
/* STD_MOD(MOD_1, reload, NULL, NULL); */
AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_DEFAULT, "Radio Repeater/Remote Base Application",
		.load = load_module,
		.unload = unload_module,
		.reload = reload,
	       );
#endif
