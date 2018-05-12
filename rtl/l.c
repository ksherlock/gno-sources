#include <texttool.h>

segment "          ",dynamic;
int testlibfunc(char *s)
{
    WriteCString(s);
}
