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

/************** DataHandle.h for AtPlotter ****************/
  

#ifndef _Jpt_PLOT_DATA_HANDLE
#define _Jpt_PLOT_DATA_HANDLE

#ifdef __cplusplus
extern "C" {
#endif
/*
#if (__STDC__ | WINNT | __VMS | VMS)
#include <float.h>
#define PLOT_HUGE_VAL   FLT_MAX
#else
#include <values.h>
#define PLOT_HUGE_VAL  MAXFLOAT
#endif
*/
#include "At.h"
#include "Plotter.h"
/*
#define arr_nsets						a.nsets
#define arr_npoints						a.npoints
#define arr_data						a.data
#define arr_hole						a.hole
#define arr_xdata						arr_data.xp
#define arr_ydata						arr_data.yp
#define arr_xel(j)						arr_xdata[j]
#define arr_yel(i, j)					arr_ydata[i][j]

#define gen_nsets						g.nsets
#define gen_data						g.data
#define gen_hole						g.hole
#define gen_npoints(i)					gen_data[i].npoints
#define gen_xdata(i)					gen_data[i].xp
#define gen_ydata(i)					gen_data[i].yp
#define gen_xel(i, j)					gen_xdata(i)[j]
#define gen_yel(i, j)					gen_ydata(i)[j]


typedef struct {
  int pix_x, pix_y;
  int yaxis;
  float x, y;
  } PlotMapResult;

typedef enum  {
  XRT_RGN_NOWHERE = 1,
  XRT_RGN_IN_GRAPH,
  XRT_RGN_IN_LEGEND,
  XRT_RGN_IN_FOOTER,
  XRT_RGN_IN_HEADER
  } PlotRegion;
*/

/*
typedef enum	{
	XRT_DATA_ARRAY ,
	XRT_DATA_GENERAL
} PlotDataType;

typedef struct	{
	float		 *xp;
	float		**yp;
} PlotArrayData;

typedef struct	{
	PlotDataType	 type;			
	float		 hole;
	int		 nsets;
	int		 npoints;
	PlotArrayData	 data;
} PlotArray;

typedef struct {			
	int		 npoints;
	float		*xp;
	float		*yp;
} PlotGeneralData;

typedef struct	{	
	PlotDataType	 type;	
	float		 hole;
	int		 nsets;
	PlotGeneralData	*data;
} PlotGeneral;

typedef union	{
	PlotArray		a;
	PlotGeneral		g;
} PlotData;
*/

/* functions */
extern Boolean ifPlotDataChanged P((PlotData *old, PlotData *new));
extern int PlotterDrawXMarker P((Widget));
extern PlotData *plotCopyPlotData P((PlotData *));
extern int plotDataChanged P((AtPlotterWidget, PlotData *, int)); 
extern Boolean plotIfDataChanged P((AtPlotterWidget, PlotData *, PlotData *, 
      int));
extern int PlotFloatToPixel P((Widget, float, float, Position *, 
      Position *));
extern int PlotterCreatePlot P((Widget, PlotData *, int));
extern void PlotterInitDefaultColor P((Widget));
extern int PlotSetLineStyle P((Widget, PlotDataStyle *));
/*extern Pixel ColorNameToPixel P((Widget, char *));*/

/*
extern PlotRegion PlotMap P((Widget, int, int, int, PlotMapResult *));
extern PlotData *PlotMakeData P((PlotDataType, int, int, int));
extern PlotData *PlotMakeDataFromFile P((char *, char *));
extern XtArgVal PlotFloatToArgVal P((float));
extern Pixel WnColorD P((String, Display *));
extern char ** PlotDupStrings P((char **));
extern PlotDataStyle ** PlotDupDataStyles P((PlotDataStyle **));
extern void PlotFreeStrings P((char **));
extern void PlotFreeDataStyles P((PlotDataStyle **));
*/

#ifdef __cplusplus
}
#endif

#endif
