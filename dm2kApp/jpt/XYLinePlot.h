/*
 *      XYLinePlot.h
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Mon Jul 27 14:19:43 1992, patchlevel 2
 *                                      Plot types steps and bars added
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 *
 *      SCCSid[] = "@(#) Plotter V6.0  92/08/15  XYLinePlot.h"
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

/*
 *   The more complicated sort of plot.
 *   A line connecting the X and Y values attached to the axes.
 */

#ifndef _Jpt_AtXYLinePlot_h
#define _Jpt_AtXYLinePlot_h

#include "At.h"
#include "XYPlot.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef P
# if defined (__STDC__) || defined (__cplusplus)
#  define P(args) args
# else
#  define P(args) ()
# endif
#endif

/* Declare specific AtXYLinePlotWidget class and instance datatypes */

typedef struct _AtXYLinePlotClassRec*   AtXYLinePlotWidgetClass;
typedef struct _AtXYLinePlotRec*        AtXYLinePlotWidget;

/* Declare the class constant */

extern WidgetClass atXYLinePlotWidgetClass;

/*
 *   Resources available to be set/fetched
 */

#define XtNplotLineType  "plotLineType"
#define XtNplotLineStyle "plotLineStyle"
#define XtNplotMarkType  "plotMarkType"
#define XtNplotMarkColor "plotMarkColor"
#define XtNplotMarkSize  "plotMarkSize"
#define XtNplotFillStyle "plotFillStyle"

#define XtCPlotLineType  "PlotLineType"
#define XtCPlotLineStyle "PlotLineStyle"
#define XtCPlotMarkType  "PlotMarkType"
#define XtCPlotMarkColor "PlotMarkColor"
#define XtCPlotMarkSize  "PlotMarkSize"
#define XtCPlotFillStyle "PlotFillStyle"

/*
 *   The plot line types
/*
*   Some types for easier attaching data
*/

typedef struct {
     float x;
     float y;
} AtDoublePoint;

typedef struct {
     float x;
     float y;
} AtFloatPoint;

typedef struct {
     int x;
     int y;
} AtIntPoint;


/*
 *   The member routines - these ones are inherited from XYPlot
 */
extern void PlotScaleMotionRescalePlot P((AtXYLinePlotWidget, AtScale *, AtScale *));

#define AtXYLinePlotAttachData(w, xp, xt, xstride, yp, yt, ystride, start, num) \
     AtXYPlotAttachData((AtXYPlotWidget) w, xp, xt, xstride, \
					    yp, yt, ystride, start, num)
#define AtXYLinePlotExtendData(w, n) \
     AtXYPlotExtendData((AtXYPlotWidget) w, n)

/*
 *   The member routines for AtxxPoint data
 */

#define AtXYLinePlotAttachDoublePoints(w, data, start, num) \
     AtXYPlotAttachData((AtXYPlotWidget) w, \
	  (XtPointer) &(data)->x, AtDouble, sizeof(AtDoublePoint), \
	  (XtPointer) &(data)->y, AtDouble, sizeof(AtDoublePoint), \
	  start, num)

#define AtXYLinePlotAttachFloatPoints(w, data, start, num) \
     AtXYPlotAttachData((AtXYPlotWidget) w, \
	  (XtPointer) &(data)->x, AtFloat, sizeof(AtFloatPoint), \
	  (XtPointer) &(data)->y, AtFloat, sizeof(AtFloatPoint), \
	  start, num)

#define AtXYLinePlotAttachIntPoints(w, data, start, num) \
     AtXYPlotAttachData((AtXYPlotWidget) w, \
	  (XtPointer) &(data)->x, AtInt, sizeof(AtIntPoints), \
	  (XtPointer) &(data)->y, AtInt, sizeof(AtIntPoints), \
	  start, num)

/* For use with X Toolkit resources and converters */

#define XtRPlotLineType  "PlotLineType"
extern void AtRegisterPlotLineTypeConverter P((void));
#define XtRPlotLineStyle "PlotLineStyle"
extern void AtRegisterPlotLineStyleConverter P((void));
#define XtRPlotMarkType  "PlotMarkType"
extern void AtRegisterPlotMarkTypeConverter P((void));
#define XtRPlotFillType "PlotFillStyle"

#ifdef __cplusplus
extern }
#endif

#endif /* _AtXYLinePlot_h */
