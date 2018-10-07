#if defined (__clang__)

/*
 * Clang doesn't have the __builtin_memcmp instrinsic, instead it merely creates an alias from that
 * name to memcmp and expects the implementation to be found in libc. Since we're compiling in
 * freestanding mode we have no libc and thus linking fails when a few of the tutorial steps are
 * built with clang. Therefore, we need to provide the implementation below.
 * This code is taken from gcc 8 sources (libgcc/memcmp.c) and is in public domain
 */
int memcmp (const void *str1, const void *str2, unsigned int count)
{
	const unsigned char *s1 = str1;
	const unsigned char *s2 = str2;

	while (count-- > 0)
	{
		if (*s1++ != *s2++)
			return s1[-1] < s2[-1] ? -1 : 1;
	}
	return 0;
}
#endif
