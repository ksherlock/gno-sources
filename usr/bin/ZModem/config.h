/* Choose ONE of the following */
#define V7
/*#define USG*/

/* This number determines how many characters will be read from the
   modem in one batch.  Current limit is 255 */

/* Name of device to open to write to modem */
#define DEVTTY ".tty"

/* Debug mode on or off? */
#define DEBUGZ

/* Misc Strings */
#define VERSION "3.17 10-30"
#define PUBDIR "/usr/spool/uucppublic"

/* Pathname of the log file */
/*#define LOGFILE "/tmp/szlog"*/
#define LOGFILE ".ttyco"

/* Inline CRC calculations or function call? */
#define NFGM /* define to use function call */

/* Determines which CRC method to use */
#define FASTCRC /* define to use a 65816 assembly CRC calculation routine */

/* Define this if unsigned calculations are faster on your machine */
#define UNSL unsigned

/* Define ONLY for Apple IIGS - high speed output buffering routines */
#define FASTOUT
