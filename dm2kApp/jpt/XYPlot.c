/*
 *      XYPlot.c
 *
 *      Based on the AthenaTools Plotter Widget Set - Version 6.0
 *      G.Lei Aug 1998,                 new resources and corresponding methods
 *                                      added 
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Mon Jul 27 14:19:54 1992, patchlevel 2
 *                                      Shorter names for procedures
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Resources XtNxOffest and XtNyOffset
 *                                      and all needed stuff added
 *                                      Changed <At/..> to <X11/At/..>.
 */
static char SCCSid[] = "@(#) Plotter V6.0  92/08/15  XYPlot.c";

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
 *   The AtXYPlot object is simply a holder for some resources to allow
 *   access to data items that are elements of a structure or an array.
 */

#include "XYPlotP.h"

#ifndef AtXYPlotGetXValue
float AtXYPlotGetXValue P((AtXYPlotWidget p, Cardinal i));
float AtXYPlotGetXValue(p, i)
AtXYPlotWidget p;
Cardinal i;
{
     XtPointer ptr = _lp_xptr(p, i);

     float ret = (p)->lplot.x_offset +
	  ((p)->lplot.xtype == AtDouble ? *((float *)ptr) :
	   (p)->lplot.xtype == AtFloat ? (float)*((float *)ptr) :
	   (p)->lplot.xtype == AtInt ? (float)*((int *)ptr) :
	   0.0);

     return ret;
}
#endif

#ifndef AtXYPlotGetYValue
float AtXYPlotGetYValue P((AtXYPlotWidget p, Cardinal i));
float AtXYPlotGetYValue(p, i)
AtXYPlotWidget p;
Cardinal i;
{
     XtPointer ptr = _lp_yptr(p, i);

     float ret = (p)->lplot.y_offset +
	  ((p)->lplot.ytype == AtDouble ? *((float *)ptr) :
	   (p)->lplot.ytype == AtFloat ? (float)*((float *)ptr) :
	   (p)->lplot.ytype == AtInt ? (float)*((int *)ptr) :
	   0.0);

     return ret;
}
#endif

static void Destroy P((AtXYPlotWidget));
static void ClassPartInitialize P((WidgetClass));
static void Initialize P((AtXYPlotWidget, AtXYPlotWidget));
static Boolean SetValues P((AtXYPlotWidget, AtXYPlotWidget, AtXYPlotWidget));
/*
 *   The resources
 */

static float dflt_offset = 0.0;

#define off(field) XtOffsetOf (AtXYPlotRec, lplot.field)
static XtResource resources[] = {
  {
     XtNxOffset, XtCXOffset,
     XtRDouble, sizeof(float),
     off(x_offset), XtRImmediate, (XtPointer) &dflt_offset
  },
  {
     XtNyOffset, XtCYOffset,
     XtRDouble, sizeof(float),
     off(y_offset), XtRImmediate, (XtPointer) &dflt_offset
  }
};
#undef  off

AtXYPlotClassRec atXYPlotClassRec = {
  { /* core fields */
    /* superclass               */      (WidgetClass) &atPlotClassRec,
    /* class_name               */      "AtXYPlot",
    /* widget_size              */      sizeof(AtXYPlotRec),
    /* class_initialize         */      NULL,
    /* class_part_initialize    */      ClassPartInitialize,
    /* class_inited             */      FALSE,
    /* initialize               */      (XtInitProc) Initialize,
    /* initialize_hook          */      NULL,
    /* pad                      */      NULL,
    /* pad                      */      NULL,
    /* pad                      */      0,
    /* resources                */      resources,
    /* num_resources            */      XtNumber(resources),
    /* xrm_class                */      NULLQUARK,
    /* pad                      */      FALSE,
    /* pad                      */      FALSE,
    /* pad                      */      FALSE,
    /* pad                      */      FALSE,
    /* destroy                  */      (XtWidgetProc) Destroy,
    /* pad                      */      NULL,
    /* pad                      */      NULL,
    /* set_values               */      (XtSetValuesFunc) SetValues,
    /* set_values_hook          */      NULL,
    /* pad                      */      NULL,
    /* get_values_hook          */      NULL,
    /* pad                      */      NULL,
    /* version                  */      XtVersion,
    /* callback_private         */      NULL,
    /* pad                      */      NULL,
    /* pad                      */      NULL,
    /* pad                      */      NULL,
    /* pad                      */      NULL
  },
  { /* atPlot fields */
    /* draw                     */      NULL,
    /* draw_icon                */      NULL,
    /* drawPS                   */      NULL,
    /* draw_iconPS              */      NULL,
    /* recalc                   */      NULL
  },
  { /* lPlot fields */
    /* attach_data              */      NULL
  }
};

WidgetClass atXYPlotWidgetClass = (WidgetClass)&atXYPlotClassRec;

/*
 *      The class initialize/initialize/destroy/setvalues procs
 */

static void ClassPartInitialize(wc)
WidgetClass wc;
{
     AtXYPlotWidgetClass super = (AtXYPlotWidgetClass) wc->core_class.superclass;
     AtXYPlotWidgetClass spc = (AtXYPlotWidgetClass) wc;
     if (spc->lplot_class.attach_data == XtInheritAttachData) {
	  spc->lplot_class.attach_data = super->lplot_class.attach_data;
     }
     *SCCSid = *SCCSid;       /* Keep gcc quiet */
}

static void Initialize(req, new)
AtXYPlotWidget req, new;
{
     AtXYPlotPart *sr = &new->lplot;

     sr->x_offset = 0.0;
     sr->y_offset = 0.0;
     sr->xdata = sr->ydata = NULL;
     sr->xstride = sr->ystride = sr->start = sr->num_points = sr->old_num_points = 0;
     sr->xtype = sr->ytype = AtInt;
     sr->pix = sr->old_pix = NULL;
}

static void Destroy(sp)
AtXYPlotWidget sp;
{
     if (sp->lplot.pix) {
       XtFree(sp->lplot.pix);
       sp->lplot.pix =NULL;
       }
     if (sp->lplot.old_pix) {
       XtFree(sp->lplot.old_pix);
       sp->lplot.old_pix = NULL;
       }
}

static Boolean SetValues(current, request, new)
AtXYPlotWidget current, request, new;
{
#define Changed(field) (new->lplot.field != current->lplot.field)
     BoundingBox bb;
     int i;

     if (Changed(x_offset) || Changed(y_offset)) {
	  bb.xmax = bb.ymax = -PLOTTER_HUGE_VAL;
	  bb.xmin = bb.ymin = PLOTTER_HUGE_VAL;
	  for (i = 0; i < new->lplot.num_points; i++) {
	       register float v;
	       v = AtXYPlotGetXValue(new, i);
	       bb.xmax = Max(bb.xmax, v);
	       bb.xmin = Min(bb.xmin, v);
	       v = AtXYPlotGetYValue(new, i);
	       bb.ymax = Max(bb.ymax, v);
	       bb.ymin = Min(bb.ymin, v);
	  }
	  AtPlotterPlotDataChanged((AtPlotWidget) new, &bb, new->plot.fast_update);
     }

     /* Nothing to redisplay */
     return False;
#undef Changed
}

/*
 *   These are the exported "member" routines
 */

int AtXYPlotReleaseData(spw)
AtXYPlotWidget spw;
{
  free(spw->lplot.xdata);
  spw->lplot.xdata = NULL;
  free(spw->lplot.ydata);
  spw->lplot.ydata = NULL;
  spw->lplot.num_points = 0;
}

int AtXYPlotCopyData(spw, xdata, xtype, xstride, ydata, ytype, ystride, start, num)
AtXYPlotWidget spw;
XtPointer xdata, ydata;
AtDataType xtype, ytype;
Cardinal xstride, ystride, start, num;
{
     BoundingBox bb;
     AtXYPlotAttachProc adp;
     int i;

     XtCheckSubclass((Widget)spw, atXYPlotWidgetClass,
		     "AtXYPlotAttachData needs an AtXYPlot object");

     if (spw->lplot.pix) {
	  XtFree((char *)spw->lplot.pix);
          spw->lplot.pix = NULL;
	  }

     spw->lplot.num_points = num;
     spw->lplot.xdata = (float *)calloc(num, sizeof(float));
     spw->lplot.xtype = xtype;
     spw->lplot.xstride = xstride;
     spw->lplot.ydata = (float *) calloc(num, sizeof(float));
     spw->lplot.ytype = ytype;
     spw->lplot.ystride = ystride;
     spw->lplot.start = start;

     if ((spw->lplot.xdata ==NULL) || (spw->lplot.ydata==NULL)) {
       spw->lplot.num_points = 0;
       printf("AtXYPlotCopyData: cannot get enough space for data\n");
       return(-1);
       }
     memcpy(spw->lplot.xdata, xdata, sizeof(float)*num);
     memcpy(spw->lplot.ydata, ydata, sizeof(float)*num);

     bb.xmax = bb.ymax = -PLOTTER_HUGE_VAL;
     bb.xmin = bb.ymin = PLOTTER_HUGE_VAL;

     for (i = 0; i < spw->lplot.num_points; i++) {
	  register float v;
/*	  spw->lplot.xdata[i] = xdata[i];
	  spw->lplot.ydata[i] = ydata[i];*/
	  v = AtXYPlotGetXValue(spw, i);
	  bb.xmax = Max(bb.xmax, v);
	  bb.xmin = Min(bb.xmin, v);
	  v = AtXYPlotGetYValue(spw, i);
	  bb.ymax = Max(bb.ymax, v);
	  bb.ymin = Min(bb.ymin, v);
     }

     if (adp = ((AtXYPlotWidgetClass)
		spw->object.widget_class)->lplot_class.attach_data) {
	  adp(spw, &bb, False);
     }

     AtPlotterPlotDataChanged((AtPlotWidget)spw, &bb, spw->plot.fast_update);
} /* end of AtXYPlotCopyData */

/*
 *   We have added some more data, num is the new total
 *   Data runs from 0 .. num-1, where x value of first is start.
 */

void AtXYPlotAttachData(spw, xdata, xtype, xstride, ydata, ytype, ystride, start, num)
AtXYPlotWidget spw;
XtPointer xdata, ydata;
AtDataType xtype, ytype;
Cardinal xstride, ystride, start, num;
{
     BoundingBox bb;
     AtXYPlotAttachProc adp;
     int i;
     float *fff=(float *)xdata;

     XtCheckSubclass((Widget)spw, atXYPlotWidgetClass,
		     "AtXYPlotAttachData needs an AtXYPlot object");

     if (spw->plot.fast_update) {
     printf("AtXYPlotAttachData in fast_update mode\n");
	  /* Save a copy of the current pix data and request a refresh */
	  if (spw->lplot.old_pix) {
	       /* Is this an error???? */
#ifdef DEBUG
	       fprintf(stderr, "In AtXYPlotAttachData - old pix still current!\n");
#endif
	       XtFree((char *)spw->lplot.old_pix);
	       spw->lplot.old_pix = NULL;
	  }
	  spw->lplot.old_pix = spw->lplot.pix;
	  spw->lplot.old_num_points = spw->lplot.num_points;
     } else if (spw->lplot.pix) {
	  XtFree((char *)spw->lplot.pix);
          spw->lplot.pix = NULL;
	  }

     spw->lplot.num_points = num;
     spw->lplot.xdata = xdata;
     spw->lplot.xtype = xtype;
     spw->lplot.xstride = xstride;
     spw->lplot.ydata = ydata;
     spw->lplot.ytype = ytype;
     spw->lplot.ystride = ystride;
     spw->lplot.start = start;

     bb.xmax = bb.ymax = -PLOTTER_HUGE_VAL;
     bb.xmin = bb.ymin = PLOTTER_HUGE_VAL;

     for (i = 0; i < spw->lplot.num_points; i++) {
	  register float v;
	  v = AtXYPlotGetXValue(spw, i);
	  bb.xmax = Max(bb.xmax, v);
	  bb.xmin = Min(bb.xmin, v);
	  v = AtXYPlotGetYValue(spw, i);
	  bb.ymax = Max(bb.ymax, v);
	  bb.ymin = Min(bb.ymin, v);
     }

     if (adp = ((AtXYPlotWidgetClass)
		spw->object.widget_class)->lplot_class.attach_data) {
	  adp(spw, &bb, False);
     }

     AtPlotterPlotDataChanged((AtPlotWidget)spw, &bb, spw->plot.fast_update);
}

/*
 *   We have added some more data, num is the new total
 *   Data runs from 0 .. num-1, where x value of first is start.
 */

void AtXYPlotExtendData(spw, num)
AtXYPlotWidget spw;
Cardinal num;
{
     BoundingBox bb;
     AtXYPlotAttachProc adp;
     int i;
     int old_num = spw->lplot.num_points;

     XtCheckSubclass((Widget)spw, atXYPlotWidgetClass,
		     "AtXYPlotExtendData needs an AtXYPlot object");

     bb.xmax = bb.ymax = -PLOTTER_HUGE_VAL;
     bb.xmin = bb.ymin = PLOTTER_HUGE_VAL;

     /* Get the BBox for THE NEW STUFF ONLY */
     for (i = old_num; i < num; i++) {
	  register float v;
	  v = AtXYPlotGetXValue(spw, i);
	  bb.xmax = Max(bb.xmax, v);
	  bb.xmin = Min(bb.xmin, v);
	  v = AtXYPlotGetYValue(spw, i);
	  bb.ymax = Max(bb.ymax, v);
	  bb.ymin = Min(bb.ymin, v);
     }
     spw->lplot.num_points = num;

     if (adp = ((AtXYPlotWidgetClass)
		spw->object.widget_class)->lplot_class.attach_data) {
	  adp(spw, &bb, True);
     }

     AtPlotterPlotExtended((AtPlotWidget)spw, &bb, old_num, num - 1);
}
