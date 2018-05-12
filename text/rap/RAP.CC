/* Rap - program to generate funny stereotypical rap tunes */
/* Originally for Unix systems by Brendan Jones (brendan@otc.otca.oz.au) */
/* Ported to Orca/C on the Apple IIgs by Ian Schmidt (irsman@iastate.edu) */

#pragma optimize -1
#pragma stacksize 4096

#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <misctool.h>

static char     col1[6][30] = {
	"Move it",
	"Get up",
	"Pump it up",
	"Get down",
	"Shake it",
	"Pump the jam"
};

static char     col2[6][30] = {
	" to triple beat ",
	" get body heat ",
	" feel the beat ",
	" get around ",
	" the joint is jumpin' ",
	" feet are stompin' "
};

static char     col3[6][30] = {
	"in the city streets\n",
	"you'll be humpin'\n",
	"'til the night is over\n",
	"shake your meat\n",
	"I'm bustin loose\n",
	"with disco heat\n"
};

main()
{
	int	chor[2][3];
        int     i,j,getpid();
        long   	now;
        time_t  now2;

/* Generate random seed */

/* Note: this next part is where a lot of the trouble came from in the */
/* original source.  I tried to preserve the functionality, although I had */
/* to remove a getpid() call and replace it with GetTick(), which provides */
/* a similarly always-incrementing source of numbers. */
	
        now = (long) time(&now2) / (long) rand();
        now2 = (time_t) now;
        srand((int) GetTick()+(int)((now >> 16) + now + time(&now2)));

/* Generate chorus and title */

	for (i = 0; i < 2; i++)
		for (j = 0; j < 3; j++)
			chor[i][j] = rand() % 6;
	printf ("\n\"%s\" by Mixmastermeatbeaters\n\n",col1[(chor[0][0])]);

/* Print out song */

	verse();
	chorus(chor);
	verse();
	chorus(chor);
	lastline(chor);
	printf("-----\n");
}

chorus(chor)
int	chor[2][3];
{
	int	i;

	printf("\n(chorus)\n");
	for (i = 0; i < 2; i++) {
		printf("%s",col1[(chor[i][0])]);
		printf("%s",col2[(chor[i][1])]);
		printf("%s",col3[(chor[i][2])]);
        }
	printf("\n");
}

verse()
{
	int	i;

	for (i = 0; i < 4; i++) {
		printf("%s",col1[rand()%6]);
		printf("%s",col2[rand()%6]);
		printf("%s",col3[rand()%6]);
        }
}

lastline(chor)
int	chor[2][3];
{
	int	i;

	for (i = 0; i < 17; i++) {
		printf("%s",col1[chor[1][0]]);
		printf("%s",col2[chor[1][1]]);
		printf("%s",col3[chor[1][2]]);
        }
	printf("\n");
}
