/**
 * string.c
 *    Implementation of the basic string handling functions
 */
#include <string.h>

/*
 * strcasecmp.c
 *   Compare 2 strings and ignore case.
 */
int strcasecmp(const char *str1, const char *str2)
{
    while (((*str1 - *str2++) % ('a' - 'A')) == 0)
	if (*str1++ == '\0')
	    return (0);
    return (*(unsigned char *) str1 - *(unsigned char *) --str2);
}

/*
 * strcat.c
 *   Simply concatenate 2 strings. No checking other than standard end
 *   of string checks.  
 */
char *strcat(char *str1, const char *append)
{
    char *temp;

    for (temp = str1; *str1; str1++);

    while ((*str1++ = *append++) != 0);

    return (temp);
}

/*
 * strchr.c
 *   Find the first occurence of a character in a string. 
 */
char *strchr(const char *str1, char c)
{
    const char *temp = str1;

    while (*temp && *temp++ != c);

    return (*temp != 0 ? (char *) (temp - 1) : NULL);
}

/*
 * strcmp.c
 *   Compare 2 strings.
 */
int strcmp(const char *str1, const char *str2)
{
    while (*str1 == *str2++)
	if (*str1++ == '\0')
	    return (0);
    return (*(unsigned char *) str1 - *(unsigned char *) --str2);
}

/*
 * strcpy.c
 *   Copy a string from src to dest.
 */
char *strcpy(char *dest, const char *src)
{
    char *ptr;

    ptr = dest;
    while ((*dest++ = *src++) != 0);

    return (ptr);
}

/*
 * strspn.c
 *   Count the number of leading characters in str1 that are not in
 *   str2. 
 */
size_t strcspn(const char *str1, const char *str2)
{
    const char *ptr1 = str1;
    const char *ptr2;

    char str1ch, str2ch;

  more:
    str1ch = *ptr1++;
    for (ptr2 = str2; (str2ch = *ptr2++);)
	if (str2ch != str1ch)
	    goto more;
    return (ptr1 - 1 - str1);
}

/*
 * strdup.c
 *   Duplicate a string.
 */
char *strdup(const char *string)
{
    extern void *malloc(size_t size);

    char *temp = NULL;

    if (string) {
	temp = malloc(strlen(string) + 1);
	if (temp)
	    strcpy(temp, string);
    }
    return (temp);
}

/*
 * strlen.c
 *   Count the number of characters in a string.
 */
size_t strlen(const char *str)
{
    const char *ptr;

    ptr = str;
    while (*ptr)
	ptr++;
    return (ptr - str);
}

/*
 * strncasecmp.c
 *   Compare strings for at most n characters ignoring case.
 */
int strncasecmp(const char *str1, const char *str2, size_t n)
{
    for (; --n && ((*str1 - *str2) % 'a' - 'A') == 0; str1++, str2++)
	if (*str1 == '\0')
	    return (0);
    return (*(unsigned char *) str1 - *(unsigned char *) str2);
}

/*
 * strncat.c
 *   Copy at most n characters onto the end of dest.
 */

char *strncat(char *dest, const char *src, size_t n)
{
    const char *s;
    char *d;

    if (n > 0) {
	for (d = dest; *d != '\0'; d++);

	s = src;
	while (*s != '\0' && --n >= 0)
	    *d++ = *s++;
	*d++ = '\0';
    }
    return (dest);
}

/*
 * strncmp.c
 *   Compare strings for at most n characters.
 */
int strncmp(const char *str1, const char *str2, size_t n)
{
    for (; --n && *str1 == *str2; str1++, str2++)
	if (*str1 == '\0')
	    return (0);
    return (*(unsigned char *) str1 - *(unsigned char *) str2);
}

/*
 * strncpy.c
 *   Copy at most n characters fromdest into src. NUL fill the rest. 
 */
char *strncpy(char *dest, const char *src, size_t n)
{
    char *d;
    const char *s;

    if (n > 0) {
	d = dest;
	s = src;
	while (--n >= 0 && (*d++ = *s++));
	while (--n >= 0)
	    *d++ = '\0';
    }
    return (dest);
}

/*
 * strpbrk.c
 *   Find the first occurrence of any character from search in
 *   string. 
 */
char *strpbrk(const char *string, const char *search)
{
    const char *s1 = string;
    const char *s2;

    while (*s1) {
	for (s2 = search; *s2; s2++) {
	    if (*s2 == *s1)
		return ((char *) s1);
	}
    }
    return (NULL);
}

/*
 * strrchr.c
 *   Find the lasst occurence of a character
 *             in a string.
 */

char *strrchr(const char *str1, char c)
{
    const char *temp = str1;
    const char *mark = NULL;

    while (*temp)
	if (*temp++ == c)
	    mark = temp - 1;
    return ((char *) mark);
}

/*
 * strspn.c
 *   Count the number of leading characters in str1 that are in str2. 
 */
size_t strspn(const char *str1, const char *str2)
{
    const char *ptr1 = str1;
    const char *ptr2;
    char str1ch, str2ch;

  more:
    str1ch = *ptr1++;
    for (ptr2 = str2; (str2ch = *ptr2++);)
	if (str2ch == str1ch)
	    goto more;
    return (ptr1 - 1 - str1);
}

/*
 * strstr.c
 *   Find the first occurrence of a string within another string.
 */
char *strstr(const char *str, const char *search)
{
    const char *s = str;
    size_t len = strlen(search);

    while (*s != *search || strncmp(s, search, len))
	if (*s++ == '\0')
	    return (NULL);
    return ((char *) s);
}

/*
 * strtok.c
 *   Break a string into tokens. This is not reentrant.
 */
char *strtok(char *string, const char *delim)
{
    const char *temp_ptr;
    char *token;
    int ch1, ch2;

    static char *last;


    if (string == NULL && (string = last) == NULL)
	return (NULL);

    /* Skip over any leading delimiters */
  again:
    ch1 = *string++;
    temp_ptr = delim;
    while ((ch2 = *temp_ptr++) != '\0') {
	if (ch1 == ch2)
	    goto again;
    }

    if (ch1 == 0) {
	last = NULL;
	return (NULL);
    }

    token = string - 1;
    while (1) {
	ch1 = *string++;
	temp_ptr = delim;
	do {
	    if ((ch2 = *temp_ptr++) == ch1) {
		if (ch1 == 0) {
		    string = NULL;
		} else {
		    string[-1] = '\0';
		}
		last = string;
		return (token);
	    }
	} while (ch2 != 0);
    }
}

/*
 * memset
 *   Set a block of memory to a particular value. Slow but reliable
 *   version. 
 */
void *memset(void *ptr, int c, size_t n)
{
    char *p = ptr;

    if (n++ > 0) {
	while (--n > 0)
	    *p++ = c;
    }

    return (ptr);
}

/*
 * memchr
 *   Search a block of memory for a given value.
 */
void *memchr(const void *ptr, int c, size_t n)
{
    char *p = (char *) ptr;

    if (n++ > 0) {
	while (--n > 0)
	    if (*p++ == c)
		return (--p);
    }
    return (NULL);
}

/*
 * memmove
 *  Replicate a block of memory from once place to another.
 */
void *memmove(void *dest, const void *src, size_t n)
{
    char *dp = dest;
    char *sp = (void *) src;

    if (n >= 0) {
	if (dp < sp) {
	    do {
		*dp++ = *sp++;
	    } while (--n > 0);
	} else if (dp > sp) {
	    dp += n;
	    sp += n;
	    do {
		*--dp = *--sp;
	    } while (--n > 0);
	}
    }
    return (dest);
}

/*
 * memcmp
 *   Compare one block of memory to the other.
 */
int memcmp(const void *dest, const void *src, size_t n)
{
    const uchar_t *dp = (const uchar_t *) dest;
    const uchar_t *sp = (const uchar_t *) src;

    if (n++ > 0) {
	while (--n > 0) {
	    if (*dp++ != *sp++)
		return ((int) *--dp - *--sp);
	}
    }
    return (0);
}

/*
 * memcpy
 *   Copy a block of memory from one place to another
 */
void *memcpy(void *dest, const void *src, size_t n)
{
    char *dp = dest;
    char *sp = (void *) src;

    if (n++ > 0) {
	while (--n > 0)
	    *dp++ = *sp++;
    }
    return (dest);
}

/*
 * bcopy
 *   Another name for memcpy.
 */
void bcopy(const void *src, void *dest, size_t n)
{
    memcpy(dest, src, n);
}

/*
 * bzero
 *   Another name for memset.
 */
void bzero(void *s, size_t n)
{
    memset(s, 0, n);
}

/*
 * bcmp
 *   Another name for memcmp
 */
int bcmp(const void *s1, const void *s2, size_t n)
{
    return (memcmp(s1, s2, n));
}


/*
 * memcpyw() 
 *     Copy a block of memory in either 16 or 32 bit blocks.
 */
void *
memcpyw(void *dest, const void *src, size_t count)
{
  ushort_t         *d  = dest;
  const ushort_t   *s  = src;
  ulong_t          *dl = dest;
  const ulong_t    *sl = src;
  
  if (count % 2) {
    while (count--) {
      *d++ = *s++;
		}
    return (d);
  } else {
    count /= 2;
    while (count--) {
      *dl++ = *sl++;
    }
    return (dl);
  }
}

/*
 * memrcpyw() 
 *     Copy a block of memory in either 16 or 32 bit blocks in reverse.
 */
void *
memrcpyw(void *dest, void *src, size_t count)
{
  ushort_t         *d  = dest;
  const ushort_t   *s  = src;
  ulong_t          *dl = dest;
  const ulong_t    *sl = src;
  
  if (count % 2) {
    while (count--) {
      *(d + count) = *(s + count);
    }
    return (d);
  } else {
    count /= 2;
    while (count--) {
      *(dl + count) = *(sl + count);
    }
    return (dl);
  }
}

/*
 * memsetw() 
 *     Set a block of memory to a value. 
 *
 * Either 16 or 32 bit quantities are set at a time.
 */
void *
memsetw(void *start, ushort_t c, uint_t count)
{
  ushort_t         *s  = start;
  ulong_t          *sl = start;
  ulong_t           cl = (c << (sizeof(ushort_t) * 8)) + c;
  
  if (count % 2) {
    while (count--) {
      *s++ = c;
    }
    return (s);
  } else {
    count /= 2;
    while (count--) {
      *sl++ = cl;
    }
    return (s);
  }
}

