/*
	<locale.h> -- definitions for internationalization functions

	public-domain implementation for GS/OS, ORCA/C 2.0

	last edit:	27-Mar-1993	Gwyn@ARL.Army.Mil

	compatible with the following standards:
		Std C	POSIX	SVID	XPG	AES

	Various standards allow addition of more macros starting with "LC_".
 */

#ifndef	__locale__
#define	__locale__		/* (not strictly necessary) */

struct lconv
	{
	/* The following are controlled by LC_NUMERIC: */
	char	*decimal_point;
	char	*thousands_sep;
	char	*grouping;
	/* The following are controlled by LC_MONETARY: */
	char	*int_curr_symbol;
	char	*currency_symbol;
	char	*mon_decimal_point;
	char	*mon_thousands_sep;
	char	*mon_grouping;
	char	*positive_sign;
	char	*negative_sign;
	char	int_frac_digits;
	char	frac_digits;
	char	p_cs_precedes;
	char	p_sep_by_space;
	char	n_cs_precedes;
	char	n_sep_by_space;
	char	p_sign_posn;
	char	n_sign_posn;
	};

extern char		*setlocale( int, const char * );
extern struct lconv	*localeconv( void );

#define	LC_ALL		0
/* The following are in POSIX.1 order of precedence: */
#define	LC_CTYPE	1
#define	LC_COLLATE	2
#define	LC_TIME		3
#define	LC_NUMERIC	4
#define	LC_MONETARY	5
/* other categories may be added here */

/* NOTE: The following must be spelled exactly as in <stddef.h> etc. */
#ifndef	NULL
#define	NULL  (void *) 0L
#endif

#endif	/* __locale__ */
