#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gsos.h>
#include <texttool.h>

void go_die(char *s)
{
    fprintf(stderr, "verify: %s\n",s);
    exit(1);
}

void usage(void)
{
    fprintf(stderr,"usage: verify devicename\n");
    exit(1);
}

int main(int argc, char *argv[])
{
DevNumRecGS dn;
char *buf;
longword cur_block;
DIORecGS dio;
int err,x;
extern GSString255Ptr __C2GSMALLOC(char *);
char cbstr[10];

    if (argc != 2) usage();

    buf = malloc(32768l);
    cur_block = 0;

    dn.pCount = 2;
    dn.devName = (GSString32Ptr) __C2GSMALLOC(argv[1]);
    GetDevNumberGS(&dn);
    if (err = _toolErr) {
	fprintf(stderr,"Error getting device number [%02X]\n",err);
        exit(1);
    }

    dio.pCount = 6;
    dio.devNum = dn.devNum;
    dio.buffer = buf;
    dio.requestCount = 32768l;
    dio.blockSize = 512;
    cbstr[0] = 0;
    for (cur_block = 0; cur_block < 1600; cur_block += 64) {
    redo:
        dio.startingBlock = cur_block;
        DRead(&dio);
        if (err = _toolErr) {
	    switch (err) {
		case 0x2E: goto redo; /* disk switch, try re-reading */
		case 0x2F: go_die("no disk in drive");
		case 0x11: go_die("bad device number");
            	default: printf("bad block(s) group %d (%02X)\n",cur_block,err);
	    }
        }
        for (x = 0; x < strlen(cbstr); x++)
            WriteChar(8);
        sprintf(cbstr, "%d",cur_block);
        for (x = 0; x < strlen(cbstr); x++)
            WriteChar(cbstr[x]);
    }
    for (x = 0; x < strlen(cbstr); x++)
        WriteChar(8);
    printf("%d\n",cur_block);
    exit(0);
}
