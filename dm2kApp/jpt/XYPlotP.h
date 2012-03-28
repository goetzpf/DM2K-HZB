/*
 *      XYPlotP.h
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Resources XtNxOffest and XtNyOffset
 *                                      added
 *                                      Changed <At/..> to <X11/At/..>.
 *
 *      SCCSid[] = "@(#) Plotter V6.0  92/08/15  XYPlotP.h"
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
 * Another pseudo class that sits between AtPlot and the line and bar
 * plots.  It provides all the resources required to access the data
 * using the elements of the structure.
 */

#ifndef _Jpt_AtXYPlotP_h
#define _Jpt_AtXYPlotP_h

#include "At.h"
#include "PlotP.h"
#include "XYPlot.h"

typedef void (*AtXYPlotAttachProc) P((AtXYPlotWidget, BoundingBox *, int));

typedef struct {
     AtXYPlotAttachProc  attach_data;
} AtXYPlotClassPart;

#define XtInheritAttachData ((AtXYPlotAttachProc) _XtInherit)

typedef struct _AtXYPlotClassRec {
     ObjectClassPart     object_class;
     AtPlotClassPart     plot_class;
     AtXYPlotClassPart   lplot_class;
} AtXYPlotClassRec;

extern AtXYPlotClassRec atXYPlotClassRec;

typedef struct {
     /* Resources */
     float         x_offset;
     float         y_offset;
     /* Private State */
     XtPointer      xdata;
     AtDataType     xtype;
     Cardinal       xstride;
     XtPointer      ydata;
     AtDataType     ytype;
     Cardinal       ystride;
     Cardinal       start;
     /* The pointer to (one part of) the pixel data */
     Cardinal       num_points;
     XtPointer      pix;
     XtPointer      zoom_pix;
     /* The pointer to the previous pixel data for use in fast_update mode */
     Cardinal       old_num_points;
     XtPointer      old_pix;
} AtXYPlotPart;

typedef struct _AtXYPlotRec {
     ObjectPart     object;
     AtPlotPart     plot;
     AtXYPlotPart   lplot;
} AtXYPlotRec;

#endif /* _AtXYPlotP_h */
