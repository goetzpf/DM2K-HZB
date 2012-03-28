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
/* Text2.h   do something the xrt text does */

#ifndef _Jpt_PLOT_TEXT2_H_
#define _Jpt_PLOT_TEXT2_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include "At.h"

#ifndef P
# ifdef __STDC__
#  define P(args) args
# else
#  define P(args) ()
# endif
#endif

#if 0
typedef enum	{
	XRT_TEXT_ATTACH_PIXEL = 0,
	XRT_TEXT_ATTACH_VALUE,
	XRT_TEXT_ATTACH_DATA,
	XRT_TEXT_ATTACH_DATA_VALUE
} PlotAttachType;

typedef enum	{
	XRT_ADJUST_LEFT = XmALIGNMENT_BEGINNING,
	XRT_ADJUST_CENTER = XmALIGNMENT_CENTER,
	XRT_ADJUST_RIGHT = XmALIGNMENT_END
} PlotAdjust;

typedef enum	{
	XRT_ANCHOR_NORTH	= 0x10,
	XRT_ANCHOR_SOUTH	= 0x20,
	XRT_ANCHOR_EAST	= 0x01,
	XRT_ANCHOR_WEST	= 0x02,
	XRT_ANCHOR_NORTHEAST	= 0x11,
	XRT_ANCHOR_NORTHWEST	= 0x12,
	XRT_ANCHOR_SOUTHEAST	= 0x21,
	XRT_ANCHOR_SOUTHWEST	= 0x22,
	/* for text areas only, not for legend */
	XRT_ANCHOR_HOME	= 0x00,
	XRT_ANCHOR_BEST	= 0x100
} PlotAnchor;


typedef union	{
	struct	{
		PlotAttachType	type;
		int	x, y;
	} pixel;
	struct	{
		PlotAttachType	    type;
		int		dataset;
		float		x, y;
	} value;
	struct	{
		PlotAttachType	type;
		int	dataset;
		int	set, point;
	} data;
	struct	{
		PlotAttachType	type;
		int	dataset;
		int	set, point;
		float	y;
	} data_value;
} PlotTextPosition;


typedef struct	{	/* Attached text structure */
	PlotTextPosition	  position;
	char			**strings;
	PlotAnchor		  anchor;
	int			  offset;
	int			  connected;
	PlotAdjust		  adjust;
	char			 *fore_color;
	char			 *back_color;
	PlotBorder	  	  border;
	int			  border_width;
	Font			  font;
	char			 *psfont;
	int			  psfont_size;
	XRectangle		  coords;	/* read-only */
} PlotTextDesc;


typedef void	*PlotTextHandle;	/* handle to attached text */

typedef enum {
    TEXT_NONE = 0,
    TEXT_CREATED,
    TEXT_NO_WINDOW,
    TEXT_ON,
    TEXT_OFF
    } TextHandleState;

typedef enum {
    UNDISPLAY = 0,
    TO_DISPLAY,
    DISPLAYED
    } TextDisplayState;

typedef struct {
   Boolean attached, positioned;
   TextDisplayState display_state;
   Pixel fore, back;
   GC          gc;
   Pixmap      newp;
   PlotTextDesc td;
   Position x, y, x1, y1, x2, y2;
   Dimension width, height;
   int str_num;
  } TextHandle;

typedef struct {
   TextHandle *th;
   void *next;
 } TextList;
#endif

extern int PlotTextCalc P((Widget, TextHandle *));
extern int PlotTextDraw P((Widget, TextHandle *));
extern int PlotTextGetPosition P((Widget, TextHandle *));


#ifdef __cplusplus
}
#endif

#endif
