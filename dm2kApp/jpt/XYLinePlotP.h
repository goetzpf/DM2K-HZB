/*
 *      XYLinePlotP.h
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 *
 *      SCCSid[] = "@(#) Plotter V6.0  92/08/15  XYLinePlotP.h"
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

#ifndef _Jpt_AtXYLinePlotP_h
#define _Jpt_AtXYLinePlotP_h

#include "At.h"
#include "XYPlotP.h"
#include "XYLinePlot.h"

typedef struct {
     int  empty;
} AtXYLinePlotClassPart;

typedef struct _AtXYLinePlotClassRec {
     ObjectClassPart          object_class;
     AtPlotClassPart          plot_class;
     AtXYPlotClassPart        lplot_class;
     AtXYLinePlotClassPart    xyplot_class;
} AtXYLinePlotClassRec;

extern AtXYLinePlotClassRec atXYLinePlotClassRec;

typedef struct {
     /* Ressources */
     PlotType            plot_type;
     AtPlotLineType      line_type;
     AtPlotLineStyle     line_style;
     AtPlotMarkType      mark_type;
     AtPlotFillStyle     fill_style;
     Pixel               mark_color; /*mark color*/
     int                 mark_size;  /*mark size*/
     /* Private state */
     short               impulse_y;
     GC                  line_gc;
     GC                  mark_gc;
} AtXYLinePlotPart;

typedef struct _AtXYLinePlotRec {
     ObjectPart          object;
     AtPlotPart          plot;
     AtXYPlotPart        lplot;
     AtXYLinePlotPart    lineplot;
} AtXYLinePlotRec;

#endif /* _AtXYLinePlotP_h */
