/*
	localeconv() -- numeric formatting convention inquiry

	public-domain implementation for Standard C

	last edit:	27-Mar-1993	Gwyn@ARL.Army.Mil

	complies with the following standards:
		Std C	POSIX	SVID	XPG	AES

	This is a minimal implementation for environments where
	internationalization is not considered important.
 */
#ifdef	__ORCAC__
#pragma	lint	-1
#pragma	noroot
#endif

#include	<limits.h>		/* for CHAR_MAX */
#include	"locale.h"

static char	none[] = "";

static struct lconv	c_lconv =	/* lconv for "C" locale */
	{
	".",				/* decimal_point */
	none,				/* thousands_sep */
	none,				/* grouping */
	none,				/* int_curr_symbol */
	none,				/* currency_symbol */
	none,				/* mon_decimal_point */
	none,				/* mon_thousands_sep */
	none,				/* mon_grouping */
	none,				/* positive_sign */
	none,				/* negative_sign */
	CHAR_MAX,			/* int_frac_digits */
	CHAR_MAX,			/* frac_digits */
	CHAR_MAX,			/* p_cs_precedes */
	CHAR_MAX,			/* p_sep_by_space */
	CHAR_MAX,			/* n_cs_precedes */
	CHAR_MAX,			/* n_sep_by_space */
	CHAR_MAX,			/* p_sign_posn */
	CHAR_MAX,			/* n_sign_posn */
	};

#undef	localeconv			/* just in case */

struct lconv	*
localeconv( void )
	{
	return &c_lconv;
	}
