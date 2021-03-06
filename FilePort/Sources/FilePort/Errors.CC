
/*
 * Copyright (c) Kopriha Software,  1991
 *       All Rights Reserved
 *
 * Errors.CC
 *
 * Description:
 *     This module is the heart of the file support of the
 *     printer port driver.
 *
 *
 *   Table of Contents:
 *
 *     error_dialog . . . . . . . . . Display an error dialog for
 *                                    our user - return whatever
 *                                    button they click on.
 *
 *
 *  History:April 13, 1991  Dave  Created this file
 *
 */

#pragma noroot

/*
 *  define DEBUG_CODE
 *                     - add # to define to create the local
 *                       debug code (IE:module)
 */

#include "errors.h"

#ifndef __WINDOW__
#include <Window.h>
#endif


char        error_string1[80];

Pointer     substitute_array[10];


extern Pointer error_strings[NUMBER_OF_ERROR_CODES+1];



/* ****************************************************************** *
 *   error_dialog - display an error dialog and return the selected   *
 *                  button (typically the 'Ok' button).               *
 *                                                                    *
 *   History: April 13, 1991  Dave  Created this routine              *
 * ****************************************************************** */

#define ROUTINE_NAME "error_dialog"

Word error_dialog(Word error_message, Word error_code)
{
    Word        button;


    /* ************************************************************** *
     *  Check for validity of the error code - if it is not a known   *
     *  error code then we will use the 'invalid error' message.      *
     * ************************************************************** */

    if ((error_message < 0) || (error_message > NUMBER_OF_ERROR_CODES))
        {
        error_message = NUMBER_OF_ERROR_CODES+1;
        };


    /* ************************************************************** *
     *  Setup the substitution strings...                             *
     * ************************************************************** */


    /* ************************************************************** *
     *  Check if the window manager is active or not...               *
     * ************************************************************** */

    if (WindStatus() != 0)
        {
        
        /* ********************************************************** *
         *  The window manager is available... so we are allowed to   *
         *  put up a simple alert window.  Setup the substitution     *
         *  array and put up the dialog...                            *
         * ********************************************************** */

        if (error_message == TOO_MANY_SPOOLFILES)
            {
            sprintf(error_string1,
                    "32/FilePort error - *0/^#0",
                    error_code);
            }
        else
            {
            sprintf(error_string1,
                    "32/FilePort error - *0 (error code $%X)/^#0",
                    error_code);
            };

        substitute_array[0] = (Pointer) error_strings[
               ((error_message >= 0) ||
                (error_message < NUMBER_OF_ERROR_CODES)) ?
                  error_message :
                  (NUMBER_OF_ERROR_CODES+1)];

        button = AlertWindow((Word) 0,
                             (Long) &substitute_array,
                             (Long) error_string1);
        }
    else

        /* ********************************************************** *
         *  The window manager isn't active... that means we can't    *
         *  put up a simple alertwindow... print the error strings    *
         *  out to standard out... lets hope a user sees it.          *
         * ********************************************************** */
        {
        printf("FilePort error - %s", (char *) error_strings[
               ((error_message >= 0) ||
                (error_message < NUMBER_OF_ERROR_CODES)) ?
                  error_message :
                  (NUMBER_OF_ERROR_CODES+1)]);

        if (error_message != TOO_MANY_SPOOLFILES)
            {
            printf(" (error code $%X)\n", error_code);
            };

        button = 0;
        }

    /* ************************************************************** *
     *  Return the button that was pressed.                           *
     * ************************************************************** */

    return(button);


}   /* End of error_dialog()                                          */
