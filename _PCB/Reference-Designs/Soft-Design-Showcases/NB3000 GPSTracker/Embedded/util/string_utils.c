#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char *
strdup (const char *s)
{
  size_t len = strlen (s) + 1;
  void *new = malloc (len);

  if (new == NULL)
    return NULL;

  return (char *) memcpy (new, s, len);
}

/* Compare no more than N characters of S1 and S2,
   ignoring case, returning less than, equal to or
   greater than zero if S1 is lexicographically less
   than, equal to or greater than S2.  */
int
strncasecmp (
     const char *s1,
     const char *s2,
     size_t n
) {
  const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;
  int result;

  if (p1 == p2 || n == 0)
    return 0;

  while ((result = tolower(*p1) - tolower(*p2++)) == 0)
    if (*p1++ == '\0' || --n == 0)
      break;

  return result;
}

// copied from strcmp.c and adjusted
int
strcasecmp( const char * s1, const char * s2 )
{
        unsigned char c1, c2;
        do
        {
                c2 = *s2++;
                c1 = *s1++;
        } while ( c1 != '\0' && tolower(c1) == tolower(c2) );
        return( (int)tolower(c1) - (int)tolower(c2) );
}

