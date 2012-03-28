/*
 *      Scale.h
 *
 *      The AthenaTools Plotter Widget Set - Version 6.0
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 *
 *      SCCSid[] = "@(#) Plotter V6.0  92/08/15  Scale.h"
 */

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

#ifndef _AtScale_h
#define _AtScale_h

#include "At.h"

typedef enum {
     AtTransformLINEAR,
     AtTransformLOGARITHMIC,
     AtTransformINVALID
} AtTransform;

typedef struct {
     float low, high;
     int lowpix, highpix;
     AtTransform transform;
     float mult, offset;
     int errorcode;
} AtScale;

extern AtScale *AtScaleCreate P((float, float, int, int, AtTransform));
extern AtScale *AtScaleCopy P((AtScale *));
extern void AtScaleDestroy P((AtScale *));
extern void AtScaleResize P((AtScale *, int, int));
extern void AtScaleRescale P((AtScale *, float, float));
extern void AtScaleChangeTransform P((AtScale *, AtTransform));
extern int AtScaleUserToPixel P((AtScale *, float));
extern float AtScalePixelToUser P((AtScale *, int));
extern char *AtScaleGetErrorMessage P((AtScale *));
extern float _AtScaleAlmostZero;
extern void AtScaleCalc P((AtScale *s));

#define SCALEERROR_NONE            0
#define SCALEERROR_BOUNDLESS       1
#define SCALEERROR_BOUNDCLOSE      2
#define SCALEERROR_LOGNEGATIVE     3
#define SCALEERROR_NOTRANSFORM     4

#define AtScaleGetLow(scale) (scale->low)
#define AtScaleGetHigh(scale) (scale->high)
#define AtScaleGetLowPix(scale) (scale->lowpix)
#define AtScaleGetHighPix(scale) (scale->highpix)
#define AtScaleGetTransform(scale) (scale->transform)
#define AtScaleGetErrorCode(scale) (scale->errorcode)

/* for use with X Toolkit resources and converters */

#define XtRTransform "Transform"
extern void AtRegisterTransformConverter P((void));

#endif /* _AtScale_h */
