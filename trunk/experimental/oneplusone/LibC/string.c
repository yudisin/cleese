/* from stddef.h */
typedef long unsigned int size_t;

int
strcmp(const char *s1, const char *s2)
{
	while (*s1++ == *s2) {
		if (*s2++ == '\0') {
			return(0);
		}
	}
	return((int)s1[-1] - (int)s2[0]);
	return(1);
}

size_t
strlen(const char *p)
{
	size_t x = 0;

	if (p == 0) {
		return(0);
	}

	while (*p++)
		++x;
	return(x);
}

void *
memcpy(void *dest, const void *src, size_t cnt)
{
	bcopy(src, dest, cnt);
	return(dest);
}

void *
memset(void *dst, int c, size_t n)
{
	if (n != 0) {
		char  *d = dst;

		do {
			*d++ = c;
		} while (--n != 0);
	}
	return (dst);
}

int
memcmp(const void *s1, const void *s2, size_t n)
{
	return(bcmp(s1, s2, n));
}
