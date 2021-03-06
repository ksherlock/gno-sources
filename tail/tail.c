/***************************************************************************
 * Tail for the IIGS. It isn't exactly the same as standard UNIX tail, but *
 * it works pretty well. I use a similar algorithim to the one used in     *
 * fgrep: I read the file in 16k blocks, and scan through for newlines.    *
 * The position of each newline is stored in an array. Then the program    *
 * figures out which newline to choose, and then seeks to that point in    *
 * the file, and then dumps the file from that point in 16k blocks.        *
 ***************************************************************************/

/* Written by Sameer Parekh zane@ddsw1.MCS.COM
 * Version 1.1 18-Jan-93
 * Copyright 1993 Sameer Parekh
 * Distribute source and executable freely
 */

#pragma memorymodel 1
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define INPUT_BUFFER	0x4000
#define NL_SIZE 0x4000

typedef size_t *newl_t;
typedef char *inpt_t;


char version[] = "Version: Tail v1.1 18-Jan-93\n";
char author[] = "Author: Sameer Parekh\n";
char copyright[] = "Copyright 1993 Sameer Parekh\n";
char distribute[] = "Distribute freely\n";
char email[] = "zane@ddsw1.MCS.COM\n";

void print_version(void)
{
	fputs(version, stderr);
	fputs(author, stderr);
	fputs(copyright, stderr);
	fputs(distribute, stderr);
	fputs(email, stderr);
	exit(0);
}

/* Find all newlines in a file, and place in array */
/* Return number of newlines found */
int findnewlines(FILE *fp, newl_t newlines)
{
	inpt_t inputbuffer;
	int index = 0;
	int in_index;
	int nl_index = 0;
	int numread;
	size_t pos = 0;
        
	inputbuffer = (inpt_t) malloc(sizeof(char) * INPUT_BUFFER);

	errno = 0;
	while((numread = /* Read in the 16k buffer */
		fread(inputbuffer, sizeof(char), INPUT_BUFFER, fp))
			== INPUT_BUFFER)
	{ /* Leave loop if less than 16k read in */
		for(in_index = 0; in_index < INPUT_BUFFER; in_index++)
		{ /* Search through buffer */
			if((inputbuffer[in_index] == '\n') || (inputbuffer[in_index] == '\r'))
			{ /* If a newline store position */
				newlines[nl_index] = pos;
				nl_index++;
			}
			pos++;
		}
		index++;
	}
	for(in_index = 0; in_index < numread; in_index++)
	{ /* End of file where one doesn't read in 16k */
		if((inputbuffer[in_index] == '\n') || (inputbuffer[in_index] == '\r'))
		{
			newlines[nl_index] = pos;
			nl_index++;
		}
		pos++;
	}
	return(nl_index); /* Return number of newlines in file */
}

void usage(char *program)
{
	fprintf(stderr, "Usage:\t%s [+n|-n] filename\n", program);
	fprintf(stderr, "\t\tMust use a file-- doesn't take stdin.\n");
	fprintf(stderr, "\n\t%s -V\n", program);
	fprintf(stderr, "\t\tFor version information.\n");
	exit(-1);
}

void dump(FILE *fp)
{ /* Dump fp from point in 16k blocks */
	inpt_t inputbuffer;
	int index, num_read;
        

	inputbuffer = (inpt_t) malloc(sizeof(char) * INPUT_BUFFER);     
	while((num_read = fread(inputbuffer, sizeof(char), INPUT_BUFFER, fp))
			== INPUT_BUFFER)
	{
		fwrite(inputbuffer, sizeof(char), INPUT_BUFFER, stdout);
	}
	fwrite(inputbuffer, sizeof(char), num_read, stdout);
}
        
void dumpfrom(FILE *fp, newl_t newlines, int line_num)
{ /* Dump file fp from given linenumber using newline position array */
	size_t pos;
        
	if(line_num == 1)
	{ /* If first line--before any newlines */
		rewind(fp);
	}
	else
	{ /* Choose the right newline number for the right linenumber */
		pos = newlines[line_num - 2] + 1;
		fseek(fp, pos, SEEK_SET); /* Position point in file */
	}
	dump(fp); /* Dump file contents */
}

	        
int main(int argc, char **argv)
{
	newl_t newlines;
	int line_begin;
	int num_lines;
	int i = 1;
	char length[20] = "-10";
	FILE *fp;
	char *filename;
	char buffer[256];
        

        if((argv[i][0] == '+') || (argv[i][0] == '-'))
        {
		if(argv[i][1] == 'V')
			print_version();
	       	strcpy(length, argv[i]);
        	i++;
        }

	if(argc != i + 1)
		usage(argv[0]);

        filename = argv[i];
        
	if((fp = fopen(filename, "r")) == NULL)
	{ /* Open file for reading */
		sprintf(buffer, "%s: Can't open file %s", argv[0], argv[2]);
		perror(buffer);
		exit(errno);
	}

	newlines = (newl_t) malloc(sizeof(size_t) * NL_SIZE);	        
	num_lines = findnewlines(fp, newlines); /* Find newlines */
	switch(length[0])
	{ /* interpret command line */
		case '+':
			line_begin = atoi(length + 1);
			break;
		case '-':
			line_begin = num_lines - atoi(length + 1) + 1;
			break;
		default:
			usage(argv[0]);
			break;
	}
        
	if((line_begin > num_lines) || (line_begin < 1))
	{
		fprintf(stderr, "%s: File %s not long enough.\n", argv[0], argv[2]);
		exit(-1);
	}
	dumpfrom(fp, newlines, line_begin);
	close(fp);
}

        

         