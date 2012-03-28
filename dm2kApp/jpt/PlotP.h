/*
 *      PlotP.h
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Mon Jul 27 14:17:10 1992, patchlevel 2
 *                                      Draw() changed for drawing
 *                                      to a pixmap instead of a window
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 *
 *      SCCSid[] = "@(#) Plotter V6.0  92/08/15  PlotP.h"
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

#ifndef _Jpt_AtPlotP_h
#define _Jpt_AtPlotP_h


#include "At.h"
#include "Scale.h"
#include "Plotter.h"
#include "Plot.h"

#include <X11/ObjectP.h>

/*
 *   This widget serves as the base class for any plot type
 *   to be displayed in the AtPlotter widget.
 *   It is never intended to be instantiated itself.
 *   Only subclasses of it are.
 */

typedef void (*AtPlotDrawProc) P((AtPlotWidget, Display *, Drawable, Region,
				  int));
typedef void (*AtPlotDrawIconProc) P((AtPlotWidget, Display *, Drawable,
				      int, int, int, int, Region));
typedef void (*AtPlotDrawPSProc) P((AtPlotWidget, FILE *, AtScale *,
				    AtScale *));
typedef void (*AtPlotDrawIconPSProc) P((AtPlotWidget, FILE *, int, int,
					int, int));
typedef void (*AtPlotRecalcProc) P((AtPlotWidget, AtScale *, AtScale *,
				    int, int));
typedef struct _AtPlotClassPart {
     AtPlotDrawProc             draw;
     AtPlotDrawIconProc         draw_icon;
     AtPlotDrawPSProc           draw_ps;
     AtPlotDrawIconPSProc       draw_icon_ps;
     AtPlotRecalcProc           recalc;
} AtPlotClassPart;

typedef struct _AtPlotClassRec {
     ObjectClassPart     object_class;
     AtPlotClassPart     plot_class;
} AtPlotClassRec;

extern AtPlotClassRec atPlotClassRec;

typedef struct _AtPlotPart {
     /* Resources */
     Pixel     foreground;
     Pixel     background;
     int       line_width;
     int       line_style;
     int       dash_length;
     Boolean   fast_update;
     /* Private state */
     GC        gc;
} AtPlotPart;

typedef struct _AtPlotRec {
     ObjectPart     object;
     AtPlotPart     plot;
} AtPlotRec;

#define XtInheritDraw ((AtPlotDrawProc) _XtInherit)
#define XtInheritDrawIcon ((AtPlotDrawIconProc) _XtInherit)
#define XtInheritDrawPS ((AtPlotDrawPSProc) _XtInherit)
#define XtInheritDrawIconPS ((AtPlotDrawIconPSProc) _XtInherit)
#define XtInheritRecalc ((AtPlotRecalcProc) _XtInherit)

#endif /* _AtPlotP_h */
