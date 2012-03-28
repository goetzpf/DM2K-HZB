/*
 *      Plot.h
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Mon Jul 27 14:16:59 1992, patchlevel 2
 *                                      Draw() changed for drawing
 *                                      to a pixmap instead of a window
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 *
 *      SCCSid[] = "@(#) Plotter V6.0  92/08/15  Plot.h"
 */

/*

Copyright 1992 by University of Paderborn
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

#ifndef _Jpt_AtPlot_h
#define _Jpt_AtPlot_h

#include "At.h"
#include "Scale.h"

#define XtNlineWidth    "lineWidth"
#define XtNlineStyle    "lineStyle"
#define XtNdashLength   "dashLength"
#define XtNfastUpdate   "fastUpdate"

#define XtCLineWidth    "LineWidth"
#define XtCLineStyle    "LineStyle"
#define XtCDashLength   "DashLength"
#define XtCFastUpdate   "FastUpdate"

extern WidgetClass atPlotWidgetClass;
typedef struct _AtPlotClassRec *AtPlotWidgetClass;
typedef struct _AtPlotRec *AtPlotWidget;

/*
 *   Extern routine for postscript output.
 *   Write a line to the file to set the postscript GC to reflect
 *   the plot instance record (linewidth, linestyle etc).
 *   Assume 1 PostScript unit is more-or-less one pixel.
 */

extern void AtPlotPSLineStyle P((FILE *, AtPlotWidget));

/*
 *   These are wrappers around the member functions
 */

extern void AtPlotDraw P((AtPlotWidget, Display *, Drawable, Region, int));
extern void AtPlotDrawIcon P((AtPlotWidget, Display *, Drawable,
			      int, int, int, int, Region));
/*extern void AtPlotDrawPS P((AtPlotWidget, FILE *, AtScale *, AtScale *));
extern void AtPlotDrawIconPS P((AtPlotWidget, FILE *, int, int, int, int)); */
extern void AtPlotRecalc P((AtPlotWidget, AtScale *, AtScale *, int, int));

typedef enum {
	AtDouble,
	AtFloat,
	AtInt
} AtDataType;

#endif /* _AtPlot_h */
