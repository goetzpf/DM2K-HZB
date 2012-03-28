#include <float.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <Xm/TransltnsP.h>
#include <Xm/XmStrDefs.h>
#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <Xm/DrawingAP.h>

#include "At.h"
#include "AtConverters.h"
#include "Scale.h"
#include "PlotterP.h"
#include "Plot.h"
#include "AxisCoreP.h"
#include "XYAxis.h"
#include "Text2.h"
#include "DataHandle.h"
#include "XYPlot.h"
#include "XYLinePlot.h"


 void AxisZoomStart P((AtPlotterWidget pw, XButtonPressedEvent *event));
 void GraphZoomStart P((AtPlotterWidget pw, XButtonPressedEvent *event));
 void ZoomMotion P((AtPlotterWidget pw, XMotionEvent *event));
 void AxisZoomEnd P((AtPlotterWidget pw, XButtonReleasedEvent *event));
 void GraphZoomEnd P((AtPlotterWidget pw, XButtonReleasedEvent *event));
 void Reset P((AtPlotterWidget pw, XEvent *event));
/* void ScaleStart P((AtPlotterWidget pw, XButtonPressedEvent *event));
 void ScaleMotion P((AtPlotterWidget pw, XMotionEvent *event));
*/

 void AxisZoomEnd(AtPlotterWidget pw, XButtonReleasedEvent *event)
{
  AtPlotterPart *pp = &pw->plotter;
  AtPlotterLayout *lp = &(pp->layout);
  AtAxisCoreWidget xaxis=pp->xaxis, yaxis=pp->yaxis;
  float tmp, x1, x2, y1, y2, y21, y22;
  int i, j, huge = 1, px1,px2,py1,py2;
  float a, b, c, d;
  AtScale *xac=xaxis->axiscore.scale, *yac=yaxis->axiscore.scale,
          *zoom_xac=xaxis->axiscore.zoom_scale, *zoom_yac=yaxis->axiscore.zoom_scale;
  float minf_old, maxf_old;
  double ratio;

  if (pp->ifzoom!=PLOT_ZOOM) {
   return;
   }
  if ((pp->zoom_box.x1==event->x) || (pp->zoom_box.y1==event->y))
    return;
  pp->zoom_box.x2 = event->x;
  pp->zoom_box.y2 = event->y;
  if (pp->zoom_box.x1 <= pp->zoom_box.x2) {
    px1 = pp->zoom_box.x1;
    px2 = pp->zoom_box.x2;
    }
  else {
    px1 = pp->zoom_box.x2;
    px2 = pp->zoom_box.x1;
    }
  if (pp->zoom_box.y1 <= pp->zoom_box.y2) {
    py1 = pp->zoom_box.y1;
    py2 = pp->zoom_box.y2;
    }
  else {
    py1 = pp->zoom_box.y2;
    py2 = pp->zoom_box.y1;
    }
  
  x1 = X1Scale(pw, pp->zoom_box.x1);
  x2 = X1Scale(pw, pp->zoom_box.x2);
  if (x1 > x2) {
    tmp = x1;
    x1 = x2;
    x2 = tmp;
    }
  y1 = Y1Scale(pw, pp->zoom_box.y1);
  y2 = Y1Scale(pw, pp->zoom_box.y2);
  if (y1 > y2) {
    tmp = y1;
    y1 = y2;
    y2 = tmp;
    }
  y21 = Y2Scale(pw, pp->zoom_box.y1);
  y22 = Y2Scale(pw, pp->zoom_box.y2);
  if (y21 > y22) {
    tmp = y21;
    y21 = y22;
    y22 = tmp;
    }
  /*XtVaSetValues((Widget)pw, XtNplotXAxisMax, PlotFloatToArgVal(x2),
    XtNplotXAxisMin, PlotFloatToArgVal(x1),
    XtNplotYAxisMax, PlotFloatToArgVal(y2),
    XtNplotYAxisMin, PlotFloatToArgVal(y1),
    XtNplotY2AxisMax, PlotFloatToArgVal(y22),
    XtNplotY2AxisMin, PlotFloatToArgVal(y21),
    NULL);*/
  
  if ((x1 == x2) || (y1==y2) || (y21==y22)) return;

  i = 8 * sizeof(short);
  for (j = 1; j<i; j++) huge = huge * 2;
  maxf_old = pp->xaxis_max_dft ? (pp->xmax_dft ? pp->raw_bounding_box.xmax :
		 pp->xmax) : pp->xaxis_max;
  minf_old = pp->xaxis_min_dft ? (pp->xmin_dft ? pp->raw_bounding_box.xmin :
		 pp->xmin) : pp->xaxis_min;
  ratio = (maxf_old - minf_old) / (x2 - x1) * pp->layout.x2;
  if (ratio>=huge) {
     /*printf("AxisZoom will overflow. Not zoom\n");*/
     XCopyArea(XtDisplay(pw), pp->pixmap, XtWindow(pw), pp->axis_gc,
       px1, py1, px2-px1+1, py2-py1+1, px1, py1);
     return;
     }
  maxf_old = pp->yaxis_max_dft ? (pp->ymax_dft ? pp->raw_bounding_box.ymax :
		 pp->ymax) : pp->yaxis_max;
  minf_old = pp->yaxis_min_dft ? (pp->ymin_dft ? pp->raw_bounding_box.ymin :
		 pp->ymin) : pp->yaxis_max;
  ratio = (maxf_old - minf_old) / (y2 - y1) * pp->layout.y2;
  if (ratio>=huge) {
     /*printf("AxisZoom will overflow. Not zoom\n");*/
     XCopyArea(XtDisplay(pw), pp->pixmap, XtWindow(pw), pp->axis_gc,
       px1, py1, px2-px1+1, py2-py1+1, px1, py1);
     return;
     }
  if (pp->plot_data2) {
    maxf_old = pp->y2axis_max_dft ?(pp->y2max_dft ?pp->raw_bounding_box.y2max :
		 pp->y2max) : pp->y2axis_max;
    minf_old = pp->y2axis_min_dft ?(pp->y2min_dft ?pp->raw_bounding_box.y2min :
		 pp->y2min) : pp->y2axis_max;
    ratio = (maxf_old - minf_old) / (y22 - y21) * pp->layout.y2;
    if (ratio>=huge) { 
      /*printf("AxisZoom will overflow. Not zoom\n");*/
      XCopyArea(XtDisplay(pw), pp->pixmap, XtWindow(pw), pp->axis_gc,
       px1, py1, px2-px1+1, py2-py1+1, px1, py1);
      return;
      }
    }
  XtVaSetValues((Widget)pp->xaxis, XtNmax, PlotFloatToArgVal(x2),
    XtNplotMaxUseDefault, False, XtNmin, PlotFloatToArgVal(x1),
    XtNplotMinUseDefault, False, NULL);
  XtVaSetValues((Widget)pp->yaxis, XtNmax, PlotFloatToArgVal(y2),
    XtNplotMaxUseDefault, False, XtNmin, PlotFloatToArgVal(y1), 
    XtNplotMinUseDefault, False, NULL);
  XtVaSetValues((Widget)pp->y2axis, XtNmax, PlotFloatToArgVal(y22),
    XtNplotMaxUseDefault, False, XtNmin, PlotFloatToArgVal(y21),
    XtNplotMinUseDefault, False, NULL);
}


 void AxisZoomStart(AtPlotterWidget pw, XButtonPressedEvent *event)
{
  AtPlotterPart *pp = &pw->plotter;

  if (pp->ifzoom == PLOT_NORMAL) {
    pp->layout_bak = pp->layout;
    }
  pp->ifzoom = PLOT_ZOOM;
  pp->zoom_box.x1 = pp->zoom_box.x2 = event->x;
  pp->zoom_box.y1 = pp->zoom_box.y2 = event->y;
}

 void GraphZoomStart(AtPlotterWidget pw, XButtonPressedEvent *event)
{
  AtPlotterPart *pp = &pw->plotter;

  if (pp->ifzoom == PLOT_NORMAL) {
    pp->layout_bak = pp->layout;
    }
  pp->ifzoom = PLOT_SCALE;
  pp->zoom_box.x1 = pp->zoom_box.x2 = event->x;
  pp->zoom_box.y1 = pp->zoom_box.y2 = event->y;
}


 void ZoomMotion(AtPlotterWidget pw, XMotionEvent *event)
{
  AtPlotterPart *pp = &pw->plotter;
  int x1, y1, x2, y2;

  if (pp->ifzoom ==PLOT_NORMAL) {
    return;
    }
  if (abs(pp->zoom_box.x1-event->x)<3 || abs(pp->zoom_box.y1-event->y)<3)
    return;
  if (pp->zoom_box.x1 <= pp->zoom_box.x2) {
    x1 = pp->zoom_box.x1;
    x2 = pp->zoom_box.x2;
    }
  else {
    x1 = pp->zoom_box.x2;
    x2 = pp->zoom_box.x1;
    }
  if (pp->zoom_box.y1 <= pp->zoom_box.y2) {
    y1 = pp->zoom_box.y1;
    y2 = pp->zoom_box.y2;
    }
  else {
    y1 = pp->zoom_box.y2;
    y2 = pp->zoom_box.y1;
    }
  XCopyArea(XtDisplay(pw), pp->pixmap, XtWindow(pw), pp->axis_gc,
     x1, y1, x2-x1+1, y2-y1+1, x1, y1);
  if (pp->zoom_box.x1 <= event->x) {
    x1 = pp->zoom_box.x1;
    x2 = event->x;
    }
  else {
    x1 = event->x;
    x2 = pp->zoom_box.x1;
    }
  if (pp->zoom_box.y1 <= event->y) {
    y1 = pp->zoom_box.y1;
    y2 = event->y;
    }
  else {
    y1 = event->y;
    y2 = pp->zoom_box.y1;
    }
  XDrawRectangle(XtDisplay(pw), XtWindow(pw), pp->axis_gc, 
     x1, y1, x2-x1-1, y2-y1-1);
  pp->zoom_box.x2 = event->x;
  pp->zoom_box.y2 = event->y;
}


 void GraphZoomEnd(AtPlotterWidget pw, XButtonReleasedEvent *event)
{
  AtPlotterPart *pp = &pw->plotter;
  AtPlotterLayout *lp = &(pp->layout);
  AtAxisCoreWidget xaxis=pp->xaxis, yaxis=pp->yaxis;
  float tmp;
  int j, th, w, h, i, x1, x2, y1, y2, left, right, top, bottom;
  double a, b, c, d, left_d, right_d, top_d, bottom_d;
  AtScale *xac=xaxis->axiscore.scale, *yac=yaxis->axiscore.scale,
          *zoom_xac=xaxis->axiscore.zoom_scale, *zoom_yac=yaxis->axiscore.zoom_scale;
  int huge = 1;
  int new_x1, new_y1, new_x2, new_y2;
  double xf1, xf2, yf1, yf2;

  if (pp->ifzoom != PLOT_SCALE) {
    return;
    }
  if ((pp->zoom_box.x1==event->x) || (pp->zoom_box.y1==event->y))
    return;
  pp->zoom_box.x2 = event->x;
  pp->zoom_box.y2 = event->y;
  if (pp->zoom_box.x1 <= pp->zoom_box.x2) {
    x1 = pp->zoom_box.x1;
    x2 = pp->zoom_box.x2;
    }
  else {
    x1 = pp->zoom_box.x2;
    x2 = pp->zoom_box.x1;
    }
  if (pp->zoom_box.y1 <= pp->zoom_box.y2) {
    y1 = pp->zoom_box.y1;
    y2 = pp->zoom_box.y2;
    }
  else {
    y1 = pp->zoom_box.y2;
    y2 = pp->zoom_box.y1;
    }
 
  if ((y2 == y1) || (x2 ==x1)) return;
  w = pw->core.width;
  h = pw->core.height;
  th = pp->shadow_thickness;
  a = (double)(h-2*th) / (y2 - y1);
  b = (double)(th - y1 * a);
  c = (double)(w-2*th) / ( x2 - x1);
  d = (double)(th - x1 * c);
  left_d = th*c + d - th;
  right_d = w - th - (c * (w-th) + d);
  top_d = th*a + b - th;
  bottom_d = h - th - (a *(h-th) + b);
  left = th*c + d - th;
  right = w - th - (c * (w-th) + d);
  top = th*a + b - th;
  bottom = h - th - (a *(h-th) + b);
  
  /* if the zoom will overflow, no zoom */
  i = 8 * sizeof(short);
  for (j = 1; j<i; j++) huge = huge * 2;
  d = (double)left;
  c = (double)(pw->core.width - right - d)/pw->core.width;
  b = (double)top;
  a = (double)(pw->core.height - bottom - b)/pw->core.height;
  /*xf1 = pp->layout.x1 * c + d;
  xf2 = pp->layout.x2 * c + d;
  yf1 = pp->layout.y1 * a + b;
  yf2 = pp->layout.y2 * a + b;*/
  new_x1 = AtScaleUserToPixel(xac, pp->raw_bounding_box.xmin);
  new_x2 = AtScaleUserToPixel(xac, pp->raw_bounding_box.xmax);
  new_y1 = AtScaleUserToPixel(yac, pp->raw_bounding_box.ymin);
  new_y2 = AtScaleUserToPixel(yac, pp->raw_bounding_box.ymax);
  xf1 = new_x1 * c + d;
  xf2 = new_x2 * c + d;
  yf1 = new_y1 * a + b;
  yf2 = new_y2 * a + b;

  if ((xf1<-huge) || (xf2>huge) || (yf1<-huge) || (yf2>huge)) {
    /*printf("GraphZoom will overflow. Not zoom\n");*/
    XCopyArea(XtDisplay(pw), pp->pixmap, XtWindow(pw), pp->axis_gc,
       x1, y1, x2-x1+1, y2-y1+1, x1, y1);
    return;
    }
  
  XtVaSetValues((Widget)pw, XtNplotGraphMarginLeft, left,
    XtNplotGraphMarginRight, right, XtNplotGraphMarginTop, top,
    XtNplotGraphMarginBottom, bottom, NULL);
}

 void Reset(AtPlotterWidget pw, XEvent *event)
{
  AtPlotterPart *pp = &pw->plotter;
  AtAxisCoreWidget xaxis, yaxis;
  AtAxisCorePart *xac, *yac;
  float x1, x2, y1, y2;
  Arg args[10];
  int n;

  if (pp->ifzoom == PLOT_NORMAL) return;
  if (XtWindow(pw))
	  pp->layout_required = True;
  
  n = 0;
  if (!pp->xaxis_max_dft) {
    XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->xaxis_max)); n++;
    XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
    }
  else if (!pp->xmax_dft) {
    XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->xmax)); n++;
    XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
    }
  else {
    XtSetArg(args[n], XtNplotMaxUseDefault, True); n++;
    }
  if (!pp->xaxis_min_dft) {
    XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->xaxis_min)); n++;
    XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
    }
  else if (!pp->xmin_dft) {
    XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->xmin)); n++;
    XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
    }
  else {
    XtSetArg(args[n], XtNplotMinUseDefault, True); n++;
    }
  XtSetValues((Widget)pp->xaxis, args, n);

  n = 0;
  if (!pp->yaxis_max_dft) {
    XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->yaxis_max)); n++;
    XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
    }
  else if (!pp->ymax_dft) {
    XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->ymax)); n++;
    XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
    }
  else {
    XtSetArg(args[n], XtNplotMaxUseDefault, True); n++;
    }
  if (!pp->yaxis_min_dft) {
    XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->yaxis_min)); n++;
    XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
    }
  else if (!pp->ymin_dft) {
    XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->ymin)); n++;
    XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
    }
  else {
    XtSetArg(args[n], XtNplotMinUseDefault, True); n++;
    }
  XtSetValues((Widget)pp->yaxis, args, n);

  n = 0;
  if (!pp->y2axis_max_dft) {
    XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->y2axis_max)); n++;
    XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
    }
  else if (!pp->y2max_dft) {
    XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->y2max)); n++;
    XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
    }
  else {
    XtSetArg(args[n], XtNplotMaxUseDefault, True); n++;
    }
  if (!pp->y2axis_min_dft) {
    XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->y2axis_min)); n++;
    XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
    }
  else if (!pp->y2min_dft) {
    XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->y2min)); n++;
    XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
    }
  else {
    XtSetArg(args[n], XtNplotMinUseDefault, True); n++;
    }
  XtSetValues((Widget)pp->y2axis, args, n);

  XtVaSetValues((Widget)pw, XtNplotGraphMarginBottomUseDefault, True,
     XtNplotGraphMarginTopUseDefault, True,
     XtNplotGraphMarginLeftUseDefault, True,
     XtNplotGraphMarginRightUseDefault, True,
     NULL);
     /*XtNplotXMinUseDefault, True,
     XtNplotXMaxUseDefault, True,
     XtNplotYMaxUseDefault, True,
     XtNplotYMinUseDefault, True,
     XtNplotY2MaxUseDefault, True,
     XtNplotY2MinUseDefault, True,
     XtNplotXAxisMinUseDefault, True,
     XtNplotXAxisMaxUseDefault, True,
     XtNplotYAxisMaxUseDefault, True,
     XtNplotYAxisMinUseDefault, True,
     XtNplotY2AxisMaxUseDefault, True,
     XtNplotY2AxisMinUseDefault, True,
     NULL);*/
  pp->ifzoom = PLOT_NORMAL;
} /*end of reset*/

/*
 void ScaleStart(AtPlotterWidget pw, XButtonPressedEvent *event)
{
  AtPlotterPart *pp = &pw->plotter;
  pp->zoom_box.x1 = event->x;
  pp->zoom_box.y1 = event->y;
  if (pp->ifzoom == PLOT_NORMAL) {
    pp->layout_bak = pp->layout;
    }
  pp->ifzoom = PLOT_SCALE;
}


 void ScaleMotion(AtPlotterWidget pw, XMotionEvent *event)
{
  AtPlotterPart *pp = &pw->plotter;
  AtPlotterLayout *lp = &(pp->layout);
  int i, fac, scale, w, h;
  int top, bottom, left, right;
  double ratio;

  if (pp->ifzoom != PLOT_SCALE) return;
  if (event->y < pp->zoom_box.y1)
    if ((lp->width < 50) || (lp->height < 50)) return;
  XtVaGetValues((Widget)pw, XtNplotGraphMarginLeft, &left,
    XtNplotGraphMarginRight, &right, XtNplotGraphMarginTop, &top,
    XtNplotGraphMarginBottom, &bottom, NULL);
  w = pw->core.width - pp->shadow_thickness*2;
  h = pw->core.height - pp->shadow_thickness *2;
  ratio = (double)(10 * h )/ w;
  *w = abs(lp->x2 - lp->x1);
  h = abs(lp->y1 - lp->y2);*
  if (event->y < pp->zoom_box.y1) {
    left += 10; top +=ratio; bottom += ratio; right += 10;
    }
  else {left -=10; top -=ratio; bottom -= ratio; right -= 10; }
  XtVaSetValues((Widget)pw, XtNplotGraphMarginLeft, left,
    XtNplotGraphMarginRight, right, XtNplotGraphMarginTop, top,
    XtNplotGraphMarginBottom, bottom, NULL);
}
*/

