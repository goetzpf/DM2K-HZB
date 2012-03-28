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
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "At.h"
#include "DataHandle.h"
#include "PlotterP.h"
#include "AxisCoreP.h"
#include "XYPlotP.h"
#include "XYLinePlotP.h"

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

#define NaN 0x200000

int PlotDataGetLastPoint P((PlotDataHandle hData, int set));
int PlotDataSetLastPoint P((PlotDataHandle hData, int set, int npoints));
char ** PlotDupStrings(char **);
PlotDataStyle ** PlotDupDataStyles(PlotDataStyle **);
void PlotDestroyData(PlotData *, Boolean);
void PlotFreeStrings(char **);
void PlotFreeDataStyles(PlotDataStyle **);
XtArgVal PlotFloatToArgVal(float value);
int PlotFloatToPixel(Widget graph, float xf, float yf, Position *xp, 
      Position *yp);
PlotRegion PlotMap(Widget, int, int, int, PlotMapResult *);
PlotData *PlotMakeData(PlotDataType, int, int, int);
PlotData *PlotMakeDataFromFile(char *filename, char *errbuf);
int PlotterCreatePlot P((Widget w, PlotData *hData, int));
void PlotterInitDefaultColor(Widget w);
int PlotSetLineStyle(Widget w, PlotDataStyle *); 
Pixel ColorNameToPixel(Widget w, char * cname);
void PlotSetNthDataStyle(Widget, int, PlotDataStyle *);
void PlotSetNthDataStyle2(Widget, int, PlotDataStyle *);
PlotData *plotCopyPlotData(PlotData *plot_data);


int PlotterDrawXMarker(Widget w)
{
     AtPlotterPart *pp = &(((AtPlotterWidget)w)->plotter);
     AtAxisCoreWidget xpw = (AtAxisCoreWidget)XtNameToWidget(w, "x_axis");
     float newx;
     int x;

     if (pp->xmarker_show) {
       newx = pp->xmarker;
       x =  AtScaleUserToPixel(xpw->axiscore.scale, newx);
       if (x<=pp->layout.x1) {
	 x = pp->layout.x1+2;
	 pp->xmarker = AtScalePixelToUser(xpw->axiscore.scale, x);
	 }
       if (x>=pp->layout.x2) {
	 x = pp->layout.x2-2;
	 pp->xmarker = AtScalePixelToUser(xpw->axiscore.scale, x);
	 }
       if (pp->marker_gc)
       XDrawLine(XtDisplay(w), XtWindow(w), pp->marker_gc, 
	 x, pp->layout.y1, x, pp->layout.y2);
       else XDrawLine(XtDisplay(w), XtWindow(w), pp->axis_gc,
	 x, pp->layout.y1, x, pp->layout.y2);
       }
}

Boolean ifPlotDataChanged(PlotData *old, PlotData *new)
{
  if (old!=new) return (True);
  if (((new->a.type & PLOT_DATA_CHANGED) == PLOT_DATA_CHANGED)
      || ((new->a.type & PLOT_DATA_NEW) == PLOT_DATA_NEW)) return(True);
  if ((old->a.type != new->a.type)||(old->a.nsets!=new->a.nsets)) return(True);
  if ((new->a.type & 1) == PLOT_DATA_ARRAY) {
    /*if ((old->a.npoints != new->a.npoints)||(old->a.data!=new->a.data))*/
    if (old->a.npoints != new->a.npoints)
      return(True);
    }
  else 
    if (old->g.data!=new->g.data) return(True);
  return(False);
}


PlotData *plotCopyPlotData(PlotData *plot_data)
{
  PlotMakeData(plot_data->a.type, plot_data->a.nsets, plot_data->a.npoints, 1);


}


char ** PlotDupStrings(char **s)
{

}


PlotDataStyle ** PlotDupDataStyles(PlotDataStyle **ds)
{

}

void PlotDestroyData(PlotData * data, Boolean all)
{
  int i;

  if (!data) return;
  if ((data->a.type & 1) == PLOT_DATA_ARRAY) {
       if (data->a.data.xp) {
	 free(data->a.data.xp);
         data->a.data.xp = NULL;
	 }
       if (data->a.data.yp) {
         for (i=0; i < data->a.nsets; i++) {
	   free(data->a.data.yp[i]);
	   data->a.data.yp[i] = NULL;
	   }
	 free(data->a.data.yp);
	 data->a.data.yp = NULL;
	 }
       free(data);
       data = NULL;
       }
  else {
       if (data->g.data) {
	 for (i=0; i<data->g.nsets; i++) {
	   free(data->g.data[i].xp);
	   free(data->g.data[i].yp);
	   data->g.data[i].xp = NULL;
	   data->g.data[i].yp = NULL;
	   }
         free(data->g.data);
         data->g.data = NULL;
       }
       free(data);
       data = NULL;
    }
}

void PlotFreeStrings(char **s)
{

}


void PlotFreeDataStyles(PlotDataStyle **ds)
{


}

PlotDataHandle PlotDataCreate(PlotDataType hData, int nsets, int npoints);
int PlotDataGetNPoints(PlotDataHandle hData, int set);
double PlotDataGetXElement(PlotDataHandle hData, int set, int point);
double PlotDataGetYElement(PlotDataHandle hData, int set, int point);
void PlotDataDestroy(PlotDataHandle hData);
int PlotDataSetHole(PlotDataHandle hData, double hole);
int PlotDataSetNPoints(PlotDataHandle hData, int set, int npoints);
int PlotDataSetXElement(PlotDataHandle hData, int set, int point, double x);
int PlotDataSetYElement(PlotDataHandle hData, int set, int point, double y);

PlotDataHandle PlotDataCreate(PlotDataType type, int nsets, int npoints) 
{
   PlotData *handle;

   handle = PlotMakeData(type,nsets,npoints,True);
   handle->a.type |= PLOT_DATA_HANDLE;
   return(handle);
}


int PlotDataGetLastPoint(PlotDataHandle hData, int set)
{

}


int PlotDataSetLastPoint(PlotDataHandle hData, int set, int npoints)
{

}


int PlotDataGetNPoints(PlotDataHandle hData, int set) 
{
    if (hData ==NULL) return(-1);
    if ((hData->a.type & 1)==PLOT_DATA_ARRAY) 
      return(hData->a.npoints);
    else 
      if (set<=hData->g.nsets) return(hData->g.data[set].npoints);
      else return(-1); 
}

int PlotDataGetNSets(PlotDataHandle hData)
{
    if (hData ==NULL) return(-1);
    return(hData->a.nsets);
}

double PlotDataGetXElement(PlotDataHandle hData, int set, int point) 
{
  double x;

    if (hData ==NULL) return(-PLOT_HUGE_VAL);
    if ((hData->a.type & 1)==PLOT_DATA_ARRAY) { 
      if ((point>=0)&&(point<hData->a.npoints))
	x = (double)(hData->a.data.xp[point]);
      else return(-PLOT_HUGE_VAL);
      }
    else { 
      if ((set>=0) && (set<hData->g.nsets))
	if ((point>=0)&&(point<hData->g.data[set].npoints)) 
	  x = (double)(hData->g.data[set].xp[point]);
        else return(-PLOT_HUGE_VAL);
      else return(-PLOT_HUGE_VAL);
      }
    return(x);
}

double PlotDataGetYElement(PlotDataHandle hData, int set, int point) 
{
  double y;

    if (hData ==NULL) return(-PLOT_HUGE_VAL);
    if ((hData->a.type & 1)==PLOT_DATA_ARRAY) { 
      if ((set>=0) && (set<hData->a.nsets)) 
	y = (double)(hData->a.data.yp[set][point]);
      else return(-PLOT_HUGE_VAL);
      }
    else {
      if ((set>=0)&&(set<hData->g.nsets))
	if ((point>=0)&&(point<hData->g.data[set].npoints))
	  y = (double)(hData->g.data[set].yp[point]);
        else return(-PLOT_HUGE_VAL);
      else return(-PLOT_HUGE_VAL);
      }
    return(y);
}

void PlotDataDestroy(PlotDataHandle hData) 
{
    if (hData !=NULL) 
    PlotDestroyData(hData,True);
}

int PlotDataSetHole(PlotDataHandle hData, double hole) 
{
    if (hData ==NULL) return(0);
    hData->g.hole = hole;
    return 1;
}

int PlotDataSetNPoints(PlotDataHandle hData, int set, int npoints) 
{
    if (hData == NULL) return(0);
    if ((hData->a.type & 1)==PLOT_DATA_ARRAY) {
      if (hData->a.npoints < npoints) { 
	int i, j;

	printf("PlotDataSetNPoints: add more points\n");
	hData->a.data.xp = (float *)realloc(hData->a.data.xp, 
	      sizeof(float) * npoints);
	for (j = 0; j<hData->a.nsets; j++)
	  hData->a.data.yp[j] = (float *)realloc(hData->a.data.yp[j],
	      sizeof(float) * npoints);
	for (i=hData->a.npoints; i<npoints; i++) {
	  hData->a.data.xp[i] = PLOT_HUGE_VAL;
	  for (j = 0; j<hData->a.nsets; j++)
            hData->a.data.yp[j][i] = PLOT_HUGE_VAL;
	  }
        }
      hData->a.npoints = npoints;
      }
    else if (set<hData->g.nsets) {
      if (hData->a.npoints < npoints) { 
        int i, j;

        printf("PlotDataSetNPoints: add more points\n");
        hData->g.data[set].xp = (float *)realloc(hData->g.data[set].xp,
	     sizeof(float) * npoints);
        hData->g.data[set].yp = (float *)realloc(hData->g.data[set].yp,
	     sizeof(float) * npoints);
        for (i = hData->g.data[set].npoints; i < npoints; i++) {
	  hData->g.data[set].xp[i] = PLOT_HUGE_VAL;
	  hData->g.data[set].yp[i] = PLOT_HUGE_VAL;
	  }
	}
      hData->g.data[set].npoints = npoints;
      }  else return(0);
      
    hData->a.type |= PLOT_DATA_CHANGED;
    return 1;
}

/*int PlotDataSetNSets(PlotDataHandle hData, int set)
{
    if (hData ==NULL) return(0);
    else {
      hData->a.nsets = set;
      return(1);
      }
}*/

int PlotDataSetXElement(PlotDataHandle hData, int set, int point, double x) 
{
    if (hData ==NULL) return(0);
    if ((hData->a.type & 1)==PLOT_DATA_ARRAY) {
      if ((point>=0) && (point<hData->a.npoints)) {
	hData->a.data.xp[point] = (float)x;
        hData->a.type |= PLOT_DATA_CHANGED;
	}
     /* else if (point==hData->a.npoints) {
	printf("PlotDataSetXElement: add new data\n");
	hData->a.data.xp = (float *)realloc(hData->a.data.xp, 
	      sizeof(float) * point);
	hData->a.data.xp[point] = (float)x;
	hData->a.type |= PLOT_DATA_CHANGED;
	}*/
      else {
	printf("PlotDataSetXElement point %d out of range\n",point);
        return(0);
	}
      }
    else {
      if ((set>=0) && (set<hData->g.nsets)) { 
	if ((point>=0) && (point<hData->g.data[set].npoints)) {
	   hData->g.data[set].xp[point] = (float)x;
           hData->a.type |= PLOT_DATA_CHANGED;
	   }
/*        else if (point==hData->g.data[set].npoints) {
	  hData->g.data[set].xp = (float *)realloc(hData->g.data[set].xp, 
	      sizeof(float) * point);
	  hData->g.data[set].xp[point] = (float)x;
	  hData->g.type |= PLOT_DATA_CHANGED;
	  }*/
	else {
	  printf("PlotDataSetXElement point %d out of range\n",point);
	  return(0);
	  }
	}
      else {
	printf("PlotDataSetXElement set %d out of range\n",set);
        return(0);
	}
      }
    
    return 1;
}

int PlotDataSetYElement(PlotDataHandle hData, int set, int point, double y) 
{
    if (hData ==NULL) return(0);
    if ((hData->a.type & 1)==PLOT_DATA_ARRAY) {
      if ((set>=0) && (set<hData->a.nsets)) {
	if ((point>=0) && (point<hData->a.npoints)) {
	  hData->a.data.yp[set][point] = (float)y;
	  hData->a.type |= PLOT_DATA_CHANGED;
	  }
        /*else if (point==hData->a.npoints) {
	 printf("PlotSetYElement: add new data\n");
	  hData->a.data.yp[set] = (float *)realloc(hData->a.data.yp[set], 
	      sizeof(float) * point);
	  hData->a.data.yp[set][point] = (float)y;
	  hData->a.type |= PLOT_DATA_CHANGED;
	  }*/
        else {
	  printf("PlotDataSetYElement point %d out of range\n",point);
          return(0);
	  }
	}
      else {
	printf("PlotDataSetYElement set %d out of range\n",set);
        return(0);
	}
      }
    else {
      if ((set>=0) && (set<hData->g.nsets)) { 
	if ((point>=0) && (point<hData->g.data[set].npoints)) {
	   hData->g.data[set].yp[point] = (float)y;
           hData->g.type |= PLOT_DATA_CHANGED;
	   }
         /*else if (point==hData->g.data[set].npoints) {
	 printf("PlotSetYElement: add new data\n");
	   hData->g.data[set].yp = (float *)realloc(hData->g.data[set].yp, 
	      sizeof(float) * point);
	   hData->g.data[set].yp[point] = (float)y;
	   hData->g.type |= PLOT_DATA_CHANGED;
	   }*/
	 else {
	   printf("PlotDataSetYElement point %d out of range\n",point);
	   return(0);
	   }
	 }
      else {
	printf("PlotDataSetYElement set %d out of range\n",set);
        return(0);
	}
      }
    return 1;
}

void PlotSetNthDataStyle(Widget w, int n, PlotDataStyle *ds)
{
  Arg args[10];
  int j=0;
  char line_name[8];
  AtXYPlotWidget line;
  AtPlotterWidget pw = (AtPlotterWidget)w;
  AtPlotterPart *pp = &(pw->plotter);
  PlotDataStyle ***pds = &(pp->plot_data_styles);

  if (!w || !ds || (n<0)) return;
  if (pp->ds_num<=n) {
    *pds = (PlotDataStyle **)realloc(pp->plot_data_styles, sizeof(PlotDataStyle *) * (n+1));
    for (j=pp->ds_num; j<n+1; j++) pds[0][j]=NULL;
    pp->ds_num = n+1;
    }
  if (*pds) {
    if (!pds[0][n])
      pds[0][n]=(PlotDataStyle *)malloc(sizeof(PlotDataStyle));
    pds[0][n]->lpat = ds->lpat;
    pds[0][n]->fpat = ds->fpat;
    pds[0][n]->color = XtNewString(ds->color);
    pds[0][n]->width = ds->width;
    pds[0][n]->point = ds->point;
    pds[0][n]->pcolor = XtNewString(ds->pcolor);
    pds[0][n]->psize = ds->psize;
    }

  sprintf(line_name, "S1L%d", n);
  line = (AtXYPlotWidget)XtNameToWidget(w, line_name);
  if (line ) {
    j = 0;
    XtSetArg(args[j], XtNplotType, pp->plot_type); j++;
    if (ds) {
      XtSetArg(args[j], XtNlineWidth, ds->width);  j++; 
      XtSetArg(args[j], XtNplotMarkType, ds->point); j++;
      XtSetArg(args[j], XtNplotMarkSize, ds->psize); j++;
      XtSetArg(args[j], XtNplotLineStyle, ds->lpat); j++; 
      XtSetArg(args[j], XtNplotFillStyle, ds->fpat); j++;
      XtSetArg(args[j], XtNforeground,
        WnColorF(w, ds->color)); j++;
      XtSetArg(args[j], XtNplotMarkColor, 
        WnColorF(w, ds->pcolor)); j++;
      XtSetValues((Widget)line, args, j);
      }
    }
}


void PlotSetNthDataStyle2(Widget w, int n, PlotDataStyle *ds)
{
  Arg args[10];
  int j=0;
  char line_name[8];
  AtXYPlotWidget line;
  AtPlotterWidget pw = (AtPlotterWidget)w;
  AtPlotterPart *pp = &(pw->plotter);
  PlotDataStyle ***pds = &(pp->plot_data_styles2);

  if (!w || !ds || (n<0)) return;
  if (pp->ds2_num<=n) {
    *pds = (PlotDataStyle **)realloc(pp->plot_data_styles2, sizeof(PlotDataStyle *) * (n+1));
    for (j=pp->ds2_num; j<n+1; j++) pds[0][j]=NULL;
    pp->ds2_num = n+1;
    }
  if (*pds) {
    if (!pds[0][n])
      pds[0][n]=(PlotDataStyle *)malloc(sizeof(PlotDataStyle));
    pds[0][n]->lpat = ds->lpat;
    pds[0][n]->fpat = ds->fpat;
    pds[0][n]->color = XtNewString(ds->color);
    pds[0][n]->width = ds->width;
    pds[0][n]->point = ds->point;
    pds[0][n]->pcolor = XtNewString(ds->pcolor);
    pds[0][n]->psize = ds->psize;
    }

  sprintf(line_name, "S2L%d", n);
  line = (AtXYPlotWidget)XtNameToWidget(w, line_name);
  if (line) {
    j = 0;
    XtSetArg(args[j], XtNplotType, pp->plot_type2); j++;
    if (ds) {
      XtSetArg(args[j], XtNlineWidth, ds->width);  j++; 
      XtSetArg(args[j], XtNplotMarkType, ds->point); j++;
      XtSetArg(args[j], XtNplotMarkSize, ds->psize); j++;
      XtSetArg(args[j], XtNplotLineStyle, ds->lpat); j++; 
      XtSetArg(args[j], XtNplotFillStyle, ds->fpat); j++;
      XtSetArg(args[j], XtNforeground,
        WnColorF(w, ds->color)); j++;
      XtSetArg(args[j], XtNplotMarkColor, 
        WnColorF(w, ds->pcolor)); j++;
      XtSetValues((Widget)line, args, j);
      }
    }
}


Pixel WnColorD(String name, Display * display)
{
   XColor col, exact;
   Colormap colmap;
   int screen_num;
   Status sta;
   Pixel white, black;

  if ( (!name) || (!display)) return(1);

   /*col.pixel = 0;*/
   screen_num = DefaultScreen(display);
   colmap = XDefaultColormap(display,screen_num);
   white = WhitePixel(display, screen_num);
   black = BlackPixel(display, screen_num);

   sta = XLookupColor(display, colmap, name, &col,&exact);
   /*if (sta==BadName) {
     printf("ERROR in WnColor: bad color name!\n");
     return(1);
     }

   if (sta==BadColor) {
     printf("ERROR in WnColor: bad colormap!\n");
     return(1);
     }*/
   if (!sta) {
     return(1);
   }

   sta = XAllocColor(display, colmap, &col);
   if (!sta) {
     /*printf("WnColorD: can not allocate pixel for color %s\n", name);*/
     return(exact.pixel);
   }   
   return(col.pixel);
} /* end of WnColor */


XtArgVal PlotFloatToArgVal(float value)
{
  union {
         float f;
	 long   i;
  }xval;
  xval.f = value;
  return((XtArgVal)xval.i);
}


int PlotFloatYToPixel(Widget graph, float f, Position *yp)
{


}


int PlotFloatToPixel(Widget graph, float xf, float yf, Position *xp, 
      Position *yp)
{
  Position x0, y0, x1, y1;
  Dimension edge;
  AtAxisCoreWidget xpw = (AtAxisCoreWidget)XtNameToWidget(graph, "x_axis");
  AtAxisCoreWidget ypw = (AtAxisCoreWidget)XtNameToWidget(graph, "y_axis");
  AtAxisCorePart *xac = &(xpw->axiscore);
  AtAxisCorePart *yac = &(ypw->axiscore);
  float xv0, xv1, yv0, yv1, x, y;

  x0 = xac->axis_segment.x1;
  y0 = xac->axis_segment.y1;
  x1 = xac->tic_segments[1].x1;
  y1 = yac->tic_segments[1].y1;
  xv0 = (float)xac->tic_values[0];
  xv1 = (float)xac->tic_values[1];
  yv0 = (float)yac->tic_values[0];
  yv1 = (float)yac->tic_values[1];
  if (xv1==xv0) *xp = x0;
  else *xp = x0 + (int)((xf-xv0) / (xv1 - xv0) * (x1 - x0));
  if (yv1==yv0) *yp = y0;
  else *yp = y0 + (int)((yf-yv0) / (yv1 - yv0) * (y1 - y0));
}


PlotRegion PlotMap(Widget graph, int yaxis, int pix_x, int pix_y, 
	   PlotMapResult *map)
{
  Position x0, y0, x1, y1;
  Dimension edge;
  AtAxisCoreWidget xpw, ypw;
  AtAxisCorePart *xac, *yac;
  float xv0, xv1, yv0, yv1, x, y;

  map->yaxis=yaxis;
  map->pix_x=pix_x; map->pix_y=pix_y;
  map->x = PLOT_HUGE_VAL;
  map->y = PLOT_HUGE_VAL;

  if (!XtIsRealized(graph) || !XtIsSubclass(graph, atPlotterWidgetClass)) {
    return(PLOT_RGN_NOWHERE);
  }
  xpw = (AtAxisCoreWidget)XtNameToWidget(graph, "x_axis");
  ypw = (AtAxisCoreWidget)XtNameToWidget(graph, "y_axis");
  if (!xpw || !ypw) return(PLOT_RGN_NOWHERE);
  xac = &(xpw->axiscore);
  yac = &(ypw->axiscore);
  
  x0 = xac->axis_segment.x1;
  y0 = xac->axis_segment.y1;
  x1 = xac->tic_segments[1].x1;
  y1 = yac->tic_segments[1].y1;
  xv0 = (float)xac->tic_values[0];
  xv1 = (float)xac->tic_values[1];
  yv0 = (float)yac->tic_values[0];
  yv1 = (float)yac->tic_values[1];
  if (x1==x0) x = xv0;
  else x = xv0 + (xv1 - xv0) / (x1 - x0) * (pix_x - x0);
  if (y1==y0) y = yv0;
  else y = yv0 + (yv1 - yv0) / (y1 - y0) * (pix_y - y0);

  map->x = x;
  map->y = y;
  return(PLOT_RGN_IN_GRAPH);
}

#if 0
/*this is the old PlotMakeData *
PlotData *PlotMakeData(PlotDataType data_type, int nsets, int npoints,
		 int all)
{
  PlotData *data;
  PlotArray *a;
  PlotGeneral *g;
  int i;

  data=(PlotData *)malloc(sizeof(PlotData));
  if (data==NULL) {
    printf("JptMakeData: can not alloc enough memory for JpytData\n");
    return(NULL);
  }

  if (data_type == PLOT_DATA_GENERAL) {
    /*g=(PlotGeneral *)malloc(sizeof(PlotGeneral));
    if (g==NULL) {
      printf("JptMakeData: can not alloc enough memory for JptGeneralData\n");
      return(NULL);
    }*/
    g->type=PLOT_DATA_GENERAL;
    g->hole=PLOT_HUGE_VAL;
    g->nsets=nsets;
    if (all) {
      g->data=(PlotGeneralData *)malloc(sizeof(PlotGeneralData)*nsets);
      if (g->data==NULL) {
	printf("JptMakeData: can't get enough memory for JptGeneralData\n");
	return(NULL);
      }
      for (i=0; i<nsets; i++) {
	g->data[i].npoints=npoints;
	g->data[i].xp=(float *)malloc(sizeof(float)*npoints);
	if (g->data[i].xp==NULL) {
	  printf("JptMakeData: can't get enough memory for JptGeneralData\n");
	  return(NULL);
        }
	g->data[i].yp=(float *)malloc(sizeof(float)*npoints);
	if (g->data[i].xp==NULL) {
          printf("JptMakeData: can't get enough memory for JptGeneralData\n");
	  return(NULL);
	}
      }
    }
    data->g=*g;
    return(data);
  }
  else if (data_type==PLOT_DATA_ARRAY) {
    /*a=(PlotArray *)malloc(sizeof(PlotArray));
    if (a==NULL) {
      printf("JptMakeData: can not alloc enough memory for JptArrayData\n");
      return(NULL);
    }*/
    a->type=PLOT_DATA_ARRAY;
    a->hole=PLOT_HUGE_VAL;
    a->nsets=nsets;
    a->npoints=npoints;
    if (all) {
      a->data.xp=(float *)malloc(sizeof(float)*npoints);
      a->data.yp=(void *)malloc(sizeof(float *)*nsets);
      if ((a->data.yp==NULL) || (a->data.xp==NULL)) {
	printf("JptMakeData: can't get enough memory for JptArrayData\n");
        return(NULL);
      }
      for (i=0; i<nsets; i++) {
	a->data.yp[i]=(float *)malloc(sizeof(float)*npoints);
	if (a->data.yp[i]==NULL) {
          printf("JptMakeData: can't get enough memory for JptArrayData\n");
	  return(NULL);
	}
      }
    }
    data->a=*a;
    return(data);
  }
} *end of JptMakeData*/
#endif

PlotData *PlotMakeData(PlotDataType data_type, int nsets, int npoints,
		 int all)
{
  PlotData *data;
 /* PlotArray *a;
  PlotGeneral *g;*/
  int i;

  data=(PlotData *)calloc(1, sizeof(PlotData));
  if (data==NULL) {
    printf("JptMakeData: can not alloc enough memory for JptData\n");
    exit(-1);
  }

  if ((data_type &1) == PLOT_DATA_GENERAL) {
    data->g.type=PLOT_DATA_GENERAL | PLOT_DATA_NEW;
    data->g.hole=PLOT_HUGE_VAL;
    data->g.nsets=nsets;
    if (all) {
      data->g.data=(PlotGeneralData *)calloc(nsets, sizeof(PlotGeneralData));
      if (data->g.data==NULL) {
	printf("JptMakeData: can't get enough memory for JptGeneralData\n");
	exit(-1);
      }
      for (i=0; i<nsets; i++) {
	data->g.data[i].npoints=npoints;
	data->g.data[i].xp=(float *)calloc(npoints, sizeof(float));
	if (data->g.data[i].xp==NULL) {
	  printf("JptMakeData: can't get enough memory for JptGeneralData\n");
	  exit(-1);
        }
	data->g.data[i].yp=(float *)calloc(npoints, sizeof(float));
	if (data->g.data[i].xp==NULL) {
          printf("JptMakeData: can't get enough memory for JptGeneralData\n");
	  exit(-1);
	}
      }
    }
    return(data);
  }
  else {
    data->a.type=PLOT_DATA_ARRAY | PLOT_DATA_NEW;
    data->a.hole=PLOT_HUGE_VAL;
    data->a.nsets=nsets;
    data->a.npoints=npoints;
    if (all) {
      data->a.data.xp=(float *)calloc(npoints, sizeof(float));
      data->a.data.yp=(void *)calloc(nsets, sizeof(float *));
      if ((data->a.data.yp==NULL) || (data->a.data.xp==NULL)) {
	printf("JptMakeData: can't get enough memory for JptArrayData\n");
        exit(-1);
      }
      for (i=0; i<nsets; i++) {
	data->a.data.yp[i]=(float *)calloc(npoints, sizeof(float));
	if (data->a.data.yp[i]==NULL) {
          printf("JptMakeData: can't get enough memory for JptArrayData\n");
	  exit(-1);
	}
      }
    }
    return(data);
  }
} /*end of JptMakeData*/


void PlotterInitDefaultColor(Widget w)
{
  AtPlotterWidget pw = (AtPlotterWidget)w;
  AtPlotterPart *pp = &(pw->plotter);

  pp->default_color[0]=WnColorF(w, "blue");
  pp->default_color[1]=WnColorF(w, "red");
  pp->default_color[2]=WnColorF(w, "darkgreen");
  pp->default_color[3]=WnColorF(w, "brown");
  pp->default_color[4]=WnColorF(w, "magenta");
  pp->default_color[5]=WnColorF(w, "green");
  /*pp->default_color[6]=WnColorF(w, "black");
  pp->default_color[7]=WnColorB(w, "white");*/
}


PlotData *PlotMakeDataFromFile(char *filename, char *errbuf)
{
  FILE *fp;
  int data_type, nsets, npoints, np;
  float x, y, hole;
  PlotData *data;
  char tmp, T, buf[256], *ch;
  int n, i;

  fp = fopen(filename, "rb");
  if (fp==NULL) {
    printf("can not open file %s \n", filename);
    return(NULL);
    }
  do { fgets(buf, sizeof(buf), fp);}
    while (buf[0]=='#');

  if (!strncmp(buf, "ARRAY", 5)) {
      data_type=PLOT_DATA_ARRAY;
      }
  else if (!strncmp(buf, "GENERAL", 7)) {
      data_type=PLOT_DATA_GENERAL;
      }
      else {
	 printf("Not Plot Formatted Data File\n");
	 return(NULL);
	 };

  data=(PlotData *)calloc(1, sizeof(PlotData));
  if (data==NULL) {
    printf("PlotMakeDataFromFile: cannot alloc enough memory to PlotData\n");
    exit(0);
    }

  if (data_type == PLOT_DATA_ARRAY) {
    sscanf(&buf[6], "%d %d", &nsets, &npoints);
    data->a.data.yp=(void *)calloc(nsets, sizeof(float *));
    if (data->a.data.yp==NULL) {
       printf("PlotMakeDataFromFile: can not alloc enough memory for data\n");
       exit(0);
       }
    for (i=0; i<nsets; i++) {
      data->a.data.yp[i]=(float *)calloc(npoints, sizeof(float));
      if (data->a.data.yp[i]==NULL) {
	printf("PlotMakeDataFromFile: can not alloc enough memory for data\n");
	exit(0);
	}
      }
    data->a.data.xp=(float *)calloc(npoints, sizeof(float));
    if (data->a.data.xp==NULL) {
      printf("PlotMakeDataFrom: can not alloc enough memory for data\n");
      exit(0);
      }
    
    /* T or not */
    tmp='T';
    ch=(char *)strchr(&buf[0], tmp);
    if (ch!=NULL) {
      for (i=0; i<npoints; i++) {
        do {fgets(buf, sizeof(buf), fp); }
	  while ((buf[0]=='#') || (buf[0]==(char)NULL));
	ch=buf;
	while ((!(isdigit(*ch))) && (*ch!='.') && (*ch!=(char)NULL)) {
	  ch++;
	  }
        if (*ch==(char )NULL) {
	  i--;
	  continue;
	  }
	sscanf(buf, "%f", &(data->a.data.xp[i]));
	for (n=0; n<nsets; n++) {
	  do {ch++;} while (*ch!=' ');
	  do {ch++;} while ((*ch!='.')&&(!(isdigit(*ch))));
	  /*do {ch++;} while (*ch==' '); */
	  sscanf(ch, "%f", &(data->a.data.yp[n][i]));
	  }
      }
    } /* *ch=='T' */
    else {
   search_data:    do { fgets(buf, sizeof(buf), fp); } 
	while ((buf[0]=='#') || (buf[0]==(char)NULL));
      ch=buf;
      while ((!(isdigit(*ch))) && (*ch!='.') && (*ch!=(char)NULL)) {
	  ch++;
	  }
	if (*ch==(char)NULL) goto search_data;
      sscanf(ch, "%f", &(data->a.data.xp[0]));
      for (i=1; i< npoints; i++) {
	do {ch++;} while (*ch!=' ');
	do {ch++;} while ((*ch!='.')&&(!(isdigit(*ch))));
	/*do {ch++;} while (*ch==' '); */
	sscanf(ch, "%f", &(data->a.data.xp[i]));
	}
      for (n=0; n<nsets; n++) {
	do { fgets(buf, sizeof(buf), fp); }
	  while ((buf[0]=='#') || (buf[0]==(char)NULL)) ;
        ch=buf;
	while ((!(isdigit(*ch))) && (*ch!='.') && (*ch!=(char)NULL)) {
	  ch++;
	  }
        if (*ch==(char)NULL) {
	  n--;
	  continue;
	  }
	sscanf(ch, "%f", &(data->a.data.yp[n][0]));
	for (i=1; i< npoints; i++) {
	  do {ch++;} while (*ch!=' ');
	  do {ch++;} while ((*ch!='.')&&(!(isdigit(*ch))));
	  /*do {ch++;} while (*ch==' '); */
	  sscanf(ch, "%f", &(data->a.data.yp[n][i]));
	  } /* for npoints */
      } /* for nsets */
    } /* not T */

    /* now make jpt_array */
    data->a.type=PLOT_DATA_ARRAY;
    data->a.hole=PLOT_HUGE_VAL;
    data->a.nsets=nsets;
    data->a.npoints=npoints;
  } /* array data */ 
  
  else { /* GENERAL DATA */
    sscanf(&buf[8], "%d %d", &nsets, &npoints);
    data->g.data=(PlotGeneralData *)calloc(nsets,sizeof(PlotGeneralData));
    if (data->g.data==NULL) {
      printf("PlotMakeDataFromFile: can not alloc enough memory for data\n");
      exit(-1);
      }
    for (n=0; n<nsets; n++) { /* malloc memory for xp and yp */
      data->g.data[n].xp=(float *) calloc(npoints, sizeof(float));
      if (data->g.data[n].xp==NULL) {
	printf("PlotMakeDataFromFile: can not calloc enough memory for data\n");
	exit(-1);
	}
      data->g.data[n].yp=(float *) calloc(npoints, sizeof(float));
      if (data->g.data[n].yp==NULL) {
	printf("PlotMakeDataFromFile: can not malloc memory for data\n");
	exit(-1);
	}
      }
    
    T='T';
    ch=(char *)strchr(buf, (int)(T));
    if (!ch) T=' ';

    /* now read data */
    for (n=0; n<nsets; n++) {
      /* get the real points number of net n */
      do {fgets(buf, sizeof(buf), fp); }
        while ((buf[0]=='#') || (buf[0]==(char)NULL));
      ch=buf;
      while ((!(isdigit(*ch))) && (*ch!=(char)NULL)) {
	  ch++;
	  }
      if (*ch==(char)NULL) {
        n--;
        continue;
        }
     
      sscanf(ch, "%d", &np);
      data->g.data[n].npoints=np;
      if (T=='T') {
        for (i=0; i<np; i++)  {
          do {fgets(buf, sizeof(buf), fp); }
	    while ((buf[0]=='#') || (buf[0]==(char)NULL));
	  ch=buf;
	  while ((*ch!='.')&&(!(isdigit(*ch)))) ch++;
          if (*ch==(char)NULL) {
	    i--;
	    continue;
	    }
	  sscanf(ch, "%f %f", &(data->g.data[n].xp[i]),
			     &(data->g.data[n].yp[i]));
          } /*successfully read adata line */
        } /* if 'T' */
      else {/* if not T */
	fgets(buf, sizeof(buf), fp);
        ch=buf;
        while ((*ch!='.')&&(!(isdigit(*ch)))) ch++;
	sscanf(ch, "%f", &(data->g.data[n].xp[0]));
	for (i=1; i<np; i++) {
	  do {ch++;} while (*ch!=' ');
	  do {ch++;} while ((*ch!='.')&&(!(isdigit(*ch))));
	  sscanf(ch, "%f", &(data->g.data[n].xp[i]));
	  } 
        fgets(buf, sizeof(buf), fp);
	ch=buf;
	while (*ch==' ') ch++;
	sscanf(ch, "%f", &(data->g.data[n].yp[0]));
	for (i=1; i<np; i++) {
	  do {ch++;} while (*ch!=' ');
	  do {ch++;} while ((*ch!='.')&&(!(isdigit(*ch))));
	  sscanf(ch, "%f", &(data->g.data[n].yp[i]));
          } 
        } /* if not T */
      } /* end of general nsets */
	
  /* make jptdata */
  data->g.type=PLOT_DATA_GENERAL;
  data->g.hole=PLOT_HUGE_VAL;
  data->g.nsets=nsets;
  } /* general data */

  data->a.type |= PLOT_DATA_NEW;
  return(data);    
}


Boolean plotIfDataChanged(AtPlotterWidget w, PlotData *hData, PlotData *oldData, int set)
{
 Widget line;
 int num, nsets, npoints, n, i, j;
 char name[7];
 Pixel fore;
 AtPlotterWidget pw = (AtPlotterWidget)w;
 AtPlotterPart *pp = &(pw->plotter);
 char str[17];
 Arg args[16];
 Boolean have_style, changed = False;
 PlotDataStyle     **ds;
 AtXYPlotWidget *widget_p;
 char **labels_p;
 PlotType plot_type;
 AtXYPlotPart *lplot;

 if (!w || (set<0)) return(0);
 
 if (set==1) {
   plot_type = pp->plot_type;
   num = pp->data_widget_num;
   labels_p = pp->set_labels;
   widget_p = (AtXYPlotWidget *)pp->data_widget;
   if (pp->plot_data_styles) {
     have_style = 1;
     ds = pp->plot_data_styles;
     }
   else have_style = 0;
   }
 else { /* set == 2 */
   plot_type = pp->plot_type2;
   num = pp->data2_widget_num;
   labels_p = pp->set_labels2;
   widget_p = (AtXYPlotWidget *)pp->data2_widget;
   if (pp->plot_data_styles2) {
     have_style = 1;
     ds = pp->plot_data_styles2;
     }
   else have_style = 0;
   }

 if (!hData) {
   for (i = 0; i<num; i++) {
      XtDestroyWidget((Widget)widget_p[i]);
      widget_p[i] = NULL;
      }
   if (set == 1) pp->data_widget_num = 0;
   else pp->data2_widget_num = 0;
   return(0);
  }

 if ((hData->a.type & 1) ==PLOT_DATA_ARRAY) {
    nsets=hData->a.nsets;
    npoints=hData->a.npoints;
   }
 else nsets=hData->g.nsets;
 
 if (num < nsets) { 
   if (set==1) {
     pp->data_widget = (Widget *)realloc(pp->data_widget, sizeof(Widget)*(nsets+1));
     widget_p = (AtXYPlotWidget *)pp->data_widget;
     if (!pp->data_widget) {
       printf("cannot get enough memory for plot widgets\n");
       return(0);
     }
   }
   else { /* set == 2 */
     pp->data2_widget = (Widget *)realloc(pp->data2_widget, sizeof(Widget)*(nsets+1));
     widget_p = (AtXYPlotWidget *) pp->data2_widget;
     if (!pp->data2_widget) {
       printf("cannot get enough memory for plot widgets\n");
       return(0);
       }
     }
   for (n=num; n<nsets; n++) {
     sprintf(name, "S%1dL%d", set, n);
     j=0;
     if ((pp->show_legend) && (labels_p))
       if (labels_p[n]) {
	  XtSetArg(args[j], XtNlegendName, (labels_p[n])); j++;
	  }
     XtSetArg(args[j], XtNplotType, plot_type); j++;
     /*XtSetArg(args[j], XtNplotLineType, AtPlotLINEPOINTS); j++;*/
     if (have_style) {
      if (ds[n]) {
        XtSetArg(args[j], XtNlineWidth, ds[n]->width);  j++; 
        XtSetArg(args[j], XtNplotMarkType, ds[n]->point); j++;
        XtSetArg(args[j], XtNplotMarkSize, ds[n]->psize); j++;
        XtSetArg(args[j], XtNplotLineStyle, ds[n]->lpat);j++; 
        XtSetArg(args[j], XtNplotFillStyle, ds[n]->fpat); j++;
        if (ds[n]->color) {
	  XtSetArg(args[j], XtNforeground,
	    WnColorF((Widget)w, ds[n]->color)); j++;
	    }
        else {
	  fore=pp->default_color[pp->default_color_num];
	  pp->default_color_num=(pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
	  XtSetArg(args[j], XtNforeground, fore); j++;
	  }
	if (ds[n]->pcolor) {
	  XtSetArg(args[j], XtNplotMarkColor, 
	    WnColorF((Widget)w, ds[n]->pcolor)); j++;
	    }
        else {
	  fore=pp->default_color[pp->default_color_num];
	  pp->default_color_num=(pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
          XtSetArg(args[j], XtNplotMarkColor,fore); j++;
	  }
        }
      else { /*line_style[n] is NULL */
        have_style = 0;
	fore=pp->default_color[pp->default_color_num];
        pp->default_color_num = (pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
        XtSetArg(args[j], XtNforeground, fore); j++;
        }
      }
    else { /* line_style is NULL */
      fore=pp->default_color[pp->default_color_num];
      pp->default_color_num = (pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
      XtSetArg(args[j], XtNforeground, fore); j++;
      } 
    if (set == 2) {
      XtSetArg(args[j], XtNuseY2Axis, True); j++;
      XtSetArg(args[j], XtNuseX2Axis, True); j++;
      }
    widget_p[n] = (AtXYPlotWidget)XtCreateWidget(name, atXYLinePlotWidgetClass, (Widget)w, args, j);
    } /*end of for n=num;n<nsets*/
   if (set==2) pp->data2_widget_num = nsets;
   else pp->data_widget_num = nsets;
 } /*end of pp->data_widget_num<nsets*/
    
 else if (num > nsets) {
   for (i = nsets; i<num; i++) {
      XtDestroyWidget((Widget)widget_p[i]);
      widget_p[i] = NULL;
      }
   if (set == 1) pp->data_widget_num = nsets;
   else pp->data2_widget_num = nsets;
  }
  
 if (ifPlotDataChanged(hData, oldData)) {
 for (i=0; i<nsets; i++) {
   if ((hData->g.type & 1) ==PLOT_DATA_GENERAL) {
     npoints=hData->g.data[i].npoints;
     /*AtXYLinePlotAttachData(widget_p[i], (XtPointer)(hData->g.data[i].xp), AtFloat,
	  sizeof(float), (XtPointer)(hData->g.data[i].yp), AtFloat,
          sizeof(float), 0, npoints);*/
     AtXYPlotReleaseData(widget_p[i]);
     AtXYPlotCopyData(widget_p[i], (XtPointer)(hData->g.data[i].xp), AtFloat,
	  sizeof(float), (XtPointer)(hData->g.data[i].yp), AtFloat,
          sizeof(float), 0, npoints);
     }
   /*else AtXYLinePlotAttachData(widget_p[i], (XtPointer)(hData->a.data.xp), 
      AtFloat, sizeof(float), (XtPointer)(hData->a.data.yp[i]), AtFloat,
      sizeof(float), 0, npoints);*/
   else {
     AtXYPlotReleaseData(widget_p[i]);
     AtXYPlotCopyData(widget_p[i], (XtPointer)(hData->a.data.xp), 
      AtFloat, sizeof(float), (XtPointer)(hData->a.data.yp[i]), AtFloat,
      sizeof(float), 0, npoints);
      }
   }
 } /*end of ifPlotDataChanged*/
 else {
   if ((hData->a.type & 1) == PLOT_DATA_ARRAY) {
     for (i=0; i<num; i++) {
       if (memcmp(widget_p[i]->lplot.xdata, hData->a.data.xp,
	 sizeof(float) * hData->a.npoints) ||
	 memcmp(widget_p[i]->lplot.ydata, hData->a.data.yp[i],
	 sizeof(float) * hData->a.npoints) ) {
         AtXYPlotReleaseData(widget_p[i]);
         AtXYPlotCopyData(widget_p[i], (XtPointer)(hData->a.data.xp),
 	   AtFloat, sizeof(float), (XtPointer)(hData->a.data.yp[i]), AtFloat,
	   sizeof(float), 0, npoints);
         changed = True;
	 }
       }
     for (i = num; i < nsets; i++) {
       AtXYPlotAttachData(widget_p[i], (XtPointer)(hData->a.data.xp), 
         AtFloat, sizeof(float), (XtPointer)(hData->a.data.yp[i]), AtFloat,
         sizeof(float), 0, npoints);
       changed = True;
       }
     }
   else {
     for (i=0; i<nsets; i++) {
       if (memcmp(hData->g.data[i].xp, widget_p[i]->lplot.xdata,
	      sizeof(float) * hData->g.data[i].npoints) || 
	  memcmp(hData->g.data[i].yp, widget_p[i]->lplot.ydata, 
	      sizeof(float) * hData->g.data[i].npoints)) {
         AtXYPlotReleaseData(widget_p[i]);
	 AtXYPlotCopyData(widget_p[i], (XtPointer)(hData->g.data[i].xp), 
	    AtFloat, sizeof(float), (XtPointer)(hData->g.data[i].yp), 
	    AtFloat,sizeof(float), 0, hData->g.data[i].npoints);
	 changed = True;
	 }
       }
     for (i=num; i<nsets; i++) {
       AtXYPlotAttachData(widget_p[i],(XtPointer)(hData->g.data[i].xp),AtFloat,
	  sizeof(float), (XtPointer)(hData->g.data[i].yp), AtFloat,
          sizeof(float), 0, npoints);
       changed = True;
       }
    } 
 }
 hData->a.type &= ~PLOT_DATA_NEW;
 hData->a.type &= ~PLOT_DATA_CHANGED;
} /*end of plotIfDataChanged*/

#if 0
int plotDataChanged(AtPlotterWidget w, PlotData *hData, int set)
{
 Widget line;
 int num, nsets, npoints, n, i, j;
 char name[7];
 Pixel fore;
 AtPlotterWidget pw = (AtPlotterWidget)w;
 AtPlotterPart *pp = &(pw->plotter);
 char str[17];
 Arg args[16];
 Boolean have_style;
 PlotDataStyle     **ds;
 Widget *widget_p;
 char **labels_p;
 PlotType plot_type;

 if (!w || (set<0)) return(0);

 if (set==1) {
   plot_type = pp->plot_type;
   num = pp->data_widget_num;
   labels_p = pp->set_labels;
   widget_p = pp->data_widget;
   if (pp->plot_data_styles) {
     have_style = 1;
     ds = pp->plot_data_styles;
     }
   else have_style = 0;
   }
 else { /* set == 2 */
   plot_type = pp->plot_type2;
   num = pp->data2_widget_num;
   labels_p = pp->set_labels2;
   widget_p = pp->data2_widget;
   if (pp->plot_data_styles2) {
     have_style = 1;
     ds = pp->plot_data_styles2;
     }
   else have_style = 0;
   }

 if (!hData) {
   for (i = 0; i<num; i++) {
      XtDestroyWidget(widget_p[i]);
      widget_p[i] = NULL;
      }
   if (set == 1) pp->data_widget_num = 0;
   else pp->data2_widget_num = 0;
   return(0);
  }

 hData->a.type &= ~PLOT_DATA_NEW;
 hData->a.type &= ~PLOT_DATA_CHANGED;
 if ((hData->a.type & 1) ==PLOT_DATA_ARRAY) {
    nsets=hData->a.nsets;
    npoints=hData->a.npoints;
   }
 else nsets=hData->g.nsets;
 
 if (num < nsets) { 
   if (set==1) {
     widget_p = pp->data_widget = (Widget *)realloc(pp->data_widget, sizeof(Widget)*(nsets+1));
     if (!pp->data_widget) {
       printf("cannot get enough memory for plot widgets\n");
       return(0);
     }
   }
   else { /* set == 2 */
     widget_p = pp->data2_widget = (Widget *)realloc(pp->data2_widget, sizeof(Widget)*(nsets+1));
     if (!pp->data2_widget) {
       printf("cannot get enough memory for plot widgets\n");
       return(0);
       }
     }
   for (n=num; n<nsets; n++) {
     sprintf(name, "S%1dL%d", set, n);
     j=0;
     if ((pp->show_legend) && (labels_p))
       if (labels_p[n]) {
	  XtSetArg(args[j], XtNlegendName, (labels_p[n])); j++;
	  }
     XtSetArg(args[j], XtNplotType, plot_type); j++;
     /*XtSetArg(args[j], XtNplotLineType, AtPlotLINEPOINTS); j++;*/
     if (have_style) {
      if (ds[n]) {
        XtSetArg(args[j], XtNlineWidth, ds[n]->width);  j++; 
        XtSetArg(args[j], XtNplotMarkType, ds[n]->point); j++;
        XtSetArg(args[j], XtNplotMarkSize, ds[n]->psize); j++;
        XtSetArg(args[j], XtNplotLineStyle, ds[n]->lpat);j++; 
        XtSetArg(args[j], XtNplotFillStyle, ds[n]->fpat); j++;
        if (ds[n]->color) {
	  XtSetArg(args[j], XtNforeground,
	    WnColorF((Widget)w, ds[n]->color)); j++;
	    }
        else {
	  fore=pp->default_color[pp->default_color_num];
	  pp->default_color_num=(pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
	  XtSetArg(args[j], XtNforeground, fore); j++;
	  }
	if (ds[n]->pcolor) {
	  XtSetArg(args[j], XtNplotMarkColor, 
	    WnColorF((Widget)w, ds[n]->pcolor)); j++;
	    }
        else {
	  fore=pp->default_color[pp->default_color_num];
	  pp->default_color_num=(pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
          XtSetArg(args[j], XtNplotMarkColor,fore); j++;
	  }
        }
      else { /*line_style[n] is NULL */
        have_style = 0;
	fore=pp->default_color[pp->default_color_num];
        pp->default_color_num = (pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
        XtSetArg(args[j], XtNforeground, fore); j++;
        }
      }
    else { /* line_style is NULL */
      fore=pp->default_color[pp->default_color_num];
      pp->default_color_num = (pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
      XtSetArg(args[j], XtNforeground, fore); j++;
      } 
    if (set == 2) {
      XtSetArg(args[j], XtNuseY2Axis, True); j++;
      XtSetArg(args[j], XtNuseX2Axis, True); j++;
      }
    widget_p[n] = XtCreateWidget(name, atXYLinePlotWidgetClass, (Widget)w, args, j);
    } /*end of for n=num;n<nsets*/
   if (set==2) pp->data2_widget_num = nsets;
   else pp->data_widget_num = nsets;
 } /*end of pp->data_widget_num<nsets*/
    
 else if (num > nsets) {
   for (i = nsets; i<num; i++) {
      XtDestroyWidget(widget_p[i]);
      widget_p[i] = NULL;
      }
   if (set == 1) pp->data_widget_num = nsets;
   else pp->data2_widget_num = nsets;
  }
  
 for (i=0; i<nsets; i++) {
   if ((hData->g.type & 1) ==PLOT_DATA_GENERAL) {
     npoints=hData->g.data[i].npoints;
     AtXYLinePlotAttachData(widget_p[i], (XtPointer)(hData->g.data[i].xp), AtFloat,
	  sizeof(float), (XtPointer)(hData->g.data[i].yp), AtFloat,
          sizeof(float), 0, npoints);
     }
   else AtXYLinePlotAttachData(widget_p[i], (XtPointer)(hData->a.data.xp), 
      AtFloat, sizeof(float), (XtPointer)(hData->a.data.yp[i]), AtFloat,
      sizeof(float), 0, npoints);
   }
} /*end of plotDataChanged*/
#endif


/*************************/
int PlotterCreatePlot(Widget w, PlotData *hData, int set)
{
 Widget line;
 int nsets, npoints, n, i, j;
 char name[7];
 Pixel fore;
 AtPlotterWidget pw = (AtPlotterWidget)w;
 AtPlotterPart *pp = &(pw->plotter);
 char str[17];
 Arg args[16];
 Boolean have_style;
 PlotDataStyle     **ds;
 PlotType plot_type;

  if (set<0) {
    printf("PlotterCreatePlot: set %d < 0\n", set);
    set = 0;
    }

  if ((hData->a.type & 1)==PLOT_DATA_ARRAY) {
    nsets=hData->a.nsets;
    npoints=hData->a.npoints;
    if (set==1) {
      plot_type = pp->plot_type;
      pp->data_widget_num = nsets;
      /*pp->data_widget = (Widget *)malloc(sizeof(Widget)*(nsets+1));*/
      pp->data_widget = (Widget *)calloc(nsets+1, sizeof(Widget));
      if (!pp->data_widget) {
        printf("cannot get enough memory for plot widgets\n");
        exit(0);
        }
      for (n=0; n<=nsets; n++) pp->data_widget[n] = NULL;
      if (pp->plot_data_styles) {
        have_style = 1;
        ds = pp->plot_data_styles;
	 }
      else have_style = 0;
      }
    else { /* set 2 */
      plot_type = pp->plot_type2;
      pp->data2_widget_num = nsets;
      /*pp->data2_widget = (Widget *)malloc(sizeof(Widget)*(nsets+1));*/
      pp->data2_widget = (Widget *)calloc(nsets+1, sizeof(Widget));
      if (!pp->data2_widget) {
        printf("cannot get enough memory for plot widgets\n");
        exit(0);
        }
      for (n=0; n<=nsets; n++) pp->data2_widget[n] = NULL;
      if (pp->plot_data_styles2) {
	have_style = 1;
	ds = pp->plot_data_styles2;
	}
      else have_style = 0;
      }

    for (n=0; n<nsets; n++) {
      /*for (i=0; i<npoints; i++) {
	sprintf(str, "%e\n", hData->a.data.xp[i]);
	if (!isdigit(str[0])) 
	  hData->a.data.xp[i] = 0;
	sprintf(str, "%e\n", hData->a.data.yp[n][i]);
	if (!isdigit(str[0])) 
	  hData->a.data.yp[n][i] = 0;
	}*/
      sprintf(name, "S%1dL%d", set, n);
      j=0;
      if ((pp->show_legend) && (pp->set_labels))
	if (pp->set_labels[n]) {
	  XtSetArg(args[j], XtNlegendName, (pp->set_labels[n])); j++;
	  }
      XtSetArg(args[j], XtNplotType, plot_type); j++;
      /*XtSetArg(args[j], XtNplotLineType, AtPlotLINEPOINTS); j++;*/
      if (have_style) {
      if (ds[n]) {
        XtSetArg(args[j], XtNlineWidth, ds[n]->width);  j++; 
        XtSetArg(args[j], XtNplotMarkType, ds[n]->point); j++;
        XtSetArg(args[j], XtNplotMarkSize, ds[n]->psize); j++;
        XtSetArg(args[j], XtNplotLineStyle, ds[n]->lpat); j++; 
        XtSetArg(args[j], XtNplotFillStyle, ds[n]->fpat); j++;
	if (ds[n]->color) {
	  XtSetArg(args[j], XtNforeground,
	    WnColorF(w, ds[n]->color)); j++;
	    }
        else {
	  fore=pp->default_color[pp->default_color_num];
	  pp->default_color_num=(pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
	  XtSetArg(args[j], XtNforeground, fore); j++;
	  }
	if (ds[n]->pcolor) {
	  XtSetArg(args[j], XtNplotMarkColor, 
	    WnColorF(w, ds[n]->pcolor)); j++;
	    }
        else {
	  fore=pp->default_color[pp->default_color_num];
	  pp->default_color_num=(pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
          XtSetArg(args[j], XtNplotMarkColor,fore); j++;
	  }
        }
      else { /*line_style[n] is NULL */
        have_style = 0;
	fore=pp->default_color[pp->default_color_num];
        pp->default_color_num = (pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
        XtSetArg(args[j], XtNforeground, fore); j++;
        }
      }
      else { /* line_style is NULL */
        fore=pp->default_color[pp->default_color_num];
        pp->default_color_num = (pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
        XtSetArg(args[j], XtNforeground, fore); j++;
        } 
      if (set==1) {
        line = XtCreateWidget(name, atXYLinePlotWidgetClass, w, args, j); 
        /*AtXYLinePlotAttachData(line, (XtPointer)(hData->a.data.xp), AtFloat,
	  sizeof(float), (XtPointer)(hData->a.data.yp[n]), AtFloat,
	  sizeof(float), 0, npoints);*/
        AtXYPlotCopyData((AtXYPlotWidget)line, (XtPointer)(hData->a.data.xp), AtFloat,
	  sizeof(float), (XtPointer)(hData->a.data.yp[n]), AtFloat,
	  sizeof(float), 0, npoints);
        pp->data_widget[n] = line;
        } 
      else {
	XtSetArg(args[j], XtNuseY2Axis, True); j++;
	XtSetArg(args[j], XtNuseX2Axis, True); j++; 
	/*XtSetArg(args[j], XtNshading, AtGRAY3); j++;
	line = XtCreateWidget(name, atBarPlotWidgetClass, w, args, j);
	AtBarPlotAttachData(line, (XtPointer)(hData->a.data.yp[n]), AtFloat,
	  sizeof(float), 0, npoints);*/
        line = XtCreateWidget(name, atXYLinePlotWidgetClass, w, args, j);
	/*AtXYLinePlotAttachData(line,  (XtPointer)(hData->a.data.xp), AtFloat,
	  sizeof(float), (XtPointer)(hData->a.data.yp[n]), AtFloat,
          sizeof(float), 0, npoints);*/
	AtXYPlotCopyData((AtXYPlotWidget)line,  (XtPointer)(hData->a.data.xp), AtFloat,
	  sizeof(float), (XtPointer)(hData->a.data.yp[n]), AtFloat,
          sizeof(float), 0, npoints);
        pp->data2_widget[n] = line;
	}
      } /*nsets */
    } /* ARRAY DATA */
  else { 
    nsets=hData->g.nsets;
    if (set==1) {
      plot_type = pp->plot_type;
      pp->data_widget_num = nsets;
      /*pp->data_widget = (Widget *)malloc(sizeof(Widget)*(nsets+1));*/
      pp->data_widget = (Widget *)calloc(nsets+1, sizeof(Widget));
      if (!pp->data_widget) {
        printf("cannot get enough memory for plot widgets\n");
        exit(0);
        }
      for (n=0; n<=nsets; n++) pp->data_widget[n] = NULL;
      if (pp->plot_data_styles) {
        have_style = 1;
        ds = pp->plot_data_styles;
	 }
      else have_style = 0;
      }
    else { /* set 2 */
      plot_type = pp->plot_type2;
      pp->data2_widget_num = nsets;
      /*pp->data2_widget = (Widget *)malloc(sizeof(Widget)*(nsets+1));*/
      pp->data2_widget = (Widget *)calloc(nsets+1, sizeof(Widget));
      if (!pp->data2_widget) {
        printf("cannot get enough memory for plot widgets\n");
        exit(0);
        }
      for (n=0; n<=nsets; n++) pp->data2_widget[n] = NULL;
      if (pp->plot_data_styles2) {
	have_style = 1;
	ds = pp->plot_data_styles2;
	}
      else have_style = 0;
      }

    for (n=0; n<nsets; n++) {
      npoints=hData->g.data[n].npoints;
      sprintf(name, "S%1dL%d", set, n);
      j=0;
      if ((pp->show_legend) && (pp->set_labels))
	if (pp->set_labels[n]) {
	  XtSetArg(args[j], XtNlegendName, (pp->set_labels[n])); j++;
	  }
      XtSetArg(args[j], XtNplotType, plot_type); j++;
      /*XtSetArg(args[j], XtNplotLineType, AtPlotLINEPOINTS); j++;*/
      if (have_style) {
      if (ds[n]) {
        XtSetArg(args[j], XtNlineWidth, ds[n]->width);  j++; 
        XtSetArg(args[j], XtNplotMarkType, ds[n]->point); j++;
        XtSetArg(args[j], XtNplotMarkSize, ds[n]->psize); j++;
        XtSetArg(args[j], XtNplotLineStyle, ds[n]->lpat); j++; 
        XtSetArg(args[j], XtNplotFillStyle, ds[n]->fpat); j++;
        if (ds[n]->color) {
	  XtSetArg(args[j], XtNforeground,
	    WnColorF(w, ds[n]->color)); j++;
	    }
        else {
	  fore=pp->default_color[pp->default_color_num];
	  pp->default_color_num=(pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
	  XtSetArg(args[j], XtNforeground, fore); j++;
	  }
	if (ds[n]->pcolor) {
	  XtSetArg(args[j], XtNplotMarkColor, 
	    WnColorF(w, ds[n]->pcolor)); j++;
	    }
        else {
	  fore=pp->default_color[pp->default_color_num];
	  pp->default_color_num=(pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
          XtSetArg(args[j], XtNplotMarkColor,fore); j++;
	  }
        }
        else { /* line_style[n] is NULL */
          have_style = 0;
          fore=pp->default_color[pp->default_color_num];
          pp->default_color_num = (pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
          XtSetArg(args[j], XtNforeground, fore); j++;
          }
        }
      else { /* line_style is NULL */
        fore=pp->default_color[pp->default_color_num];
        pp->default_color_num = (pp->default_color_num+1)%DEFAULT_COLOR_TOTAL;
        XtSetArg(args[j], XtNforeground, fore); j++;
        } 
      if (set==1) {
        line = XtCreateWidget(name, atXYLinePlotWidgetClass, w, args, j); 
        /*AtXYLinePlotAttachData(line, (XtPointer)(hData->g.data[n].xp), AtFloat,
	  sizeof(float), (XtPointer)(hData->g.data[n].yp), AtFloat,
	  sizeof(float), 0, npoints);*/
        AtXYPlotCopyData((AtXYPlotWidget)line, (XtPointer)(hData->g.data[n].xp), AtFloat,
	  sizeof(float), (XtPointer)(hData->g.data[n].yp), AtFloat,
	  sizeof(float), 0, npoints);
        pp->data_widget[n] = line;
        } 
      else {
	XtSetArg(args[j], XtNuseY2Axis, True); j++;
	XtSetArg(args[j], XtNuseX2Axis, True); j++; 
	/*XtSetArg(args[j], XtNshading, AtGRAY3); j++;
	line = XtCreateWidget(name, atBarPlotWidgetClass, w, args, j);
	AtBarPlotAttachData(line, (XtPointer)(hData->a.data.yp[n]), AtFloat,
	  sizeof(float), 0, npoints);*/
        line = XtCreateWidget(name, atXYLinePlotWidgetClass, w, args, j);
	/*AtXYLinePlotAttachData(line, (XtPointer)(hData->g.data[n].xp), AtFloat,
	  sizeof(float), (XtPointer)(hData->g.data[n].yp), AtFloat,
          sizeof(float), 0, npoints);*/
	AtXYPlotCopyData((AtXYPlotWidget)line, (XtPointer)(hData->g.data[n].xp), AtFloat,
	  sizeof(float), (XtPointer)(hData->g.data[n].yp), AtFloat,
          sizeof(float), 0, npoints);
        pp->data2_widget[n] = line;
	}
      } /*nsets */
    } /* RENERAL DATA */
  hData->a.type &= ~PLOT_DATA_NEW;
  hData->a.type &= ~PLOT_DATA_CHANGED;
}


int PlotSetLineStyle(Widget w, PlotDataStyle *ds)
{
  Display *display;
  Pixel fore, mcolor;
  XrmValue from_value, to_value;
  char from_s[256]; 
  Arg args[6];
  int j;

  if ((!w) || (!ds)) return(0);
  /* check if widget is an AtXYLinePlot widget */
  XtCheckSubclass((Widget)w, xtXYPlotWidgetClass, 
	     "PlotSetLineStyle need AtXYLinePlot Widget");
  j=0;
  XtSetArg(args[j], XtNlineWidth, ds->width);    j++; 
  XtSetArg(args[j], XtNplotMarkType, ds->point); j++;
  XtSetArg(args[j], XtNplotMarkSize, ds->psize); j++;
  XtSetArg(args[j], XtNplotLineStyle, ds->lpat); j++; 
  display = XtDisplay(XtParent(w));
  if (DefaultDepthOfScreen (DefaultScreenOfDisplay(display)) !=1) {
    if (ds->color) {
      sprintf(from_s, ds->color);
      from_value.size = strlen(from_s) + 1;
      from_value.addr = from_s;
      XtConvert(w, XmRString, &from_value, XmRPixel, &to_value);
      fore = *(int *)to_value.addr;
      XtSetArg(args[j], XtNforeground, fore); j++;
      }
    if (ds->pcolor) {
      sprintf(from_s, ds->pcolor);
      from_value.size = strlen(from_s) + 1;
      from_value.addr = from_s;
      XtConvert(w, XmRString, &from_value, XmRPixel, &to_value);
      mcolor = *(int *)to_value.addr;
      XtSetArg(args[j], XtNplotMarkColor, mcolor); j++;
      }
  }
  
  XtSetValues(w, args, j);
}

Pixel PlotColor P((Widget, char *, char *));
Pixel PlotColor(Widget w, char *name, char *default_name)
{
  AtPlotterWidget pw = (AtPlotterWidget)w;
  AtPlotterPart *pp = &(pw->plotter);
  static Pixel fore;
  static XrmValue from_value, to_value;
  char from_s[256]; 

  if ((w ==NULL) ||(name ==NULL)) return(1);
    sprintf(from_s, name);
    from_value.size = strlen(from_s) + 1;
    from_value.addr = from_s;
    XtConvert(w, XmRString, &from_value, XmRPixel, &to_value);
    if (to_value.addr) fore = *(int *)to_value.addr;
    else {
      printf("Warning: can not allocate pixel for color %s\n", name);
      sprintf(from_s, default_name);
      from_value.size = strlen(from_s) + 1;
      from_value.addr = from_s;
      XtConvert(w, XmRString, &from_value, XmRPixel, &to_value);
      if (to_value.addr) fore = *(int *)to_value.addr;
      }
    return(fore); 
}

Pixel WnColorB(Widget w, char * cname)
{
  AtPlotterWidget pw = (AtPlotterWidget)w;
  AtPlotterPart *pp = &(pw->plotter);
  Display *display=XtDisplay(XtParent(w));
  static Pixel fore;
  static XrmValue from_value, to_value;
  char from_s[256]; 

  if ((cname == NULL) || (w ==NULL)) return(1);
    sprintf(from_s, cname);
    from_value.size = strlen(from_s) + 1;
    from_value.addr = from_s;
    XtConvert(w, XmRString, &from_value, XmRPixel, &to_value);
    if (to_value.addr) fore = *(int *)to_value.addr;
    else {
      printf("Warning: can not allocate pixel for color %s\n", cname);
      fore = pp->default_color[7];
      }   
    return(fore);
}

Pixel WnColorF(Widget w, char * cname)
{
  AtPlotterWidget pw = (AtPlotterWidget)w;
  AtPlotterPart *pp = &(pw->plotter);
  Display *display=XtDisplay(XtParent(w));
  static Pixel fore;
  static XrmValue from_value, to_value;
  char from_s[256]; 

  if ((w==NULL)||(cname==NULL))return(1);
    sprintf(from_s, cname);
    from_value.size = strlen(from_s) + 1;
    from_value.addr = from_s;
    XtConvert(w, XmRString, &from_value, XmRPixel, &to_value);
    if (to_value.addr) fore = *(int *)to_value.addr;
    else {
      /*printf("Warning: can not allocate pixel for color %s\n", cname);*/
      fore = pp->default_color[6];
      }
    return(fore);
}


Pixel ColorNameToPixel(Widget w, char * cname)
{
  Display *display=XtDisplay(XtParent(w));
  static Pixel fore;
  static XrmValue from_value, to_value;
  char from_s[256]; 

  if ((w==NULL)||(cname==NULL)) return(1);
  /*if (DefaultDepthOfScreen (DefaultScreenOfDisplay(display)) !=1) {*/
    sprintf(from_s, cname);
    from_value.size = strlen(from_s) + 1;
    from_value.addr = from_s;
    XtConvert(w, XmRString, &from_value, XmRPixel, &to_value);
    if (to_value.addr) fore = *(int *)to_value.addr;
    else {
      printf("Warning: can not allocate pixel for color %s\n", cname);
      cname = "white";
      sprintf(from_s, cname);
      from_value.size = strlen(from_s) + 1;
      from_value.addr = from_s;
      XtConvert(w, XmRString, &from_value, XmRPixel, &to_value);
      if (to_value.addr) fore = *(int *)to_value.addr;
      else return(1);
      }
    return(fore);
}

