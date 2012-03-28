/*
 *      AtConverters.c
 *
 *      The AthenaTools Plotter Widget Set - Version 6.0
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Fri Aug  7 09:43:32 1992, Cast type converters to keep
 *                                      ANSI C compilers quiet.
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 */
static char SCCSid[] = "@(#) Plotter V6.0  92/08/15  AtConverters.c";

/*

Copyright 1990,1991 by the Massachusetts Institute of Technology

All rights reserved.

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of the firms, institutes
or employers of the authors not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.

THE AUTHORS AND THEIR FIRMS, INSTITUTES OR EMPLOYERS DISCLAIM ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THE AUTHORS AND THEIR FIRMS,
INSTITUTES OR EMPLOYERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*/

#include "AtConverters.h"
#include "Text.h"
#include "Plot.h"

/*typedef char* caddr_t; */

/**********************************************************************/
static Boolean AtCvtStringToFloat (disp, args, num_args, from, to, data)
Display *disp;
XrmValue *args;
Cardinal *num_args;
XrmValue *from;
XrmValue *to;
XtPointer *data;
{
     static float value;
     Boolean Failed = False;
printf("AtCvtStringToFloat\n");
     if (*num_args != 0)
	  XtAppErrorMsg(XtDisplayToApplicationContext(disp),
			"cvtStringToFloat", "wrong Parameters", "XtToolkitError",
			"String to float conversion takes no arguments",
			(String *)NULL, (Cardinal *)NULL);

     value = (float)atof(from->addr);
     if (!to->addr) to->addr = (caddr_t) &value;
     else if (to->size < sizeof(float)) Failed = True;
     else *(float *)to->addr = value;
     to->size = sizeof(float);

     return !Failed;
}

void AtRegisterFloatConverter()
{
     static Boolean registered = False;

     if (registered == False) {
	  XtSetTypeConverter(XtRString, XtRFloat,
			     (XtTypeConverter) AtCvtStringToFloat,
			     NULL, 0, XtCacheAll, NULL);
	  registered = True;
     }
     *SCCSid = *SCCSid;         /* Keep gcc quiet */
}

/**********************************************************************/

static Boolean AtCvtStringToDouble (disp, args, num_args, from, to, data)
Display *disp;
XrmValue *args;
Cardinal *num_args;
XrmValue *from;
XrmValue *to;
XtPointer *data;
{
     static double value;
     extern double atof();
     Boolean Failed = False;
printf("AtCvtStringToDouble\n");
     if (*num_args != 0)
	  XtAppErrorMsg(XtDisplayToApplicationContext(disp),
			"cvtStringToDouble", "wrong Parameters", "XtToolkitError",
			"String to double conversion takes no arguments",
			(String *)NULL, (Cardinal *)NULL);

     value = atof(from->addr);
     if (!to->addr) to->addr = (caddr_t) &value;
     else if (to->size < sizeof(double)) Failed = True;
     else *(double *)to->addr = value;
     to->size = sizeof(double);

     return !Failed;
}

void AtRegisterDoubleConverter()
{
     static Boolean registered = False;

     if (registered == False) {
	  XtSetTypeConverter(XtRString, XtRDouble,
			     (XtTypeConverter) AtCvtStringToDouble,
			     NULL, 0, XtCacheAll, NULL);
	  registered = True;
     }
     *SCCSid = *SCCSid;         /* Keep gcc quiet */
}

/**********************************************************************/


static Boolean AtCvtStringToJustify (dpy, args, nargs, from, to, dp)
Display *dpy;
XrmValue *args;
Cardinal *nargs;
XrmValue *from, *to;
XtPointer *dp;
{
     static int result;
     Boolean Failed = False;
     char *inp = (char *)from->addr;

     if (*nargs != 0)
	  XtAppErrorMsg(XtDisplayToApplicationContext(dpy),
			"cvtStringToAtjustify", "wrong Parameters",
			"XtToolkitError",
			"String to AtJustify conversion takes no arguments",
			(String *)NULL, (Cardinal *)NULL);

     if (!strncasecmp(inp, "justify", 7)) inp += 7;
     if (!strncasecmp(inp, "atjustify", 9)) inp += 9;

     if (!strcasecmp (inp, "left"))
	  result =  AtTextJUSTIFY_LEFT;
     else if (!strcasecmp (inp, "top"))
	  result =  AtTextJUSTIFY_TOP;
     else if (!strcasecmp (inp, "center"))
	  result =  AtTextJUSTIFY_CENTER;
     else if (!strcasecmp (inp, "right"))
	  result = AtTextJUSTIFY_RIGHT;
     else if (!strcasecmp (inp, "bottom"))
	  result = AtTextJUSTIFY_BOTTOM;
     else {
	  Failed = True;
     }

     if (Failed) {
	  XtDisplayStringConversionWarning(dpy, from->addr, "AtJustify");
     } else {
	  if (!to->addr) to->addr = (caddr_t) &result;
	  else if (to->size < sizeof(int)) Failed = True;
	  else *(int *)to->addr = result;
	  to->size = sizeof(int);
     }
     return !Failed;
}

void AtRegisterJustifyConverter()
{
     static Boolean registered = False;

     if (registered == False) {
	  XtSetTypeConverter(XtRString, XtRAtJustify,
			     (XtTypeConverter) AtCvtStringToJustify,
			     NULL, 0, XtCacheAll, NULL);
	  registered = True;
     }
}

/**********************************************************************/

static Boolean AtCvtStringToLinestyle (dpy, args, nargs, from, to, dp)
Display *dpy;
XrmValue *args;
Cardinal *nargs;
XrmValue *from, *to;
XtPointer *dp;
{
     static int result;
     Boolean Failed = False;

     if (*nargs != 0)
	  XtAppErrorMsg(XtDisplayToApplicationContext(dpy),
			"cvtStringToLinestyle", "wrong Parameters",
			"XtToolkitError",
			"String to Linestyle conversion takes no arguments",
			(String *)NULL, (Cardinal *)NULL);

     if (!strcasecmp ((char*)from->addr, "linesolid"))
	  result =  LineSolid;
     else if (!strcasecmp ((char*)from->addr, "linedoubledash"))
	  result =  LineDoubleDash;
     else if (!strcasecmp ((char*)from->addr, "lineonoffdash"))
	  result =  LineOnOffDash;
     else {
	  Failed = True;
     }

     if (Failed) {
	  XtDisplayStringConversionWarning(dpy, from->addr,
					   "Linestyle");
     } else {
	  if (!to->addr) to->addr = (caddr_t) &result;
	  else if (to->size < sizeof(int)) Failed = True;
	  else *(int *)to->addr = result;
	  to->size = sizeof(int);
     }
     return !Failed;
}

void AtRegisterLinestyleConverter()
{
     static Boolean registered = False;

     if (registered == False) {
	  XtSetTypeConverter(XtRString, XtRLinestyle,
			     (XtTypeConverter) AtCvtStringToLinestyle,
			     NULL, 0, XtCacheAll, NULL);
	  registered = True;
     }
}
