
/* sum -- checksum and count the blocks in a file
   Copyright (C) 1986, 1989, 1991 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Like BSD sum or SysV sum -r; results depend on integer size. */
/* Written by Kayvan Aghaiepour and David MacKenzie. */
/* VERY Crude Mega-Alpha port to GNO shell by Marek Pawlowski, Sep 28 1991 */
/* Updated to work without system.h October 13 1991, Marek Pawlowski */
/* Special thanks to Martin Hill for tips here and there - Marek */

#include <stdio.h>
#include <types.h>
#include <shell.h>
#pragma keep "sum"
#pragma optimize 63
#pragma stacksize 0x0400   /* Let's keep Tim happy with these two */

int sum_file ();

/* The name this program was run with. */
char *program_name;

/* Nonzero if any of the files read were the standard input. */
int have_read_stdin;

/* Right-rotate 32-bit integer variable C. */
#define ROTATE(c) if ((c) & 01) (c) = ((c) >>1) + 0x8000; else (c) >>= 1;

void
main (argc, argv)
     int argc;
     char **argv;
{
  int errors;
  int optind;
  int errno;

  program_name = argv[0];
  have_read_stdin = 0;
  errors = 0;
  optind = 1;
  if (optind == argc)
    {
      if (sum_file ("-", 0) < 0)
        errors = 1;
    }
  else
    for (; optind < argc; optind++)
      if (sum_file (argv[optind], 1) < 0)
        errors = 1;

  if (have_read_stdin && fclose (stdin) == EOF)
  exit (errors);
}

/* Calculate and print the checksum and size in 1K blocks
   of file FILE, or of the standard input if FILE is "-".
   If PRINT_NAME is nonzero, print FILE next to the checksum and size.
   Return 0 if successful, -1 if an error occurs. */

int
sum_file (file, print_name)
     char *file;
     int print_name;
{
  int errno;
  register FILE *readfile;
  register unsigned long checksum; /* The checksum mod 2^16. */
  register long bytenum;        /* The number of bytes. */
  register int ch;              /* Each character read. */
  int stop;

  if (!strcmp (file, "-"))
    {
      readfile = stdin;
      have_read_stdin = 1;
    }
  else
    {
      readfile = fopen (file, "r");
      if (readfile == NULL)
        {
          return -1;
        }
    }
 
  checksum = 0;
  bytenum = 0;

  while ((ch = getc(readfile)) != EOF)
    {
      STOP(&stop);
      if (stop)
         exit(0);
      bytenum++;
      ROTATE(checksum);
      checksum += ch;
      checksum &= 0xffff;       /* Keep it within bounds. */
    }

  if (ferror (readfile))
    {
      if (strcmp (file, "-"))
        fclose (readfile);
      return -1;
    }

  if (strcmp (file, "-") && fclose (readfile) == EOF)
    {
      return -1;
    }

  printf ("%05lu %5ld", checksum, (bytenum + 1024 - 1) / 1024);
  if (print_name)
    printf (" %s", file);
  putchar ('\n');

  return 0;
}
