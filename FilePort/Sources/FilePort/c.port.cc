
/*
 * Copyright (c) Kopriha Software,  1990
 *       All Rights Reserved
 *
 * C.Port.CC
 *
 * Description:
 *     This module is the heart of the file support of the
 *     printer port driver.
 *
 *
 *   Table of Contents:
 *
 *     c_port_open . . . . . . . . . . Open the data/command files
 *     c_port_write  . . . . . . . . . Write the buffer of data
 *     c_port_close  . . . . . . . . . Close the data/command files
 *                                     (if opened)
 *
 *
 *  History:Mar 25, 1991  Dave  Created this file
 *
 */

/*
 *  define DEBUG_CODE
 *                     - add # to define to create the local
 *                       debug code (IE:module)
 */


#pragma noroot

#define PATH_LPQ ":usr:spool:lpq"

#include "../includes/ks.fileio.e"

#include "../includes/ks.memory.h"
#include <types.h>
#include <quickdraw.h>
#include <control.h>

#ifndef _ERRORS_
#include "Errors.h"
#endif

#ifndef __MEMORY__
#include <memory.h>
#endif

#ifndef __MISCTOOL__
#include <MiscTool.h>
#endif

#ifndef __WINDOW__
#include <Window.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <gno/gno.h>
#include <ctype.h>
#include <sys/ports.h>

/*
 * Local variable declarations...
 *
 *  Start with structures...
 */

ResultBuf255  next_filename;
Word          optionList[20];
DirEntryRecGS dir_entry;
GSString32Ptr new_data_path;


/*
 * Now define all the pointers to structures...
 */

KS_FILE_PTR   data_file_ptr = (KS_FILE_PTR) NULL;
                                /* Buffered file structure ptr           */
KS_FILE_PTR   dir_file_ptr;     /* Used to get all of the filenames      */
                                /*  from spool directory                 */
Handle        spoolpath_handle; /* Handle allocated for spool path       */
GSString32Ptr spool_directory_ptr;
                                /* Pointer to the spoolfile directory    */
                                /*  pathname (as we've read it from our  */
                                /*  datafile).                           */

/*
 * On to the simple variables...
 */

KS_STRUCT_ID  ks_expected_id;   /* The structure ID we expected to find  */
                                /*  during a structure id check          */
Word          ProgramID;        /* Our Memory Manager UserId             */


KS_E_ERROR  error;              /* Used to catch error codes             */
KS_E_ERROR  error2;             /* Used to catch secondary errors        */

size_t      pathname_length;    /* Length (in bytes) of the spool path   */
Word        max_spool_number;   /* Which is the mac spool number found?  */
Word        num_entries;        /* How many directory entries are there? */
Word        spoolfile_number;   /* The spoolfile # we decided to use     */
char        cnumber;            /* Used during conversion of spoolfile # */
                                /*  into part of the spool pathname      */
char        delimiter;          /* Delimiter for pathname                */
Word        index, index2;      /* Index variables used in for loops     */

Word        error_message;      /* The error message to display          */
Word        button;             /* Which button of our error dialog did  */
                                /*  our user hit?                        */
Word        open_count = 0;     /* Count of how many open's we've done...*/

char 	    tmp[128];
Word	    port_spoolnum = 1;	/* count of how many open's we've done   */

/*
 * Now define some constants...
 */
typedef struct
    {
    Word length;
    char text[35];
    } GSString35;

/* GSString35 fileport_datapath = {22, ":Natasha:FilePort.Data" }; */
GSString35 fileport_datapath = {30, "*:System:Drivers:FilePort.Data" };

#define       SPOOL_FILENAME_EXTENSION     3 /* 2 for ##, 1 for NULL */
#define       BASE_SPOOL_FILENAME_LENGTH   9

char          base_spool_filename[BASE_SPOOL_FILENAME_LENGTH+1] =
                                                         {"SpoolFile"};

#define SPOOLFILE_FILETYPE ((Word) 4)
#define SPOOLFILE_AUXTYPE  ((LongWord) 0)

/*
 * Port flag states
 */

enum port_flags {PORT_NOT_ACTIVE = 1,
                 PORT_READY,
                 PORT_SPOOLING,
                 PORT_ERROR};

Word        port_status = PORT_NOT_ACTIVE;
                           /* Current status of port driver...      */

void *malloc(size_t size)
{
handle h;
	h = NewHandle(size,ProgramID,0,0l);
	return *h;
}

void free(void *data)
{
	DisposeHandle(FindHandle(data));
}


/* ****************************************************************** *
 *   c_port_open - Create and open the data/command spool files for   *
 *                 the port driver.                                   *
 *                                                                    *
 *   History: Mar 25, 1991  Dave  Created this routine                *
 *                                                                    *
 *            Dec 23, 1991  Dave  Completed pathname expansion (we    *
 *                                now handle prefixes (#:),           *
 *                                appleshare users (@:), and system   *
 *                                disk (*:) references and any        *
 *                                set of delimiters.                  *
 *                                                                    *
 * ****************************************************************** */

#define ROUTINE_NAME "c_port_open"

void c_port_open(void)
{
extern GSString32Ptr __C2GSMALLOC(char *);

    /* ************************************************************** *
     *  No local variables...                                         *
     * ************************************************************** */


    /* ************************************************************** *
     *  Check if we already have a file open... Apple said this should*
     *  never happen... but I really don't believe them...            *
     * ************************************************************** */

    if ((data_file_ptr != (KS_FILE_PTR) NULL) ||
        (port_status != PORT_NOT_ACTIVE))
        {
        open_count++;
        return;
        };


    /* ************************************************************** *
     *  Get the applications memory manager user ID - We'll need it   *
     *  to be able to allocate memory...                              *
     * ************************************************************** */

    ProgramID = MMStartUp();


    /* ************************************************************** *
     *  Set the size of the data buffer to 64k...                     *
     * ************************************************************** */

    KS_FILE_SET_BUFFER_SIZE((LongWord) (64*1024),
                            error);
    if (error != KS_E_SUCCESS)
        {
        error_message = BAD_BUFFER_SIZE;
        goto ERROR_BAD_BUFFER_SIZE;
        };


    /*  > Open our data file in System:filePort.data: and read it
     *
     *	The line print spool directory is hard-coded
     */

    /*	Create a spool file name, and set it up in GS/OS format for use
     *	by the KS_FILE_OPEN routine
     */

    sprintf(tmp,PATH_LPQ ":tmp.%08lX",GetTick());
    spool_directory_ptr = __C2GSMALLOC(tmp);

    /* create the spool file now */
    KS_FILE_CREATE((GSString255Ptr) spool_directory_ptr,
                   SPOOLFILE_FILETYPE,
                   SPOOLFILE_AUXTYPE,
                   error);
    if (error != KS_E_SUCCESS)
        {
        error_message = CANT_CREATE_SPOOL_FILE;
        goto ERROR_CANT_CREATE_SPOOL_FILE;
        };

    /* ************************************************************** *
     *  Open the data spool file                                      *
     * ************************************************************** */

    KS_FILE_OPEN((GSString255Ptr) spool_directory_ptr,
                 KS_FILE_WRITE_ACCESS,
                 KS_FILE_DATA_FORK,
                 KS_FILE_BUFFER_IO,
                 &data_file_ptr,
                 error);
    if (error != KS_E_SUCCESS)
        {
        error_message = CANT_OPEN_SPOOL_FILE;
        goto ERROR_CANT_OPEN_SPOOL_FILE;
        };


    /* ************************************************************** *
     *  Releasing pathname back to free memory...                     *
     *  No error checking because what would we do??                  *
     * ************************************************************** */


    KS_MEMORY_DEALLOCATE(spoolpath_handle,
                         error);


    /* ************************************************************** *
     *  Set a flag and a counter...                                   *
     * ************************************************************** */

    port_status = PORT_SPOOLING;
    open_count = 1;


    /* ************************************************************** *
     *  Everything is all set - return to our caller...               *
     * ************************************************************** */

    return;



    /* ************************************************************** *
     *  Error processing for all of the error cases of the above code *
     *                                                                *
     *  Once we clean up whatever needs to be cleaned up, then we     *
     *  will get around to putting up a dialog box for our user.      *
     * ************************************************************** */

ERROR_CANT_OPEN_SPOOL_FILE:

    KS_FILE_DELETE((GSString255Ptr) spool_directory_ptr,
                   error2);

ERROR_TOO_MANY_SPOOLFILES:

ERROR_CANT_CREATE_SPOOL_FILE:

ERROR_CANT_GET_NEXT_FILE:

ERROR_CANT_GET_ENTRY_COUNT:

    KS_FILE_CLOSE(dir_file_ptr, error2);

ERROR_CANT_OPEN_DIR:

    KS_MEMORY_DEALLOCATE(spoolpath_handle,
                         error2);

    goto ERROR_MESSAGE;

ERROR_CANT_READ_PATHNAME:

    KS_MEMORY_DEALLOCATE(spoolpath_handle,
                         error2);

ERROR_CANT_ALLOCATE_PATHNAME:

    KS_FILE_CLOSE(dir_file_ptr, error2);

ERROR_CANT_OPEN_CONFIG_FILE:

ERROR_BAD_BUFFER_SIZE:

ERROR_MESSAGE:

    port_status = PORT_ERROR;

    ERROR_DIALOG(error_message, error, button);

    return;


}   /* End of c_port_open()                                           */



/* ****************************************************************** *
 *   c_port_write - Write to the data/command spool files for         *
 *                  the port driver.                                  *
 *                                                                    *
 *   History: Mar 27, 1991  Dave  Created this routine                *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "c_port_write"

void c_port_write(char *buffer, Word size)
{
    /* ************************************************************** *
     *  No local variables...                                         *
     * ************************************************************** */


    /* ************************************************************** *
     *  Now lets see if it looks like we are allowed to be printing...*
     *  Check for the following:                                      *
     *    1) We are at level 1 for spool files...                     *
     *    2) We have an open spool file                               *
     *    3) No errors have occured (yet)                             *
     * ************************************************************** */

    if ((open_count != 1) ||
        (data_file_ptr == (KS_FILE_PTR) NULL) ||
        (port_status != PORT_SPOOLING))
        {
        return;
        };


    /* ************************************************************** *
     *  Looks good... write this data to the buffered output file.    *
     *  Remember to check for an error...                             *
     * ************************************************************** */

    KS_FILE_WRITE(data_file_ptr,
                  KS_NEXT_FILE_POSITION,
                  (LongWord) size,
                  TO_POINTER(buffer),
                  error);
    if (error != KS_E_SUCCESS)
        {
        error_message = CANT_WRITE_DATA;
        goto ERROR_CANT_WRITE_DATA;
        };


    /* ************************************************************** *
     *  Everything is all done here - return to our caller...         *
     * ************************************************************** */

    return;


    /* ************************************************************** *
     *  Error processing for all of the error cases of the above code *
     *                                                                *
     *  Something bad happened... that means we have to give up       *
     *  on printing and give our user an error dialog that tells      *
     *  them what happened.                                           *
     * ************************************************************** */

ERROR_CANT_WRITE_DATA:

    /* ************************************************************** *
     *  First we close the spool file (can't use it anymore now...)   *
     *  then give our user some feed back (like an error message).    *
     * ************************************************************** */

    KS_FILE_CLOSE(data_file_ptr, error2);

    data_file_ptr = (KS_FILE_PTR) NULL;
    open_count--;
    port_status = PORT_ERROR;

    ERROR_DIALOG(error_message, error, button);

    return;
}



/* ****************************************************************** *
 *   c_port_close - Close the data/command spool files for            *
 *                  the port driver.                                  *
 *                                                                    *
 *   History: Mar 27, 1991  Dave  Created this routine                *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "c_port_close"

void c_port_close(void)
{
static char cf[128];
int slf;
int lpdport;
int cfid;
long msg;


    /* ************************************************************** *
     *  Now lets see if it looks like we can close the spoolfile      *
     *  Check for the following:                                      *
     *    1) We are at level 1 for spool files...                     *
     *       (otherwise simply decrement the count and return)        *
     *    2) We have an open spool file                               *
     *    3) No errors have occured                                   *
     * ************************************************************** */

    if (open_count > 1)
        {
        open_count--;
        return;
        };

    if ((data_file_ptr == (KS_FILE_PTR) NULL) ||
        (port_status != PORT_SPOOLING))
        {
        return;
        };


    /* ************************************************************** *
     *  Close the spool file and reset the file pointer to be NULL    *
     *  so we know that the file has been closed.                     *
     *                                                                *
     *  Note: I haven't added error checking here because there really*
     *        isn't anything we can do if an error occurs...          *
     * ************************************************************** */

    KS_FILE_CLOSE(data_file_ptr,
                  error);

    data_file_ptr = NULL;
    open_count = 0;
    port_status = PORT_NOT_ACTIVE;

    lpdport = pgetport("LPD");
    if (lpdport == -1) {
        error_message = DAEMON_NOT_ACTIVE;
        goto ERROR_DAEMON_NOT_ACTIVE;
    }
    msg = (1l << 16) | getpid();
    psend(lpdport,msg);
    while ((cfid = (int) procreceive()) != -1);

    sprintf(cf,PATH_LPQ ":cf.%05d",cfid);
    slf = open(cf,O_WRONLY);
    if (slf < 0) { SysBeep(); SysBeep(); }
    write(slf,"l",1);
    write(slf,tmp,strlen(tmp));
    write(slf,"\r",1);
    write(slf,"U",1);
    write(slf,tmp,strlen(tmp));
    write(slf,"\r",1);
    close(slf);
    /* register the job */
    msg = (2l << 16) | cfid;
    if (psend(lpdport,msg) == -1) SysBeep();

    /* ************************************************************** *
     *  Everything is all set - return to our caller...               *
     * ************************************************************** */

    return;

ERROR_DAEMON_NOT_ACTIVE:

    /* ************************************************************** *
     *  First we close the spool file (can't use it anymore now...)   *
     *  then give our user some feed back (like an error message).    *
     * ************************************************************** */

    KS_FILE_CLOSE(data_file_ptr, error2);
    /* delete the spool file */
    remove(tmp);

    data_file_ptr = (KS_FILE_PTR) NULL;
    open_count--;
    port_status = PORT_ERROR;

    ERROR_DIALOG(error_message, error, button);

    return;
}
