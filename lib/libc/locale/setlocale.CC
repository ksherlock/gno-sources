/*
	setlocale() -- locale control function

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

#include	"locale.h"

#undef	setlocale			/* just in case */

char	*
setlocale( int category, const char *locale )
	{
	switch( category )
		{
	case LC_ALL:
	case LC_COLLATE:
	case LC_CTYPE:
	case LC_MONETARY:
	case LC_NUMERIC:
	case LC_TIME:
		return "";		/* native == "C" == "POSIX" */

	default:
		return NULL;		/* unsupported category */
		}
	}
