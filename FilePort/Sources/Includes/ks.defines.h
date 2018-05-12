
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 * ks.defines.h
 *
 * Description: This include file contains all the master
 *              definitions.
 *
 *
 * Table of contents:
 *
 *   Structures:
 *
 *     KS_STRUCT_ID . . . . . . . . . Structure for structure id.
 *
 *   External variables:
 *
 *     ks_expected_id . . . . . . . . Structure ID that was expected
 *                                    in the called routine (this will
 *                                    give the error processing code
 *                                    a hint as to what structure caused
 *                                    the error).
 *
 *
 *   Macros:
 *
 *     KS_SUCCESS() . . . . . . . . . Return to a calling procedure
 *                                    with a success.
 *     KS_ERROR() . . . . . . . . . . Return to a calling procedure
 *                                    with an error.
 *
 *
 *  Structure IDs for all Kopriha Software Structures:
 *
 *    KS_FILE_ID  . . . . . . . . . . KS FILE structure id
 *
 *
 *  Error codes for all Kopriha Software:
 *
 *    KS_E_SUCCESS . . . . . . . Success (same as ((word) 0))
 *    KS_E_INVALID_STRUCT_ID . . Structure has a bad structure id
 *    KS_E_INVALID_OPTION  . . . Invalid option for a routine
 *
 *
 *  History:July 13, 1990  Dave  Created this file
 *
 *          March 3, 1991  Dave  Added KS_FILE_ID
 *
 */

#ifndef _KS_DEFINES_
#define _KS_DEFINES_

#ifndef __TYPES__
#include <types.h>
#endif

#ifndef _PORTABLE_C_
#include "../includes/Portable.C.h"
#endif

#ifndef _KS_ROUTINES_
#include "../includes/ks.routines.h"
#endif

#ifndef __orca__
#include <orca.h>
#endif


#ifndef EXTERNAL
#define EXTERNAL extern
#endif



/* ****************************************************************** *
 * Structure definitions:                                             *
 * ****************************************************************** */

/*
 *     KS_E_ERROR . . . . . . . . . Type for any error code returned.
 */

typedef Word  KS_E_ERROR;


/*
 *     KS_STRUCT_ID . . . . . . . . . Structure for structure id.
 *                   (IE: a unique identifier for each structure)
 */

typedef Longword KS_STRUCT_ID;



/* ****************************************************************** *
 * External definitions:                                              *
 * ****************************************************************** */


/* ****************************************************************** *
 *   ks_expected_id - The structure id of the structure we took an    *
 *                    error dealing with (this may indicate what was  *
 *                    happening when an error occured...)  For memErr *
 *                    it indicates which structure we couldn't get    *
 *                    allocate.                                       *
 *                                                                    *
 *   History: July 13, 1990  Dave  Created these structures           *
 * ****************************************************************** */

EXTERNAL KS_STRUCT_ID ks_expected_id;





/* ****************************************************************** *
 * Macro definitions:                                                 *
 * ****************************************************************** */

/*
 *  define DEBUG_CODE
 *                     - add # to define to create all modules
 *                       with debug code.
 */

/*
 * KS_SUCCESS macro - used to return a success to our caller.
 */

#define KS_SUCCESS()                                                 \
                                                                     \
        ROUTINE_EXIT( KS_E_SUCCESS );                                \
        return( KS_E_SUCCESS )


/*
 * KS_ERROR macro - used to return an error to our caller.
 */

#define KS_ERROR(_error, _struct_id)                                 \
                                                                     \
        ks_expected_id = (_struct_id);                               \
        ROUTINE_EXIT( (_error) );                                    \
        return( (_error) )



/*
 * ProgramID - User ID of this program.  Used to allocate memory
 *             through the memory manager.
 *
 * Note: Secondary user IDs used throughout this program are:
 *       UserID + 0x0100: Used by Orca/C and Tools loaded
 *       UserID + 0x0200: Used for list management
 *       UserID + 0x0300: Used to load the port driver
 *       UserID + 0x0400: Used to allocate memory for buffers/shared data
 */

EXTERNAL Word ProgramID;

#define WORK_USERID   (ProgramID + 0x0100)
#define LIST_USERID   (ProgramID + 0x0200)
#define PORT_USERID   (ProgramID + 0x0300)
#define BUFFER_USERID (ProgramID + 0x0400)



/*
 *  Structure IDs for all Kopriha Software Structures.
 */

#define KS_BASE_STRUCT_ID  0xcbd30000  /* Start of structure ids    */
                                       /* (cbd3 == "KS" for Kopriha */
                                       /*  Software).               */

#define KS_FILE_ID KS_BASE_STRUCT_ID+1

#define KS_SHARED_DATA_ID KS_FILE_ID+1



/*
 *  Define all the Kopriha Software specific error codes:
 */

#define KS_E_BASE_ID   0xcb00

#define KS_E_SUCCESS  noError
        /* Success (same as ((word) 0)) */

#define KS_E_INVALID_STRUCT_ID  KS_E_BASE_ID+1
        /* The expected structure has a corrupted structure id */

#define KS_E_INVALID_OPTION KS_E_INVALID_STRUCT_ID+1
        /* A routine received an invalid option - this indicates   */
        /*  a programming error somewhere (usually the error       */
        /*  returning function is being called incorrectly).       */



#endif
