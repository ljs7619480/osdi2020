#include "string.h"

int strcmp(const char *s1, const char *s2)
{
    for(;*s1 && *s1 == *s2; s1++, s2++);
    return *s1 - *s2;
}

char *strcpy(char *dest, const char *src)
{
    char *odest = dest;
    while( (*dest++ = *src++) );
    return odest;
}

char *strncpy(char *dest, const char *src, unsigned long n)
{
    char *odest = dest;
    while( n-- && (*dest++ = *src++) );
    return odest;
}

//len of char in string whit '\0' not included
unsigned int strlen(const char *s)
{
    unsigned int cnt;
    for(cnt=0; s[cnt]!='\0'; cnt++);
    return cnt;
}

void *memset(void *s, int c, unsigned long n)
{
    char *xs = s;
    while (n--)
        *xs++ = c;
    return s;
}

void *memcpy(void *dest, const void *src, unsigned long n)
{
    char *tmp = dest;
    const char *s = src;
    while (n--)
        *tmp++ = *s++;
    return dest;
}