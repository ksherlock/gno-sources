'connect' Script Language
-------------------------

Connect utilizes a very simple script language to accomplish its work.
You need to customize the script for each system you intend to use
connect with, so read up!

Script commands are single character commands followed by arguments
as required by the particular command.  Currently, there are only
a few commands:

----------------------------------------------------------------------------
w[#]/string
-----------

Waits for string to come across the TTY.  For example, if you wanted
connect to pause until the prompt "login: " appears, use

w/login:

The '/' is a separator. Its function is to make it easier to read
'w' command lines with an argument starting with a space.
The '#' is an optional integer value that indicates a timeout, in seconds.
If the indicated string does not appear after the specified time, the
wait is aborted and the wait function returns false.  If the string
is detected, w/ returns true.  If you do not use a timeout, if something
strange occurs w/ may wait forever!
----------------------------------------------------------------------------
p/string
--------

Sends the specified text to the TTY.  Our little sample script now becomes
something like:

w/login:
p/bazyar\n

Note the '\n'; like C, you must specifically tell connect to print
carriage returns and other control codes with an escape sequence.
This allows connect to work with highly interactive systems and software.
The currently supported escape sequences are:

	\\	- send a backslash character
	\n	- send a newline (LF - ascii 10)
	\r	- send a carriage return (CR - ascii 13)

The value of the p/ is always true.

----------------------------------------------------------------------------
i{t|f|e|n}

This is a conditional command.  The character following the 'i' indicates
what action to take.

	t	Executes the block following the i command if the last
		command returned true
	f	Executes ... false
	e	Else clause follows.  Negates the value of the return value
	n       End the conditional section.  This is required.
----------------------------------------------------------------------------

That's all there is to it!  See some of the sample scripts (and if you
have GNO 1.1, test them with PTYs) and learn how to make your IIGS
automatically talk to the rest of the world behind your back. :-)
