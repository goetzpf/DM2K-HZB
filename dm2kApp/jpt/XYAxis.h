/*
 *      XYAxis.h
 *
 *      Based on the AthenaTools Plotter Widget Set - Version 6.0
 *      G.Lei Aug 1998,                 new resources and corresponding methods
 *                                      added 
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 *
 *      SCCSid[] = "@(#) Plotter V6.0  92/08/15  XYAxis.h.h"
 */

/*

Copyright 1992 by University of Paderborn

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

#ifndef _Jpt_XYAxis_h
#define _Jpt_XYAxis_h

#include "At.h"
#include "AxisCore.h"
#include "Text.h"
#include "Scale.h"
/*#include "Axis.h"*/

/* Declare specific AtXYAxisWidget class and instance datatypes */

typedef struct _AtXYAxisClassRec*  AtXYAxisWidgetClass;
typedef struct _AtXYAxisRec*       AtXYAxisWidget;

/* Declare the class constant */

extern WidgetClass atXYAxisWidgetClass;

/*
 * Resources available to be set/fetched
 */
#define XtNaxisTransform  "axisTransform"
#define XtNautoScale      "autoScale"
#define XtNautoTics       "autoTics"
#define XtNroundEndpoints "roundEndpoints"
#define XtNlinTicFormat   "linTicFormat"
#define XtNlogTicFormat   "logTicFormat"

#define XtCAxisTransform  "AxisTransform"
#define XtCAutoScale      "AutoScale"
#define XtCAutoTics       "AutoTics"
#define XtCRoundEndpoints "RoundEndpoints"
#define XtNplotTickUseDefault "plotTickUseDefault"
#define XtNplotNumUseDefault "plotNumUseDefault"
/*#define XtNplotMaxUseDefault "plotMaxUseDefault"
#define XtNplotMinUseDefault "plotMinUseDefault"*/
#define XtCLinTicFormat   "LinTicFormat"
#define XtCLogTicFormat   "LogTicFormat"

/* For use with X Toolkit resources and converters */

extern void AtRegisterAxisTransformConverter P((void));

#endif /* _XYAxis_h */
