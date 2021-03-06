
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 * ks.fileio.e
 *
 * Description: This include file contains all the generic file
 *              support macros from Kopriha Software.
 *
 *
 * Table of contents:
 *
 *   Structures:
 *
 *     KS_FILE . . . . . . . . . Structure to abstract a file
 *
 *   Macros:
 *
 *   + KS_FILE_CLOSE()  . . . . . . . Close a file
 *   + KS_FILE_CHANGE_AUXTYPE() . . . Change the aux type of a file
 *   + KS_FILE_CREATE() . . . . . . . Create a file (w ftype,auxtype)
 *   + KS_FILE_DELETE() . . . . . . . Delete a file
 *   + KS_FILE_GET_BUFFER_SIZE(). . . Get the input/output buffer size
 *   + KS_FILE_GET_DIRECTORY()  . . . Get the current directory
 *   + KS_FILE_GET_NEXT_FILE()  . . . Get next filename from a directory
 *   + KS_FILE_GET_VOLUME() . . . . . Get volume name from pathname
 *   + KS_FILE_GET_VOLUME_SPACE() . . Get the total/available space on
 *                                    the specified volume
 *   + KS_FILE_OPEN() . . . . . . . . Open a file
 *   + KS_FILE_PATH_ADD_FILE()  . . . Append a filename to a path/dir
 *   + KS_FILE_READ() . . . . . . . . Read from a file
 *     KS_FILE_RENAME() . . . . . . . Rename a file (given pathnames)
 *   + KS_FILE_SET_BUFFER_SIZE(). . . Set the input/output buffer size
 *   * KS_FILE_SET_DIRECTORY()  . . . Set the current directory
 *   + KS_FILE_WRITE()  . . . . . . . Write to a file
 *
 *  + indicates that the routine was debugged
 *  * indicates the routine exists but was not debugged
 *
 *
 *  Notes:
 *
 *  History:Oct 13, 1990  Dave  Created this file for basic
 *                              I/O (read/write)
 *
 *          Feb 25, 1991  Dave  Added buffer enhancements/create.
 *
 *          Mar 5, 1991   Dave  Added directory, file, path and
 *                              volume calls
 *
 *          Mar 24, 1991  Dave  Added delete and change aux type
 *
 *          Jun 07, 1991  Dave  Broke the ks.fileio.cc module into
 *                              lots of pieces so we could easily
 *                              build a library.
 *
 *          Jun 30, 1991  Dave  Added a rename function.
 *
 */

#ifndef _KS_FILEIO_E_
#define _KS_FILEIO_E_

#ifndef _KS_DEFINES_
#include "../includes/ks.defines.h"
#endif

#ifndef _GSOS_
#include <gsos.h>
#endif


/*
 * Buffer flag constants - used to determine if a buffer should
 * be used for I/O.  The idea here is that if you specify a buffer,
 * then you have to read/write without specifying a byte position
 * (KS_NEXT_FILE_POSITION).
 */

#define KS_FILE_NO_BUFFER ((Word) 0)
#define KS_FILE_BUFFER_IO ((Word) 1)

#define KS_NEXT_FILE_POSITION ((Word) -1)

/*
 * The following are definitions that I expected to find in the
 * APW 1.1/Orca/C update, but could not.  I want to be able to
 * recompile without worrying about hard-coding constants in my
 * program, I defined the following macros:
 */

#define KS_FILE_DATA_FORK      ((Word) 0)       /* For GS/OS Open */
#define KS_FILE_RESOURCE_FORK  ((Word) 1)

#define KS_FILE_READ_ACCESS    ((Word) readEnable)
#define KS_FILE_WRITE_ACCESS   ((Word) writeEnable)

#define KS_BLOCK_DEVICE ((Word) 0x0080)  /* used with DInfo */

/*
 * KS_FILE - generic file structure
 */

typedef struct
    {
    KS_STRUCT_ID struct_id;/* Structure id                           */
    Handle   file_handle;  /* Handle that created this structure     */
    Word     refNum;       /* Reference # (or channel #) of the file */
    Word     access;       /* Access mode of file (read or write)    */
    LongWord eof;          /* End of File of data fork               */
    LongWord resource_eof; /* End of file for resource fork          */
                           /* Note: Not all systems support filetypes*/
    Word     fileType;     /* Value of file's fileType attribute     */
    LongWord auxType;      /* Auxilary file type                     */

    LongWord data_size;    /* Amount of data read/written for user   */

    Word     end_of_file;  /* TRUE==We've read to end of file        */
    Word     end_of_dir;   /* TRUE==We've read to end of directory   */

    Handle   buffer_handle;
                           /* Handle to the buffer (if specified)    */
    LongWord buffer_size;  /* Size of the buffer (NULL if not        */
                           /*  specified)                            */
    LongWord buffer_available;
                           /* Number of bytes read into the buffer   */
                           /*  Typically this offset is the same as  */
                           /*  buffer_size, however end of file is a */
                           /*  special case where the number of      */
                           /*  read bytes typically can't be the same*/
                           /*  the size of the buffer.               */
    Byte    *buffer;       /* Pointer to the file buffer             */
    LongWord buffer_offset;
                           /* Offset of next available byte in buffer*/

    } KS_FILE, *KS_FILE_PTR, **KS_FILE_HDL;



/*
 * KS_FILE_CLOSE() - close a file.
 */

#define KS_FILE_CLOSE(_file_ptr, _error)                               \
                                                                       \
    (_error) = ks_file_close( (_file_ptr))


/*
 * KS_FILE_CREATE() - creates a file with the specified name, filetype
 *                    and auxtype.
 */

#define KS_FILE_CREATE(_gspath_ptr, _file_type, _auxtype, _error)      \
                                                                       \
    (_error) = ks_file_create( (_gspath_ptr),                          \
                               (_file_type),                           \
                               (_auxtype))


/*
 * KS_FILE_CHANGE_AUXTYPE() - change the auxtype of an existing file.
 *
 * Note: This call is only works when the file is closed!
 */

#define KS_FILE_CHANGE_AUXTYPE(_gspath_ptr, _auxtype, _error)          \
                                                                       \
    (_error) = ks_file_change_auxtype( (_gspath_ptr),                  \
                                       (_auxtype))


/*
 * KS_FILE_DELETE() - Deletes the specified file
 */

#define KS_FILE_DELETE(_gspath_ptr, _error)                            \
                                                                       \
    (_error) = ks_file_delete( (_gspath_ptr))


/*
 * KS_FILE_GET_BUFFER_SIZE() - returns the current buffer size used
 *
 * Note: If the buffer size can only be changed if there are no
 *       open files (this restriction will change in the future).
 */

#define KS_FILE_GET_BUFFER_SIZE(_buffer_size, _error)                  \
                                                                       \
    (_error) = ks_file_get_buffer_size( (_buffer_size))


/*
 * KS_FILE_GET_NEXT_FILE(FILE_PTR, DirEntryRecPtrGS) - Get next
 *                                    filename from a directory
 */

#define KS_FILE_GET_NEXT_FILE(_file_ptr, _next_file_ptr, _error)        \
                                                                        \
    (_error) = ks_file_get_next_file( (_file_ptr),                      \
                                      (_next_file_ptr))


/*
 * KS_FILE_GET_VOLUME() - Get volume name from pathname
 */

#define KS_FILE_GET_VOLUME(_file_ptr, _volume_ptr, _error)              \
                                                                        \
    (_error) = ks_file_get_volume( (_file_ptr),                         \
                                   (_volume_ptr))


/*
 * KS_FILE_GET_VOLUME_SPACE() - Get the total/available space on
 *                              the specified volume
 */

#define KS_FILE_GET_VOLUME_SPACE(_volume_ptr, _unused, _total, _error)  \
                                                                        \
    (_error) = ks_file_get_volume_space( (_volume_ptr),                 \
                                         (_unused),                     \
                                         (_total))


/*
 * KS_FILE_OPEN() - opens a file (either data or resource fork)
 *                 returns a KS_FILE structure to our caller.
 */

#define KS_FILE_OPEN(_gspath_ptr, _access, _file_fork, _buf_flag, _file_ptr_ptr, _error)\
                                                                       \
    (_error) = ks_file_open( (_gspath_ptr),                            \
                             (_access),                                \
                             (_file_fork),                             \
                             (_buf_flag),                              \
                             (_file_ptr_ptr))


/*
 * KS_FILE_PATH_ADD_FILE() - Append a filename to a path/dir
 */

#define KS_FILE_PATH_ADD_FILE(_path_ptr, _file_ptr, _error)             \
                                                                        \
    (_error) = ks_file_path_add_file( (_path_ptr),                      \
                                      (_file_ptr))


/*
 * KS_FILE_READ() - read from a file.
 */

#define KS_FILE_READ(_file_ptr, _position, _byte_size, _buffer, _error)\
                                                                       \
    (_error) = ks_file_read( (_file_ptr),                              \
                             (_position),                              \
                             (_byte_size),                             \
                             (_buffer))


/*
 * KS_FILE_RENAME() - rename a file.
 */

#define KS_FILE_RENAME(_old_pathname, _new_pathname, _error)           \
                                                                       \
   (_error) = ks_file_rename( (_old_pathname),                         \
                              (_new_pathname))


/*
 * KS_FILE_SET_BUFFER_SIZE() - sets the current buffer size used
 *
 * Note: If the buffer size can only be changed if there are no
 *       open files (this restriction will change in the future).
 */

#define KS_FILE_SET_BUFFER_SIZE(_buffer_size, _error)                  \
                                                                       \
    (_error) = ks_file_set_buffer_size( (_buffer_size))


/*
 * KS_FILE_SET_DIRECTORY() - Set the current directory
 */

#define KS_FILE_SET_DIRECTORY(_directory_ptr, _error)                  \
                                                                       \
    (_error) = ks_file_set_directory( (_directory_ptr))


/*
 * KS_FILE_SET_EOF() - set the end of the specified file
 */

#define KS_FILE_SET_EOF(_file_ptr, _eof, _error)                       \
                                                                       \
     (_error) = ks_file_set_eof( (_file_ptr),                          \
                                 (_eof))


/*
 * KS_FILE_WRITE() - write to a file.
 */

#define KS_FILE_WRITE(_file_ptr, _position, _byte_size, _buffer, _error)\
                                                                       \
    (_error) = ks_file_write( (_file_ptr),                             \
                              (_position),                             \
                              (_byte_size),                            \
                              (_buffer))



/* ****************************************************************** *
 *  File I/O routine prototypes:                                      *
 * ****************************************************************** */

KS_E_ERROR ks_file_close(KS_FILE_PTR);

KS_E_ERROR ks_file_create(GSString255Ptr,
                          Word,
                          LongWord);

KS_E_ERROR ks_file_change_auxtype(GSString255Ptr,
                                  LongWord);

KS_E_ERROR ks_file_get_buffer_size(LongWord *);

KS_E_ERROR ks_file_delete(GSString255Ptr);

KS_E_ERROR ks_file_get_next_file(KS_FILE_PTR,
                                 DirEntryRecPtrGS);

KS_E_ERROR ks_file_get_volume(GSString255Ptr,
                              GSString255Ptr);

KS_E_ERROR ks_file_get_volume_space(GSString255Ptr,
                                    LongWord *,
                                    LongWord *);

KS_E_ERROR ks_file_open(GSString255Ptr,
                        Word,
                        Word,
                        Word,
                        KS_FILE_PTR *);

KS_E_ERROR ks_file_read(KS_FILE_PTR,
                        LongWord,
                        LongWord,
                        Pointer);

KS_E_ERROR ks_file_rename(GSString255Ptr,
                          GSString255Ptr);

KS_E_ERROR ks_file_path_add_file(GSString255Ptr,
                                 GSString255Ptr);

KS_E_ERROR ks_file_set_buffer_size(LongWord);

KS_E_ERROR ks_file_set_directory(GSString255Ptr);

KS_E_ERROR ks_file_set_eof(KS_FILE_PTR,
                           LongWord);

KS_E_ERROR ks_file_write(KS_FILE_PTR,
                         LongWord,
                         LongWord,
                         Pointer);

#endif
