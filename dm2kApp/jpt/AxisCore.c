/*
 *      AxisCore.c
 *
 *      Based on the AthenaTools Plotter Widget Set - Version 6.0
 *      G.Lei Aug 1998,                 new resources and corresponding methods
 *                                      added 
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Wed Jul 22 09:29:41 1992, patchlevel 1
 *                                      Bug in SetValues() fixed (reported by
 *                                      Ken Rempe 92/07/21, ken@caesar.uucp)
 *      klin, Mon Jul 27 14:16:10 1992, patchlevel 2
 *                                      Added new resource XtNnumberWidth
 *                                      and function AtAxisGetNumberWidth().
 *                                      Draw() changed for drawing
 *                                      to a pixmap instead of a window.
 *                                      Shorter procedure names.
 *      klin, Fri Aug 14 15:45:52 1992, patchlevel 4
 *                                      Minor changes in PS output.
 *                                      Changed <At/..> to <X11/At/..>.
 */
static char SCCSid[] = "@(#) Plotter V6.0  92/08/15  AxisCore.c";

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
 *   This contains the heart of the pixel fiddling code for the axes.
 *   It is a meta class that relies on subclass methods to decide where
 *   and what to label the axes, and then handles all the calculations,
 *   redisplays etc internally.
 *
 *   Should not be instansiated, but will sort-of work if required.
 */

#include "AxisCoreP.h"
#include "AtConverters.h"
#include "PlotterP.h"

#define PS_MARGIN   6    /* Same for PS */

#define SubticOnTic(ac, i, j) (ac->subtic_pos[i] > (ac->tic_pos[j] - 2) && \
			       ac->subtic_pos[i] < (ac->tic_pos[j] + 2))

static void ClassPartInitialize P((WidgetClass));
static void ClassInitialize P((WidgetClass));
static void Initialize P((AtAxisCoreWidget, AtAxisCoreWidget));
static void Destroy P((AtAxisCoreWidget));
static Boolean SetValues P((AtAxisCoreWidget, AtAxisCoreWidget, AtAxisCoreWidget));
static void Draw P((AtPlotWidget, Display *, Drawable, Region, int));
static void Recalc P((AtPlotWidget, AtScale *, AtScale *, int, int));
static void RangeProc P((AtAxisCoreWidget, float *, float *, float *, int *));
static void CalcProc P((AtAxisCoreWidget));

/* A helper routine for handling labels */

static void ReformatLabels P((AtAxisCoreWidget, int));
static void CalcAxisWidth P((AtAxisCoreWidget));

/*  The following routins are to set font to axis number */
static int setNumberFont P(());


/* The resources */

static float dflt_min = LIN_MIN;
static float dflt_max = LIN_MAX;
static float dflt_tic = TIC_INT;
#define off(field) XtOffsetOf(AtAxisCoreRec, axiscore.field)
static XtResource resources[] = {
  {
     XtNmax, XtCMax,
     XtRDouble, sizeof(float),
     off(max), XtRDouble, (XtPointer) &dflt_max
  },
  {
     XtNmin, XtCMin,
     XtRDouble, sizeof(float),
     off(min), XtRDouble, (XtPointer) &dflt_min
  },
  {
     XtNplotMaxUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(max_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotMinUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(min_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNzoomMax, XtCMax,
     XtRDouble, sizeof(float),
     off(zoom_max), XtRDouble, (XtPointer) &dflt_max
  },
  {
     XtNzoomMin, XtCMin,
     XtRDouble, sizeof(float),
     off(zoom_min), XtRDouble, (XtPointer) &dflt_min
  },
  {
     XtNticInterval, XtCTicInterval,
     XtRDouble, sizeof(float),
     off(tic_interval), XtRImmediate, (XtPointer) &dflt_tic
  },
  {
     XtNsubticInterval, XtCSubticInterval,
     XtRDouble, sizeof(float),
     off(subtic_interval), XtRImmediate, (XtPointer) &dflt_tic
  },
  {
     XtNvertical, XtCVertical,
     XtRBoolean, sizeof(Boolean),
     off(vertical), XtRImmediate, (XtPointer) False
  },
  {
     XtNmirror, XtCMirror,
     XtRBoolean, sizeof(Boolean),
     off(mirror), XtRImmediate, (XtPointer) False
  },
  {
     XtNrangeCallback, XtCCallback,
     XtRCallback, sizeof(XtCallbackList),
     off(range_callback), XtRImmediate, (XtPointer) NULL
  },
  {
     XtNlabel, XtCLabel,
     XtRString, sizeof(String),
     off(label), XtRImmediate, (XtPointer) NULL
  },
  {
     XtNnumberWidth, XtCNumberWidth,
     XtRDimension, sizeof(Dimension),
     off(default_number_width), XtRImmediate, (XtPointer) 0
  },
  {
     XtNticLength, XtCTicLength,
     XtRDimension, sizeof(Dimension),
     off(tic_length), XtRImmediate, (XtPointer) 5
  },
  {
     XtNsubticLength, XtCTicLength,
     XtRDimension, sizeof(Dimension),
     off(subtic_length), XtRImmediate, (XtPointer) 2
  },
  {
     XtNdrawNumbers, XtCDrawNumbers,
     XtRBoolean, sizeof(Boolean),
     off(draw_numbers), XtRImmediate, (XtPointer) True
  },
  {
     XtNdrawGrid, XtCDrawGrid,
     XtRBoolean, sizeof(Boolean),
     off(draw_grid), XtRImmediate, (XtPointer) False
  },
  {
     XtNdrawSubgrid, XtCDrawSubgrid,
     XtRBoolean, sizeof(Boolean),
     off(draw_subgrid), XtRImmediate, (XtPointer) False
  },
  {
     XtNdrawOrigin, XtCDrawOrigin,
     XtRBoolean, sizeof(Boolean),
     off(draw_origin), XtRImmediate, (XtPointer) True
  },
  {
     XtNdrawFrame, XtCDrawFrame,
     XtRBoolean, sizeof(Boolean),
     off(draw_frame), XtRImmediate, (XtPointer) False
  },
  {
     XtNaxisWidth, XtCAxisWidth,
     XtRDimension, sizeof(Dimension),
     off(axis_linewidth), XtRImmediate, (XtPointer) 1
  },
  {
     XtNaxisColor, XtCForeground,
     XtRPixel, sizeof(Pixel),
     off(axis_color), XtRString, (XtPointer) XtDefaultForeground
  },
  {  XtNplotAxisFont, XtCFont,
     XtRInt, sizeof(Font),
     off(axis_font), XtRImmediate, NULL
  },
  {  XtNplotTicLabelStrings, XtCPlotStrings,
     XtRPlotStrings, sizeof(char **),
     off(tic_label_string), XtRPlotStrings, (XtPointer)NULL
  },
  {
     XtNplotTicLabelUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(tic_label_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotAnnotationMethod, XtCPlotAnnotationMethod,
     XtRInt, sizeof(int),
     off(anno_method), XtRImmediate, (XtPointer) PLOT_ANNO_VALUES
  },
  {
     XtNplotTimeFormat, XtCPlotTimeFormat,
     XtRInt, sizeof(int),
     off(time_format), XtRImmediate, (XtPointer) NULL
  },
  {
     XtNplotYAxisConst, XtCPlotYAxisConst,
     XtRFloat, sizeof(float),
     off(yaxis_const), XtRImmediate, (XtPointer) 0
  },
  {
     XtNplotYAxisMult, XtCPlotYAxisMult,
     XtRFloat, sizeof(float),
     off(yaxis_mult), XtRImmediate, (XtPointer) 0
  }
};
#undef off

AtAxisCoreClassRec atAxisCoreClassRec = {
  { /* core fields */
     /* superclass              */      (WidgetClass) &atPlotClassRec,
     /* class_name              */      "AtAxisCore",
     /* widget_size             */      sizeof(AtAxisCoreRec),
     /* class_initialize        */      (XtProc) ClassInitialize,
     /* class_part_initialize   */      (XtWidgetClassProc) ClassPartInitialize,
     /* class_inited            */      FALSE,
     /* initialize              */      (XtInitProc) Initialize,
     /* initialize_hook         */      NULL,
     /* pad                     */      NULL,
     /* pad                     */      NULL,
     /* pad                     */      0,
     /* resources               */      resources,
     /* num_resources           */      XtNumber(resources),
     /* xrm_class               */      NULLQUARK,
     /* pad                     */      FALSE,
     /* pad                     */      FALSE,
     /* pad                     */      FALSE,
     /* pad                     */      FALSE,
     /* destroy                 */      (XtWidgetProc) Destroy,
     /* pad                     */      NULL,
     /* pad                     */      NULL,
     /* set_values              */      (XtSetValuesFunc) SetValues,
     /* set_values_hook         */      NULL,
     /* pad                     */      NULL,
     /* get_values_hook         */      NULL,
     /* pad                     */      NULL,
     /* version                 */      XtVersion,
     /* callback_private        */      NULL,
     /* pad                     */      NULL,
     /* pad                     */      NULL,
     /* pad                     */      NULL,
     /* pad                     */      NULL
     },
  { /* atPlot fields */
     /* draw                    */      Draw,
     /* draw_icon               */      XtInheritDrawIcon,
     /* drawPS                  */      NULL,
     /* draw_iconPS             */      NULL,
     /* recalc                  */      Recalc
  },
  { /* axisCore fields */
     /* range_proc              */      (AtAxisRangeProc) RangeProc,
     /* calc_proc               */      (AtAxisCalcProc) CalcProc
  }
};

WidgetClass atAxisCoreWidgetClass = (WidgetClass)&atAxisCoreClassRec;

/*
 *   The core member procs
 */

static void ClassPartInitialize(wc)
WidgetClass wc;
{
     AtAxisCoreWidgetClass super =
	  (AtAxisCoreWidgetClass) wc->core_class.superclass;
     AtAxisCoreWidgetClass spc = (AtAxisCoreWidgetClass) wc;

#define CheckInherit(fld, inherit) \
     if (spc->axiscore_class.fld == inherit) \
	  spc->axiscore_class.fld = super->axiscore_class.fld;

     CheckInherit(range_proc, XtInheritRangeProc);
     CheckInherit(calc_proc, XtInheritCalcProc);
#undef CheckInherit
     *SCCSid = *SCCSid;       /* Keeps gcc quiet */
}

static void ClassInitialize(wc)
WidgetClass wc;
{
     /*AtRegisterFontSizeConverter();
     AtRegisterFontStyleConverter();*/
}

/*
 *   Some helper functions for GC management
 */
/*
static void GetLabelGC P((AtAxisCoreWidget ac));
static void GetLabelGC(ac)
AtAxisCoreWidget ac;
{
     XGCValues v;

     v.foreground = ac->axiscore.label_color;
     v.background = ac->plot.background;
     ac->axiscore.label_gc = XtGetGC(XtParent((Widget) ac),
				     GCForeground | GCBackground, &v);
}

/*static void GetAxisGC P((AtAxisCoreWidget ac));
static void GetAxisGC(ac)
AtAxisCoreWidget ac;
{
    XGCValues v;
     XtGCMask mask;

     v.foreground = ac->axiscore.axis_color;
     v.line_width = ac->axiscore.axis_linewidth;
     v.line_style = LineSolid;
     mask = GCForeground | GCLineWidth | GCLineStyle;
     ac->axiscore.axis_gc =
	  XtGetGC(XtParent((Widget) ac), mask, &v);
} 

static void GetTicGC P((AtAxisCoreWidget ac));
static void GetTicGC(ac)
AtAxisCoreWidget ac;
{
     XGCValues v;

     v.foreground = ac->axiscore.axis_color;
     v.line_style = LineSolid;
     ac->axiscore.tic_gc =
	  XtGetGC(XtParent((Widget) ac), GCForeground | GCLineStyle, &v);
}

static void GetNumberGC P((AtAxisCoreWidget ac));
static void GetNumberGC(ac)
AtAxisCoreWidget ac;
{
     XGCValues v;

     v.foreground = ac->axiscore.number_color;
     v.background = ac->plot.background;
     ac->axiscore.number_gc = XtGetGC(XtParent((Widget) ac),
				      GCForeground | GCBackground, &v);
}

static void GetGridGC P((AtAxisCoreWidget ac));
static void GetGridGC(ac)
AtAxisCoreWidget ac;
{
     XGCValues v;

     v.foreground = ac->axiscore.axis_color;
     v.line_style = LineOnOffDash;
     v.dashes = (char)1;

     ac->axiscore.grid_gc =
	  XtGetGC(XtParent((Widget) ac),
		  GCForeground | GCLineStyle | GCDashList, &v);
}

static void GetSubgridGC P((AtAxisCoreWidget ac));
static void GetSubgridGC(ac)
AtAxisCoreWidget ac;
{
     static char dashes[] = { 1, 3 };
     XGCValues v;

     v.foreground = ac->axiscore.axis_color;
     v.line_style = LineOnOffDash;
     v.dashes = (char) 222;

     ac->axiscore.subgrid_gc =
	  XtGetGC(XtParent((Widget) ac),
		  GCForeground | GCLineStyle | GCDashList, &v);
     XSetDashes(XtDisplay(XtParent((Widget) ac)), ac->axiscore.subgrid_gc,
			  0, dashes, 2);

}*/

#define FreeLabelGC(w)   XtReleaseGC((Widget) w, w->axiscore.label_gc)
/*#define FreeAxisGC(w)    XtReleaseGC((Widget) w, w->axiscore.axis_gc)*/
#define FreeTicGC(w)     XtReleaseGC((Widget) w, w->axiscore.tic_gc);
#define FreeNumberGC(w)  XtReleaseGC((Widget) w, w->axiscore.number_gc)
#define FreeGridGC(w)    XtReleaseGC((Widget) w, w->axiscore.grid_gc)
#define FreeSubgridGC(w) XtReleaseGC((Widget) w, w->axiscore.subgrid_gc)


/*
 *   Helper routines for the AtText management
 */

static void GetLabelText P((AtAxisCoreWidget acw));
static void GetLabelText(acw)
AtAxisCoreWidget acw;
{
     AtAxisCorePart *ac = &acw->axiscore;

     if (ac->label && *ac->label) {
	  ac->label_text =
	       At2TextCreate(XtParent((Widget)acw), ac->label, ac->axis_font);
	  if (ac->vertical)
	       /*AtTextRotate(ac->label_text);*/
	       ac->label_text->rotated = TRUE;
     } else
	  ac->label_text = NULL;
}

#define FreeLabelText(a) AtTextDestroy(a->axiscore.label_text)


#define CopyString(str) str = XtNewString(str)

void initAxisWidth(AtAxisCoreWidget w)
{
     AtAxisCorePart *ac = &w->axiscore;
     AtPlotterPart *pp = & (((AtPlotterWidget)XtParent(w))->plotter);

}

/*
 *   The initialize/destroy/set values procs
 */

static void Initialize(req, new)
AtAxisCoreWidget req, new;
{
     AtAxisCorePart *ac = &new->axiscore;
     AtPlotterPart *pp = & (((AtPlotterWidget)XtParent(new))->plotter);
     if ( !new->axiscore.label)
	  new->axiscore.label = XtNewString("");
     else new->axiscore.label = XtNewString(new->axiscore.label);
     GetLabelText(new);

 /* only use plotter axis gc*/
     if (new->axiscore.min >= new->axiscore.max) {
	  /*XtAppWarning(XtWidgetToApplicationContext(XtParent((Widget) new)),
		       "AxisCore Initialize - Min is > Max in AtAxisCore");*/
	  new->axiscore.min = 0.0;
	  new->axiscore.max = 1.0;
     }
     /*new->axiscore.max_bak = new->axiscore.max;
     new->axiscore.min_bak = new->axiscore.min;*/
     new->axiscore.max_bak = -PLOT_HUGE_VAL;
     new->axiscore.min_bak = -PLOT_HUGE_VAL;

/*     if (new->axiscore.min = new->axiscore.max) {
	  XtAppWarning(XtWidgetToApplicationContext(XtParent((Widget) new)),
		       "AxisCore Initialize - Min is = Max in AtAxisCore");
	  new->axiscore.min = new->axiscore.min -1;
	  new->axiscore.max = new->axiscore.max +1;
     } */

     new->axiscore.scale = new->axiscore.zoom_scale =
	  AtScaleCreate(new->axiscore.min, new->axiscore.max, 0, 1,
			AtTransformLINEAR);

     new->axiscore.num_ticsegments = new->axiscore.num_tics =
	  new->axiscore.num_subtics = 0;
     new->axiscore.x1 = new->axiscore.y1 = 0;
     new->axiscore.x2 = new->axiscore.y2 = 10;
     new->axiscore.grid_length = 10;
     new->axiscore.max_num_width = new->axiscore.actual_num_width =
	  new->axiscore.axis_width = new->axiscore.label_line =
	  new->axiscore.tic_label_line = 0;
     new->axiscore.max_num_height = 0;
     new->axiscore.tic_values = NULL;
     new->axiscore.tic_label_string = NULL;
     new->axiscore.subtic_values = NULL;
     new->axiscore.tic_segments = NULL;
     new->axiscore.tic_pos = NULL;
     new->axiscore.subtic_pos = NULL;
     new->axiscore.grid_segments = NULL;
     new->axiscore.subgrid_segments = NULL;
     new->axiscore.numbers_changed = new->axiscore.position_changed = True;

  /*if ((new->axiscore.tic_label_string) && !(new->axiscore.tic_label_dft)) 
    new->axiscore.num_tics = copyStrings(new->axiscore.tic_label_string,
	       new->axiscore.tic_label_string);*/

  /*axis font structure */
  if (!new->axiscore.axis_font) 
    new->axiscore.axis_font = XLoadFont(XtDisplay(new), "fixed");
  new->axiscore.axis_fs = XQueryFont(XtDisplay(new), new->axiscore.axis_font);
  initAxisWidth(new);
}

static void Destroy(ac)
AtAxisCoreWidget ac;
{
     XtFree(ac->axiscore.label);
     XtFree((char *) ac->axiscore.tic_segments);
     XtFree((char *) ac->axiscore.grid_segments);
     XtFree((char *) ac->axiscore.subgrid_segments);
     FreeLabelText(ac);
     AtScaleDestroy(ac->axiscore.scale);
     if (ac->axiscore.scale != ac->axiscore.zoom_scale)
	AtScaleDestroy(ac->axiscore.zoom_scale);
}

static Boolean SetValues(old, req, new)
AtAxisCoreWidget old, req, new;
{
#define Changed(fld)      (old->axiscore.fld != new->axiscore.fld)
     Boolean refresh = False;
     Boolean redraw = False;
     Boolean recalc = False;
     Boolean relayout = False;
     int old_w, new_w;
     AtAxisCorePart *ac = &new->axiscore;
     AtPlotterPart *pp = & (((AtPlotterWidget)XtParent(new))->plotter);

     if (new->axiscore.min >= new->axiscore.max) {
	  /*XtAppWarning(XtWidgetToApplicationContext(XtParent((Widget) new)),
		       "Min is > Max in AtAxisCore");*/ 
	  new->axiscore.min = old->axiscore.min;
	  new->axiscore.max = old->axiscore.max;
     }
     /*if (new->axiscore.min = new->axiscore.max) 
       if (new->axiscore.max < (PLOTTER_HUGE_VAL-2)) new->axiscore.max++;
       else new->axiscore.min--;*/

     /*if (Changed(max_dft)) {
       if (!new->axis.max_dft) new->axis.max = new->axiscore.max;
       renum = True;
       }

     if (Changed(min_dft)) {
       if (!new->axis.min_dft) new->axis.min = new->axiscore.min;
       renum = True;
       }*/
     
     /*if (pp->ifzoom==PLOT_NORMAL) {
       ac->max_bak = ac->max;
       ac->min_bak = ac->min;
       }*/
     if (Changed(min) || Changed(max)) {
	  recalc = redraw = True;
	  ac->numbers_changed = True;
          /*if (pp->ifzoom==PLOT_NORMAL) {
	     new->axiscore.max_bak = new->axiscore.max;
             new->axiscore.min_bak = new->axiscore.min;
             }*/
          if (ac->scale) 
	    AtScaleRescale(ac->scale, new->axiscore.min, new->axiscore.max);
	  /*if (ac->zoom_scale) 
	    AtScaleRescale(ac->zoom_scale, new->axiscore.min, new->axiscore.max);*/
     }

     if (Changed(tic_interval)) {
	  recalc = True;
	  ac->numbers_changed = True;
     }

     if (Changed(vertical)) {
	  XtAppWarning(XtWidgetToApplicationContext(XtParent((Widget) new)),
		       "Can't change XtNvertical for an axis");
	  ac->vertical = old->axiscore.vertical;
     }
     if (Changed(mirror)) {
	  XtAppWarning(XtWidgetToApplicationContext(XtParent((Widget) new)),
		       "Can't change XtNmirror for an axis");
	  ac->vertical = old->axiscore.vertical;
     }
     if (Changed(label) || Changed(axis_font)) {
	  /*old_w = (ac->vertical ? AtTextWidth(ac->label_text) :
		   AtTextHeight(ac->label_text));*/
	  old_w = AtTextHeight(ac->label_text);
	  FreeLabelText(new);
	  GetLabelText(new);
	  /*new_w = (ac->vertical ? AtTextWidth(ac->label_text) :
		   AtTextHeight(ac->label_text)); */
	  new_w = AtTextHeight(ac->label_text);
	  redraw = True;
	  if (old_w != new_w) {
	       relayout = True;
	  }
     }
     if (Changed(default_number_width)) {
	  relayout = True;
     }
     if (Changed(label)) {
	  XtFree(old->axiscore.label);
	  old->axiscore.label = NULL;
	  CopyString(ac->label);
	  redraw = True;
     }
     
     if (Changed(draw_grid) || Changed(draw_subgrid)) {
	  redraw = True;
     }
     if (Changed(draw_origin) || Changed(draw_frame)) {
	  redraw = True;
     }

    /* if (Changed(tics_inside) || Changed(tics_outside) ||
	 Changed(subtic_length) || Changed(tic_length) ||
	 Changed(numbers_outside)) {
	  ac->position_changed = True;
	  relayout = True;
	  redraw = True;
     }*/

     if (Changed(subtic_length) || Changed(tic_length)) {
	  relayout = True;
     }

     if (recalc)
	  AtPlotterRecalcThisPlot((AtPlotWidget)new);
     if (relayout) {
	  AtPlotterLayoutRequired((AtPlotWidget)new);
	  CalcAxisWidth(new);
     }
     if (redraw)
	  AtPlotterRedrawRequired((AtPlotWidget)new);
     else if (refresh)
	  AtPlotterRefreshRequired((AtPlotWidget)new);
     return False;
#undef Changed
}

static void CalcAxisWidth(acw)
AtAxisCoreWidget acw;
{
     AtAxisCorePart *ac = &acw->axiscore;
     int tl, mw;
     AtPlotterPart *pp = & (((AtPlotterWidget)XtParent(acw))->plotter);

  if ((acw==pp->y2axis) && !pp->plot_data2) {
     ac->axis_width = 4*MARGIN;/*if y2axis has no data, not display it*/
  } else {
     tl = Max(0, Max(ac->tic_length, ac->subtic_length));
     /*mw = ac->default_number_width > 0 ? ac->default_number_width :
					 ac->max_num_width;*/
     mw = ac->max_num_width;

     /*if (ac->vertical) {
	  ac->axis_width = 4*MARGIN + tl +
	       (ac->label_text ? AtTextWidth(ac->label_text) + MARGIN : 0)
		    + mw + MARGIN;
     } else {
	  ac->axis_width = 4*MARGIN + tl +
	       (ac->label_text ? AtTextHeight(ac->label_text) + MARGIN : 0)
		    + mw + MARGIN; 
     } */
     ac->axis_width = 4*MARGIN + tl +
	       (ac->label_text ? AtTextHeight(ac->label_text) + MARGIN : 0)
		    + mw + MARGIN;
  } 
}


/*
 *   The AtPlot member proc
 */

static void Draw(pw, dpy, drw, region, refresh)
AtPlotWidget pw;
Display *dpy;
Drawable drw;
Region region;
int refresh;
{
     AtAxisCorePart *ac = &(((AtAxisCoreWidget)pw)->axiscore);
     Window win = XtWindow(XtParent((Widget) pw));
     int i, th;
     int baseline;
     AtPlotterPart *pp = & (((AtPlotterWidget)XtParent(pw))->plotter);
     XFontStruct *fs;

     ac->max_bak = ac->max;
     ac->min_bak = ac->min;
     /* Draw axis/origin/frame */
     if (region)
	   XSetRegion(dpy, pp->axis_gc, region);
     XDrawSegments(dpy, drw, pp->axis_gc, &ac->axis_segment, 1);
     if (ac->draw_origin && ac->origin_segment.x1 > 0)
	  XDrawSegments(dpy, drw, pp->axis_gc, &ac->origin_segment, 1);
     if (ac->draw_frame)
	  XDrawSegments(dpy, drw, pp->axis_gc, &ac->frame_segment, 1);
     if (region)
	  XSetClipMask(dpy, pp->axis_gc, None);

     /* Now the tics and subtics */
     if (ac->num_ticsegments) {
	  if (region)
	       XSetRegion(dpy, pp->axis_gc, region);
	  XDrawSegments(dpy, drw, pp->axis_gc, ac->tic_segments,
			ac->num_ticsegments);
	  if (region)
	       XSetClipMask(dpy, pp->axis_gc, None);
     }

     /* Now the label */
     if (ac->label_text) {
	  if (ac->vertical)
	       AtTextDrawJustified(dpy, win, drw,
				   pp->axis_gc, ac->label_text,
				   AtTextJUSTIFY_CENTER,
				   AtTextJUSTIFY_CENTER,
				   ac->label_line, ac->y2,
				   AtTextHeight(ac->label_text),
				   ac->y1 - ac->y2);
	  else
	       AtTextDrawJustified(dpy, win, drw,
				   pp->axis_gc, ac->label_text,
				   AtTextJUSTIFY_CENTER,
				   AtTextJUSTIFY_CENTER,
				   ac->x1, ac->label_line,
				   ac->x2 - ac->x1,
				   AtTextHeight(ac->label_text));
     }

     /* Now the numbers */
     if (ac->draw_numbers) {
	  if (region)
	       XSetRegion(dpy, pp->axis_gc, region);
	  if (ac->vertical) {
	       th = (pp->axis_fs->ascent + pp->axis_fs->descent) / 2 - pp->axis_fs->descent;
	       for (i = 0; i < ac->num_tics; i++) {
	         if (ac->tic_label_string[i]) {
                   baseline = ac->max_num_width - XTextWidth(pp->axis_fs, 
		     ac->tic_label_string[i], ac->tic_label_string[i] ? 
		     strlen(ac->tic_label_string[i]) : 0);
		   XDrawString(dpy, drw, pp->axis_gc, 
		        ac->tic_label_line + baseline, 
			ac->tic_pos[i] + th, ac->tic_label_string[i],
			ac->tic_label_string[i] ? 
			strlen(ac->tic_label_string[i]) : 0);
	          }
                 else {
                   printf("AxisCore:Draw - tic_label_string[%D] NULL\n", i);
                   break;
                   }
                }
	  } else {
	    int j, step;

	    th = ac->tic_label_string[0] ? 
	      XTextWidth(pp->axis_fs, ac->tic_label_string[0],
		strlen(ac->tic_label_string[0])) : 0;
	    if (ac->tic_label_string[0])  
	      XDrawString(dpy, drw, pp->axis_gc, ac->tic_pos[0] - th/2,
		ac->tic_label_line, 
		ac->tic_label_string[0], 
		strlen(ac->tic_label_string[0]));
	    /*step = ac->max_num_height / (ac->tic_pos[1] - ac->tic_pos[0]);
            j = ac->max_num_height % (ac->tic_pos[1] - ac->tic_pos[0]);*/
	    j = ac->tic_pos[1] - ac->tic_pos[0];
	    if (j>0) {
	      step = ac->max_num_height / j;
	      j = ac->max_num_height % j;
	      if (j>0) step++;
	      if (step<=0) step = 1;
	      for (i = step; i < ac->num_tics; i+=step) {
	        if (ac->tic_label_string[i]) {
		  th = XTextWidth(pp->axis_fs, ac->tic_label_string[i],
	            strlen(ac->tic_label_string[i]));
		  XDrawString(dpy, drw, pp->axis_gc, ac->tic_pos[i] - th/2,
		    ac->tic_label_line, 
		    ac->tic_label_string[i], 
		    strlen(ac->tic_label_string[i]));
	          } else break;
               }
       	     }
	  }
         if (region)
	       XSetClipMask(dpy, pp->axis_gc, None);
     }

     /* Now the grid and subgrid */
     if (ac->draw_grid) {
	  if (ac->draw_subgrid && ac->subgrid_segments) {
	       if (region)
		    XSetRegion(dpy, pp->axis_gc, region);
	       XDrawSegments(dpy, drw, pp->axis_gc, ac->subgrid_segments,
			     ac->num_subtics);
	       if (region)
		    XSetClipMask(dpy, pp->axis_gc, None);
	  }
	  if (region)
	       XSetRegion(dpy, pp->axis_gc, region);
	  XDrawSegments(dpy, drw, pp->axis_gc, ac->grid_segments, ac->num_tics);
	  if (region)
	       XSetClipMask(dpy, pp->axis_gc, None);
     }
}

void AxisNewTics P((AtAxisCoreWidget, float, float));
void AxisNewTics(AtAxisCoreWidget pw, float min, float max)
{
  AtAxisCorePart *ac = &(pw->axiscore);
  int pos, i, j, tp, tm, stp, stm, old_num_tics = ac->num_tics;
  XSegment *sp;
  float *tmp_tics;
  Dimension *tmp_tic_pos;


     /* First the axis */
/*     ac->axis_segment.x1 = ac->x1;
     ac->axis_segment.y1 = ac->y1;
     ac->axis_segment.x2 = ac->x2;
     ac->axis_segment.y2 = ac->y2; */

     /* Now the origin */
/*     if (ac->draw_origin) {
	  pos = AtScaleUserToPixel(ac->scale, 0.0);
	  if(ac->vertical) {
	       if (pos > ac->y2 && pos < ac->y1) {
		    ac->origin_segment.x1 = ac->x1;
		    ac->origin_segment.x2 = ac->x1 +
			 (ac->mirror ? -ac->grid_length : ac->grid_length);
		    ac->origin_segment.y1 = ac->origin_segment.y2 = pos;
	       }
	       else
		    ac->origin_segment.x1 = 0;
	  }
	  else {
	       if (pos > ac->x1 && pos < ac->x2) {
		    ac->origin_segment.x1 = ac->origin_segment.x2 = pos;
		    ac->origin_segment.y1 = ac->y1;
		    ac->origin_segment.y2 = ac->y1 -
			 (ac->mirror ? -ac->grid_length : ac->grid_length);
	       }
	       else
		    ac->origin_segment.x1 = 0;
	  }
     }
     else
	  ac->origin_segment.x1 = 0; */

     /* Now the frame */
     if (ac->vertical) {
	  ac->frame_segment.x1 = ac->frame_segment.x2 =
	       ac->x1 + (ac->mirror ? -ac->grid_length : ac->grid_length);
	  ac->frame_segment.y1 = ac->y1;
	  ac->frame_segment.y2 = ac->y2;
     }
     else {
	  ac->frame_segment.x1 = ac->x1;
	  ac->frame_segment.x2 = ac->x2;
	  ac->frame_segment.y1 = ac->frame_segment.y2 =
	       ac->y1 - (ac->mirror ? -ac->grid_length : ac->grid_length);
     }

     /* Now the tics */
     ac->num_ticsegments = ac->num_tics + ac->num_subtics;
     ac->tic_segments = sp =
	  (XSegment *) XtMalloc(sizeof(XSegment) * ac->num_ticsegments);
     ac->tic_pos = (int *) XtMalloc(ac->num_tics * sizeof(int));
     if(ac->num_subtics > 0)
	  ac->subtic_pos = (int *) XtMalloc(ac->num_subtics * sizeof(int));
     /*ac->tic_label_text = (AtText **) XtMalloc(sizeof(AtText *) * ac->num_tics); */

     tp = tm = stp = stm = 0;
    /* if (ac->tics_inside && !ac->mirror || ac->tics_outside && ac->mirror) {
	  tp = ac->tic_length;
	  stp = ac->subtic_length;
     }
     if (ac->tics_inside && ac->mirror || ac->tics_outside && !ac->mirror) {
	  tm = ac->tic_length;
	  stm = ac->subtic_length;
     }*/
     if (ac->mirror) {
       tp = ac->tic_length;
       stp = ac->subtic_length;
       }
     else {
       tm = ac->tic_length;
       stm = ac->subtic_length;
       }
     for (i = 0; i < ac->num_tics; i++, sp++) {
	  pos = AtScaleUserToPixel(ac->scale, ac->tic_values[i]);
	  ac->tic_pos[i] = pos;
	  assert((sp - ac->tic_segments) < ac->num_ticsegments);
	  if (ac->vertical) {
	       sp->y1 = sp->y2 = pos;
	       sp->x1 = ac->x1 - tm;
	       sp->x2 = ac->x1 + tp;
	  } else {
	       sp->x1 = sp->x2 = pos;
	       /* remember +ve pixel is downward */
	       sp->y1 = ac->y1 + tm;
	       sp->y2 = ac->y1 - tp;
	  }
     }
     /* Now the subtics */
     for (i = 0; i < ac->num_subtics; i++, sp++) {
	  pos = AtScaleUserToPixel(ac->scale, ac->subtic_values[i]);
	  ac->subtic_pos[i] = pos;
	  assert((sp - ac->tic_segments) < ac->num_ticsegments);
	  if (ac->vertical) {
	       sp->y1 = sp->y2 = pos;
	       sp->x1 = ac->x1 - stm;
	       sp->x2 = ac->x1 + stp;
	  } else {
	       sp->x1 = sp->x2 = pos;
	       /* remember +ve pixel is downward */
	       sp->y1 = ac->y1 + stm;
	       sp->y2 = ac->y1 - stp;
	  }
     }

     /* Now define the line against which tic labels are displayed */
     /* tp = ac->tics_outside && ac->numbers_outside ||
	  ac->tics_inside && !ac->numbers_outside ?
	       Max(0, Max(ac->tic_length, ac->subtic_length)) : 0;*/
     tp = Max(0, Max(ac->tic_length, ac->subtic_length));
     if (ac->vertical) {
	  ac->tic_label_line =
	       (ac->mirror) ?
		    ac->x2 + tp + MARGIN :
			 ac->x1 - tp - MARGIN - ac->max_num_width;
     } else {
	  /* Remember, towards bottom is DECREASING pixel address!! */
	  ac->tic_label_line =
	       (ac->mirror) ?
		    ac->y2 - MARGIN - tp :
			 ac->y1 + MARGIN + tp + ac->max_num_width;
     }

     /*
      * Now the grid segments
      */
     ac->grid_segments = sp =
	  (XSegment *) XtMalloc(sizeof(XSegment) * ac->num_tics);
     if (ac->vertical) {
	  for (i = 0; i < ac->num_tics; i++, sp++) {
	       sp->y1 = sp->y2 = ac->tic_pos[i];
	       sp->x1 = ac->x1;
	       sp->x2 = ac->x1 +
		    (ac->mirror ? -ac->grid_length : ac->grid_length);
	  }

     } else {
	  for (i = 0; i < ac->num_tics; i++, sp++) {
	       sp->x1 = sp->x2 = ac->tic_pos[i];
	       sp->y1 = ac->y1;
	       sp->y2 = ac->y1 -
		    (ac->mirror ? -ac->grid_length : ac->grid_length);
	  }
     }

     /*
      * Now the subgrid segments
      */
     if(ac->num_subtics > 0) {
	  ac->subgrid_segments = sp =
	       (XSegment *) XtMalloc(sizeof(XSegment) * ac->num_subtics);
	  if (ac->vertical) {
	       for (i = j = 0; i < ac->num_subtics && j < ac->num_tics; i++, sp++) {
		    sp->y1 = sp->y2 = ac->subtic_pos[i];
		    sp->x1 = ac->x1;
		    if (SubticOnTic(ac, i, j)) {
			 sp->x2 = ac->x1;
			 ++j;
		    }
		    else
			 sp->x2 = ac->x1 +
			      (ac->mirror ? -ac->grid_length : ac->grid_length);
	       }
	  } else {
	       for (i = j = 0; i < ac->num_subtics && j < ac->num_tics; i++, sp++) {
		    sp->x1 = sp->x2 = ac->subtic_pos[i];
		    sp->y1 = ac->y1;
		    if (SubticOnTic(ac, i, j)) {
			 sp->y2 = ac->y1;
			 ++j;
		    }
		    else
			 sp->y2 = ac->y1 -
			      (ac->mirror ? -ac->grid_length : ac->grid_length);
	       }
	  }
     }
     else
	  ac->subgrid_segments = NULL;

     /*
      * Now convert the tic labels to AtText format
      */
     /*ReformatLabels((AtAxisCoreWidget) pw, False);*/

     /*
      * Now, layout the label etc
      */
     if (ac->label_text) {
	  tp = Max(0, Max(ac->tic_length, ac->subtic_length));
	  tp += MARGIN +
	       (ac->draw_numbers ?
		ac->max_num_width + MARGIN*3 : 0);
	  if (ac->vertical) {
	       ac->label_line = ac->mirror ? ac->x1 + tp :
		    ac->x1 - tp - AtTextHeight(ac->label_text);
	  } else {
	       /* +y is towards bottom! */
	       ac->label_line = ac->mirror ? ac->y1 - tp :
		    ac->y1 + tp;
		    /*ac->y1 + tp + AtTextHeight(ac->label_text);*/
	  }
     }

     ac->numbers_changed = ac->position_changed = False;

  ac->max = max;
  ac->min = min;

}


void PlotScaleMotionRecalcAxis(AtAxisCoreWidget pw, int x1, int x2, int y1, int y2)
{
     AtAxisCorePart *ac = &(pw->axiscore);
     int pos, i, j, tp, tm, stp, stm, old_num_tics = ac->num_tics;
     XSegment *sp;

     ac->x1 = x1;
     ac->x2 = x2;
     ac->y1 = y1;
     ac->y2 = y2;
     ac->grid_length = ac->vertical ? ac->y2 - ac->y1 : ac->x2 - ac->x1;
     if (ac->vertical) {
          AtScaleResize(ac->scale, y1, y2);
          AtScaleResize(ac->zoom_scale, y1, y2);
     } else {
          AtScaleResize(ac->scale, x1, x2);
          AtScaleResize(ac->zoom_scale, x1, x2);
     }

     /* First the axis */
     ac->axis_segment.x1 = ac->x1;
     ac->axis_segment.y1 = ac->y1;
     ac->axis_segment.x2 = ac->x2;
     ac->axis_segment.y2 = ac->y2;

     /* Now the origin */
     if (ac->draw_origin) {
	  pos = AtScaleUserToPixel(ac->scale, 0.0);
	  if(ac->vertical) {
	       if (pos > ac->y2 && pos < ac->y1) {
		    ac->origin_segment.x1 = ac->x1;
		    ac->origin_segment.x2 = ac->x1 +
			 (ac->mirror ? -ac->grid_length : ac->grid_length);
		    ac->origin_segment.y1 = ac->origin_segment.y2 = pos;
	       }
	       else
		    ac->origin_segment.x1 = 0;
	  }
	  else {
	       if (pos > ac->x1 && pos < ac->x2) {
		    ac->origin_segment.x1 = ac->origin_segment.x2 = pos;
		    ac->origin_segment.y1 = ac->y1;
		    ac->origin_segment.y2 = ac->y1 -
			 (ac->mirror ? -ac->grid_length : ac->grid_length);
	       }
	       else
		    ac->origin_segment.x1 = 0;
	  }
     }
     else
	  ac->origin_segment.x1 = 0;

     /* Now the frame */
     if (ac->vertical) {
	  ac->frame_segment.x1 = ac->frame_segment.x2 =
	       ac->x1 + (ac->mirror ? -ac->grid_length : ac->grid_length);
	  ac->frame_segment.y1 = ac->y1;
	  ac->frame_segment.y2 = ac->y2;
     }
     else {
	  ac->frame_segment.x1 = ac->x1;
	  ac->frame_segment.x2 = ac->x2;
	  ac->frame_segment.y1 = ac->frame_segment.y2 =
	       ac->y1 - (ac->mirror ? -ac->grid_length : ac->grid_length);
     }

     /* Now the tics */
     sp = ac->tic_segments;
     /*ac->num_ticsegments = ac->num_tics + ac->num_subtics;
     ac->tic_segments = sp =
	  (XSegment *) XtMalloc(sizeof(XSegment) * ac->num_ticsegments);
     ac->tic_pos = (Dimension *) XtMalloc(ac->num_tics * sizeof(Dimension));
     if(ac->num_subtics > 0)
	  ac->subtic_pos = (Dimension *) XtMalloc(ac->num_subtics * sizeof(Dimension)); */
     /*ac->tic_label_text = (AtText **) XtMalloc(sizeof(AtText *) * ac->num_tics); */

     tp = tm = stp = stm = 0;
     if (ac->mirror) {
       tp = ac->tic_length;
       stp = ac->subtic_length;
       }
     else {
       tm = ac->tic_length;
       stm = ac->subtic_length;
       }
     for (i = 0; i < ac->num_tics; i++, sp++) {
	  pos = AtScaleUserToPixel(ac->scale, ac->tic_values[i]);
	  ac->tic_pos[i] = pos;
	  assert((sp - ac->tic_segments) < ac->num_ticsegments);
	  if (ac->vertical) {
	       sp->y1 = sp->y2 = pos;
	       sp->x1 = ac->x1 - tm;
	       sp->x2 = ac->x1 + tp;
	  } else {
	       sp->x1 = sp->x2 = pos;
	       /* remember +ve pixel is downward */
	       sp->y1 = ac->y1 + tm;
	       sp->y2 = ac->y1 - tp;
	  }
     }
     /* Now the subtics */
     for (i = 0; i < ac->num_subtics; i++, sp++) {
	  pos = AtScaleUserToPixel(ac->scale, ac->subtic_values[i]);
	  ac->subtic_pos[i] = pos;
	  assert((sp - ac->tic_segments) < ac->num_ticsegments);
	  if (ac->vertical) {
	       sp->y1 = sp->y2 = pos;
	       sp->x1 = ac->x1 - stm;
	       sp->x2 = ac->x1 + stp;
	  } else {
	       sp->x1 = sp->x2 = pos;
	       /* remember +ve pixel is downward */
	       sp->y1 = ac->y1 + stm;
	       sp->y2 = ac->y1 - stp;
	  }
     }

     /* Now define the line against which tic labels are displayed */
     tp = Max(0, Max(ac->tic_length, ac->subtic_length));
     if (ac->vertical) {
	  ac->tic_label_line =
	       (ac->mirror) ?
		    ac->x2 + tp + MARGIN :
			 ac->x1 - tp - MARGIN - ac->max_num_width;
     } else {
	  /* Remember, towards bottom is DECREASING pixel address!! */
	  ac->tic_label_line =
	       (ac->mirror) ?
		    ac->y2 - MARGIN - tp :
			 ac->y1 + MARGIN + tp + ac->max_num_width;
     }

     /*
      * Now the grid segments
      */
     if (!ac->grid_segments) 
       ac->grid_segments = 
	  (XSegment *) XtMalloc(sizeof(XSegment) * ac->num_tics);
     sp = ac->grid_segments;
     if (ac->vertical) {
	  for (i = 0; i < ac->num_tics; i++, sp++) {
	       sp->y1 = sp->y2 = ac->tic_pos[i];
	       sp->x1 = ac->x1;
	       sp->x2 = ac->x1 +
		    (ac->mirror ? -ac->grid_length : ac->grid_length);
	  }

     } else {
	  for (i = 0; i < ac->num_tics; i++, sp++) {
	       sp->x1 = sp->x2 = ac->tic_pos[i];
	       sp->y1 = ac->y1;
	       sp->y2 = ac->y1 -
		    (ac->mirror ? -ac->grid_length : ac->grid_length);
	  }
     }

     /*
      * Now the subgrid segments
      */
     if(ac->num_subtics > 0) {
          if (!ac->subgrid_segments) 
             ac->subgrid_segments = 
               (XSegment *) XtMalloc(sizeof(XSegment) * ac->num_subtics);
          sp = ac->subgrid_segments;
	  if (ac->vertical) {
	       for (i = j = 0; i < ac->num_subtics && j < ac->num_tics; i++, sp++) {
		    sp->y1 = sp->y2 = ac->subtic_pos[i];
		    sp->x1 = ac->x1;
		    if (SubticOnTic(ac, i, j)) {
			 sp->x2 = ac->x1;
			 ++j;
		    }
		    else
			 sp->x2 = ac->x1 +
			      (ac->mirror ? -ac->grid_length : ac->grid_length);
	       }
	  } else {
	       for (i = j = 0; i < ac->num_subtics && j < ac->num_tics; i++, sp++) {
		    sp->x1 = sp->x2 = ac->subtic_pos[i];
		    sp->y1 = ac->y1;
		    if (SubticOnTic(ac, i, j)) {
			 sp->y2 = ac->y1;
			 ++j;
		    }
		    else
			 sp->y2 = ac->y1 -
			      (ac->mirror ? -ac->grid_length : ac->grid_length);
	       }
	  }
     }
     else
	  ac->subgrid_segments = NULL;

     /*
      * Now, layout the label etc
      */
     if (ac->label_text) {
	  tp = Max(0, Max(ac->tic_length, ac->subtic_length));
	  tp += MARGIN +
	       (ac->draw_numbers ?
		ac->max_num_width + MARGIN*3 : 0);
	  if (ac->vertical) {
	       ac->label_line = ac->mirror ? ac->x1 + tp :
		    ac->x1 - tp - AtTextHeight(ac->label_text);
	  } else {
	       /* +y is towards bottom! */
	       ac->label_line = ac->mirror ? ac->y1 - tp :
		    ac->y1 + tp;
		    /*ac->y1 + tp + AtTextHeight(ac->label_text);*/
	  }
     }

     ac->position_changed = False;
}


/*
 *   The recalc routine.
 *
 *   The recalculation happens in two parts.
 *
 *   This routine is the plot.recalc routine and is called by the
 *   Plotter parent when recalc is desired.  It calls the subclass
 *   recalc routine stored in axiscore.recalc_proc to set up the tics
 *   and subtics array, then sets up the pixel values here.  Subclasses
 *   should inherit this routine (from the AtPlot class) unless special
 *   pixel calculation is required.
 */

static void Recalc(pw, xs, ys, from, to)
AtPlotWidget pw;
AtScale *xs, *ys;
int from, to;
{
     AtAxisCalcProc fn;
     AtAxisCorePart *ac = &((AtAxisCoreWidget)pw)->axiscore;
     int pos, i, j, tp, tm, stp, stm, old_num_tics = ac->num_tics;
     XSegment *sp;

     if (ac->numbers_changed) {
	  XtFree((char *) ac->tic_values);
	  XtFree((char *) ac->subtic_values);
	  /*if (ac->tic_label_dft) {*/
	    for (i = 0; ac->tic_label_string && i < ac->num_tics; i++){
	       XtFree(ac->tic_label_string[i]);
	       ac->tic_label_string[i] = NULL;
	       }
	    XtFree((char *) ac->tic_label_string);
           /* ac->num_tics = 0;
	    }*/

	  ac->tic_label_string = NULL;
	  ac->tic_values = NULL;
	  ac->subtic_values = NULL;
	  ac->num_tics = ac->num_subtics = 0;

	  /*
	   * Call the subclass calc function
	   */
	  fn = ((AtAxisCoreWidgetClass)
		XtClass((Widget) pw))->axiscore_class.calc_proc;
	  if (fn) {
	       fn((AtAxisCoreWidget) pw);
	  }
     }

     if ( !(ac->numbers_changed || ac->position_changed)) {
#ifdef DEBUG
	  * Hmm. how come we got called if there is nothing to change?? *
	  XtAppWarning(XtWidgetToApplicationContext(XtParent((Widget) pw)),
		       "AtAxis Recalc called without a recalc pending?");
#endif
	  return;
     }

     /*
      * Now convert the tic labels to AtText format
      */
     ReformatLabels((AtAxisCoreWidget) pw, False);

     /*
      * Make the pixel arrays from the stores position arrays
      *
      * First, the segments
      */
     if (ac->num_tics > 0) {  
       XtFree((char *) ac->grid_segments);
       XtFree((char *) ac->tic_pos);
       }
     if ((ac->num_tics + ac->num_subtics) > 0)
       XtFree((char *) ac->tic_segments);
     if(ac->num_subtics > 0) {
       XtFree((char *) ac->subgrid_segments);
       XtFree((char *) ac->subtic_pos);
       }
     ac->tic_segments = ac->grid_segments = ac->subgrid_segments = NULL;
     ac->tic_pos = ac->subtic_pos = NULL;

     /* First the axis */
     ac->axis_segment.x1 = ac->x1;
     ac->axis_segment.y1 = ac->y1;
     ac->axis_segment.x2 = ac->x2;
     ac->axis_segment.y2 = ac->y2;

     /* Now the origin */
     if (ac->draw_origin) {
	  pos = AtScaleUserToPixel(ac->scale, 0.0);
	  if(ac->vertical) {
	       if (pos > ac->y2 && pos < ac->y1) {
		    ac->origin_segment.x1 = ac->x1;
		    ac->origin_segment.x2 = ac->x1 +
			 (ac->mirror ? -ac->grid_length : ac->grid_length);
		    ac->origin_segment.y1 = ac->origin_segment.y2 = pos;
	       }
	       else
		    ac->origin_segment.x1 = 0;
	  }
	  else {
	       if (pos > ac->x1 && pos < ac->x2) {
		    ac->origin_segment.x1 = ac->origin_segment.x2 = pos;
		    ac->origin_segment.y1 = ac->y1;
		    ac->origin_segment.y2 = ac->y1 -
			 (ac->mirror ? -ac->grid_length : ac->grid_length);
	       }
	       else
		    ac->origin_segment.x1 = 0;
	  }
     }
     else
	  ac->origin_segment.x1 = 0;

     /* Now the frame */
     if (ac->vertical) {
	  ac->frame_segment.x1 = ac->frame_segment.x2 =
	       ac->x1 + (ac->mirror ? -ac->grid_length : ac->grid_length);
	  ac->frame_segment.y1 = ac->y1;
	  ac->frame_segment.y2 = ac->y2;
     }
     else {
	  ac->frame_segment.x1 = ac->x1;
	  ac->frame_segment.x2 = ac->x2;
	  ac->frame_segment.y1 = ac->frame_segment.y2 =
	       ac->y1 - (ac->mirror ? -ac->grid_length : ac->grid_length);
     }
     /* Now the tics */
     ac->num_ticsegments = ac->num_tics + ac->num_subtics;
     if (ac->num_ticsegments > 0) 
       ac->tic_segments = sp =
	  (XSegment *) XtMalloc(sizeof(XSegment) * ac->num_ticsegments);
     else ac->tic_segments = NULL;
     if (ac->num_tics) 
       ac->tic_pos = (int *) XtMalloc(ac->num_tics * sizeof(int));
     else ac->tic_pos = NULL;
     if(ac->num_subtics > 0)
	  ac->subtic_pos = (int *) XtMalloc(ac->num_subtics * sizeof(int));
     else ac->subtic_pos = NULL;
     /*ac->tic_label_text = (AtText **) XtMalloc(sizeof(AtText *) * ac->num_tics); */

     tp = tm = stp = stm = 0;
    /* if (ac->tics_inside && !ac->mirror || ac->tics_outside && ac->mirror) {
	  tp = ac->tic_length;
	  stp = ac->subtic_length;
     }
     if (ac->tics_inside && ac->mirror || ac->tics_outside && !ac->mirror) {
	  tm = ac->tic_length;
	  stm = ac->subtic_length;
     }*/
     if (ac->mirror) {
       tp = ac->tic_length;
       stp = ac->subtic_length;
       }
     else {
       tm = ac->tic_length;
       stm = ac->subtic_length;
       }
     for (i = 0; i < ac->num_tics; i++, sp++) {
	  pos = AtScaleUserToPixel(ac->scale, ac->tic_values[i]);
	  ac->tic_pos[i] = pos;
	  assert((sp - ac->tic_segments) < ac->num_ticsegments);
	  if (ac->vertical) {
	       sp->y1 = sp->y2 = pos;
	       sp->x1 = ac->x1 - tm;
	       sp->x2 = ac->x1 + tp;
	  } else {
	       sp->x1 = sp->x2 = pos;
	       /* remember +ve pixel is downward */
	       sp->y1 = ac->y1 + tm;
	       sp->y2 = ac->y1 - tp;
	  }
     }
     /* Now the subtics */
     for (i = 0; i < ac->num_subtics; i++, sp++) {
	  pos = AtScaleUserToPixel(ac->scale, ac->subtic_values[i]);
	  ac->subtic_pos[i] = pos;
	  assert((sp - ac->tic_segments) < ac->num_ticsegments);
	  if (ac->vertical) {
	       sp->y1 = sp->y2 = pos;
	       sp->x1 = ac->x1 - stm;
	       sp->x2 = ac->x1 + stp;
	  } else {
	       sp->x1 = sp->x2 = pos;
	       /* remember +ve pixel is downward */
	       sp->y1 = ac->y1 + stm;
	       sp->y2 = ac->y1 - stp;
	  }
     }

     /* Now define the line against which tic labels are displayed */
     /* tp = ac->tics_outside && ac->numbers_outside ||
	  ac->tics_inside && !ac->numbers_outside ?
	       Max(0, Max(ac->tic_length, ac->subtic_length)) : 0;*/
     tp = Max(0, Max(ac->tic_length, ac->subtic_length));
     if (ac->vertical) {
	  ac->tic_label_line =
	       (ac->mirror) ?
		    ac->x2 + tp + MARGIN :
			 ac->x1 - tp - MARGIN - ac->max_num_width;
     } else {
	  /* Remember, towards bottom is DECREASING pixel address!! */
	  ac->tic_label_line =
	       (ac->mirror) ?
		    ac->y2 - MARGIN - tp :
			 ac->y1 + MARGIN + tp + ac->max_num_width;
     }

     /*
      * Now the grid segments
      */
     ac->grid_segments = sp =
	  (XSegment *) XtMalloc(sizeof(XSegment) * ac->num_tics);
     if (ac->vertical) {
	  for (i = 0; i < ac->num_tics; i++, sp++) {
	       sp->y1 = sp->y2 = ac->tic_pos[i];
	       sp->x1 = ac->x1;
	       sp->x2 = ac->x1 +
		    (ac->mirror ? -ac->grid_length : ac->grid_length);
	  }

     } else {
	  for (i = 0; i < ac->num_tics; i++, sp++) {
	       sp->x1 = sp->x2 = ac->tic_pos[i];
	       sp->y1 = ac->y1;
	       sp->y2 = ac->y1 -
		    (ac->mirror ? -ac->grid_length : ac->grid_length);
	  }
     }

     /*
      * Now the subgrid segments
      */
     if(ac->num_subtics > 0) {
	  ac->subgrid_segments = sp =
	       (XSegment *) XtMalloc(sizeof(XSegment) * ac->num_subtics);
	  if (ac->vertical) {
	       for (i = j = 0; i < ac->num_subtics && j < ac->num_tics; i++, sp++) {
		    sp->y1 = sp->y2 = ac->subtic_pos[i];
		    sp->x1 = ac->x1;
		    if (SubticOnTic(ac, i, j)) {
			 sp->x2 = ac->x1;
			 ++j;
		    }
		    else
			 sp->x2 = ac->x1 +
			      (ac->mirror ? -ac->grid_length : ac->grid_length);
	       }
	  } else {
	       for (i = j = 0; i < ac->num_subtics && j < ac->num_tics; i++, sp++) {
		    sp->x1 = sp->x2 = ac->subtic_pos[i];
		    sp->y1 = ac->y1;
		    if (SubticOnTic(ac, i, j)) {
			 sp->y2 = ac->y1;
			 ++j;
		    }
		    else
			 sp->y2 = ac->y1 -
			      (ac->mirror ? -ac->grid_length : ac->grid_length);
	       }
	  }
     }
     else
	  ac->subgrid_segments = NULL;

     /*
      * Now convert the tic labels to AtText format
      */
     /*ReformatLabels((AtAxisCoreWidget) pw, False);*/

     /*
      * Now, layout the label etc
      */
     if (ac->label_text) {
	  tp = Max(0, Max(ac->tic_length, ac->subtic_length));
	  tp += MARGIN +
	       (ac->draw_numbers ?
		ac->max_num_width + MARGIN*3 : 0);
	  if (ac->vertical) {
	       ac->label_line = ac->mirror ? ac->x1 + tp :
		    ac->x1 - tp - AtTextHeight(ac->label_text);
	  } else {
	       /* +y is towards bottom! */
	       ac->label_line = ac->mirror ? ac->y1 - tp :
		    ac->y1 + tp;
		    /*ac->y1 + tp + AtTextHeight(ac->label_text);*/
	  }
     }

     ac->numbers_changed = ac->position_changed = False;
} /*end of Recalc*/


static void ReformatLabels(acw, free_them)
AtAxisCoreWidget acw;
int free_them;
{
     AtAxisCorePart *ac = &acw->axiscore;
     int wid=0, maxwid=0, i=0;
     AtPlotterPart *pp=&(((AtPlotterWidget)XtParent((Widget)acw))->plotter);
     int oldw = ac->max_num_width, oldh = ac->max_num_height;
     char lbl[60];

  if (pp->axis_fs) {
     if (ac->vertical) {
       ac->max_num_height = pp->axis_fs->ascent + pp->axis_fs->descent;
       /*sprintf(lbl, "%.0g", *maxp);*/
       maxwid = wid = 0;
       for (i = 0; i < ac->num_tics; i++) 
	   if (ac->tic_label_string[i]) {
             wid = XTextWidth(pp->axis_fs, ac->tic_label_string[i],
		       strlen(ac->tic_label_string[i]));
             maxwid = Max(maxwid, wid);
             }
       ac->max_num_width = maxwid;
       }
     else {
       ac->max_num_width = pp->axis_fs->ascent + pp->axis_fs->descent;
       if (ac->anno_method == PLOT_ANNO_TIME_LABELS){
         ac->max_num_height = ac->tic_label_string[0] ? 
	   XTextWidth(pp->axis_fs, ac->tic_label_string[0],
             strlen(ac->tic_label_string[0])) : 0;
         }
       else {
         maxwid = wid = 0;
         for (i = 0; i < ac->num_tics; i++) 
	   if (ac->tic_label_string[i]) {
	      wid = ac->tic_label_string[i] ?
		XTextWidth(pp->axis_fs, ac->tic_label_string[i],
		       strlen(ac->tic_label_string[i])) : 0;
             maxwid = Max(maxwid, wid);
             }
         ac->max_num_height = maxwid;
         }
       }
   } else { /* no plotter axis font structure */
     printf("No font structure for axis\n");
     if (ac->vertical) {
        ac->max_num_width = 8 * 10;
        ac->max_num_height = 20;
        }
      else {
        ac->max_num_width = 20;
        ac->max_num_height = 8 * 10;
        }
   }

     if (ac->draw_numbers && (oldw != ac->max_num_width || 
         oldh != ac->max_num_height)) {
	  /*
	   * The width of the numbers has changed, so request a rescale
	   * (so the max_num_width can be calculated). Its a pity,
	   * relayout is pretty much all that changes.
	   */
	  ac->numbers_changed = True;
	  AtPlotterRescaleRequired((AtPlotWidget) acw);
     }
}

/*
 *   The AxisCore member functions
 *
 *   The default range proc just accepts the answers, and calculates the
 *   number_width based on the stored actual_number_width or (for
 *   startup) by formatting max as a guess, and makes tic_interval
 *   equal to the range.
 */

static void RangeProc(w, minp, maxp, tip, nwp)
AtAxisCoreWidget w;
float *minp, *maxp, *tip;
int *nwp;
{
     AtAxisCorePart *ac = &w->axiscore;
     AtText *tmp;
     char lbl[40];
     AtPlotterPart *pp=&(((AtPlotterWidget)XtParent((Widget)w))->plotter);
     int width;

  if (ac->max_num_width > 0) *nwp = ac->max_num_width;
  else {
    sprintf(lbl, "%.0g", *maxp);
    if (pp->axis_fs)
       width = ac->vertical ? XTextWidth(pp->axis_fs, lbl, strlen(lbl)):
	     pp->axis_fs->ascent + pp->axis_fs->descent;
    else width = ac->vertical ? strlen(lbl) * 8 : 16; 
    *nwp = width;
    }
  *tip = *maxp - *minp;
}

/*
 *   The default Calc proc just has two tics (at min and max) and three subtics.
 */
static void CalcProc(w)
AtAxisCoreWidget w;
{
  AtAxisCorePart *ac = &w->axiscore;
  char lbl[20];

  ac->tic_values = (float *) XtMalloc(sizeof(float) * 2);
  ac->tic_values[0] = ac->min;
  ac->tic_values[1] = ac->max;

  if ((ac->vertical) || ac->anno_method!=PLOT_ANNO_TIME_LABELS) {
    ac->tic_label_string = (String *) XtMalloc(sizeof (String) * 2);
    ac->num_tics = 2;
    sprintf(lbl, "%.0f", ac->min);
    ac->tic_label_string[0] = XtNewString(lbl);
    sprintf(lbl, "%.0f", ac->max);
    ac->tic_label_string[1] = XtNewString(lbl);
    }
  else {
    char str[TMSTR_MAXLEN];
    struct tm *xtm;
    AtPlotterPart *pp=&(((AtPlotterWidget)XtParent((Widget)w))->plotter);

    xtm = (struct tm *)gmtime(&(pp->time_base));
    strftime(str, TMSTR_MAXLEN, pp->time_format, xtm);
    ac->tic_label_string = (String *) XtMalloc(sizeof (String) * 1);
    ac->num_tics = 1;
    ac->tic_label_string[0] = XtNewString(str);
    }

  ac->num_subtics = 3;
  ac->subtic_values = (float *) XtMalloc(sizeof(float) * 3);
  ac->subtic_values[0] = (ac->max + ac->min) / 2;
  ac->subtic_values[1] = (ac->max + ac->subtic_values[0]) / 2;
  ac->subtic_values[2] = (ac->min + ac->subtic_values[0]) / 2;
}

/*
 *
 *   The wrappers for the member functions that get called by the parent
 */

void AtAxisAskRange(acw, minp, maxp)
AtAxisCoreWidget acw;
float *minp, *maxp;
{
     AtAxisRangeProc fn;
     AtAxisRangeArgs ra;
     AtAxisCorePart *ac = &acw->axiscore;
     AtPlotterPart *pp = & (((AtPlotterWidget)XtParent(acw))->plotter);
     float new_ti, *tip = &new_ti;
     int old_width = ac->axis_width;
     int new_num_width = ac->max_num_width; /*ac->actual_num_width;*/
     int *widp = &new_num_width;
     Boolean rescale = False;

     /*if (!ac->max_dft && !ac->min_dft)
       if ((ac->max==ac->max_bak)&&(ac->min==ac->min_bak)) {
	 return;
         }*/

     ra.minp = minp;
     ra.maxp = maxp;
     ra.max_widthp = widp;
     ra.tic_intervalp = tip;

     /*
      * First, ask the class method to suggest
      * min/max/tic interval/max label width
      */
     if(fn = ((AtAxisCoreWidgetClass) XtClass((Widget) acw))->axiscore_class.range_proc)
	  fn(acw, minp, maxp, tip, widp);

     /*
      * Then call the callbacks so they can suggest same
      */
     XtCallCallbackList((Widget) acw, ac->range_callback, (XtPointer) &ra);

     /*
      * Decide if we need to recalculate our tics and subtics,
      * depending on whether or not min/max/ticinterval changed
      */
     if (ac->min != *minp || ac->max != *maxp) {
#ifdef TRACE
	  fprintf(stderr, "Axis (label %s) changed endpoints to %.1f,%.1f\n",
		  ac->label, *minp, *maxp);
#endif
       /*if (pp->ifzoom==PLOT_NORMAL) {
	  ac->max_bak = *maxp;
          ac->min_bak = *minp;
        }*/
	  AtScaleRescale(ac->scale, *minp, *maxp);
	 /* AtScaleRescale(ac->zoom_scale, *minp, *maxp);*/
	  rescale = True;
     }
     if (*minp != ac->scale->low || *maxp != ac->scale->high) {
        /*if (pp->ifzoom==PLOT_NORMAL) {
	  ac->max_bak = *maxp;
          ac->min_bak = *minp;
          }*/
	  AtScaleRescale(ac->scale, *minp, *maxp);
	  /*AtScaleRescale(ac->zoom_scale, *minp, *maxp);*/
	  rescale = True;
     }
     if (rescale || ac->tic_interval != *tip) {
	  ac->numbers_changed = True;
	  AtPlotterRecalcThisPlot((AtPlotWidget)acw);
     }

     /* Accept the values for min, max and ticinterval */
     ac->min = *minp;
     ac->max = *maxp;
     ac->tic_interval = *tip;
     ac->max_num_width = *widp;
     /*
      * Now calculate the actual axis width based on the max number
      * width and things like labels etc.
      */
     CalcAxisWidth(acw);
     if (ac->axis_width != old_width) {
#ifdef TRACE
	  fprintf(stderr,
		  "AtAxisRange (label %s) changed max_width from %d to %d\n",
		  ac->label, old_width, ac->axis_width);
#endif
	  ac->position_changed = True;
	  AtPlotterLayoutRequired((AtPlotWidget)acw);
     }
} /*end of AtAxisAskRange*/

void Plot2AxisAskRange(AtAxisCoreWidget acw, AtPlotterBoundingBox obb, AtPlotterBoundingBox *bbp)
{
     AtAxisCorePart *ac = &acw->axiscore;
     AtPlotterPart *pp = & (((AtPlotterWidget)XtParent(acw))->plotter);
     float min, max; 
     Boolean rescale = False;

   if (ac->vertical) {
       return;
       /*min = bbp->y2min = obb.y2min+(obb.y2max-obb.y2min)*(bbp->ymin-obb.ymin)/(obb.ymax-obb.ymin);
       max = bbp->y2max = obb.y2min+(obb.y2max-obb.y2min)*(bbp->ymax-obb.ymin)/(obb.ymax-obb.ymin);
       */
       }
   else {
       min = bbp->x2min = obb.x2min + (obb.x2max-obb.x2min)*(bbp->xmin-obb.xmin)/(obb.xmax-obb.xmin);
       max = bbp->x2max = obb.x2min+(obb.x2max-obb.x2min)*(bbp->xmax-obb.xmin)/(obb.xmax-obb.xmin);

     if (ac->min!=min || ac->max!=max || min!=ac->scale->low || max!=ac->scale->high) {
       /*if (pp->ifzoom==PLOT_NORMAL) {
	  ac->max_bak = max;
          ac->min_bak = min;
        }*/
	  AtScaleRescale(ac->scale, min, max);
	 /* AtScaleRescale(ac->zoom_scale, *minp, *maxp);*/
	  rescale = True;
     }

     ac->min = min;
     ac->max = max;
   }     
} /*end of Plot2AxisAskRange */

/*
 *   This routine is called by the parent to set the location of the axis.
 *
 */

Boolean AtAxisSetPosition(acw, x1, y1, x2, y2, grid_length)
AtAxisCoreWidget acw;
int x1, y1, x2, y2, grid_length;
{
     AtAxisCorePart *ac = &acw->axiscore;
     int old_len, len;
     Boolean len_changed = False;

     XtCheckSubclass((Widget) acw, atAxisCoreWidgetClass,
		     "AtAxisSetPosition needs an AtAxisCoreWidget");

#define dif(var)     (ac->var != var)
     if (dif(x1) || dif(y1) || dif(x2) || dif(y2) || dif(grid_length)) {
	  AtPlotterRecalcThisPlot((AtPlotWidget)acw);
	  ac->position_changed = True;
#ifdef TRACE
	  fprintf(stderr, "Axis (label %s) changed position\n",
		  ac->label);
#endif
     }
#undef dif

     if (ac->vertical) {
	  old_len = ac->y1 - ac->y2;
	  len = y1 - y2;
     } else {
	  old_len = ac->x2 - ac->x1;
	  len = x2 - x1;
     }
     if (old_len < (len - (len >> 2)) || old_len > (len + (len >> 2))) {
	  /* Has changed length by 25%, so need to rethink tic_interval! */
	  len_changed = ac->numbers_changed = True;
#ifdef TRACE
	  fprintf(stderr, "Axis (label %s) changed in length\n",
		  ac->label);
#endif
     }

     ac->x1 = x1;
     ac->x2 = x2;
     ac->y1 = y1;
     ac->y2 = y2;
     ac->grid_length = grid_length;

     if (ac->vertical && x1 != x2) {
	  XtAppError(XtWidgetToApplicationContext((Widget) acw),
		     "Vertical axis given non-vertical position");
     }
     if (!ac->vertical && y1 != y2) {
	  XtAppError(XtWidgetToApplicationContext((Widget) acw),
		     "Horizontal axis given non-horizontal position");
     }

     /* Now change the scale */
     if (ac->vertical) {
	  AtScaleResize(ac->scale, y1, y2);
	  AtScaleResize(ac->zoom_scale, y1, y2);
     } else {
	  AtScaleResize(ac->scale, x1, x2);
	  AtScaleResize(ac->zoom_scale, x1, x2);
     }

     return len_changed;
}

/*
 *   The simple member functions that have no virtual functions
 */

AtScale *AtAxisGetScale(acw)
AtAxisCoreWidget acw;
{
     if (!acw) return NULL;     /* Can happen with textplot classes */

     XtCheckSubclass((Widget) acw, atAxisCoreWidgetClass,
		  "AtAxisGetScale needs an AtAxisCoreWidget");

     return acw->axiscore.scale;
}

int AtAxisWidth(acw)
AtAxisCoreWidget acw;
{
     XtCheckSubclass((Widget) acw, atAxisCoreWidgetClass,
		  "AtAxisWidth needs an AtAxisCoreWidget");
     return acw->axiscore.axis_width;
}

void AtAxisGetBounds(acw, minp, maxp)
AtAxisCoreWidget acw;
float *minp, *maxp;
{
     *minp = acw->axiscore.min;
     *maxp = acw->axiscore.max;
}

AtTransform AtAxisGetTransform(acw)
AtAxisCoreWidget acw;
{
     return AtScaleGetTransform(acw->axiscore.scale);
}


/*
 *   Return the maximal number width.
 *   This may be called from applications to get the maximal
 *   number width and then to set a default number width.
 */

int AtAxisGetNumberWidth(acw)
AtAxisCoreWidget acw;
{
     AtAxisCorePart *ac;
     int w, mw, i;
     AtPlotterPart *pp=&(((AtPlotterWidget)XtParent((Widget)acw))->plotter);

     if ( !XtIsRealized((Widget) acw) || acw->axiscore.tic_label_string == NULL)
	  return 0;

     ac = &acw->axiscore;
     mw = 0;
     if (ac->vertical)
       for (i = 0; i < ac->num_tics; i++) {
	  if (ac->tic_label_string[i]) {
	     w = XTextWidth(pp->axis_fs, ac->tic_label_string[i], 
	             strlen(ac->tic_label_string[i])); 
	     mw = Max(mw, w);
	  }
       }
     else mw = pp->axis_fs->ascent + pp->axis_fs->descent;
     return mw;
}
