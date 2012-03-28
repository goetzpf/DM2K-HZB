/*
 *      XYPlot.h
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Resources XtNxOffest and XtNyOffset
 *                                      added
 *                                      Changed <At/..> to <X11/At/..>.
 *
 *      SCCSid[] = "@(#) Plotter V6.0  92/08/15  XYPlot.h"
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
 * AtSPlot is  non-instanced class that contains resources and routines
 * for accessing data that is currently stored in an array of structures.
 */

#ifndef _Jpt_AtXYPlot_h
#define _Jpt_AtXYPlot_h

#include "At.h"
#include "Plot.h"

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

/* Declare specific AtXYPlotWidget class and instance datatypes */

typedef struct _AtXYPlotClassRec*  AtXYPlotWidgetClass;
typedef struct _AtXYPlotRec*       AtXYPlotWidget;

/* Declare the class constant */

extern WidgetClass atXYPlotWidgetClass;

/*
 *   Resources available to be set/fetched
 */

#define XtNxOffset       "xOffset"
#define XtNyOffset       "yOffset"

#define XtCXOffset       "XOffset"
#define XtCYOffset       "YOffset"

#ifndef XtRDouble
# define XtRDouble       "Double"
#endif

/*
 * The member routine to attach the data.
 * The subclasses of AtXPlot export a #define for this routine.
 */
extern int AtXYPlotReleaseData P((AtXYPlotWidget spw));
extern int AtXYPlotCopyData P((AtXYPlotWidget,
		  XtPointer xdata, AtDataType xtype, Cardinal xstride,
		  XtPointer ydata, AtDataType ytype, Cardinal ystride,
		  Cardinal start, Cardinal num));
extern void AtXYPlotAttachData P((AtXYPlotWidget,
				  XtPointer xdata, AtDataType xtype, Cardinal xstride,
				  XtPointer ydata, AtDataType ytype, Cardinal ystride,
				  Cardinal start, Cardinal num));

extern void AtXYPlotExtendData P((AtXYPlotWidget, Cardinal num));

/*
 * Get the i'th value from the AtXYPlot data.  This declares the function
 * version, a macro is available below.  This is intended mainly for
 * subclasses of AtXYPlot, but applications may find it useful.
 */

extern float AtXYPlotGetValue P((AtXYPlotWidget, Cardinal));

/*
 * A macro version of the above.  May be faster, but death to debug!
 */

/* First, get the address of the i'th element */

#define _lp_xptr(p, i) ((XtPointer)((char *)p->lplot.xdata + \
				    p->lplot.xstride * (i)))
#define _lp_yptr(p, i) ((XtPointer)((char *)p->lplot.ydata + \
				    p->lplot.ystride * (i)))

#define AtXYPlotGetXValue(p, i) \
     ((p)->lplot.x_offset + (float) \
      ((p)->lplot.xtype == AtDouble ? *((float *)_lp_xptr((p), (i))) : \
       (p)->lplot.xtype == AtFloat ? (float)*((float *)_lp_xptr((p), (i))) : \
       (p)->lplot.xtype == AtInt ? (float)*((int *)_lp_xptr((p), (i))) : 0.0))

#define AtXYPlotGetYValue(p, i) \
     ((p)->lplot.y_offset + (float) \
      ((p)->lplot.ytype == AtDouble ? *((float *)_lp_yptr((p), (i))) : \
       (p)->lplot.ytype == AtFloat ? (float)*((float *)_lp_yptr((p), (i))) : \
       (p)->lplot.ytype == AtInt ? (float)*((int *)_lp_yptr((p), (i))) : 0.0))

#ifdef __cplusplus
extern }
#endif

#endif /* _AtXYPlot_h */
