/*
 *      Plot.c
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Mon Jul 27 14:43:20 1992, patchlevel 2
 *                                      Sometimes the line width from
 *                                      PostScript output is too large.
 *                                      Reported and fixed by Gustaf Neumann
 *                                      (neumann@dec4.wu-wien.ac.at)
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 */
static char SCCSid[] = "@(#) Plotter V6.0  92/08/15  Plot.c";

/*

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

#include "PlotP.h"
#include "AtConverters.h"


/* Make the default background the same as the parent background */
static void defBG(w, off, val)
Widget w;
int off;
XrmValue *val;
{
     Widget p = XtParent(w);
     val->addr = (XtPointer)&p->core.background_pixel;
}

/* The resources */

#define off(field) XtOffset(AtPlotWidget, plot.field)
static XtResource resources[] = {
  {
     XtNforeground, XtCForeground,
     XtRPixel, sizeof(Pixel),
     off(foreground), XtRString, (XtPointer) XtDefaultForeground
  },
  {
     XtNbackground, XtCBackground,
     XtRPixel, sizeof(Pixel),
     off(background), XtRCallProc, (XtPointer) defBG
		      /* XtRString,   XtDefaultBackground */
  },
  {
     XtNlineWidth, XtCLineWidth,
     XtRInt, sizeof(int),
     off(line_width), XtRImmediate, (XtPointer) 0
  },
  {
     XtNlineStyle, XtCLineStyle,
     XtRLinestyle, sizeof(int),
     off(line_style),  XtRImmediate, (XtPointer) LineSolid
  },
  {
     XtNdashLength, XtCDashLength,
     XtRInt, sizeof(int),
     off(dash_length), XtRImmediate, (XtPointer) 4
  },
  {
     XtNfastUpdate, XtCFastUpdate,
     XtRBoolean, sizeof(Boolean),
     off(fast_update), XtRImmediate, (XtPointer) False
  },
};
#undef off

static void Initialize P((AtPlotWidget, AtPlotWidget));
static void Destroy P((AtPlotWidget));
static Boolean SetValues P((AtPlotWidget, AtPlotWidget, AtPlotWidget));
static void ClassPartInitialize P((WidgetClass));

AtPlotClassRec atPlotClassRec = {
{ /* core part */
    /* superclass         */    (WidgetClass) &objectClassRec,
    /* class_name         */    "AtPlot",
    /* widget_size        */    sizeof(AtPlotRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize*/  ClassPartInitialize,
    /* class_inited       */    FALSE,
    /* initialize         */    (XtInitProc) Initialize,
    /* initialize_hook    */    NULL,
    /* pad                */    NULL,
    /* pad                */    NULL,
    /* pad                */    0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* pad                */    FALSE,
    /* pad                */    FALSE,
    /* pad                */    FALSE,
    /* pad                */    FALSE,
    /* destroy            */    (XtWidgetProc)Destroy,
    /* pad                */    NULL,
    /* pad                */    NULL,
    /* set_values         */    (XtSetValuesFunc) SetValues,
    /* set_values_hook    */    NULL,
    /* pad                */    NULL,
    /* get_values_hook    */    NULL,
    /* pad                */    NULL,
    /* version            */    XtVersion,
    /* callback_offsets   */    NULL,
    /* pad                */    NULL,
    /* pad                */    NULL,
    /* pad                */    NULL,
    /* extension            */  NULL
},
  /* AtPlotClassPart initialization */
  {
    /* draw()           */      NULL,
    /* draw_icon()      */      NULL,
    /* draw_ps()        */      NULL,
    /* draw_icon_ps()   */      NULL,
    /* recalc()         */      NULL
  }
};

WidgetClass atPlotWidgetClass = (WidgetClass)&atPlotClassRec;

static void GetGC P((AtPlotWidget w));
static void GetGC(w)
AtPlotWidget w;
{
     XGCValues gcv;
     XtGCMask gcmask;

     if (w->plot.fast_update)  {
	  gcv.foreground = w->plot.background ^ w->plot.foreground;
	  gcv.function = GXxor;
	  gcmask = GCForeground | GCFunction;
     } else {
	  gcv.foreground = w->plot.foreground;
	  gcv.background = w->plot.background;
	  gcmask = GCForeground | GCBackground;
     }
     gcv.line_width = w->plot.line_width;
     gcv.line_style = w->plot.line_style;
     gcv.dashes = (char) w->plot.dash_length;

     gcmask |= GCLineWidth | GCLineStyle | GCDashList;
     w->plot.gc = XtGetGC(XtParent((Widget)w), gcmask, &gcv);
}

static void FreeGC P((AtPlotWidget w));
static void FreeGC(w)
AtPlotWidget w;
{
     XtReleaseGC(XtParent((Widget)w), w->plot.gc);
}

static void Initialize(request, new)
AtPlotWidget request, new;
{
     GetGC(new);
}

static void Destroy(w)
AtPlotWidget w;
{
     FreeGC(w);
}

static Boolean SetValues(current, request, new)
AtPlotWidget current, request, new;
{
#define Changed(field) (new->plot.field != current->plot.field)
     Boolean redraw = False;

     if (Changed(background) ||
	 Changed(line_width) || Changed(line_style) || Changed(dash_length) ||
	 Changed(fast_update)) {
	  redraw = True;
     }

     if (redraw || Changed(foreground)) {
	  FreeGC(new);
	  GetGC(new);
	  if (redraw)
	       AtPlotterRedrawRequired(new);
	  else
	       AtPlotterRefreshRequired(new);
     }

     /* nothing to redisplay */
     return False;
#undef Changed
}

static void ClassPartInitialize(class)
WidgetClass class;
{
     AtPlotWidgetClass plot, super;

     AtRegisterLinestyleConverter();
     /* AtRegisterMarkerConverter(); */

     plot = (AtPlotWidgetClass) class;
     super = (AtPlotWidgetClass) plot->object_class.superclass;

#define CheckInherit(proc, inherit) \
     if (plot->plot_class.proc == inherit)\
	  plot->plot_class.proc = super->plot_class.proc

	       CheckInherit(draw, XtInheritDraw);
     CheckInherit(draw_icon, XtInheritDrawIcon);
    /* CheckInherit(draw_ps, XtInheritDrawPS);
     CheckInherit(draw_icon_ps, XtInheritDrawIconPS); */
     CheckInherit(recalc, XtInheritRecalc);

#undef CheckInherit
     *SCCSid = *SCCSid;       /* Keep gcc quiet */
}


/*
 * Wrappers to call the member routines
 */

void AtPlotDraw(pw, dpy, win, region, refresh)
AtPlotWidget pw;
Display *dpy;
Window win;
Region region;
int refresh;
{
     AtPlotDrawProc fn;
     
  if (XtIsSubclass((Widget)pw, atPlotWidgetClass)) {
     fn = ((AtPlotWidgetClass)
			  XtClass((Widget)pw))->plot_class.draw;
     if (fn)
	  fn(pw, dpy, win, region, refresh);
  }
}

void AtPlotDrawIcon(pw, dpy, win, x1, y1, x2, y2, region)
AtPlotWidget pw;
Display *dpy;
Window win;
int x1, y1, x2, y2;
Region region;
{
     AtPlotDrawIconProc fn = ((AtPlotWidgetClass)
			      XtClass((Widget)pw))->plot_class.draw_icon;
     if (fn)
	  fn(pw, dpy, win, x1, y1, x2, y2, region);
}


void AtPlotRecalc(pw, xs, ys, from, to)
AtPlotWidget pw;
AtScale *xs, *ys;
int from, to;
{
  WidgetClass class = XtClass((Widget)pw);

  /*if ((class==atXYLinePlotWidgetClass) || (class==atXYAxisWidgetClass)) {*/
  if (XtIsSubclass((Widget)pw, atPlotWidgetClass)) {
     AtPlotRecalcProc fn = ((AtPlotWidgetClass)
				XtClass((Widget)pw))->plot_class.recalc;
     if (fn)
	  fn(pw, xs, ys, from, to);
  }
}
