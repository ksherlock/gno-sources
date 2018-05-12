/* qtime.c      Displays time in real English, also chimes
** This version current from 18/9/89
** 10/91	Tim Meekins    Fixed more bong code. Sheesh!
** 09/91	Tim Meekins    Optimizations. Removed \n on ding dong.
**			       Fixed bong code.
** 02/91        Jawaid Bazyar  fixed return type and value of main
** 01/91        Derek Taubert  Few changes to port to Apple IIgs
** 09/89        Ade Lovett     Complete rewrite
** 04/86        Mark Dapoz     Converted to C for UNIX
** 12/79-12/82  Mike Cowlishaw
**
*/

#pragma optimize -1
#pragma stacksize 1024

#include <stdio.h>
#include <types.h>
#include <misctool.h>
#include <texttool.h>

char *minutesaying[] = {
    "","just after ","a little after ","nearly ","almost "
};
char *fiveminsaying[] = {
    "","five past ","ten past ","a quarter past ","twenty past ",
    "twenty-five past ","half past ","twenty-five to ","twenty to ",
    "a quarter to ","ten to ","five to ",""
};
char *hoursaying[] = {
    "one","two","three","four","five","six","seven","eight","nine",
    "ten","eleven",""
};

int main()
{
    char qtime[200];
    unsigned int i, hr, mn;
    TimeRec clock;

    clock = ReadTimeHex();
    mn = clock.minute+(clock.second>29);
    hr = clock.hour+(clock.minute>32);

    strcpy(qtime, "It's ");
    strcat(qtime, minutesaying[mn%5]);
    strcat(qtime, fiveminsaying[mn/5+(mn%5>2)]);
    
    if (hr%12) {
        strcat(qtime, hoursaying[(hr -= (hr>12)*12+1)]);
        strcat(qtime, (mn%60) ? "." : " o'clock.");
    } else {
        strcat(qtime, (hr == 12) ? "Noon." : "Midnight.");
        hr = 12;
    }
    
    if (!(mn%15))
        if (mn%60)
            WriteCString("(Ding-Dong!) ");
        else {
            WriteChar('(');
            for (i=0; i<=hr; i++ )
            {
                WriteCString("Bong");
                if (i==hr)
                    WriteCString("!) ");
                else
                    WriteChar(',');
            }
        }
    WriteCString(qtime);
    WriteChar(13);
    WriteChar(10);
    return 0;
}
