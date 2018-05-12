char *getenv(char *);
int tgetent(char *, char *);
int tgetnum(char *);
int tgetflag(char *);
char *tgetstr(char *, char **);
char *tgoto(char *, int, int);
void tputs(char *, int, int (*outc)());
putchar(char);

char mp[1024];
char buf[1024];

ctlprint(char *str)
{
  char c;

  while (c = *str++)
  {
    if (c<' ') printf("[%d]",c);
    else printf("%c",c);
  }
}

main()
{
	char *term,*bufptr=buf,*cm;
        int i;

	if (!(term = getenv("TERM")))
        {
        	(void)printf("test: no TERM environment variable.\n");
		exit(1);
	}

        printf("TERM = %s\n",term);

  	printf("tgetent returned: %d\n",tgetent(mp,term));

	printf("number of lines li = %d\n",tgetnum("li"));
	printf("number of cols  co = %d\n",tgetnum("co"));

	printf("has hardware tabs pt = %d\n",tgetflag("pt"));
	printf("has meta key      km = %d\n",tgetflag("km"));

	cm = tgetstr("cm",&bufptr);
	printf("cursor motion cm = ");
        ctlprint(cm); printf("\n");

        for (i=5;i<=10; i++)
          { tputs(tgoto(cm,i,i),1,putchar); ctlprint(tgoto(cm,i,i)); }

        tputs(tgoto(cm,0,20),1,putchar);

}
