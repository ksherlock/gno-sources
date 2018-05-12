/*                

Chmod for GNO v0.50                                       James Brookes

This version will work just fine for one filename, error handling hasn't
really been implemented yet.  Won't work for multiple filenames, I plan
to add that soon (as well as generally cleaning up the program).  You can
use both octal numbers and flags as arguments.  Allowable flags are +,-
and = and then rwbndi.  Note that this version also is only guarenteed to
work with the ProDOS FST ... I have no idea what will happen if you try it
on an HFS volume, although I plan to add any/all HFS support I can.  Also,
freeing NameStruct caused the program to hang sometimes (had to ^C out)
so I commented that out ^_^ and havn't had any problems yet.  If anyone can
tell me why that's happening, I'd appreciate it.

*/

#pragma stacksize 1024

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<ctype.h>
#include<gsos.h>

int usage(char *modname)

   {
   printf("\nChmod v0.5.  Single file use at the moment\n");
   printf("\n\tUsage: %s [octnum]|[+-=][rwbndi] file ...\n",modname);
   exit(0);
   }

int main (int argc, char *argv[])

   {
   int usage(char *modname);
   unsigned int octnum, junk;
   int i, plus,backupset = FALSE;
   char flag;
   FileInfoRecGS *InfoStruct;
   NameRecGS *NameStruct;

   if (argc != 3)
      usage(argv[0]);

   plus = TRUE;   /* defaults to 'set' */

   if (isdigit(argv[1][0]))       /* octal mode given */

      {
      sscanf(argv[1],"%o",&octnum);
      chmod(argv[2],octnum);
      }

   else            /* handle flags */

      {
      i = 0;
      InfoStruct = (FileInfoRecGS *) malloc (sizeof(FileInfoRecGS));
      InfoStruct->pathname = malloc (sizeof(argv[2])+3);
      InfoStruct->pCount = 2;
      InfoStruct->pathname->length = (word) strlen(argv[2]);
      strcpy(InfoStruct->pathname->text,argv[2]);
      GetFileInfoGS(InfoStruct); 
      octnum = InfoStruct->access;
      if ((junk = octnum & backupNeeded) != 0)  /* backup bit set? */
         backupset = TRUE; 
      while (argv[1][i] != 0x00)

         {
         flag = argv[1][i];
         switch (flag)

            {
            case '='  :
               octnum = 0; 
               if (argv[1][i+1] == 'b') backupset = TRUE; /* handle =b */
               else backupset = FALSE;        /* rest of =flags */ 
            case '+'  :  plus = TRUE;   break;
            case '-'  :  plus = FALSE;  break;
            case 'r'  :
               if (plus)
                  octnum |= readEnable;
               else
                  octnum &= (~readEnable);
               break;
            case 'w'  :
               if (plus)
                  octnum |= writeEnable;
               else
                  octnum &= (~writeEnable);
               break;
            case 'd'  :
               if (plus)
                  octnum |= destroyEnable;
               else
                  octnum &= (~destroyEnable);
               break;
            case 'n'  :
               if (plus)
                  octnum |= renameEnable;
               else
                  octnum &= (~renameEnable);
               break;
            case 'i'  :
               if (plus)
                  octnum |= fileInvisible;
               else
                  octnum &= (~fileInvisible);
               break;
            case 'b'  :
               if (plus)
                  backupset = TRUE; 
               else
                  backupset = FALSE;   /* get rid of backup bit */
               break;
            default   :
               backupset = TRUE;
            }

         i++;
         }

      InfoStruct->access = octnum;
      SetFileInfoGS(InfoStruct);

      if (!backupset)

         {
         NameStruct = (NameRecGS *) malloc (sizeof(NameRecGS));
         NameStruct->pathname = malloc (sizeof(argv[02]+3));
         NameStruct->pCount = 1;
         NameStruct->pathname->length = InfoStruct->pathname->length;
         strcpy(NameStruct->pathname->text,InfoStruct->pathname->text);
         ClearBackupBit(NameStruct);
      /* free(NameStruct); */
         }

      }

   free(InfoStruct->pathname);
   free(InfoStruct);
   exit(0);
   }
