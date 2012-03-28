/*      AxisCore.h
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Mon Jul 27 14:16:10 1992, patchlevel 2
 *                                      Added new resource XtNnumberWidth
 *                                      and function AtAxisGetNumberWidth()
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 *
 *      SCCSid[] = "@(#) Plotter V6.0  92/08/15  AxisCore.h"
 */

/*

Copyright 1992 by University of Paderborn
Copyright 1991 by Burdett, Buckeridge & Young Ltd.

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
 *   The AtAxisCore class is the parent placeholder for for all Axis
 *   classes.  It handles maintaing and calling the callback lists, and
 *   knows how to draw the axis and labels from the stuff stored in the
 *   private part of the record.  It has no recalc proceedure so
 *   actually setting up these arrays has to be done by subclasses.
 *
 *   The range and position functions can be chained from subclass range
 *   and position functions.
 */

#ifndef _Jpt_AxisCore_h
#define _Jpt_AxisCore_h

#include "At.h"
#include "Plot.h"
#include "Text.h"
#include "Scale.h"

/* declare specific AtAxisCoreWidget class and instance datatypes */

typedef struct _AtAxisCoreClassRec*     AtAxisCoreWidgetClass;
typedef struct _AtAxisCoreRec*          AtAxisCoreWidget;

/* declare the class constant */

extern WidgetClass  atAxisCoreWidgetClass;

/*
 *   Resources available to be set/fetched
 */

#define XtNmin                "min"
#define XtNmax                "max"
#define XtNplotMaxUseDefault "plotMaxUseDefault"
#define XtNplotMinUseDefault "plotMinUseDefault"
#define XtNzoomMax            "zoomMax"
#define XtNzoomMin            "zoomMin"
#define XtNticInterval        "ticInterval"
#define XtNsubticInterval     "subticInterval"
#define XtNvertical           "vertical"
#define XtNmirror             "mirror"
#define XtNrangeCallback      "rangeCallback"

#define XtNlabelSize          "labelSize"
#define XtNlabelStyle         "labelStyle"
#define XtNlabelColor         "labelColor"

#define XtNnumberWidth        "numberWidth"
#define XtNnumberSize         "numberSize"
#define XtNnumberStyle        "numberStyle"
#define XtNnumberColor        "numberColor"

#define XtNticsInside         "ticsInside"
#define XtNticsOutside        "ticsOutside"
#define XtNticLength          "ticLength"
#define XtNsubticLength       "subticLength"

#define XtNdrawNumbers        "drawNumbers"
#define XtNnumbersOutside     "numbersOutside"

#define XtNdrawGrid           "drawGrid"
#define XtNdrawSubgrid        "drawSubgrid"
#define XtNdrawOrigin         "drawOrigin"
#define XtNdrawFrame          "drawFrame"

#define XtNaxisWidth          "axisWidth"
#define XtNaxisColor          "axisColor"

#define XtNplotAnnotationMethod "plotAnnotationMethod"
#define XtNplotTicLabelStrings "plotTicLabelStrings"
#define XtNplotTicLabelUseDefault "plotTicLabelUseDefault"

#define XtCMin                "Min"
#define XtCMax                "Max"
#define XtCTicInterval        "TicInterval"
#define XtCSubticInterval     "SubticInterval"
#define XtCVertical           "Vertical"
#define XtCMirror             "Mirror"

#define XtCNumberWidth        "NumberWidth"

#define XtCTicsInside         "TicsInside"
#define XtCTicsOutside        "TicsOutside"
#define XtCTicLength          "TicLength"

#define XtCDrawNumbers        "DrawNumbers"
#define XtCNumbersOutside     "NumbersOutside"

#define XtCDrawGrid           "DrawGrid"
#define XtCDrawSubgrid        "DrawSubgrid"
#define XtCDrawOrigin         "DrawOrigin"
#define XtCDrawFrame          "DrawFrame"

#define XtCAxisWidth          "AxisWidth"

#ifndef XtRDouble
# define XtRDouble            "Double"
#endif

/*
 *   These are the argument types for the callback lists:
 */

typedef struct {
     float     *minp, *maxp, *tic_intervalp;
     int        *max_widthp;
} AtAxisRangeArgs;


/*
 *   Call the member functions via this wrapper
 */

extern void PlotScaleMotionRecalcAxis P((AtAxisCoreWidget, int, int, int, int));
extern void AtAxisAskRange P((AtAxisCoreWidget, float *minp, float *maxp));
extern void AxisNewTics P((AtAxisCoreWidget, float, float));

/*
 * Returns TRUE if size change requires tic_interval to be recalculated
 * via a call to AtAxisAskRange (again)
 */

extern Boolean AtAxisSetPosition P((AtAxisCoreWidget, int x1, int y1,
				    int x2, int y2, int grid_len));

/*
 *      Simple member functions
 */

extern AtScale *AtAxisGetScale P((AtAxisCoreWidget));
extern int AtAxisWidth P((AtAxisCoreWidget));
extern int AtAxisWidthPS P((AtAxisCoreWidget));
extern void AtAxisDrawPS P((AtAxisCoreWidget, FILE *, AtScale *,
			    int, int, int, int, int));
extern void AtAxisGetBounds P((AtAxisCoreWidget, float *, float *));
extern AtTransform AtAxisGetTransform P((AtAxisCoreWidget));
extern int AtAxisGetNumberWidth P((AtAxisCoreWidget));

#endif /* _AxisCore_h */
