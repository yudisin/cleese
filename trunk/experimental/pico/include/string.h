/**
 * string.h
 */
#ifndef ___STRING_H___
#define ___STRING_H___

#include <sys/types.h>

/*
 * True string functions
 */
extern  size_t  strlen(const char* str);
extern  char*   strdup(const char* str);
extern  char*   strcat(char* dest, const char* src);
extern  char*   strncat(char* dest, const char* src, size_t count);
extern  char*   strcpy(char* dest, const char* src);
extern  char*   strncpy(char* dest, const char* src, size_t count);
extern  int     strcmp(const char* str1, const char* str2);
extern  int     strncmp(const char* str1, const char* str2, size_t count);
extern  char*   strchr(const char* str, char c);
extern  char*   strstr(const char* str1, const char* str2);
extern  char*   strpbrk(const char* str1, const char* str2);
extern  char*   strrchr(const char* str, char c);
extern  size_t  strspn(const char* str1, const char* str2);
extern  size_t  strcspn(const char* cs, const char* ct);
extern  char*   strtok(char* str, const char* delim);

/*
 * Old-style memory routines
 */
extern void bcopy(const void* src, void* dest, size_t n);
extern void bzero(void* s, size_t n);
extern int  bcmp(const void* s1, const void* s2, size_t n);

/*
 * Memory routines
 */
extern void* memcpy(void* dest, const void* src, size_t cnt);
extern int   memcmp(const void* s1, const void* s2, size_t n);
extern void* memmove(void* dest, const void* src, size_t length);
extern void* memchr(const void* s, int c, size_t n);
extern void* memset(void* dest, int c, size_t n);

/*
 * Extensions
 */
extern void* memcpyw(void *dest, const void *src, size_t count);
extern void* memrcpyw(void *dest, void *src, size_t count);
extern void* memsetw(void *start, ushort_t c, uint_t count);

#endif   /* #ifndef ___STRING_H___ */
