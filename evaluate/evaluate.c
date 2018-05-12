/* ------------------------------------------------------- */
/* evaluate: a program to interpret command line arguments */
/*           and place the results in a shell variable     */
/* Designed to work with the ORCA shell.                   */
/*                                                         */
/* This program is freeware and may distributed freely,    */
/* but all commercial rights are retained by Dave Tribby.  */
/*                                                         */
/* Written by Dave Tribby in ORCA/C for the Apple IIGS     */
/*   beginning February 1992                               */
/* GEnie: D.TRIBBY       Internet: tribby@cup.hp.com       */
/* ------------------------------------------------------- */

/* Program version number */
#define PROG_VERSION "1.0"

/* Use the "small" memory model (must match library) */
#pragma memorymodel 0

/* Set DEBUG_ON to non-0 to print allocated memory addresses */
/* and list the steps performed by the program.              */
#define DEBUG_ON 0

#if DEBUG_ON
/* If something goes wrong, print detailed information */
#pragma debug 25
#endif

/* Optimized the final version of the code */
#pragma optimize -1

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gsos.h>
#include <orca.h>
#include <shell.h>

/* Let the compiler do as much checking as possible */
#pragma lint -1

/* ----------------------------------------------------------------- */
/* Declarations for expanded shell calls, in case you don't have the
/* version of SHELL.H that was distributed with system software 6.0,
/* or you haven't fixed the error.
/* ----------------------------------------------------------------- */
#ifndef InitWildcardGS
                                                                       
struct InitWildcardGSPB {
   Word                 pCount;
   GSString255Ptr       wFile;
   Word                 flags;
};
typedef struct InitWildcardGSPB InitWildcardGSPB, *InitWildcardPtrGS;

struct NextWildcardGSPB {
   Word                 pCount;
   ResultBuf255Ptr      pathName;
   Word                 access;
   Word                 fileType;
   LongWord             auxType;
   Word                 storageType;
   TimeRec              createDateTime;
   TimeRec              modDateTime;
   ResultBuf255Ptr      optionList;
   LongWord             eof;
   LongWord             blocksUsed;
   LongWord             resourceeof;
   LongWord             resourceBlocks;
};
typedef struct NextWildcardGSPB NextWildcardGSPB, *NextWildcardPtrGS;


struct ReadVariableGSPB {
   Word                 pCount;
   GSString255Ptr       name;
   ResultBuf255Ptr      value;
   Word                 export;
};
typedef struct ReadVariableGSPB ReadVariableGSPB, *ReadVariablePtrGS;

struct SetGSPB {
   Word                 pCount;
   GSString255Ptr       name;
   GSString255Ptr       value;
   Word                 export;
};
typedef struct SetGSPB SetGSPB, *SetPtrGS;


struct StopGSPB {
   Word                 pCount;
   Word                 flag;
};
typedef struct StopGSPB StopGSPB, *StopPtrGS;


struct ExecuteGSPB {
   Word                 pCount;
   Word                 flag;
   char                *comm;
};
typedef struct ExecuteGSPB ExecuteGSPB, *ExecutePtrGS;


struct RedirectGSPB {
   Word                 pCount;
   Word                 device;
   Word                 append;
   GSString255Ptr       file;
};
typedef struct RedirectGSPB RedirectGSPB, *RedirectPtrGS;

#define InitWildcardGS(pBlockPtr) (PDosInt(0x0149,pBlockPtr))
#define NextWildcardGS(pBlockPtr) (PDosInt(0x014A,pBlockPtr))
#define ReadVariableGS(pBlockPtr) (PDosInt(0x014B,pBlockPtr))
#define SetGS(pBlockPtr)  	  (PDosInt(0x0146,pBlockPtr))
#define StopGS(pBlockPtr)         (PDosInt(0x0153,pBlockPtr))
#define ExecuteGS(pBlockPtr)      (PDosInt(0x014D,pBlockPtr))
#define RedirectGS(pBlockPtr)     (PDosInt(0x0150,pBlockPtr))
                                                     
#endif
/* ----------------------------------------------------------------- */



/* ----- Command line from shell, before evaluation ----- */
char   *cmd_string;

/* ----- Results of evaluation ----- */
/* Limit the number of bytes that can be allocated for result */
#define MAX_RESULT_SIZE 32768
/* Allocate this many bytes at a time for result */
#define RSLT_ALLOC_SIZE 256
char           *result=NULL;    /* Pointer to allocated bytes */
unsigned int    result_limit;   /* # of bytes allocated */
unsigned int    result_size;    /* # bytes being used */
int             no_memory;      /* TRUE when there's no memory for result */
int		expand;     	/* Should wildcards be expanded? */

/* Allocate this many entries at a time for token pointers */
#define TOK_ALLOC_SIZE 10

int    *tokoffset=NULL; /* Offset to each token in result string */
int     max_tokens;     /* Number that can be held in allocated memory */
int     num_tokens;     /* Number of tokens in result */

/* Get the program name from the command line  */
/* It should be "evaluate", but may be renamed */
char   *prog_name;

/* Error message used in several places */
char   *out_of_memory="[%s] Out of memory: can't allocate %d bytes\n";


/* ----------------------------------------------------------------- */
/* Check whether the user has pressed the shell "stop" key.          */
/* ----------------------------------------------------------------- */
void CheckStopKey(void)
{
StopGSPB       stop_pb;

stop_pb.pCount = 1;
StopGS(&stop_pb);
if (stop_pb.flag)  {
        fprintf(stderr, "[%s] User stop key detected\n",prog_name);
        exit(2);
        }
}   /* CheckStopKey */



/* ----------------------------------------------------------------- */
/* Debugging subroutines to check out pointer variable
/* ----------------------------------------------------------------- */
#if DEBUG_ON
void DebugVar(char *msg, void *var_pnt, int len)
{
fprintf(stderr, "== %s at 0x%06lX = %d bytes\n",  msg, var_pnt, len);
}     
/* Use this macro to print a single variable's address & size */
#define DebugVar1(v) DebugVar(#v,&v,(int)sizeof(v))
#else
#define DebugVar(msg,var_pnt,len)
#define DebugVar1(var)
#endif


#if DEBUG_ON
void DumpVar(char *msg, void *var_pnt, int len)
{
char   *pnt;
char    ascii[17];
int     i, column;

fprintf(stderr, "== %s at 0x%06lX = %d byte(s)\n",  msg, var_pnt, len);
if (len == 0) return;
pnt = (char *)var_pnt;
fprintf(stderr,"0x%06lX:", pnt);
strcpy(ascii,"                ");
for (i=0; i<len; i++)  {
        fprintf(stderr," %02X", *pnt);
        column = i & 0x0F;
        if ((*pnt < ' ') || (*pnt > '~'))
                ascii[column] = '.';
        else
                ascii[column] = *pnt;
        pnt++;
        if ( (column == 0x0F) && (i < len-1) ) {
                fprintf(stderr," *%s\n0x%06lX:", ascii,pnt);
                strcpy(ascii,"                ");
                }
        CheckStopKey;                              
        }

for (i=column; i<15; i++) fprintf(stderr, "   ");
ascii[column+1] = '\0';
fprintf(stderr," *%s\n", ascii);
fprintf(stderr,"\n");
}
#else
#define DumpVar(msg,var_pnt,len)
#endif


/* ----------------------------------------------------------------- */
/* When there's a tool or GS/OS problem, report it                   */
/* ----------------------------------------------------------------- */
void ReportError(int error, char *msg)
{
fprintf(stderr, "[%s] %s: tool error $%x\n", prog_name, msg, error);
}


/* ----------------------------------------------------------------- */
/* Release allocated memory (called at exit)                         */
/* ----------------------------------------------------------------- */
void ReleaseMem(void)
{
if (tokoffset != NULL)  {
        free(tokoffset);
        tokoffset = NULL;
        }
if (result != NULL)  {
        free(result);
        result = NULL;
        }
}   /* ReleaseMem */


/* ----------------------------------------------------------------- */
/* Set a shell variable to a value                                   */
/* ----------------------------------------------------------------- */
void SetVar(char *name, char *value)
{
int                     error;
GSString255Ptr          var_value;      /* Holds variable sized string */
SetGSPB         	set_var_pb;
static GSString255      var_name;

#if DEBUG_ON
fprintf(stderr, ">> Setting shell variable %s to '%s'\n", name,value);
#endif           
      
/* Shell call requires three parameters */
set_var_pb.pCount = 3;

/* Create GSOS string that holds the name of the variable */
/* Truncate value if > size of GSString255                */
var_name.length = strlen(name);
if (var_name.length > sizeof(var_name.text))   {
        var_name.length = sizeof(var_name.text);
        strncpy(var_name.text, name, sizeof(var_name.text));
        }
else    {
        strcpy(var_name.text, name);
        }
set_var_pb.name = &var_name;

/* Allocate a GS string large enough to hold the value */
var_value = (GSString255Ptr) malloc(strlen(value)+sizeof(Word));
if (var_value == NULL) {
        fprintf(stderr, out_of_memory, prog_name, strlen(value)+sizeof(Word) );
        exit(1);
        }
var_value->length = strlen(value);
strcpy(var_value->text, value);
DumpVar("result value GSString", var_value, strlen(value)+sizeof(Word));

set_var_pb.value = var_value;
set_var_pb.export = 1;

/* Make the shell call to set the variable */
SetGS(&set_var_pb);
if (error = toolerror())   {
        ReportError(error,"SetGS");
        }

free(var_value);
}   /* SetVar */


/* ----------------------------------------------------------------- */
/* Append a string to the end of the result string                   */
/* ----------------------------------------------------------------- */
void AppendParam(char *val, int len)  
{
void           *pnt;
unsigned int	new_result_size;
unsigned int	allocate_size;

/* In case this is taking too long, allow user to interrupt */
CheckStopKey();

#if DEBUG_ON
fprintf(stderr, ">> Appending %d byte(s) to result\n", len);
#endif

/* Check size and allocate memory, if required */
new_result_size = result_size+len;
if (new_result_size >= result_limit)    {
        allocate_size = result_limit+RSLT_ALLOC_SIZE;
	while (allocate_size < new_result_size) allocate_size+=RSLT_ALLOC_SIZE;
        if ( (allocate_size > MAX_RESULT_SIZE) ||
             ((pnt = realloc(result,allocate_size)) == NULL) ) {
                strncat(result, val, result_limit-result_size-1);
                fprintf(stderr, "[%s] Warning: result truncated to %d bytes\n",
                        prog_name, MAX_RESULT_SIZE);
                result_size = MAX_RESULT_SIZE;
                no_memory = TRUE;
                return;
                }
        result = pnt;
        result_limit = allocate_size;
        DebugVar("Result memory block", result, result_limit);
        };

/* Append the characters and update the length */
strncat(result,val,len);
result_size += len;
#if DEBUG_ON
fprintf(stderr, ">> Result = %s\n", result);
#endif
}   /* AppendParam */


/* ----------------------------------------------------------------- */
/* Execute a shell command and append output to the result string    */
/* ----------------------------------------------------------------- */
void ExecuteCommand(char *cmd,      /* command surrounded by ` quotes */
                    int cmd_len)    /* length of command + quotes */
{
ExecuteGSPB     exec_pb;
RedirectGSPB    redirect_pb;
GSString255Ptr  redirect_name;  /* Holds variable sized string */
char           *cmd_pnt;
char            temp_name[L_tmpnam];
FILE           *temp_file;
size_t          buf_len;
int             error;
int             cmd_out_ch;
int             cmd_out_len;
char           *cmd_out_str;

/* Get a temporary file name to hold the output */
tmpnam(temp_name);

/* Allocate memory to hold the command - "``" + c-rtn + extra null */
buf_len = cmd_len + 1;
cmd_pnt = malloc(buf_len);
if (cmd_pnt == NULL)   {
        fprintf(stderr, out_of_memory, prog_name,buf_len);
        exit(1);
        };
/* Copy the command, skipping leading and trailing "`" */
strncpy(cmd_pnt,&cmd[1],cmd_len-2);
/* Make sure the string is properly terminated. */
cmd_pnt[buf_len-3] = '\r';
cmd_pnt[buf_len-2] = '\0';
cmd_pnt[buf_len-1] = '\0';
                 
DumpVar("command value", cmd_pnt, buf_len);
           
/* Originally, I appended "> tempfilename" to the end of the string */
/* to capture the output of the string. The 2.0 version of the      */
/* ORCA shell can go into an infinite loop for short commands!!!    */
/* Shawn Quick on GEnie suggested I redirect output as a            */
/* work-around. That works...thanks, Shawn!                         */

/* Redirect output to point to the temporary file */
redirect_pb.pCount = 3;
redirect_pb.device = 1;
redirect_pb.append = TRUE;

/* Allocate a GS string to hold the temp file name */
#if DEBUG_ON
fprintf(stderr, ">> Redirecting output to %s\n", temp_name);
#endif
redirect_name = (GSString255Ptr) malloc(strlen(temp_name)+sizeof(Word));
if (redirect_name == NULL) {
        fprintf(stderr,out_of_memory,prog_name,strlen(temp_name)+sizeof(Word));
        exit(1);
        }
redirect_name->length = strlen(temp_name);
strcpy(redirect_name->text, temp_name);
redirect_pb.file = redirect_name;

DumpVar("redirect name GSString",redirect_name,
                strlen(temp_name)+sizeof(Word));

RedirectGS(&redirect_pb);
if (error = toolerror())   {
        ReportError(error,"RedirectGS");
        fprintf(stderr, "[%s]   file: %s\n", prog_name,temp_name);
        }

/* Set up the parameter block and execute the command */
exec_pb.pCount = 2;
exec_pb.flag = 0;               /* Define new variable table */
exec_pb.comm = cmd_pnt;

#if DEBUG_ON
fprintf(stderr, ">> Executing command %s",cmd_pnt);
#endif
ExecuteGS(&exec_pb);
if (error = toolerror())   {
        ReportError(error,"ExecuteGS");
        cmd_pnt[buf_len-3] = '\0';
        fprintf(stderr, "[%s]   command was `%s`\n", prog_name,cmd_pnt);
        }

/* Change standard output back to the console */
strcpy(redirect_name->text, ".CONSOLE:");
redirect_name->length = strlen(redirect_name->text);
redirect_name = realloc(redirect_name,
                        redirect_name->length+sizeof(Word) );
redirect_pb.file = redirect_name;
RedirectGS(&redirect_pb);
if (error = toolerror())   {
        ReportError(error,"RedirectGS");
        }

free(redirect_name);

/* Get output from the command, changing carriage-returns to blanks */
temp_file = fopen(temp_name,"r");
if (temp_file == NULL)   {
        fprintf(stderr, "[%s] Error opening temporary file %s",
                prog_name,temp_name);
        perror("");
        exit(1);
        }

cmd_out_str = malloc(RSLT_ALLOC_SIZE);
if (cmd_out_str == NULL) {
        fprintf(stderr, out_of_memory,  prog_name, RSLT_ALLOC_SIZE);
        exit(1);
        }
DebugVar("command output string", cmd_out_str, RSLT_ALLOC_SIZE);
               
cmd_out_len = 0;

while ( ((cmd_out_ch = fgetc(temp_file)) != EOF)  &&  !no_memory)   {
        /* Convert control characters to blanks */
        if ( (cmd_out_ch < ' ')  ||  (cmd_out_ch > 127) ) cmd_out_ch = ' ';
        /* Append character to string. When full, add to result */
        cmd_out_str[cmd_out_len] = cmd_out_ch;
        if (++cmd_out_len == RSLT_ALLOC_SIZE)   {
                AppendParam(cmd_out_str, cmd_out_len);
                cmd_out_len = 0;
                }
        }
if (cmd_out_len)  AppendParam(cmd_out_str, cmd_out_len);
                          
fclose(temp_file);

/* Clean up: Purge the temp file, Free memory */
free(cmd_pnt);
free(cmd_out_str);
if (remove(temp_name)) {
        fprintf(stderr, "[%s] Error removing temp file %s",
                prog_name,temp_name);
        perror("");
        }
}   /* ExecuteCommand */



/* ----------------------------------------------------------------- */
/* Point to beginning of next parameter and calculate its length     */
/* ----------------------------------------------------------------- */
int NextParam(char **scan_string)
{
char   *nxt_ch;
char    quote;
int     p_len;

/* Skip over leading blanks */
while (**scan_string == ' ') (*scan_string)++;

/* Get length of string by looking at each character */
nxt_ch = *scan_string;

/* Check for 3 types of quoted strings */
if ( (*nxt_ch == '"')  ||  (*nxt_ch == '\'') || (*nxt_ch == '`') )  {
        quote = *nxt_ch;
        do   {
                *nxt_ch++;
                CheckStopKey();
                }
        while (*nxt_ch  &&  *nxt_ch != quote);
        if (*nxt_ch != quote)  {
                fprintf(stderr, "[%s] Non-terminated %c string\n",
                        prog_name, quote);
                exit (1);
                }
        *nxt_ch++;
        }
/* If it's not a quoted string, scan ahead to the next blank or quote */
else while (*nxt_ch  &&  (*nxt_ch != ' ')
             &&  (*nxt_ch != '"')  &&  (*nxt_ch != '\'') ) {
        *nxt_ch++;
        CheckStopKey();
        }                                    

/* Calculate length by subtracting starting from ending address */
p_len = nxt_ch - *scan_string;
DumpVar("Parsed param", *scan_string, p_len);
return(p_len);
}  /* NextParam */


/* ----------------------------------------------------------------- */
/* Expand wildcards in parameter.                                    */
/* ----------------------------------------------------------------- */
void ExpandWildcard(char *param, int len)
{
int                     num_expand;
int                     error;
GSString255Ptr          mask_value;      /* Holds variable sized string */
InitWildcardGSPB        wc_init_pb;
NextWildcardGSPB        wc_next_pb;
static ResultBuf255     file_name;

DebugVar1(mask_value);

num_expand = 0;

/* Allocate a GS string large enough to hold the file mask */
mask_value = (GSString255Ptr) malloc(len+sizeof(Word));
if (mask_value == NULL) {
        fprintf(stderr, out_of_memory, prog_name, len+sizeof(Word) );
        exit(1);
        }
mask_value->length = len;
strncpy(mask_value->text, param, len);

/* Initialize the InitWildcardGS and NextWildcardGS parameter blocks */
wc_init_pb.pCount = 2;
wc_init_pb.wFile = mask_value;
wc_init_pb.flags  = 0x8000;             /* No prompting wildcard */
wc_next_pb.pCount = 1;
wc_next_pb.pathName = &file_name;
file_name.bufSize = sizeof(file_name);

#if DEBUG_ON
fprintf(stderr, ">> Expanding wildcard (len = %d)\n", len);
#endif           
/* See if param can be expanded */
InitWildcardGS(&wc_init_pb);
if (!toolerror())   do   {
        NextWildcardGS(&wc_next_pb);
        if (error = toolerror())   {
                ReportError(error,"NextWildcardGS");
                }
        else if (file_name.bufString.length != 0) {
                /* Add a blank before name, except at start */
                if (num_expand) AppendParam(" ",1);
                /* Add expanded parameter to result string */
                AppendParam(file_name.bufString.text,
                            file_name.bufString.length);
                num_expand++;
                #if DEBUG_ON
                fprintf(stderr, ">> Expansion %d ", num_expand);
                DumpVar("Expansion",file_name.bufString.text,
	                 file_name.bufString.length);
                #endif           
                }
        CheckStopKey();
        }       
while ( (!no_memory) && (file_name.bufString.length != 0) );
#if DEBUG_ON
fprintf(stderr, ">> %d expansion(s)\n", num_expand);
#endif           

/* Don't need to hold mask_value in memory any longer */
free(mask_value);

/* If there was no legal wildcard expansion, add the original value */
if (!num_expand)  {
        AppendParam(param,len);
        }
}       /* ExpandWildcard */



/* ----------------------------------------------------------------- */
/* Build token offset array by scanning result string                */
/*  (only called when expression is indexed)                         */
/* ----------------------------------------------------------------- */
void BuildTokenOffset(void)
{
char   *index_pnt;	/* Pointer into result string */
int	len;		/* Length of element in result string */
void   *pnt;
int     token_mem;      /* Amount of memory needed to hold token pntrs */

index_pnt = result;
while ( (index_pnt) && (len = NextParam(&index_pnt)) ) {
	/* If current offset table is full, allocate more entries */
	if (num_tokens >= max_tokens)   {
        	max_tokens += TOK_ALLOC_SIZE;
	        token_mem = max_tokens * sizeof(int);
        	if ( (pnt = realloc(tokoffset,token_mem)) == NULL) {
                	fprintf(stderr, out_of_memory,  prog_name,token_mem);
	                exit (1);
        	        }
	        tokoffset = pnt;
        	DebugVar("token offset array" ,tokoffset, token_mem);
	        }                 

	/* Add entry to offset table */
	tokoffset[num_tokens++] = index_pnt - result;

        /* Point to next element and loop again */
	index_pnt += len;
	}
}


/* ----------------------------------------------------------------- */
/* Parse a single index value                                        */
/* ----------------------------------------------------------------- */
long GetIndex(void)
{
long result;

/* Skip over any leading blanks */
while (*cmd_string == ' ') cmd_string++;

/* Check for special value '#', indicating the final index */
if (*cmd_string == '#')   {
        cmd_string++;
#if DEBUG_ON
        fprintf(stderr, ">> '#' (final index) found\n");
#endif           
        result = num_tokens;
        }
else    {
        result = strtoul(cmd_string,&cmd_string,0);
        }

/* Skip over any trailing blanks */
while (*cmd_string == ' ') cmd_string++;

return result;
}   /* GetIndex */


/* ----------------------------------------------------------------- */
/* Parse the two index values                                        */
/* ----------------------------------------------------------------- */
void GetIndices(long *i1, long *i2)
{
#if DEBUG_ON
fprintf(stderr, ">> Parsing indices\n");
#endif           

/* Skip over the leading '[' */
cmd_string++;

*i1 = GetIndex();

/* Check for range, delimited by a '-' */
if (*cmd_string == '-')   {
#if DEBUG_ON
        fprintf(stderr, ">> '-' (index range) found after 1st parameter\n");
#endif           
        cmd_string++;
        *i2 = GetIndex();
        }
else    {
        *i2 = *i1;
        }

#if DEBUG_ON
fprintf(stderr, ">> Parsed index values %ld - %ld\n", *i1,*i2);
#endif           

if ( (cmd_string == NULL) || (*cmd_string != ']')
    || (*i1 < 1)  || (*i1 > num_tokens)
    || (*i2 < *i1) || (*i2 > num_tokens) )  {
        fprintf(stderr, "[%s] Illegal index\n", prog_name);
        exit(1);
        }
}   /* GetIndices */


/* ----------------------------------------------------------------- */
/* Main program                                                      */
/* ----------------------------------------------------------------- */
int main (int argc, char **argv)
{
int     error;
int     parm_len;
int     index_found;	/* Did "[" appear in expression? */
long    index1,index2;	/* Low and high index values */
int	min_param;	/* Minimum # of parameters required */
char   *final_value, *last_char, *var_string;

prog_name = *argv;

/* Check for "disallow expansion of wildcards" parameter */
expand = TRUE;
if ( (argc > 1)  &&  (argv[1][0] == '-') ) {
        min_param = 4;
        var_string = argv[2];
	switch (argv[1][1]) {
	        case 'w':
                case 'W':
			expand = FALSE;
			break;
                default:
                        fprintf(stderr,
                        	"[%s]: Unrecognized option %s\n",
                                prog_name, argv[1]);
                        min_param = 32767;
                }
        }
else  {
        min_param = 3;
        var_string = argv[1];
        }                    

/* Were enough parameters provided? */
if (argc < min_param)   {
        fprintf(stderr, "Usage: %s [-w] <variable> <expression>\n", prog_name);
        fprintf(stderr, "\n Version %s (%s)\n", PROG_VERSION,__DATE__);
        fprintf(stderr,
" This program is freeware with commercial rights retained by\n");
        fprintf(stderr,
" Dave Tribby, 1529 Fantial Court, Sunnyvale, CA  94087\n");
        fprintf(stderr,
" E-mail --  GEnie: D.TRIBBY  *  Internet: tribby@cup.hp.com\n\n");
        fprintf(stderr,
"This program uses material from the ORCA/C run-time libraries,\n");
        fprintf(stderr,
"copyright 1987-1989 by Byte Works, Inc. Used with permission.\n");
        exit(1);
        }

/* Get the raw command line. Skip over prog name, option, and variable name */
cmd_string = commandline();
while (--min_param) {
	parm_len = NextParam(&cmd_string);
	cmd_string += parm_len;
        }

/* Start out with null string in result */
result = malloc(RSLT_ALLOC_SIZE);
if (result == NULL) {
        fprintf(stderr, out_of_memory, prog_name,RSLT_ALLOC_SIZE);
        exit(1);
        }
DebugVar("result string", result, RSLT_ALLOC_SIZE);
                 
result[0] = '\0';
result_size = 0;
no_memory = FALSE;

/* Keep track of the number of result tokens */
num_tokens = 0;
max_tokens = 0;
tokoffset = NULL;

/* Release memory when exiting the program */
atexit(ReleaseMem);

/* Clear "index found" flag */
index_found = FALSE;

/* Go through the raw parameters and evaluate them */
while (*cmd_string  &&  !no_memory  &&  !index_found
	&& (parm_len = NextParam(&cmd_string)) ) {
        switch (*cmd_string) {
                /* Quoted strings */
                case '"':
                case '\'':
                        /* Remove quotes and append string */
                        cmd_string++;
                        AppendParam(cmd_string, --parm_len-1);
                        /* Bump cmd_string pointer beyond the current parameter */
                        cmd_string += parm_len;
                        /* If raw values are separated by a space, add one here */
                        if (*cmd_string == ' ') AppendParam(" ",1);
                        break;
                /* Quoted command string */
                case '`':
                        ExecuteCommand(cmd_string, parm_len);
                        cmd_string += parm_len;
                        if (*cmd_string == ' ') AppendParam(" ",1);
                        break;
                /* Index */
                case '[':
                        index_found = TRUE;
			BuildTokenOffset();
                        GetIndices(&index1, &index2);
                        cmd_string++;       /* Skip over ']' */
                        while (*cmd_string == ' ') cmd_string++;
                        if (*cmd_string)   {
                                fprintf(stderr,
                                  "[%s] Text following index ignored: %s\n",
                                  prog_name,cmd_string);
                                }
                        break;
                /* If it's not special, try to expand wildcards */
                default:
                        if (expand) {
                        	ExpandWildcard(cmd_string,parm_len);
                                }
                        else  {
                        	AppendParam(cmd_string, parm_len);
	                        }
                        cmd_string += parm_len;
                        if (*cmd_string == ' ') AppendParam(" ",1);
                }
        }

/* The final value may be modified by indexing. Don't modify */
/* result, since it must used to free the memory when done!  */
final_value = result;
if (index_found)  {
        /* Modify result to contain only indexed values */
        if (index1 > 1)  final_value = &(result[tokoffset[index1-1]]);
        if (index2 < num_tokens)   {
                last_char = &result[tokoffset[index2]-1];
                *last_char = '\0';
                }
        }

SetVar(var_string,final_value);

#if DEBUG_ON
fprintf(stderr, ">> End of program\n");
#endif
exit (0);
}
