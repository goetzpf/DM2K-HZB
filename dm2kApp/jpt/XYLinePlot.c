/*
 *      XYLinePlot.c
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Mon Jul 27 14:19:31 1992, patchlevel 2
 *                                      Bugs in DrawPS() and DrawStylePS() fixed.
 *                                      Draw() changed for drawing
 *                                      to a pixmap instead of a window.
 *                                      Plot types steps and bars added.
 *                                      Shorter procedure names.
 *      klin, Fri Aug  7 10:07:59 1992, Cast type converters to keep
 *                                      ANSI C compilers quiet.
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Minor changes in PS output
 *                                      Changed <At/..> to <X11/At/..>.
 */
static char SCCSid[] = "@(#) Plotter V6.0  92/08/15  XYLinePlot.c";

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
 *   The line plot widget
 */

#include "PlotterP.h"
#include "AxisCoreP.h"
#include "XYLinePlotP.h"
#include "Plot.h"

/*
 *   Forward declare all the private widgetclass routines
 */

static void Draw P((AtXYLinePlotWidget, Display *, Drawable, Region, int));
static void DrawIcon P((AtXYLinePlotWidget, Display *, Drawable, int, int, int, int, Region));
/*static void DrawPS P((AtXYLinePlotWidget, FILE *, AtScale *, AtScale *));
static void DrawIconPS P((AtXYLinePlotWidget, FILE *, int, int, int, int));*/
static void Recalc P((AtXYLinePlotWidget, AtScale *, AtScale *, int, int));
static void Attach P((AtXYLinePlotWidget, BoundingBox *, int));
static void ClassInitialize P((void));
static void Initialize P((AtXYLinePlotWidget, AtXYLinePlotWidget));
static void Destroy P((AtXYLinePlotWidget));
static Boolean SetValues P((AtXYLinePlotWidget, AtXYLinePlotWidget, AtXYLinePlotWidget));

static int CalcMarkOffset P((AtXYLinePlotWidget));
static void DrawOneMark P((AtXYLinePlotWidget, Display *, Drawable, int, int, int));
static void DrawMarks P((AtXYLinePlotWidget, Display *, Drawable));
static void DrawImpulses P((AtXYLinePlotWidget, Display *, Drawable));
static void DrawSteps P((AtXYLinePlotWidget, Display *, Drawable));
static void DrawBars P((AtXYLinePlotWidget, Display *, Drawable));

/*
 *   The resources
 */

#define off(field) XtOffsetOf (AtXYLinePlotRec, lineplot.field)
static XtResource resources[] = {
  {
     XtNplotType, XtCPlotType,
     XtRInt, sizeof(PlotType),
     off(plot_type), XtRImmediate, (XtPointer) PLOT_PLOT
  },
  {
     XtNplotLineType, XtCPlotLineType,
     XtRPlotLineType, sizeof(AtPlotLineType),
     off(line_type), XtRImmediate, (XtPointer) AtPlotLINES
  },
  {
     XtNplotLineStyle, XtCPlotLineStyle,
     XtRPlotLineStyle, sizeof(AtPlotLineStyle),
     off(line_style), XtRImmediate, (XtPointer) AtLineSOLID
  },
  {
     XtNplotMarkType, XtCPlotMarkType,
     XtRPlotMarkType, sizeof(AtPlotMarkType),
     off(mark_type), XtRImmediate, (XtPointer) AtMarkNONE
  },
  {
     XtNplotFillStyle, XtCPlotFillStyle,
     XtRInt, sizeof(AtPlotFillStyle),
     off(fill_style), XtRImmediate, (XtPointer) PLOT_FPAT_NONE
  },
  {
     XtNplotMarkColor, XtCPlotMarkColor,
     XtRPixel, sizeof(Pixel),
     off(mark_color), XtRString, (XtPointer) NULL
  },
  {
     XtNplotMarkSize, XtCPlotMarkSize,
     XtRInt, sizeof(int),
     off(mark_size), XtRImmediate, (XtPointer) 0
  }
};
#undef  off

AtXYLinePlotClassRec atXYLinePlotClassRec = {
  { /* core fields */
    /* superclass               */      (WidgetClass) &atXYPlotClassRec,
    /* class_name               */      "AtXYLinePlot",
    /* widget_size              */      sizeof(AtXYLinePlotRec),
    /* class_initialize         */      (XtProc) ClassInitialize,
    /* class_part_initialize    */      NULL,
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
    /* draw                     */      (AtPlotDrawProc) Draw,
    /* draw_icon                */      (AtPlotDrawIconProc) DrawIcon,
    /* drawPS                   */      NULL,
    /* draw_iconPS              */      NULL,
    /* recalc                   */      (AtPlotRecalcProc) Recalc
  },
  { /* atXYPlot fields */
    /* attach_data              */      (AtXYPlotAttachProc) Attach,
  },
  { /* atXYLinePlot fields */
    /* empty                    */      0
  }
};

WidgetClass atXYLinePlotWidgetClass = (WidgetClass)&atXYLinePlotClassRec;

/*
 *   The core member procs
 */

static void ClassInitialize()
{
     AtRegisterPlotLineStyleConverter();
     AtRegisterPlotLineTypeConverter();
     AtRegisterPlotMarkTypeConverter();
     *SCCSid = *SCCSid;       /* Keep gcc quiet */
}

/*
 *   Some helper functions for GC management
 */

#define DOTTED      0         /* the line style dashes */
static char dotted[]     = { 1, 1 };
#define DASHED      1
static char dashed[]     = { 2, 2 };
#define DOTDASHED   2
static char dotdashed[]  = { 3, 2, 1, 2 };
#define DOTTED2     3
static char dotted2[]    = { 1, 2 };
#define DOTTED3     4
static char dotted3[]    = { 1, 3 };
#define DOTTED4     5
static char dotted4[]    = { 1, 4 };
#define DOTTED5     6
static char dotted5[]    = { 1, 5 };
#define DASHED3     7
static char dashed3[]    = { 4, 4 };
#define DASHED4     8
static char dashed4[]    = { 6, 6 };
#define DASHED5     9
static char dashed5[]    = { 8, 8 };
#define DOTDASHED2  10
static char dotdashed2[] = { 4, 4, 1, 4 };

static struct {               /* The dashes list */
     char *dashes;
     int  length;
} dashlist[] = {
     { dotted,      2 },
     { dashed,      2 },
     { dotdashed,   4 },
     { dotted2,     2 },
     { dotted3,     2 },
     { dotted4,     2 },
     { dotted5,     2 },
     { dashed3,     2 },
     { dashed4,     2 },
     { dashed5,     2 },
     { dotdashed2,  4 }
};
#define HUGE_DASH   200

static void GetLineGC P((AtXYLinePlotWidget w));
static void GetLineGC(w)
AtXYLinePlotWidget w;
{
     XGCValues gcv;
     XtGCMask gcmask;
     int dash;

     if (w->plot.fast_update)  {
	  gcv.foreground = w->plot.background ^ w->plot.foreground;
	  gcv.function = GXxor;
	  gcmask = GCForeground | GCFunction;
     } else {
	  gcv.foreground = w->plot.foreground;
	  gcv.background = w->plot.background;
	  gcmask = GCForeground | GCBackground;
     }

     /*
      *   We have to create a unique GC with the appropriate dash list
      *   for each line type. XtGetGC() shares same GCs between different
      *   widgets and we had to create a new GC for each line using the
      *   Xlib function XCreateGC(). Setting the dashes member in the GC
      *   to a unlikely value (>200) allows us to use XtGetGC() and so
      *   to share same GCs between line widgets with the same line type.
      */
     switch(w->lineplot.line_style) {
	  case AtLineDOTTED:
	       gcv.line_style = LineOnOffDash;
	       dash = DOTTED;
	       gcv.dashes     = (char) HUGE_DASH+DOTTED;
	       gcmask |= GCDashList;
	       break;
	  case AtLineDASHED:
	       gcv.line_style = LineOnOffDash;
	       dash = DASHED;
	       gcv.dashes     = (char) HUGE_DASH+DASHED;
	       gcmask |= GCDashList;
	       break;
	  case AtLineDOTDASHED:
	       gcv.line_style = LineOnOffDash;
	       dash = DOTDASHED;
	       gcv.dashes     = (char) HUGE_DASH+DOTDASHED;
	       gcmask |= GCDashList;
	       break;
	  case AtLineDOTTED2:
	       gcv.line_style = LineOnOffDash;
	       dash = DOTTED2;
	       gcv.dashes     = (char) HUGE_DASH+DOTTED2;
	       gcmask |= GCDashList;
	       break;
	  case AtLineDOTTED3:
	       gcv.line_style = LineOnOffDash;
	       dash = DOTTED3;
	       gcv.dashes     = (char) HUGE_DASH+DOTTED3;
	       gcmask |= GCDashList;
	       break;
	  case AtLineDOTTED4:
	       gcv.line_style = LineOnOffDash;
	       dash = DOTTED4;
	       gcv.dashes     = (char) HUGE_DASH+DOTTED4;
	       gcmask |= GCDashList;
	       break;
	  case AtLineDOTTED5:
	       gcv.line_style = LineOnOffDash;
	       dash = DOTTED5;
	       gcv.dashes     = (char) HUGE_DASH+DOTTED5;
	       gcmask |= GCDashList;
	       break;
	  case AtLineDASHED3:
	       gcv.line_style = LineOnOffDash;
	       dash = DASHED3;
	       gcv.dashes     = (char) HUGE_DASH+DASHED3;
	       gcmask |= GCDashList;
	       break;
	  case AtLineDASHED4:
	       gcv.line_style = LineOnOffDash;
	       dash = DASHED4;
	       gcv.dashes     = (char) HUGE_DASH+DASHED4;
	       gcmask |= GCDashList;
	       break;
	  case AtLineDASHED5:
	       gcv.line_style = LineOnOffDash;
	       dash = DASHED5;
	       gcv.dashes     = (char) HUGE_DASH+DASHED5;
	       gcmask |= GCDashList;
	       break;
	  case AtLineDOTDASHED2:
	       gcv.line_style = LineOnOffDash;
	       dash = DOTDASHED2;
	       gcv.dashes     = (char) HUGE_DASH+DOTDASHED2;
	       gcmask |= GCDashList;
	       break;
	  case AtLineSOLID:
	  default:
	       gcv.line_style = LineSolid;
	       dash = -1;
	       break;
     }
     gcv.line_width = w->plot.line_width;
     gcmask |= GCLineStyle | GCLineWidth;

     w->lineplot.line_gc = XtGetGC(XtParent((Widget)w), gcmask, &gcv);

     if (dash >= 0) {         /* now set the dashlist */
	  XSetDashes(XtDisplay(XtParent((Widget)w)), w->lineplot.line_gc,
		     0, dashlist[dash].dashes, dashlist[dash].length);
     }
}

static void GetMarkGC P((AtXYLinePlotWidget w));
static void GetMarkGC(w)
AtXYLinePlotWidget w;
{
     XGCValues gcv;
     XtGCMask gcmask;

     gcv.line_style = LineSolid;
     if (w->lineplot.mark_color==(Pixel)NULL) 
	w->lineplot.mark_color=w->plot.foreground;
     if (w->plot.fast_update)  {
	  gcv.foreground = w->plot.background ^ w->lineplot.mark_color;
	  gcv.function = GXxor;
	  gcmask = GCForeground | GCFunction | GCLineStyle;
     } else {
	  gcv.foreground = w->lineplot.mark_color;
	  gcv.background = w->plot.background;
	  gcmask = GCForeground | GCBackground | GCLineStyle;
     }

     w->lineplot.mark_gc = XtGetGC(XtParent((Widget)w), gcmask, &gcv);
}

#define FreeLineGC(w) XtReleaseGC((Widget)w, w->lineplot.line_gc)
#define FreeMarkGC(w) XtReleaseGC((Widget)w, w->lineplot.mark_gc)

/*
 *   The initialize/destroy/setvalues procs
 */

static void Initialize(request, new)
AtXYLinePlotWidget request, new;
{
     GetLineGC(new);
     GetMarkGC(new);
}

static void Destroy(w)
AtXYLinePlotWidget w;
{
     FreeLineGC(w);
     FreeMarkGC(w);
}

static Boolean SetValues(current, request, new)
AtXYLinePlotWidget current, request, new;
{
#define Changed(field) (new->lineplot.field != current->lineplot.field)
     Boolean redraw = False;

     if ((new->plot.foreground != current->plot.foreground) ||
	 (new->plot.background != current->plot.background) ||
	 (new->plot.line_width != current->plot.line_width)) {
	  FreeLineGC(new);
	  GetLineGC(new);
	  redraw = True;
     }
     else if (Changed(line_style)) {
	  FreeLineGC(new);
	  GetLineGC(new);
	  redraw = True;
     }
     if ((new->lineplot.mark_color != current->lineplot.mark_color) ||
	 (new->plot.background != current->plot.background) ||
	 (new->plot.line_width != current->plot.line_width)) {
	  FreeMarkGC(new);
	  GetMarkGC(new);
	  redraw = True;
     }  

     if (Changed(line_type) || Changed(mark_type) || Changed(mark_size)) {
	  redraw = True;
     }
     if (redraw) {
	  AtPlotterRedrawRequired((AtPlotWidget) new);
     }
     /* Nothing to redisplay */
     return False;
#undef Changed
}

/*
 *   These routines are the ones called by the parent plot widget
 */

#define lp ((AtXYLinePlotWidget)self)
#define PIX ((XPoint *) lp->lplot.pix)
#define ZOOM_PIX ((XPoint *) lp->lplot.zoom_pix)
/* NB: PIX is NOT an lvalue (on some very picky compilers!!!) */

/*
 *   Don't need to adjust the bbox, only to allocate the memory.
 */

static void Attach(self, bbp, extending)
AtXYLinePlotWidget self;
BoundingBox *bbp;
int extending;
{
     if (extending)
	  lp->lplot.pix = XtRealloc((char *) PIX,
		    lp->lplot.num_points * sizeof (XPoint));
     else
	  lp->lplot.pix = XtMalloc(lp->lplot.num_points * sizeof (XPoint));
}

/*
 *   Internal procs for drawing marks/impulses/steps/bars
 */

static int CalcMarkOffset(self)
AtXYLinePlotWidget self;
{
     int w, off;

     w = lp->lineplot.mark_size/2+1;
     if (w<1) w=1;
     
     /*if (lp->lineplot.mark_size < 2)
	  w = 1;
     else if( lp->lineplot.mark_size < 4)
	  w = 2;
     else
	  w = 3;
*/
     switch (lp->lineplot.mark_type) {
	  case AtMarkPLUS:
	  case AtMarkXMARK:
	  case AtMarkSTAR:
	  case AtMarkTRIANGLE1:
	  case AtMarkTRIANGLE2:
	  case AtMarkTRIANGLE3:
	  case AtMarkTRIANGLE4:
	  case AtMarkDIAMOND:
	       off = 1.5 * w;
	       break;
	  case AtMarkRECTANGLE:
	  default:
	       off = w;
	       break;
     }
     return off; 
}

static void DrawOneMark(self, dpy, drw, x, y, off)
AtXYLinePlotWidget self;
Display *dpy;
Drawable drw;
int x, y, off;
{
     int len;
     XPoint p[5];

     switch (lp->lineplot.mark_type) {
	  case AtMarkCIRCLE:
               XDrawArc(dpy, drw, lp->lineplot.mark_gc,
		  x-off, y-off, off*2, off*2, 0, 360*64);
               break;
	  case AtMarkDOT:
	       XFillArc(dpy, drw, lp->lineplot.mark_gc,
		   x-off, y-off, off*2, off*2, 0, 360*64);
               break;
          case AtMarkCROSS:
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x, y - off, x, y + off*2);
               XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y, x + off, y);
	  case AtMarkPLUS:
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x, y - off, x, y + off);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y, x + off, y);
	       break;
	  case AtMarkXMARK:
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y - off, x + off, y + off);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y+off, x+off, y - off);
	       break;
	  case AtMarkSTAR:
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x, y - off, x, y + off);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y, x + off, y);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y - off, x + off, y + off);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y + off, x + off, y - off);
	       break;
	  case AtMarkTRIANGLE1:
	      /* XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y + off, x + off, y + off);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x + off, y + off, x, y - off);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x, y - off, x - off, y + off);
	      */
	       p[0].x = x - off; p[0].y = y + off;
	       p[1].x = x + off; p[1].y = y + off;
	       p[2].x = x;       p[2].y = y - off;
	       p[3].x = x - off; p[3].y = y + off;
	       XFillPolygon(dpy, drw, lp->lineplot.mark_gc,
		    &p[0], 4, Complex, CoordModeOrigin);
	       break;
	  case AtMarkTRIANGLE2:
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y - off, x + off, y - off);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x + off, y - off, x, y + off);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x, y + off, x - off, y - off);
	       break;
	  case AtMarkTRIANGLE3:
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y - off, x - off, y + off);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y + off, x + off, y);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x + off, y, x - off, y - off);
	       break;
	  case AtMarkTRIANGLE4:
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x + off, y - off, x + off, y + off);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x + off, y + off, x - off, y);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y, x + off, y - off);
	       break;
	  case AtMarkDIAMOND:
	       /*XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x, y - off, x - off, y);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x - off, y, x, y + off);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x, y + off, x + off, y);
	       XDrawLine(dpy, drw, lp->lineplot.mark_gc,
			 x + off, y, x, y - off);
	       */
	       p[0].x = x;        p[0].y = y - off;
	       p[1].x = x - off;  p[1].y = y;
	       p[2].x = x;        p[2].y = y + off;
	       p[3].x = x + off;  p[3].y = y;
	       p[4].x = x;        p[4].y = y - off;
	       XFillPolygon(dpy, drw, lp->lineplot.mark_gc,
		    &p[0], 5, Complex, CoordModeOrigin);
	       break;
	  case AtMarkRECTANGLE:
	       len = off + off;
	       /*XDrawRectangle(dpy, drw, lp->lineplot.mark_gc,
			      x - off, y - off, len, len);*/
               XFillRectangle(dpy, drw, lp->lineplot.mark_gc,
		       x - off, y - off, len, len);
               break;
     }

}

static void DrawMarks(self, dpy, drw)
AtXYLinePlotWidget self;
Display *dpy;
Drawable drw;
{
     int i, off;

     off = CalcMarkOffset(self);
     for (i = 0; i < lp->lplot.num_points; i++) {
       /*if (pp->ifzoom)
	 DrawOneMark(self, dpy, drw, ZOOM_PIX[i].x, ZOOM_PIX[i].y, off);
       else */
       DrawOneMark(self, dpy, drw, PIX[i].x, PIX[i].y, off);
     }
}

static void DrawImpulses(self, dpy, drw)
AtXYLinePlotWidget self;
Display *dpy;
Drawable drw;
{
     int i;

     for (i = 0; i < lp->lplot.num_points; i++) {
	  XDrawLine(dpy, drw, lp->lineplot.line_gc,
		    PIX[i].x, PIX[i].y, PIX[i].x, lp->lineplot.impulse_y);
     }
}

static void DrawSteps(self, dpy, drw)
AtXYLinePlotWidget self;
Display *dpy;
Drawable drw;
{
     int i;

     for (i = 1; i < lp->lplot.num_points; i++) {
	  XDrawLine(dpy, drw, lp->lineplot.line_gc,
		    PIX[i-1].x, PIX[i-1].y, PIX[i].x, PIX[i-1].y);
	  XDrawLine(dpy, drw, lp->lineplot.line_gc,
		    PIX[i].x, PIX[i-1].y, PIX[i].x, PIX[i].y);
     }
}

static void DrawBars(self, dpy, drw)
AtXYLinePlotWidget self;
Display *dpy;
Drawable drw;
{
     int i;

     if (lp->lplot.num_points > 1) {
	  XDrawLine(dpy, drw, lp->lineplot.line_gc,
		    PIX[0].x, PIX[0].y, PIX[0].x, lp->lineplot.impulse_y);
     }
     for (i = 1; i < lp->lplot.num_points; i++) {
	  XDrawLine(dpy, drw, lp->lineplot.line_gc,
		    PIX[i-1].x, PIX[i-1].y, PIX[i].x, PIX[i-1].y);
	  XDrawLine(dpy, drw, lp->lineplot.line_gc,
		    PIX[i].x, PIX[i-1].y, PIX[i].x, PIX[i].y);
	  XDrawLine(dpy, drw, lp->lineplot.line_gc,
		    PIX[i].x, PIX[i].y, PIX[i].x, lp->lineplot.impulse_y);
     }
}

/*
 *   Draw the line clipped by the given region.
 */

static void Draw(self, dpy, drw, region, refresh)
AtXYLinePlotWidget self;
Display *dpy;
Drawable drw;
Region region;
int refresh;
{
     AtPlotterWidget pw = (AtPlotterWidget) XtParent((Widget) self);
     AtPlotterLayout *pwl = &pw->plotter.layout;
     AtPlotterPart *pp = &(pw->plotter);
     XRectangle linerectangle, markrectangle;
     Region lineregion, markregion;
     int i, x1, x2, y1, y2, off, width, height;
     XPoint *fillp;

     /* Get clip regions for lines and marks from the plotter layout */
     /*if (!pp->xmax_dft) 
       if (pp->xmax < pw1->x1) x2 = pp->xmax;*/
     linerectangle.x = pwl->x1+1;
     linerectangle.y = pwl->y1;
     linerectangle.width  = pwl->width-1;
     linerectangle.height = pwl->height;
     off = CalcMarkOffset(self);
     markrectangle.x = pwl->x1 - off;
     markrectangle.y = pwl->y1 - off;
     markrectangle.width  = pwl->width  + 2*off;
     markrectangle.height = pwl->height + 2*off;
/*
printf("XYLinePlot:Draw plotter xmin %f, xmax %f, xaxis_min %f, xaxis_max %f, ymin %f, ymax %f, yaxis_min %f, yaxis_max %f\n", pp->xmin, pp->xmax, pp->xaxis_min, pp->xaxis_max, pp->ymin, pp->ymax, pp->yaxis_min, pp->yaxis_max);
     off = CalcMarkOffset(self);
     x1 = AtScaleUserToPixel(pp->xaxis->axiscore.scale, pp->xmin); 
     y1 = AtScaleUserToPixel(pp->yaxis->axiscore.scale, pp->ymax);
     width = AtScaleUserToPixel(pp->xaxis->axiscore.scale, pp->xmax-pp->xmin); 
     height = AtScaleUserToPixel(pp->yaxis->axiscore.scale, pp->ymax-pp->ymin);
printf("XYLinePlot:Draw, region x %d, y %d, w %d, h %d\n", x1,y1,width,height);
     linerectangle.x = x1+1;
     linerectangle.y = y1;
     linerectangle.width  = width-1;
     linerectangle.height = height;
     markrectangle.x = x1 - off;
     markrectangle.y = y1 - off;
     markrectangle.width  = width  + 2*off;
     markrectangle.height = height + 2*off;*/

     /* Now create and set the regions */
     lineregion = XCreateRegion();
     markregion = XCreateRegion();
     XUnionRectWithRegion(&linerectangle, lineregion, lineregion);
     XUnionRectWithRegion(&markrectangle, markregion, markregion);
     XSetRegion(dpy, lp->lineplot.line_gc, lineregion);
     XSetRegion(dpy, lp->lineplot.mark_gc, markregion);
     XSetRegion(dpy, pp->axis_gc, lineregion);

#ifdef TRACE
     fprintf(stderr, "Draw %d lines/points\n",
	     lp->lplot.num_points);
#endif

     if (lp->lplot.old_pix) {
	  if (lp->plot.fast_update && refresh) {
	       /*
		* We are in fast update mode, doing a refresh and have old
		* pixpoints, so draw them to "erase" the old s first
		*/
	       switch (lp->lineplot.line_type) {
		    case AtPlotPOINTS:
			 DrawMarks(self, dpy, drw);
			 break;
		    case AtPlotLINEPOINTS:
			 XDrawLines(dpy, drw, lp->lineplot.line_gc,
				    (XPoint *) lp->lplot.old_pix,
				    lp->lplot.old_num_points, CoordModeOrigin);
			 DrawMarks(self, dpy, drw);
			 break;
		    case AtPlotIMPULSES:
			 DrawImpulses(self, dpy, drw);
			 break;
		    case AtPlotLINEIMPULSES:
			 DrawImpulses(self, dpy, drw);
			 XDrawLines(dpy, drw, lp->lineplot.line_gc,
				    (XPoint *) lp->lplot.old_pix,
				    lp->lplot.old_num_points, CoordModeOrigin);
			 break;
		    case AtPlotSTEPS:
			 DrawSteps(self, dpy, drw);
			 break;
		    case AtPlotBARS:
			 DrawBars(self, dpy, drw);
		    case AtPlotLINES:
		    default:
			 XDrawLines(dpy, drw, lp->lineplot.line_gc,
				    (XPoint *) lp->lplot.old_pix,
				    lp->lplot.old_num_points, CoordModeOrigin);
			 break;
	       }

	  }
	  XtFree((char *) lp->lplot.old_pix);
	  lp->lplot.old_pix = NULL;
	  lp->lplot.old_num_points = 0;
     } /*end of fast update*/

  switch (lp->lineplot.plot_type) {
    case PLOT_BAR:
        DrawBars(self, dpy, drw);
        break;
    case PLOT_AREA:
        fillp = (XPoint *)calloc((lp->lplot.num_points+3), sizeof(XPoint));
	if (fillp) {
	   XPoint *pt = (XPoint *)lp->lplot.pix;
	   fillp[0].x = fillp[lp->lplot.num_points+2].x = 
		    (short) ((XPoint *)lp->lplot.pix)[0].x;
           fillp[0].y = fillp[lp->lplot.num_points+2].y=(short) (pwl->y2);
           for (i=0; i < lp->lplot.num_points; i++) {
	     fillp[i+1].x=(short) pt[i].x;
	     fillp[i+1].y=(short) pt[i].y;
	     }
           fillp[lp->lplot.num_points+1].x=
	     (short) ((XPoint *)lp->lplot.pix)[lp->lplot.num_points-1].x;
	   fillp[lp->lplot.num_points+1].y=(short) (pwl->y2-1);
	   XFillPolygon(dpy,drw,lp->lineplot.line_gc, fillp, 
	      lp->lplot.num_points+3, Complex, CoordModeOrigin);
	   free(fillp);
	   }
	XDrawLines(dpy, drw, pp->axis_gc, PIX,
	   lp->lplot.num_points, CoordModeOrigin);
	break;
    case PLOT_PLOT:
    default:
	if (lp->lineplot.line_style != AtLineNONE) 
	  /*if (pp->ifzoom) 
	    XDrawLines(dpy, drw, lp->lineplot.line_gc, ZOOM_PIX,
	      lp->lplot.num_points, CoordModeOrigin);*/
	  /*else*/
               XDrawLines(dpy, drw, lp->lineplot.line_gc, PIX,
	    lp->lplot.num_points, CoordModeOrigin);
	if (lp->lineplot.mark_type != AtMarkNONE) DrawMarks(self, dpy, drw);
	break;  
    }

     /* Unset and destroy the regions */
     XSetClipMask(dpy, lp->lineplot.line_gc, None);
     XSetClipMask(dpy, lp->lineplot.mark_gc, None);
     XSetClipMask(dpy, pp->axis_gc, None);
     XDestroyRegion(lineregion);
     XDestroyRegion(markregion);
}

/*
 *   Draw the "icon" in the given place.
 */

static void DrawIcon(self, dpy, drw, x1, y1, width, height, region)
AtXYLinePlotWidget self;
Display *dpy;
Drawable drw;
int x1, y1, width, height;
Region region;
{
     int off;

     y1 += height >> 1;
     switch (lp->lineplot.line_type) {
	  case AtPlotPOINTS:
	       off = CalcMarkOffset(self);
	       x1 += width >> 1;
	       DrawOneMark(self, dpy, drw, x1, y1, off);
	       break;
	  case AtPlotLINEPOINTS:
	       XDrawLine(dpy, drw, lp->lineplot.line_gc,
			 x1, y1, x1 + width, y1);
	       off = CalcMarkOffset(self);
	       x1 += width >> 1;
	       DrawOneMark(self, dpy, drw, x1, y1, off);
	       break;
	  case AtPlotLINEIMPULSES:
	  case AtPlotIMPULSES:
	  case AtPlotSTEPS:
	  case AtPlotBARS:
	  case AtPlotLINES:
	  default:
	       XDrawLine(dpy, drw, lp->lineplot.line_gc,
			 x1, y1, x1 + width, y1);
	       break;
     }
}


void PlotScaleMotionRescalePlot(AtXYLinePlotWidget self, AtScale *xs, AtScale *ys)
{
  int i;
  float l, x, y;
  AtPlotterWidget pw = (AtPlotterWidget) XtParent((Widget) self);
  AtPlotterLayout *pwl = &pw->plotter.layout;
  AtPlotterPart *pp = &(pw->plotter);

  for (i = 0; i < lp->lplot.num_points; i++) {
          x = AtXYPlotGetXValue((AtXYPlotWidget) lp, i);
          y = AtXYPlotGetYValue((AtXYPlotWidget) lp, i);
          if (xs->transform == AtTransformLOGARITHMIC && x <= 0.0) {
               l = log10(xs->low) - 5.0;
               x = pow(10.0, l);
          }
          if (ys->transform == AtTransformLOGARITHMIC && y <= 0.0) {
               l = log10(ys->low) - 5.0;
               y = pow(10.0, l);
          }
         /* if (pp->ifzoom==PLOT_ZOOM) {
            ZOOM_PIX[i].x = AtScaleUserToPixel(xs, x);
            ZOOM_PIX[i].y = AtScaleUserToPixel(ys, y);
            }
          else { */
            PIX[i].x = AtScaleUserToPixel(xs, x);
            PIX[i].y = AtScaleUserToPixel(ys, y);
            /*} */
     }
  if (ys->low <= 0.0)
          lp->lineplot.impulse_y = AtScaleUserToPixel(ys, 0.0);
  else
          lp->lineplot.impulse_y = AtScaleUserToPixel(ys, ys->low);


}


/*
 *   Recalc the data according to the passed x and y scales
 */

static void Recalc(self, xs, ys, from, to)
AtXYLinePlotWidget self;
AtScale *xs, *ys;
int from, to;
{
     float x, y, l;
     int i;
     XtPointer val;
     AtXYPlotPart *lplot;

     lplot = &(self->lplot);
#ifdef TRACE
     fprintf(stderr, " -- Recalc from %d to %d\n", from, to);
#endif
     if (from > to) {
	  from = 0;
	  to = self->lplot.num_points - 1;
     }
     for (i = from; i <= to; i++) {
	  val = (XtPointer)((char *)lplot->xdata + lplot->xstride * (i));
	  x = (float)lplot->x_offset + (float)*((float *) val);
	  val = (XtPointer)((char *)lplot->ydata + lplot->ystride * (i));
	  y = (float)lplot->y_offset + (float)*((float *) val);
	  /*
	  x = AtXYPlotGetXValue((AtXYPlotWidget) lp, i);
	  y = AtXYPlotGetYValue((AtXYPlotWidget) lp, i);
	  */
	  if (xs->transform == AtTransformLOGARITHMIC && x <= 0.0) {
	       l = log10(xs->low) - 5.0;
	       x = pow(10.0, l);
	  }
	  if (ys->transform == AtTransformLOGARITHMIC && y <= 0.0) {
	       l = log10(ys->low) - 5.0;
	       y = pow(10.0, l);
	  }
	  /*if (pp->lfzoom) {
	    ZOOM_PIX[i].x = AtScaleUserToPixel(xs, x);
	    ZOOM_PIX[i].y = AtScaleUserToPixel(ys, y);
            }
          else {*/
	    PIX[i].x = (short)AtScaleUserToPixel(xs, x);
	    PIX[i].y = (short)AtScaleUserToPixel(ys, y);
            /*}*/
     }
     if (ys->low <= 0.0)
	  lp->lineplot.impulse_y = AtScaleUserToPixel(ys, 0.0);
     else
	  lp->lineplot.impulse_y = AtScaleUserToPixel(ys, ys->low);
}
#undef lp

/*
 *   Resource converters
 */

static void AtCvtStringToPlotLineType(args, num_args, from, to)
XrmValue *args;
Cardinal num_args;
XrmValue *from, *to;
{
     static AtPlotLineType linetype;

     linetype = AtPlotINVALID;

     if (strcasecmp(from->addr, "lines") == 0)
	  linetype = AtPlotLINES;
     else if (strcasecmp(from->addr, "points") == 0)
	  linetype = AtPlotPOINTS;
     else if (strcasecmp(from->addr, "impulses") == 0)
	  linetype = AtPlotIMPULSES;
     else if (strcasecmp(from->addr, "steps") == 0)
	  linetype = AtPlotSTEPS;
     else if (strcasecmp(from->addr, "bars") == 0)
	  linetype = AtPlotBARS;
     else if (strcasecmp(from->addr, "linepoints") == 0)
	  linetype = AtPlotLINEPOINTS;
     else if (strcasecmp(from->addr, "lineimpulses") == 0)
	  linetype = AtPlotLINEIMPULSES;

     if (linetype == AtPlotINVALID)
	  XtStringConversionWarning(from->addr, XtRPlotLineType);
     else {
	  to->addr = (caddr_t) &linetype;
	  to->size = sizeof(AtPlotLineType);
     }
}

void AtRegisterPlotLineTypeConverter()
{
     static Boolean registered = False;

     if (!registered) {
	  XtAddConverter(XtRString, XtRPlotLineType,
			 (XtConverter) AtCvtStringToPlotLineType, NULL,0);
	  registered = True;
     }
}

static void AtCvtStringToPlotLineStyle(args, num_args, from, to)
XrmValue *args;
Cardinal num_args;
XrmValue *from, *to;
{
     static AtPlotLineStyle linestyle;

     linestyle = AtLineINVALID;

     if (strcasecmp(from->addr, "solid") == 0)
	  linestyle = AtLineSOLID;
     else if (strcasecmp(from->addr, "dotted") == 0)
	  linestyle = AtLineDOTTED;
     else if (strcasecmp(from->addr, "dashed") == 0)
	  linestyle = AtLineDASHED;
     else if (strcasecmp(from->addr, "dotdashed") == 0)
	  linestyle = AtLineDOTDASHED;
     else if (strcasecmp(from->addr, "dotted2") == 0)
	  linestyle = AtLineDOTTED2;
     else if (strcasecmp(from->addr, "dotted3") == 0)
	  linestyle = AtLineDOTTED3;
     else if (strcasecmp(from->addr, "dotted4") == 0)
	  linestyle = AtLineDOTTED4;
     else if (strcasecmp(from->addr, "dotted5") == 0)
	  linestyle = AtLineDOTTED5;
     else if (strcasecmp(from->addr, "dashed3") == 0)
	  linestyle = AtLineDASHED3;
     else if (strcasecmp(from->addr, "dashed4") == 0)
	  linestyle = AtLineDASHED4;
     else if (strcasecmp(from->addr, "dashed5") == 0)
	  linestyle = AtLineDASHED5;
     else if (strcasecmp(from->addr, "dotdashed2") == 0)
	  linestyle = AtLineDOTDASHED2;

     if (linestyle == AtLineINVALID)
	  XtStringConversionWarning(from->addr, XtRPlotLineStyle);
     else {
	  to->addr = (caddr_t) &linestyle;
	  to->size = sizeof(AtPlotLineStyle);
     }
}

void AtRegisterPlotLineStyleConverter()
{
     static Boolean registered = False;

     if (!registered) {
	  XtAddConverter(XtRString, XtRPlotLineStyle,
			 (XtConverter) AtCvtStringToPlotLineStyle, NULL,0);
	  registered = True;
     }
}

static void AtCvtStringToPlotMarkType(args, num_args, from, to)
XrmValue *args;
Cardinal num_args;
XrmValue *from, *to;
{
     static AtPlotMarkType marktype;

     marktype = AtMarkINVALID;

     if (strcasecmp(from->addr, "rectangle") == 0)
	  marktype = AtMarkRECTANGLE;
     else if (strcasecmp(from->addr, "plus") == 0)
	  marktype = AtMarkPLUS;
     else if (strcasecmp(from->addr, "xmark") == 0)
	  marktype = AtMarkXMARK;
     else if (strcasecmp(from->addr, "star") == 0)
	  marktype = AtMarkSTAR;
     else if (strcasecmp(from->addr, "triangle1") == 0)
	  marktype = AtMarkTRIANGLE1;
     else if (strcasecmp(from->addr, "triangle2") == 0)
	  marktype = AtMarkTRIANGLE2;
     else if (strcasecmp(from->addr, "triangle3") == 0)
	  marktype = AtMarkTRIANGLE3;
     else if (strcasecmp(from->addr, "triangle4") == 0)
	  marktype = AtMarkTRIANGLE4;
     else if (strcasecmp(from->addr, "diamond") == 0)
	  marktype = AtMarkDIAMOND;

     if (marktype == AtMarkINVALID)
	  XtStringConversionWarning(from->addr, XtRPlotMarkType);
     else {
	  to->addr = (caddr_t) &marktype;
	  to->size = sizeof(AtPlotMarkType);
     }
}

void AtRegisterPlotMarkTypeConverter()
{
     static Boolean registered = False;

     if (!registered) {
	  XtAddConverter(XtRString, XtRPlotMarkType,
			 (XtConverter) AtCvtStringToPlotMarkType, NULL,0);
	  registered = True;
     }
}
