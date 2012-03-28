/*
 *      Scale.c
 *
 *      The AthenaTools Plotter Widget Set - Version 6.0
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Fri Aug  7 10:09:47 1992, Cast type converter to keep
 *                                      ANSI C compilers quiet.
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Check value before calling log10().
 *                                      Changed <At/..> to <X11/At/..>.
 */
static char SCCSid[] = "@(#) Plotter V6.0  92/08/15  Scale.c";

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

#include "Scale.h"

#define New(t) ((t *) XtMalloc(sizeof(t)))

static char *error_messages[] = {
     "AtScale:  No Error.",
     "AtScaleCalc: high bound is less than low bound.",
     "AtScaleCalc: high bound and low bound are too close together.",
     "AtScaleCalc: logarithmic scale must have positive bounds",
     "AtScaleCalc: unknown transform type.",
};

float _AtScaleAlmostZero = 1.0e-12;

void AtScaleCalc P((AtScale *s));
void AtScaleCalc(s)
AtScale *s;
{
     s->errorcode = SCALEERROR_NONE;

     if (s->high < s->low) {
	  s->errorcode = SCALEERROR_BOUNDLESS;
	  s->transform = AtTransformINVALID;
	  return;
     }

     if (s->high - s->low < _AtScaleAlmostZero) {
	  s->errorcode = SCALEERROR_BOUNDCLOSE;
	  s->transform = AtTransformINVALID;
	  return;
     }

     if ((s->transform == AtTransformLOGARITHMIC) && (s->low <= 0.0)) {
	  s->errorcode = SCALEERROR_LOGNEGATIVE;
	  s->transform = AtTransformINVALID;
	  return;
     }

     switch (s->transform) {
     case AtTransformLINEAR:
	  s->mult = (s->highpix - s->lowpix)/(s->high - s->low);
	  s->offset = s->low;
	  break;
     case AtTransformLOGARITHMIC:
	  s->offset = log10(s->low);
	  s->mult = (s->highpix - s->lowpix)/(log10(s->high) - s->offset);
	  break;
     default:
	  s->errorcode = SCALEERROR_NOTRANSFORM;
	  break;
     }
     return;
}

AtScale *AtScaleCreate(float low, float high, int lowpix, int highpix, 
    AtTransform transform)
{
     AtScale *scale;

     scale = New(AtScale);
     scale->low = low;
     scale->high = high;
     scale->lowpix = lowpix;
     scale->highpix = highpix;
     scale->transform = transform;
     AtScaleCalc(scale);
     return scale;
}

AtScale *AtScaleCopy(s)
AtScale *s;
{
     AtScale *new;
     new = New(AtScale);
     *new = *s;
     return new;
}

void AtScaleDestroy(s)
AtScale *s;
{
     XtFree((char *)s);
     s = NULL;
}

int AtScaleUserToPixel(AtScale *s, float x)
{
     switch (s->transform) {
     case AtTransformLINEAR:
	  return (int)((x - s->offset) * s->mult) + s->lowpix;
     case AtTransformLOGARITHMIC:
	  if (x > 0.0)
	       return (int)((log10(x) - s->offset) * s->mult) + s->lowpix;
	  s->errorcode = SCALEERROR_LOGNEGATIVE;
	  return 0;
     case AtTransformINVALID:
	  return 0;
     default:
	  s->errorcode = SCALEERROR_NOTRANSFORM;
	  return 0;
     }
}

float AtScalePixelToUser(AtScale *s, int i)
{
     switch (s->transform) {
     case AtTransformLINEAR:
	  return (float)((i - s->lowpix) / s->mult) + s->offset;
     case AtTransformLOGARITHMIC:
	  return pow(10.0, ((i - s->lowpix) / s->mult) + s->offset);
     case AtTransformINVALID:
	  return 0.0;
     default:
	  s->errorcode = SCALEERROR_NOTRANSFORM;
	  return 0.0;
     }
}

void AtScaleResize(s, new_lowpix, new_highpix)
AtScale *s;
int new_lowpix, new_highpix;
{
     s->lowpix = new_lowpix;
     s->highpix = new_highpix;
     (void) AtScaleCalc(s);
}

void AtScaleRescale(AtScale *s, float new_low, float new_high)
{
     s->low = new_low;
     s->high = new_high;
     (void) AtScaleCalc(s);
}

void AtScaleChangeTransform(AtScale *s, AtTransform t)
{
     s->transform = t;
     (void) AtScaleCalc(s);
}

char *AtScaleGetErrorMessage(AtScale *s)
{
     if (s->errorcode < XtNumber(error_messages))
	  return error_messages[s->errorcode];
     else
	  return error_messages[0];
}


static void AtCvtStringToTransform(args, num_args, from, to)
XrmValue *args;
Cardinal num_args;
XrmValue *from, *to;
{
     static AtTransform transform;

     transform = AtTransformINVALID;

     if (strcasecmp(from->addr, "linear") == 0)
	  transform = AtTransformLINEAR;
     else if ((strcasecmp(from->addr, "log") == 0) ||
	      (strcasecmp(from->addr, "logarithmic") == 0))
	  transform = AtTransformLOGARITHMIC;

     if (transform == AtTransformINVALID)
	  XtStringConversionWarning(from->addr, XtRTransform);
     else {
	  to->addr = (caddr_t) &transform;
	  to->size = sizeof(AtTransform);
     }
}

void AtRegisterTransformConverter()
{
     static Boolean registered = False;

     if (!registered) {
	  XtAddConverter(XtRString, XtRTransform,
			 (XtConverter) AtCvtStringToTransform, NULL,0);
	  registered = True;
     }
     *SCCSid = *SCCSid;       /* Keep gcc quiet */
}
