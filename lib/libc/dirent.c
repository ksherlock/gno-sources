/*
 * simple implementation of directory(3) routines for V7 and Minix.
 * not designed to be efficient.
 * missing telldir and seekdir.

 Modifications for Apple IIgs GS/OS made by Derek Taubert and Jawaid Bazyar

 */
segment "lbsd      ";

#pragma noroot

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <gsos.h>
#include <dirent.h>
#include <shell.h>

#define DIRSIZ  15

DirEntryRecGS entr = {6,0,0,0x0001,0x0001,NULL,0};

DIR *opendir(char *filename)
{
        static OpenRecGS openme = {2,0,NULL};
        static Expand_DevicesPB expnd;
        char nameonly[255];
        DIR *dirp;
        GSString255Ptr fname;

        entr.pCount = 6;
        entr.base = 1;
        entr.displacement = 1;

        dirp = (DIR *) malloc(sizeof(DIR));
        if (dirp == NULL)
                return NULL;
        fname = (GSString255Ptr) malloc(sizeof(GSString255));
        if (fname == NULL) {
                free((char *) dirp);
                return NULL;
        }

        nameonly[0]=(char)strlen(filename);   strcpy(nameonly+1,filename);
        expnd.pathname = nameonly;
        EXPAND_DEVICES(&expnd);
        if (toolerror()) {
                free((char *) dirp);
                free((char *) fname);
                return NULL;
        }
        strncpy(fname->text,nameonly+1,(fname->length = nameonly[0]));
	fname->text[fname->length] = 0;

        openme.pathname = fname;
        OpenGS(&openme);
        if (toolerror()) {
                free((char *) dirp);
                free((char *) fname);
                return NULL;
        }
        free((char *) fname);
        dirp->fd = openme.refNum;
        return dirp;
}

struct dirent *readdir(dirp)
        register DIR *dirp;
{
        entr.name = (ResultBuf255Ptr) malloc(sizeof(ResultBuf32)+1);
        if (entr.name == NULL)
            return NULL;
        entr.name->bufSize = 36;
        entr.refNum = dirp->fd;
        GetDirEntryGS(&entr);
        if (toolerror()) {
            free((char *)entr.name);
            return (struct dirent *) NULL;
        }
        dirp->ent.d_ino = entr.entryNum;
        strncpy(dirp->ent.d_name, entr.name->bufString.text,
            entr.name->bufString.length);
        dirp->ent.d_name[dirp->ent.d_namlen = entr.name->bufString.length] = 0;
        free((char *)entr.name);
        return &(dirp->ent);
}

void rewinddir(dirp)
        DIR *dirp;
{
        entr.pCount = 6;
        entr.base = 0;
        entr.displacement = 0;

        GetDirEntryGS(&entr);
        entr.base = 1;
        entr.displacement = 1;
}

closedir(dirp)
        DIR *dirp;
{
        static RefNumRecGS closeme = {1,0};

        closeme.refNum = dirp->fd;
        CloseGS(&closeme);
        dirp->fd = -1;
        free((char *) dirp);
        return 0;
}
