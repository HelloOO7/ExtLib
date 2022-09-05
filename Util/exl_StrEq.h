#ifndef __EXL_STREQ_H
#define __EXL_STREQ_H

#include "exl_Types.h"

INLINE int strequal(const char* str1, const char* str2)
{
    int ctr=0;

    while(str1[ctr]==str2[ctr])
    {
        if(str1[ctr]=='\0'||str2[ctr]=='\0')
            break;
        ctr++;
    }
    if(str1[ctr]=='\0' && str2[ctr]=='\0')
        return true;
    else
        return false;
}

#endif