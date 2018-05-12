
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

#include "21/ks.fileio.e"

#include "21/ks.memory.h"

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


    /* ************************************************************** *
     *  Open our data file (in *:System:FilePort.Data), allocate a    *
     *  buffer to read the pathname into then read the target         *
     *  folder/directory into our buffer and set it up for use...     *
     *                                                                *
     *  Note: We allocate a word + 32 bytes more than the file        *
     *        length because we are allocating a GSString which will  *
     *        be our spool pathname.  This means that the word is a   *
     *        length word and the 32 bytes is where we will build the *
     *        spoolfile name.                                         *
     * ************************************************************** */

    KS_FILE_OPEN((GSString255Ptr) &fileport_datapath,
                 KS_FILE_READ_ACCESS,
                 KS_FILE_DATA_FORK,
                 KS_FILE_NO_BUFFER,
                 &dir_file_ptr,
                 error);
    if (error != KS_E_SUCCESS)
        {
        error_message = CANT_OPEN_CONFIG_FILE;
        goto ERROR_CANT_OPEN_CONFIG_FILE;
        };

    pathname_length = (dir_file_ptr->eof + sizeof(Word) +
                       SPOOL_FILENAME_EXTENSION +
                       BASE_SPOOL_FILENAME_LENGTH +
                       10 /* Just in case... */ );

    KS_MEMORY_ALLOCATE(attrNoSpec + attrLocked,
                       pathname_length,
                       ProgramID,
                       spoolpath_handle,
                       error);
    if (error != KS_E_SUCCESS)
        {
        error_message = CANT_ALLOCATE_PATHNAME;
        goto ERROR_CANT_ALLOCATE_PATHNAME;
        };

    spool_directory_ptr = (GSString32Ptr) *spoolpath_handle;

    KS_FILE_READ(dir_file_ptr,
                 0,
                 (LongWord) pathname_length,
                 TO_POINTER(&spool_directory_ptr->text),
                 error);
    if (error != KS_E_SUCCESS)
        {
        error_message = CANT_READ_PATHNAME;
        goto ERROR_CANT_READ_PATHNAME;
        };


    /* ************************************************************** *
     *  Scan the data we've read from FilePort.Data looking for two   *
     *  items:                                                        *
     *     1) The end of the string (first non-printable character)   *
     *     2) Delimiter to use between directory and filename...      *
     *  After we come to what looks like the end of the string, then  *
     *  we have to save the length.                                   *
     * ************************************************************** */

    spool_directory_ptr->length = 0;
    delimiter = ' ';

    for (index = 1;
         (index < pathname_length) &&
          (spool_directory_ptr->text[index] > ' ');
         index++)
        {
        if ((delimiter == ' ') &&
            ((spool_directory_ptr->text[index] == ':') ||
             (spool_directory_ptr->text[index] == '/')))
           {
           delimiter = spool_directory_ptr->text[index];
           };
        };
    if (delimiter = ' ')
        {
        delimiter = ':';
        };

    if (spool_directory_ptr->text[index] <= ' ')
        {
        spool_directory_ptr->length = index;
        spool_directory_ptr->text[index] = '\00';
        };
    if (spool_directory_ptr->length == 0)
        {
        spool_directory_ptr->length = (Word) pathname_length-1;
        };

    KS_FILE_CLOSE(dir_file_ptr, error2);


    /* ************************************************************** *
     *  Open the spool directory...                                   *
     * ************************************************************** */

    KS_FILE_OPEN((GSString255Ptr) spool_directory_ptr,
                 KS_FILE_READ_ACCESS,
                 KS_FILE_DATA_FORK,
                 KS_FILE_NO_BUFFER,
                 &dir_file_ptr,
                 error);
    if (error != KS_E_SUCCESS)
        {
        error_message = CANT_OPEN_DIR;
        goto ERROR_CANT_OPEN_DIR;
        };


    /* ************************************************************** *
     *  Get the number of entries in the directory...                 *
     * ************************************************************** */

    dir_entry.pCount = 10;
    next_filename.bufSize = 255;
    ZERO(next_filename.bufString);
    dir_entry.name = &next_filename;
    dir_entry.optionList = (ResultBuf255Ptr) &(optionList[0]);
    optionList[0] = sizeof(optionList) - sizeof(Word);

    KS_FILE_GET_NEXT_FILE(dir_file_ptr, &dir_entry, error);

    if (error != KS_E_SUCCESS)
        {
        error_message = CANT_GET_ENTRY_COUNT;
        goto ERROR_CANT_GET_ENTRY_COUNT;
        };


    /* ************************************************************** *
     *  Go looking through the directory entries trying to determine  *
     *  the largest spool file number.                                *
     * ************************************************************** */

    num_entries = dir_entry.entryNum;

    max_spool_number = 0;

    for (index = 0;
         index < num_entries;
         index++)
        {
        dir_entry.pCount = 10;
        next_filename.bufSize = 255;
        ZERO(next_filename.bufString);
        dir_entry.name = &next_filename;
        dir_entry.optionList = (ResultBuf255Ptr) &(optionList[0]);
        optionList[0] = sizeof(optionList) - sizeof(Word);

        KS_FILE_GET_NEXT_FILE(dir_file_ptr, &dir_entry, error);

        if (error != KS_E_SUCCESS)
            {
            error_message = CANT_GET_NEXT_FILE;
            goto ERROR_CANT_GET_NEXT_FILE;
            };


        /* ********************************************************** *
         *  Check if this directory entry is a spool file that we     *
         *  created.                                                  *
         *                                                            *
         *  The check will only be done on the first N-2 characters   *
         *  (if the filename is that long).                           *
         * ********************************************************** */

        next_filename.bufString.text[next_filename.bufString.length] =
              '\0';
        if (next_filename.bufString.length ==
               (int) (BASE_SPOOL_FILENAME_LENGTH+2))
            {
            for (index2 = 0;
                 (index2 < BASE_SPOOL_FILENAME_LENGTH) &&
                  (next_filename.bufString.text[index2] ==
                      base_spool_filename[index2]);
                 index2++)
                ;
            if (index2 == BASE_SPOOL_FILENAME_LENGTH)
                {

                /* ************************************************** *
                 *  The filename matches!  Lets try to extract the    *
                 *  the spool file number.                            *
                 *                                                    *
                 *  Note: In order to correctly determine the next    *
                 *        available spoolfile number we have to check *
                 *        to see that the last two characters are     *
                 *        digits (0-9).                               *
                 * ************************************************** */

                spoolfile_number = 0;

                cnumber = next_filename.bufString.text[
                            BASE_SPOOL_FILENAME_LENGTH];

                if ((cnumber >= '0') && (cnumber <= '9'))
                    {
                    spoolfile_number = (Word) (cnumber - '0');


                    cnumber = next_filename.bufString.text[
                            BASE_SPOOL_FILENAME_LENGTH+1];

                    if ((cnumber >= '0') && (cnumber <= '9'))
                        {
                        spoolfile_number = (spoolfile_number * 10) +
                                            ((Word) (cnumber - '0'));


                        /* ****************************************** *
                         *  If the current spool file number is       *
                         *  greater than any others, then lets        *
                         *  remember the new spool file number.       *
                         * ****************************************** */

                        if (spoolfile_number > max_spool_number)
                            {
                            max_spool_number = spoolfile_number;
                            };

                        };  /* End if last character is a digit       */

                    };  /* End if the next to last character is digit */

                };  /* End if the filename matches...                 */

            };  /* End if the filename is the correct length          */

        };  /* End of for all filenames in the spool directory        */



    /* ************************************************************** *
     *  Before anything bad happens - lets close the spool            *
     *  directory                                                     *
     * ************************************************************** */

    KS_FILE_CLOSE(dir_file_ptr, error2);


    /* ************************************************************** *
     *  Now lets build the pathname to the spool data file            *
     *  We'll do this by simply appending a 'delimiter'spoolfilename  *
     *  on to the directory path.                                     *
     * ************************************************************** */

    if (spool_directory_ptr->text[spool_directory_ptr->length] !=
        delimiter)
       {
       spool_directory_ptr->text[spool_directory_ptr->length] =
           delimiter;
       spool_directory_ptr->length -= 1;
       };

    COPY_BYTES(&base_spool_filename,
               0,
               spool_directory_ptr->text,
               spool_directory_ptr->length+1,
               BASE_SPOOL_FILENAME_LENGTH);

    spool_directory_ptr->length += (1 + BASE_SPOOL_FILENAME_LENGTH);

    max_spool_number = max_spool_number + 1;
    if (max_spool_number > 99)
        max_spool_number = 1;



    /* ************************************************************** *
     *  Finally, lets append a spool file number onto the end of the  *
     *  spool path.  We'll loop around till we either get a good      *
     *  spool file number, or we give up...                           *
     * ************************************************************** */

RETRY_BUILDING_PATH:

    spool_directory_ptr->text[spool_directory_ptr->length + 1] = (char)
                              ((max_spool_number % 10) + '0');

    spool_directory_ptr->text[spool_directory_ptr->length] =
                              (((max_spool_number/10) % 10) + '0');

    spool_directory_ptr->length += 2;

    spool_directory_ptr->text[spool_directory_ptr->length] = '\00';


    /* ************************************************************** *
     *  Create the data spool file.                                   *
     * ************************************************************** */

    KS_FILE_CREATE((GSString255Ptr) spool_directory_ptr,
                   SPOOLFILE_FILETYPE,
                   SPOOLFILE_AUXTYPE,
                   error);
    if (error != KS_E_SUCCESS)
        {
        if (error == dupPathname)
            {
            if (max_spool_number < 99)
                {
                spool_directory_ptr->length -= 2;

                max_spool_number++;

                goto RETRY_BUILDING_PATH;
                }
            else
                {
                error_message = TOO_MANY_SPOOLFILES;
                goto ERROR_TOO_MANY_SPOOLFILES;
                };
            };

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
char buf[10], *b;
int lpdpid,slf;

    /* ************************************************************** *
     *  No local variables...                                         *
     * ************************************************************** */


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

    while ((slf = open("31:usr:spool:lpq:lock",O_RDONLY)) == -1)
       sleep(2);
    read(slf,buf,10);
    close(slf);
    lpdpid = 0; b = buf;
    while (isdigit(*b)) lpdpid = lpdpid * 10 + (*b++ - '0');
    kill(lpdpid,SIGUSR1);

    /* ************************************************************** *
     *  Everything is all set - return to our caller...               *
     * ************************************************************** */

    return;
}
