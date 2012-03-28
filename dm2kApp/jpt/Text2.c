/*************************************************************************
* Jpt Jefferson Lab Plotting Toolkit 
*               Version 1.0 
* Copyright (c) 1998 Southeastern Universities Research Association,
*               Thomas Jefferson National Accelerator Facility
*
* This software was developed under a United States Government license
* described in the NOTICE file included as part of this distribution.
* Author:     G.Lei
**************************************************************************/ 

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>

#include "Text2.h"
#include "PlotterP.h"
#include "Plotter.h"
#include "DataHandle.h"
#include "XYPlotP.h"
#include "Scale.h"
#include "XYPlot.h"
#include "AxisCoreP.h"

#define CONSTRAINTS(cw) \
     ((AtPlotterConstraints)(((Widget) (cw))->core.constraints))

#define ISDISPLAYED(cw)  (CONSTRAINTS(cw)->plotter.displayed)

static int draw_region P((AtPlotterWidget, TextHandle *));
static int PlotterGetLineValue P((Widget w, int dataset, int set, int point,
      Position *x, Position *y));
static int PlotterGetDataValue P((Widget w, int dataset, int set, int point,
      float fy, Position *x, Position *y));
extern int PlotTextGetPosition P((Widget, TextHandle *));
extern int PlotTextCalc P((Widget w, TextHandle *th));
extern PlotTextHandle PlotTextCreate P((Widget, PlotTextDesc *));
extern int PlotTextDetail P((Widget, PlotTextHandle, PlotTextDesc *));
extern void PlotTextUpdate P((Widget, PlotTextHandle, PlotTextDesc *));
extern int PlotTextDetach P((Widget, PlotTextHandle));
extern int PlotTextAttach P((Widget, PlotTextHandle));
extern void PlotTextDestroy P((Widget, PlotTextHandle));
extern int PlotGetTextHandles P((Widget graph, PlotTextHandle **list));
extern int PlotTextDraw P((Widget, TextHandle *));

static int draw_region(AtPlotterWidget w, TextHandle *th)
{
  Region region;
  XRectangle rectangle;
  AtPlotterPart *pp = &(w->plotter);

  rectangle.x = (th->x1 < th->x) ? th->x1 : th->x;
  rectangle.y = (th->y1 < th->y) ? th->y1 : th->y;
  if (th->x1 < th->x)  rectangle.width = th->x - th->x1 +1 + th->width;
    else if ((th->x1 - th->x) > th->width) rectangle.width = th->x1 - th->x +1;
      else rectangle.width = th->width;
  if (th->y1 < th->y) rectangle.height = 1 + th->y - th->y1 + th->height;
    else if ((th->y1 - th->y)>th->height) rectangle.height = 1 + th->y1 - th->y;
      else rectangle.height = th->height;
  region = XCreateRegion();
  XUnionRectWithRegion(&rectangle, region, region);
  Redraw((AtPlotterWidget)w, XtWindow(w), pp->pixmap, region);
  XCopyArea(XtDisplay(w), pp->pixmap, XtWindow(w), th->gc,
       rectangle.x, rectangle.y, rectangle.width, rectangle.height,
       rectangle.x, rectangle.y);
}



static int PlotterGetLineValue(Widget w, int dataset, int set, int point,
      Position *x, Position *y)
{
  char line_name[7];
  AtXYPlotWidget line;
  float  fy;
  float fx;
  /* will do the sataset soon later */
  sprintf(line_name, "S%1dL%d", dataset, set);
  line = (AtXYPlotWidget)XtNameToWidget(w, line_name);
  if (line) {
    fx = AtXYPlotGetXValue(line, point);
    fy = (float)AtXYPlotGetYValue(line, point);
    PlotFloatToPixel(w, fx, fy, x, y);
  }
  else {
    printf("PlotterGetLineValue WARNING: Invalid dataset or set number\n");
    *x=1; *y=1;
  }
}

static int PlotterGetDataValue(Widget w, int dataset, int set, int point,
      float fy, Position *x, Position *y)
{
  char line_name[7];
  AtXYPlotWidget line;
  float fx;
  
  sprintf(line_name, "S%1dL%d", dataset, set);
  line = (AtXYPlotWidget)XtNameToWidget(w, line_name);
  if (line) {
    fx = (float)AtXYPlotGetXValue(line, point);
    PlotFloatToPixel(w, fx, fy, x, y);
  }  
  else {
      printf("PlotterGetDataValue WARNING: Invalid dataset or set number\n");
      *x=1; *y=1;
  }
}

static int __StrHeight(Widget w, Font font)
{
  XmFontList   fontlist;
  XFontStruct *mfinfo;
  XmString xstr;
  int height;
  
  mfinfo = XQueryFont(XtDisplay(w), font);
  if (!mfinfo) return(16);
/*  fontlist = XmFontListCreate (mfinfo, XmSTRING_DEFAULT_CHARSET);
  xstr=XmStringCreateLtoR("LLLL", XmSTRING_DEFAULT_CHARSET);
  height=XmStringHeight (fontlist, xstr);
  XmStringFree(xstr);
  XFree(mfinfo);
  return(height);
  */
  return(mfinfo->ascent + mfinfo->descent);
}

int PlotTextGetGC(Widget w, TextHandle *th)
{
  int i;
  XGCValues gcv;
  XtGCMask gcv_mask;
  int screen_num;

  if ((!w) || (!th)) return(0);
/* get pixmap and gc for the text */
  th->fore = gcv.foreground = WnColorF(w, th->td.fore_color);
  th->back = gcv.background = WnColorB(w, th->td.back_color);
  gcv_mask = GCForeground | GCBackground;
  if (th->fore == th->back)
    if (strcmp(th->td.back_color, th->td.fore_color)) {
      th->fore = gcv.foreground = WnColorF(w, "Black");
      th->back = gcv.background = WnColorB(w, "White");
      }  
      /*some terminals have not enough colors. To prevent foreground and 
      back ground are not the same color but result in the same pixels, 
      the above four sentences are needed. */

  if (th->td.font) {
    gcv.font = th->td.font;
    gcv_mask |= GCFont;
    }
  th->gc = XtGetGC((Widget)w, gcv_mask, &gcv);
  i = DefaultDepthOfScreen(XtScreen(w));
  th->newp = XCreatePixmap(XtDisplay(w), XtWindow(w), th->width,
	  th->height, i);
}

int PlotTextDraw(Widget w, TextHandle *th)
{
  Pixel fore, back, top_shadow, bottom_shadow;
  int i, str_hei;
  /*XFontStruct  *fontstruct;*/
  XTextItem items;
  XPoint p[5];
  Colormap colormap;
  int screen_num;
  GC top_gc, bottom_gc;
  XGCValues gcv;
  AtPlotterPart *pp;

  if ((!w) || (!th)) return(0);

  pp = &(((AtPlotterWidget)w)->plotter);
  if (!XtDisplay(w) || !(th->newp) || !(th->gc) ) return(-1);
  screen_num = DefaultScreen(XtDisplay(w));
  colormap = XDefaultColormap(XtDisplay(w),screen_num);
  /*if (th->td.font) fontstruct = XQueryFont(XtDisplay(w), th->td.font);
  if (!fontstruct) 
    if (pp->axis_fs && pp->axis_font) {
      th->td.font = pp->axis_font;
      fontstruct = pp->axis_fs;
      }
    else {
      th->td.font = XLoadFont(XtDisplay(w), "fixed");
      fontstruct = XQueryFont(XtDisplay(w), th->td.font);
      }*/
  items.font = th->td.font;
  str_hei = __StrHeight(w, th->td.font);
  fore = th->fore;
  back = th->back;
  gcv.foreground = th->back;
  gcv.background = th->fore;
  top_gc = XtGetGC((Widget)w, GCForeground | GCBackground, &gcv);
  XFillRectangle(XtDisplay(w), th->newp, top_gc, 0, 0, th->width, th->height);
  XtReleaseGC(w, top_gc);
  top_gc=NULL;
  
  for (i=0; i < th->str_num; i++) {
    if (th->td.strings[i])
      XDrawString(XtDisplay(w), th->newp, th->gc, 7 + th->td.border_width,
      7 + th->td.border_width + str_hei*(i+1) + 4*i, th->td.strings[i],
      strlen(th->td.strings[i]));
  }

  /* now draw shadow and highlight for the text area border */
  if (th->td.border==PLOT_BORDER_PLAIN) {
  /*  XSetForeground(XtDisplay(w), th->gc, fore);*/
    XFillRectangle(XtDisplay(w), th->newp, th->gc, 0, 0, th->width, 
	 th->td.border_width);
    XFillRectangle(XtDisplay(w), th->newp, th->gc, 0, th->td.border_width,
	th->td.border_width, th->height - th->td.border_width); 
    XFillRectangle(XtDisplay(w), th->newp, th->gc, 0, 
	th->height - th->td.border_width,
	th->width, th->td.border_width);
    XFillRectangle(XtDisplay(w), th->newp, th->gc, 
	th->width - th->td.border_width,
	0, th->td.border_width, th->height);
  }
  else {
    XmGetColors(XtScreen(w), colormap, back, NULL, &top_shadow, 
	 &bottom_shadow, NULL);
    switch (th->td.border) {
      case PLOT_BORDER_3D_OUT :
      case PLOT_BORDER_ETCHED_OUT :
	gcv.foreground = top_shadow;
	gcv.background = bottom_shadow;
	top_gc = XtGetGC(w, GCForeground |GCBackground, &gcv);
	gcv.foreground = bottom_shadow;
	gcv.background = top_shadow;
	bottom_gc = XtGetGC(w, GCForeground |GCBackground, &gcv);
	break;
      case PLOT_BORDER_3D_IN :
      case PLOT_BORDER_ETCHED_IN:
      default:
	gcv.foreground = top_shadow;
	gcv.background = bottom_shadow;
	bottom_gc = XtGetGC(w, GCForeground |GCBackground, &gcv);
	gcv.foreground = bottom_shadow;
	gcv.background = top_shadow;
	top_gc = XtGetGC(w, GCForeground |GCBackground, &gcv);
	break;
      }
    p[0].x = 0; p[0].y = 0;
    p[1].x = th->width; p[1].y = 0;
    /*p[2].x = th->width -1 - th->td.border_width; 
    p[2].y = th->td.border_width-1;
    p[3].x = th->td.border_width-1; 
    p[3].y = th->td.border_width-1; */
    p[2].x = th->width - th->td.border_width; 
    p[2].y = th->td.border_width;
    p[3].x = th->td.border_width; 
    p[3].y = th->td.border_width;
    p[4].x = 0; p[4].y = 0;
    XFillPolygon(XtDisplay(w), th->newp, top_gc, p, 5, Complex, 
	   CoordModeOrigin);

    p[1].x = 0; p[1].y = th->height;
    p[2].x = th->td.border_width; p[2].y = th->height-th->td.border_width;
    XFillPolygon(XtDisplay(w), th->newp, top_gc, p, 5, Complex, 
	   CoordModeOrigin);

   /* XSetForeground(XtDisplay(w), th->gc, bottom_shadow);*/
    p[0].x = th->width; p[0].y = th->height;
    p[1].x = th->width  - th->td.border_width; 
    p[1].y = th->height  - th->td.border_width;
    p[2].x = th->td.border_width; 
    p[2].y = th->height  - th->td.border_width;
    p[3].x = 0; p[3].y = th->height;
    p[4].x = th->width; p[4].y = th->height;
    XFillPolygon(XtDisplay(w), th->newp, bottom_gc, p, 5, Complex, CoordModeOrigin);

    p[2].x = th->width - th->td.border_width; p[2].y = th->td.border_width;
    p[3].x = th->width; p[3].y = 0;
    XFillPolygon(XtDisplay(w), th->newp, bottom_gc, p, 5, Complex, CoordModeOrigin);
    XtReleaseGC(w, top_gc);
    XtReleaseGC(w, bottom_gc);
  }
}

int PlotTextGetPosition(Widget w, TextHandle *th)
{
  AtPlotterPart *pp = &(((AtPlotterWidget)w)->plotter);
  float xf, yf;

  if ((!w) || (!th)) return(0);
/*calculate the location of the text area */
  switch (th->td.position.pixel.type) {
    case PLOT_TEXT_ATTACH_PIXEL :
      th->x1 = th->td.position.pixel.x;
      th->y1 = th->td.position.pixel.y;
      break;
    case PLOT_TEXT_ATTACH_VALUE:
      /*PlotFloatToPixel(w, th->td.position.value.x, th->td.position.value.y,
	&th->x1, &th->y1);*/
      if (pp->xaxis->axiscore.scale!=NULL) 
	th->x1 = AtScaleUserToPixel(pp->xaxis->axiscore.scale, th->td.position.value.x);
      else th->x1 = 1;
      if (th->td.position.value.dataset == 1) {
	if (pp->yaxis->axiscore.scale != NULL) th->y1 = 
          AtScaleUserToPixel(pp->yaxis->axiscore.scale,th->td.position.value.y);
	else th->y1 = 1;
	}
      else {
	if (pp->y2axis->axiscore.scale != NULL) th->y1 = AtScaleUserToPixel(pp->y2axis->axiscore.scale,th->td.position.value.y);
	else th->y1 = 1;
	}
      break;
    case PLOT_TEXT_ATTACH_DATA:
      /*PlotterGetLineValue(w, th->td.position.data.dataset, 
      th->td.position.data.set, th->td.position.data.point, &th->x1, &th->y1);*/
      if (th->td.position.data.dataset == 1)  {
	if ( (pp->data_widget!=NULL) && (pp->data_widget_num > th->td.position.data.set)) {
          xf = AtXYPlotGetXValue((AtXYPlotWidget)pp->data_widget[th->td.position.data.set], th->td.position.data.point);
          yf = AtXYPlotGetYValue((AtXYPlotWidget)pp->data_widget[th->td.position.data.set], th->td.position.data.point);
          th->x1 = AtScaleUserToPixel(pp->xaxis->axiscore.scale, xf);
	  th->y1 = AtScaleUserToPixel(pp->yaxis->axiscore.scale, yf);
	  }
        else th->x1 = th->y1 = 1;
	}
      else {
	if (pp->data2_widget && (pp->data2_widget_num > th->td.position.data.set)) {
          xf = AtXYPlotGetXValue((AtXYPlotWidget)pp->data2_widget[th->td.position.data.set], th->td.position.data.point);
          yf = AtXYPlotGetYValue((AtXYPlotWidget)pp->data2_widget[th->td.position.data.set], th->td.position.data.point);
          th->x1 = AtScaleUserToPixel(pp->xaxis->axiscore.scale, xf);
	  th->y1 = AtScaleUserToPixel(pp->y2axis->axiscore.scale, yf);
          }
        else th->x1 = th->y1 = 1;
	}
      break;
    case PLOT_TEXT_ATTACH_DATA_VALUE:
      /*PlotterGetDataValue(w, th->td.position.data_value.dataset,
	  th->td.position.data_value.set, th->td.position.data_value.point,
	  th->td.position.data_value.y, &th->x1, &th->y1);*/
      if (th->td.position.data_value.dataset == 1)  {
	if (pp->data_widget && (pp->data_widget_num > th->td.position.data_value.set)) {
          xf = AtXYPlotGetXValue((AtXYPlotWidget)pp->data_widget[th->td.position.data_value.set], th->td.position.data.point);
          th->x1 = AtScaleUserToPixel(pp->xaxis->axiscore.scale, xf);
	  th->y1 = AtScaleUserToPixel(pp->yaxis->axiscore.scale, th->td.position.data_value.y);
	  }
        else th->x1 = th->y1 = 1;
	}
      else {
	if (pp->data2_widget && (pp->data2_widget_num > th->td.position.data_value.set)) {
          xf = AtXYPlotGetXValue((AtXYPlotWidget)pp->data2_widget[th->td.position.data_value.set], th->td.position.data.point);
          th->x1 = AtScaleUserToPixel(pp->xaxis->axiscore.scale, xf);
	  th->y1 = AtScaleUserToPixel(pp->y2axis->axiscore.scale, th->td.position.data_value.y);
	  }
        else th->x1 = th->y1 = 1;
	}

  }

  switch (th->td.anchor) {
    case PLOT_ANCHOR_NORTH:
      th->x = th->x1 - th->width/2;
      th->y = th->y1 - th->td.offset - th->height;
      th->x2 = th->x1;
      th->y2 = th->y1 - th->td.offset;
      break;
    case PLOT_ANCHOR_SOUTH:
      th->x = th->x1 - th->width/2;
      th->y = th->y1 + th->td.offset;
      th->x2 = th->x1;
      th->y2 = th->y;
      break;
    case PLOT_ANCHOR_EAST:
      th->x = th->x1 + th->td.offset;
      th->y = th->y1 - th->height/2;
      th->x2 = th->x;
      th->y2 = th->y1;
      break;
    case PLOT_ANCHOR_WEST:
      th->x = th->x1 - th->td.offset - th->width;
      th->y = th->y1 - th->height/2;
      th->x1 = th->x1 - th->td.offset;
      th->y2 = th->y1;
      break;
    case PLOT_ANCHOR_NORTHEAST:
      th->x = th->x1 + (int)(th->td.offset * 0.7);
      th->y = th->y1 - th->height - (int)(th->td.offset * 0.7);
      th->x2 = th->x;
      th->y2 = th->y - th->height;
      break;
    case PLOT_ANCHOR_NORTHWEST:
      th->x = th->x1 - (int)(th->td.offset * 0.7) - th->width;
      th->y = th->y1 - th->height - (int)(th->td.offset * 0.7);
      th->x2 = th->x + th->width;
      th->y2 = th->y + th->height;
      break;
    case PLOT_ANCHOR_SOUTHEAST:
      th->x = th->x1 + (int)(th->td.offset * 0.7);
      th->y = th->y1 + (int)(th->td.offset * 0.7);
      th->x2 = th->x;
      th->y2 = th->y;
      break;
    case PLOT_ANCHOR_SOUTHWEST:
      th->x = th->x1 - (int)(th->td.offset * 0.7)- th->width;
      th->y = th->y1 + (int)(th->td.offset * 0.7);
      th->x2 = th->x + th->width;
      th->y2 = th->y;
      break;
    case PLOT_ANCHOR_HOME:
    case PLOT_ANCHOR_BEST:
    default:
      th->x = th->x1 - th->width/2;
      th->y = th->y1 - th->height/2;
      th->x2 = th->x1;
      th->y2 = th->y1;
  }

  th->positioned = True;
} /* end of PlotTextGetPosition */
  
  
 int PlotTextCalc(Widget w, TextHandle *th)
 {
  unsigned int i;
  Dimension lxl = 0, str_hei=0;
  XTextItem items;
  XFontStruct *fontstruct;

  if ((!w) || (!th)) return(0);
/* get the width and height of the text area */
  fontstruct = XQueryFont(XtDisplay(w), th->td.font);
  items.font= th->td.font;
  items.delta=0;
  for (i=0; i < th->str_num; i++) {
    items.chars= th->td.strings[i];
    items.nchars=th->td.strings[i] ? strlen(th->td.strings[i]) : 0;
    if (fontstruct)
        lxl =XTextWidth(fontstruct, items.chars, items.nchars); 
    else lxl=items.nchars * th->td.psfont_size; /* not good idea */
    if (lxl > th->width) th->width = lxl;
  }
  str_hei = __StrHeight(w, th->td.font);
  th->height = (th->str_num - 1 )*4 + 8*2 + th->str_num * str_hei + 
	       2 * th->td.border_width;
  th->width += 16 + 2 * th->td.border_width;
}

/* When PlotTextCreate and PlotTextAttach, th->attached = True, that means 
   the text is supposed to be displayed on the plotter widget. 
   When PlotTextDetach, th->attached = False, means text is supposed not to 
   be displayed on the plotter widget. 
   When text is really displayed on the plotter, th->display_state = True.
   PlotTextAttach and PlotTextDetach will first check "display_state" to 
   decide whether to do the attach or detach really.
   Plotter.c:Redraw will check the "attached" of the text to decide whether
   to draw the text on the plotter.
   PlotTextCreate uses the "pp->displayed" of the plotter to decide if it's 
   the right time to set position for the text_area and draw the strings 
   and borders on the text_area.  
*/

PlotTextHandle PlotTextCreate(Widget w, PlotTextDesc *text)
{ 
  Pixel fore, back, top_shadow, bottom_shadow;
  TextHandle * th;
  unsigned int i, str_num = 0;
  Position x, y, xoff, yoff;
  Dimension height = 0, width = 0, lxl = 0, str_hei=0;
  XTextItem items;
  XFontStruct  *fontstruct;
  XGCValues gcv;
  GC gc;
  AtPlotterPart *pp;
  TextList **new;
  Colormap colormap;
  int screen_num;
  XPoint p[5];
  Display * display = XtDisplay(w);

  if ((!w) || (!text)) return(NULL);
  th=(TextHandle *)malloc(sizeof(TextHandle));
  if (!th) {
    printf("PlotTextCreate: cannot malloc enough space for TextHandle\n");
    return(NULL);
    }
  pp = &(((AtPlotterWidget)w)->plotter);
  screen_num = DefaultScreen(XtDisplay(w));
  colormap = XDefaultColormap(XtDisplay(w),screen_num);

/* get the width and height of the text area */
  if (text->strings) {
    while (text->strings[str_num]) str_num++; /*get string quantity*/
    th->td.strings = (char **)calloc(str_num, sizeof(char *));
    if (!th->td.strings) {
      printf("PlotTextCreate: cannot alloc enough memory for text strings\n");
      exit(0);
      }
    for (i=0; i<str_num; i++) {
      th->td.strings[i] = (char *)XtNewString(text->strings[i]);
      }
    }
  else th->td.strings = NULL;
/*  fontstruct = XQueryFont(XtDisplay(w), text->font);
  
  if (text->strings) {
    str_num = 0;
    while (text->strings[str_num]) str_num++; 
    th->td.strings = (char **)calloc(str_num, sizeof(char *));
    if (!th->td.strings) {
      printf("PlotTextCreate: cannot alloc enough memory for text strings\n");
      exit(0);
    }
    items.font=text->font;
    items.delta=0;
    for (i=0; i<str_num; i++) {
      th->td.strings[i] = (char *)XtNewString(text->strings[i]);
      items.chars= text->strings[i];
      items.nchars=text->strings[i] ? strlen(text->strings[i]) : 0;
      if (fontstruct)
        lxl =XTextWidth(fontstruct, items.chars, items.nchars); 
      else lxl=items.nchars * text->psfont_size; 
      if (lxl > th->width) th->width = lxl;
    }
  }
  th->str_num = str_num;
  str_hei = __StrHeight(w, text->font);
  th->height = (str_num - 1 )*4 + 8*2 + str_num * str_hei + 
	       2 * text->border_width;
  th->width += 16 + 2 * text->border_width;
*/
  /*make a copy of the textdesc */
  th->attached = False;
  th->positioned = False;
  th->display_state = UNDISPLAY;
  th->str_num = str_num;
  th->td.position=text->position;
  th->td.anchor = text->anchor;
  th->td.offset = text->offset;
  th->td.connected = text->connected;
  th->td.adjust = text->adjust;
  th->td.fore_color = (char *)XtNewString(text->fore_color);
  th->td.back_color = (char *)XtNewString(text->back_color);
  th->td.border = text->border;
  th->td.border_width = text->border_width;
  th->td.font = text->font;
  th->td.psfont = (char *)XtNewString(text->psfont);
  th->td.psfont_size = text->psfont_size;
  th->display_state = UNDISPLAY;
  th->positioned = False;
  th->attached = False;
  th->width = th->height = 0;
 
 /*calculate the location of the text area */
 /* switch (text->position.pixel.type) {
    case PLOT_TEXT_ATTACH_PIXEL :
      th->x = text->position.pixel.x;
      th->y = text->position.pixel.y;
      break;
    case PLOT_TEXT_ATTACH_VALUE:
      PlotFloatToPixel(w, text->position.value.x, text->position.value.y,
	&th->x, &th->y);
      break;
    case PLOT_TEXT_ATTACH_DATA:
      PlotterGetLineValue(w, text->position.data.dataset, 
	text->position.data.set, text->position.data.point, &th->x, &th->y);
      break;
    case PLOT_TEXT_ATTACH_DATA_VALUE:
      PlotterGetDataValue(w, text->position.data_value.dataset,
	  text->position.data_value.set, text->position.data_value.point,
	  text->position.data_value.y, &th->x, &th->y);

  }

  switch (text->anchor) {
    case PLOT_ANCHOR_NORTH:
      th->x += -th->width/2;
      th->y += - text->offset - th->height;
      break;
    case PLOT_ANCHOR_SOUTH:
      th->x += - th->width/2;
      th->y += text->offset;
      break;
    case PLOT_ANCHOR_EAST:
      th->x += + text->offset;
      th->y += - th->height/2;
      break;
    case PLOT_ANCHOR_WEST:
      th->x += - text->offset - th->width;
      th->y += - th->height/2;
      break;
    case PLOT_ANCHOR_NORTHEAST:
      th->x += + (int)(text->offset * 0.7);
      th->y += - th->height - (int)(text->offset * 0.7);
      break;
    case PLOT_ANCHOR_NORTHWEST:
      th->x += - (int)(text->offset * 0.7) - th->width;
      th->y += - th->height - (int)(text->offset * 0.7);
      break;
    case PLOT_ANCHOR_SOUTHEAST:
      th->x += (int)(text->offset * 0.7);
      th->y += (int)(text->offset * 0.7);
      break;
    case PLOT_ANCHOR_SOUTHWEST:
      th->x += (int)(text->offset * 0.7)- th->width;
      th->y += (int)(text->offset * 0.7);
      break;
    case PLOT_ANCHOR_HOME:
    case PLOT_ANCHOR_BEST:
    default:
      th->x += - th->width/2;
      th->y += - th->height/2;
  }
  */

/* create a pixmap and draw the text on it */
  /*th->fore = gcv.foreground = WnColorD(text->fore_color, XtDisplay(w));
  th->back = gcv.background = WnColorD(text->back_color, XtDisplay(w));
  th->gc = XtGetGC((Widget)w, GCForeground | GCBackground, &gcv);
  i = DefaultDepthOfScreen(XtScreen(w));
  if (!XtWindow(w)) {
    th->state = TEXT_NO_WINDOW;
  }
  else {
    th->state = TEXT_OFF;
    th->newp = XCreatePixmap(XtDisplay(w), XtWindow(w), th->width,
	  th->height, i);
    PlotTextDraw(w, th);
  } */

  /* link to the text list of potter widget parent */
  new = &(pp->text_list);
  while ((TextList *)*new) {
    new=(TextList **)&((*new)->next);
  }
  *new=(TextList *)malloc(sizeof(TextList));
  (*new)->th=(TextHandle *)th;
  (*new)->next=NULL;
  
  /* If the jpt plotter is displayed, display the text area*/ 
  if (XtIsRealized(w)) {
    PlotTextCalc(w, th);
    PlotTextGetGC(w, th);
    PlotTextDraw(w, th);
    }
  if (pp->displayed) {
    PlotTextGetPosition(w, th); 
    XCopyArea(XtDisplay(w), th->newp, XtWindow(w), th->gc, 0, 0,
	  th->width, th->height, th->x, th->y);
    XCopyArea(XtDisplay(w), th->newp, pp->pixmap, th->gc, 0, 0,
	  th->width, th->height, th->x, th->y);
    /*XDrawLine(XtDisplay(w), XtWindow(w), th->gc, th->x1, th->y1, th->x2, 
       th->y2);
    XDrawLine(XtDisplay(w), pp->pixmap, th->gc, th->x1,th->y1,th->x2,th->y2);*/
    th->attached = True;
    th->display_state = DISPLAYED;
    PlotterDrawXMarker(w);
    }
 
  /*th->state = TEXT_OFF;
  if (ISDISPLAYED(w)) {
    th->state = TEXT_ON;
    XCopyArea(XtDisplay(w), XtWindow(w), th->oldp, th->gc, th->x, th->y, 
	  th->width, th->height, 0, 0);
    XCopyArea(XtDisplay(w), th->newp, XtWindow(w), th->gc, 0, 0,
      th->width, th->height, th->x, th->y);
  }*/

  return((PlotTextHandle)th);
} /* end of PlotTextCreate */
 
 
 int PlotTextDetail(Widget w, PlotTextHandle th, PlotTextDesc *td)
{
}

void PlotTextUpdate(Widget w, PlotTextHandle th, PlotTextDesc *td)
{
 
}

int PlotTextDetach(Widget w, PlotTextHandle th)
{
  TextHandle *th1;
  XExposeEvent ev;
  AtPlotterWidget pw;
  AtPlotterPart *pp;

  if ((!th) || (!w)) return(0);
  pw = (AtPlotterWidget)w;
  pp = &pw->plotter;

  th1 = (TextHandle *)th;
  th1->attached = False;

 th1->attached = False;
 if (th1->display_state == DISPLAYED) {
   if (!th1->positioned) PlotTextGetPosition(w, th1);
   draw_region((AtPlotterWidget)w, th1);
   /*XCopyArea(XtDisplay(w), pp->pixmap, XtWindow(w), th1->gc,
       th1->x, th1->y, th1->width, th1->height, th1->x, th1->y); */
   th1->display_state = TO_DISPLAY;
   PlotterDrawXMarker(w);
   }

 /*if (th1->state == TEXT_ON) {
    th1->state = TEXT_OFF;
    if (!pp->pixmap) printf("TextDetach: null pixmap of plotter\n");
    draw_region((AtPlotterWidget)w, th1);
    XCopyArea(XtDisplay(w), pp->pixmap, XtWindow(w), th1->gc, 
      th1->x, th1->y, th1->width, th1->height, th1->x, th1->y);
   }*/
   /* draw_region(w, th1);
    ev.type = Expose;
    ev.display = XtDisplay(pw);
    ev.window = XtWindow(pw);
    ev.x = ev.y = ev.width = ev.height = ev.count = 0;
    XSendEvent(XtDisplay(pw), XtWindow(pw), False, 0, (XEvent *) &ev);
    pw->plotter.expose_requested = True;
    XCopyArea(XtDisplay(w), th1->oldp, XtWindow(w), th1->gc, 0, 0,
      th1->width, th1->height, th1->x, th1->y); 
   */
} 

int PlotTextAttach(Widget w, PlotTextHandle th)
{
  TextHandle *th1;
  AtPlotterPart *pp;
  
  if ((!w) || (!th)) return(0);
  th1 = (TextHandle *)th;
  pp = &(((AtPlotterWidget)w)->plotter);
  /*
  TextList **new; 
  new = &(pp->text_list);
  while ((TextList *)*new) {
    new=(TextList **)&((*new)->next);
  }
  *new=(TextList *)malloc(sizeof(TextList));
  (*new)->th=(TextHandle *)th;
  (*new)->next=NULL;
 */

  /*if ((XtIsManaged(w)) && (XtIsRealized(w))) 
    if (th1->state == TEXT_OFF) {
      th1->state = TEXT_ON;
      XCopyArea(XtDisplay(w), th1->newp, XtWindow(w), th1->gc, 0, 0,
        th1->width, th1->height, th1->x, th1->y);
    }*/
  
  th1->attached = True;
  if (th1->display_state == TO_DISPLAY) {
    th1->display_state = DISPLAYED;
    XCopyArea(XtDisplay(w), th1->newp, XtWindow(w), th1->gc, 0, 0,
	    th1->width, th1->height, th1->x, th1->y);
    XCopyArea(XtDisplay(w), th1->newp, pp->pixmap, th1->gc, 0, 0,
	    th1->width, th1->height, th1->x, th1->y);
    if (th1->td.connected) {
	XDrawLine(XtDisplay(w), pp->pixmap, th1->gc, th1->x1,
	    th1->y1, th1->x2, th1->y2); 
        XDrawLine(XtDisplay(w), XtWindow(w), th1->gc, th1->x1,
	    th1->y1, th1->x2, th1->y2); 
        }
    PlotterDrawXMarker(w);
    } 
}
 
 void PlotTextDestroy(Widget w, PlotTextHandle th)
 {
   TextHandle *th1 = (TextHandle *)th;

   if (w && th1) {
     if (th1->gc) XtReleaseGC(w, th1->gc);
     if (th1->newp) XFreePixmap(XtDisplay(w), th1->newp);
     XtFree(th);
     th = NULL;
     }
 }
 
 int PlotGetTextHandles(Widget graph, PlotTextHandle **list)
{

}

