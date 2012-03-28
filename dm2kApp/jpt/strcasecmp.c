/* A strcasecmp for older os's that don't have it... */

#include <ctype.h>

#ifndef tolower
/* New sunos version is a function - but undefined!!! */
extern int tolower();
#endif 

int strcasecmp (a, b)
char *a, *b;
{
     int ca, cb;
     
     while (1) {
	  ca = *a++;
	  cb = *b++;
	  if (isupper(ca)) ca = tolower(ca);
	  if (isupper(cb)) cb = tolower(cb);
	  if (ca != cb) return (ca > cb ? 1 : -1);
	  if (!ca) return 0;
     }
}

int strncasecmp (a, b, n)
char *a, *b;
int n;
{
     int ca, cb;
     
     while (n-- > 0) {
	  ca = *a++;
	  cb = *b++;
	  if (isupper(ca)) ca = tolower(ca);
	  if (isupper(cb)) cb = tolower(cb);
	  if (ca != cb) return (ca > cb ? 1 : -1);
	  if (!ca) return 0;
     }
     return 0;
}
