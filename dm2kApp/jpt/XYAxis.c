/*
 *      XYAxis.c
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Sun Jul 19 19:23:41 1992, patchlevel 1
 *                                      Bug in LinAxisCalc() fixed
 *      klin, Mon Jul 27 14:19:19 1992, patchlevel 2
 *                                      Shorter names for procedures.
 *                                      Register transform converter in
 *                                      ClassInitialize().
 *      klin, Sun Aug  2 18:25:28 1992, patchlevel 3
 *                                      Accept min/max values in Initialize().
 *      klin, Fri Aug  7 10:07:59 1992, Cast type converters to keep
 *                                      ANSI C compilers quiet
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 *      klin, Fri Dec 11 15:55:12 1992, patchlevel 5
 *                                      Bug in setting tic interval fixed.
 *                                      Calculate number of subtics dependent
 *                                      on length in pixels between the tics.
 */
static char SCCSid[] = "@(#) Plotter V6.0  92/12/11  XYAxis.c";

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
 *   A more complicated form numeric axis with linear or logarithmic
 *   coordinates transformation. This class calculates endpoints,
 *   ticInterval, tic labels and positions, then lets the AxisCore
 *   superclass handle all that mucking about with pixels.
 */
#include <time.h>
#include "XYAxisP.h"
#include "PlotterP.h"

static void ClassInitialize P((void));
static void Initialize P((AtXYAxisWidget, AtXYAxisWidget));
static void Destroy P((AtXYAxisWidget));
static Boolean SetValues P((AtXYAxisWidget, AtXYAxisWidget, AtXYAxisWidget));
static void RangeProc P((AtAxisCoreWidget, float *, float *, float *, int *));
static void CalcProc P((AtAxisCoreWidget));

/* The resources */

#define off(field) XtOffsetOf (AtXYAxisRec, axis.field)
static XtResource resources[] = {
  {
     XtNaxisTransform, XtCAxisTransform,
     XtRTransform, sizeof(AtTransform),
     off(axis_transform), XtRTransform, (XtPointer) AtTransformLINEAR
  },
  {
     XtNautoScale, XtCAutoScale,
     XtRBoolean, sizeof(Boolean),
     off(auto_scale), XtRImmediate, (XtPointer) True
  },
  {
     XtNautoTics, XtCAutoTics,
     XtRBoolean, sizeof(Boolean),
     off(auto_tics), XtRImmediate, (XtPointer) True
  },
  {
     XtNroundEndpoints, XtCRoundEndpoints,
     XtRBoolean, sizeof(Boolean),
     off(round_endpoints), XtRImmediate, (XtPointer) True
  },
  /*{
     XtNplotMaxUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(max_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotMinUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(min_dft), XtRImmediate, (XtPointer) True
  },*/
  {
     XtNplotNumUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(num_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotTickUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(tic_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNlinTicFormat, XtCLinTicFormat,
     XtRString, sizeof(String),
     off(lintic_format), XtRImmediate, "%g"
  },
  {
     XtNlogTicFormat, XtCLogTicFormat,
     XtRString, sizeof(String),
     off(logtic_format), XtRImmediate, "%g"
  }
};
#undef off

AtXYAxisClassRec atXYAxisClassRec = {
  { /* core fields */
     /* superclass              */      (WidgetClass) &atAxisCoreClassRec,
     /* class_name              */      "AtXYAxis",
     /* widget_size             */      sizeof(AtXYAxisRec),
     /* class_initialize        */      ClassInitialize,
     /* class_part_initialize   */      NULL,
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
     /* draw                    */      XtInheritDraw,
     /* draw_icon               */      XtInheritDrawIcon,
     /* drawPS                  */      NULL,
     /* draw_iconPS             */      NULL,
     /* recalc                  */      XtInheritRecalc
  },
  { /* Axis fields */
     /* range_proc              */      (AtAxisRangeProc) RangeProc,
     /* calc_proc               */      (AtAxisCalcProc) CalcProc
  }
};

WidgetClass atXYAxisWidgetClass = (WidgetClass)&atXYAxisClassRec;

/*
 *   Internal initialization procs
 */

static void LinAxisInitialize P((AtXYAxisWidget));
static void LogAxisInitialize P((AtXYAxisWidget));

static void LinAxisInitialize(new)
AtXYAxisWidget new;
{
     AtScale *as = new->axiscore.scale;

     new->axiscore.draw_origin = new->axis.draw_origin;
     new->axiscore.min = as->low  = LIN_MIN;
     new->axiscore.max = as->high = LIN_MAX;
     AtScaleChangeTransform(as, AtTransformLINEAR);
}

static void LogAxisInitialize(new)
AtXYAxisWidget new;
{
     AtScale *as = new->axiscore.scale;

     new->axis.draw_origin     = new->axiscore.draw_origin;
     new->axiscore.draw_origin = False;      /* Not useful */
     new->axiscore.min = as->low  = LOG_MIN;
     new->axiscore.max = as->high = LOG_MAX;
     AtScaleChangeTransform(as, AtTransformLOGARITHMIC);
}

/*
 *   The core member procs
 */

static void ClassInitialize()
{

     AtRegisterAxisTransformConverter();
     *SCCSid = *SCCSid;       /* Keep gcc quiet */
}

static void Initialize(req, new)
AtXYAxisWidget req, new;
{
     AtXYAxisPart   *ax = &new->axis;
     AtAxisCorePart *ac = &new->axiscore;
     AtScale        *as = new->axiscore.scale;
     AtPlotterPart *pp = & (((AtPlotterWidget)XtParent(new))->plotter);

     ax->draw_origin   = ac->draw_origin;
     ax->lintic_format = XtNewString(ax->lintic_format);
     ax->logtic_format = XtNewString(ax->logtic_format);
     ax->subtics_per_tic = 0;

     /*if (ac->min >= ac->max) {
	  XtAppWarning(XtWidgetToApplicationContext(XtParent((Widget) new)),
		       "Min is >= Max in AtXYAxis");
	  ax->auto_scale = False;
     }*/

     if (ac->min > ac->max) {
	  XtAppWarning(XtWidgetToApplicationContext(XtParent((Widget) new)),
		       "Min is >= Max in AtXYAxis");
	  ac->max_dft = False;
	  ac->min_dft = False;
	  ax->auto_scale = False;
     }
     if (ac->min == ac->max) /*be careful*/
       if (ac->max < PLOTTER_HUGE_VAL-100) ac->max++;
       else ac->min--;

     if (ax->axis_transform == AtTransformLOGARITHMIC) {
	  /*if ( !ax->auto_scale) {
	       ax->min = ac->min;
	       ax->max = ac->max;
	  }
	  if (ax->auto_scale) {
	       ax->min = LOG_MIN;
	       ax->max = LOG_MAX;
	  }*/
	  if (ac->max_dft) ax->max = LOG_MAX;
	  else ax->max = ac->max;
	  if (ac->min_dft) ax->min = LOG_MIN;
	  else ax->min = ac->min;

	  if (ax->min <= 0.0) {
	       XtAppWarning(XtWidgetToApplicationContext(XtParent((Widget) new)),
			    "Can't create logarithmic axis for min <= 0.0");
	       ax->axis_transform = AtTransformLINEAR;
	  }
	  else {
	       ax->tic_interval = ax->auto_tics ? TIC_INT : ac->tic_interval;
	       LogAxisInitialize(new);
	  }
     }

     if (ax->axis_transform == AtTransformLINEAR) {
/*	  if (ax->auto_scale) {
	       ax->min = LIN_MIN;
	       ax->max = LIN_MAX;
	  }
	  else {
	       ax->min = ac->min;
	       ax->max = ac->max;
	  }*/
	  if (ac->max_dft) ax->max = LIN_MAX;
	  else ax->max = ac->max;
	  if (ac->min_dft) ax->min = LIN_MIN;
	  else ax->min = ac->min;

	  ax->tic_interval = ax->auto_tics ? TIC_INT : ac->tic_interval;
	  LinAxisInitialize(new);
     }
     /*ac->min_bak = ac->min;
     ac->max_bak = ac->max;*/
}

static void Destroy(w)
AtXYAxisWidget w;
{
     XtFree(w->axis.lintic_format);
     XtFree(w->axis.logtic_format);
}

static Boolean SetValues(old, req, new)
AtXYAxisWidget old, req, new;
{

#define Changed(fld)      (old->axis.fld != new->axis.fld)
     AtScale *as = new->axiscore.scale;
     Boolean renum  = False;
     Boolean redraw = False;
     Boolean recalc = False;
     AtPlotterWidget pw = (AtPlotterWidget)XtParent(old);

     if (Changed(lintic_format)) {
	  XtFree(old->axis.lintic_format);
	  old->axis.lintic_format = NULL;
	  new->axis.lintic_format = XtNewString(new->axis.lintic_format);
	  renum = True;
     }
     if (Changed(logtic_format)) {
	  XtFree(old->axis.logtic_format);
	  old->axis.logtic_format = NULL;
	  new->axis.logtic_format = XtNewString(new->axis.logtic_format);
	  renum = True;
     }

     if ((new->axiscore.min != old->axiscore.min) ||
	 (new->axiscore.max != old->axiscore.max)) {
	  if ( !new->axis.auto_scale) {
               new->axis.min = new->axiscore.min;
	       new->axis.max = new->axiscore.max;
	  }
	  if (!new->axiscore.max_dft) new->axis.max = new->axiscore.max;
	  if (!new->axiscore.min_dft) new->axis.min = new->axiscore.min;
	  renum = redraw = True;
     }
     if (new->axiscore.tic_interval != old->axiscore.tic_interval) {
	  if ( !new->axis.auto_tics) {
	       if (new->axiscore.tic_interval <= 0) {
		  /*printf("WARNING: XYAxis.c:SetValues tic_interval <= 0\n");*/
		  new->axiscore.tic_interval = 1;
		  }
	       new->axis.tic_interval = new->axiscore.tic_interval;
	  }
	  renum = True;
     }

     if (Changed(auto_scale)) {
	  if ( !new->axis.auto_scale) {
	       new->axis.min = new->axiscore.min;
	       new->axis.max = new->axiscore.max;
	  }
	  renum = True;
     }

     if (Changed(auto_tics)) {
	  if ( !new->axis.auto_tics) {
	       new->axis.tic_interval = new->axiscore.tic_interval;
	  }
	  renum = True;
     }

  /*   if (Changed(max_dft)) {*/
     if (new->axiscore.max_dft != old->axiscore.max_dft) {
       if (!new->axiscore.max_dft) new->axis.max = new->axiscore.max;
       renum = True;
       }

    /* if (Changed(min_dft)) {*/
     if (new->axiscore.min_dft != old->axiscore.min_dft) {
       if (!new->axiscore.min_dft) new->axis.min = new->axiscore.min;
       renum = True;
       } 
     
     if (Changed(num_dft)) {
       if (!new->axis.num_dft) 
	 new->axis.tic_interval = new->axiscore.tic_interval;
       renum = True;
       }
     
     if (Changed(round_endpoints)) {
	  renum = True;
	  if (!new->axis.round_endpoints) {
	       /* XXXXXXXXXXXXXXX HACK !!!! XXXXXXXXXXXXX */
	       /*
		* We need to force the parent to throw away all the
		* extant bounding boxes, as the endpoints on this axis
		* just decreased.  We do this by faking a call to
		* AtPlotterPlotDataChanged, with a bogus bbox.
		*/
	       static BoundingBox bb = { 1.0, 0.0, 0.0, 0.0 };
	       AtPlotterPlotDataChanged((AtPlotWidget)new, &bb, False);
	  }
     }

     if (Changed(axis_transform)) {
	  if (new->axis.axis_transform == AtTransformLOGARITHMIC)
	       LogAxisInitialize(new);
	  else      /* AtTransformLINEAR */
	       LinAxisInitialize(new);
	  renum = recalc = redraw = True;
     }

    /* new->axiscore.max_bak = new->axiscore.max;
     new->axiscore.min_bak = new->axiscore.min;*/

     if (recalc)
	  AtPlotterRecalcThisPlot((AtPlotWidget) new);
     if (renum) {
	  new->axiscore.numbers_changed = True;
	  AtPlotterRescaleRequired((AtPlotWidget) new);
     }
     if (redraw)
	  AtPlotterRedrawRequired((AtPlotWidget) new);
     return False;

#undef Changed
}


/*
 *   Internal calc and range procs for linear axes
 */

static float CalcLinTicInterval P((AtXYAxisWidget, float, float));
static void LinAxisRange P((AtAxisCoreWidget, float *, float *,
			    float *, int *));
static void LinAxisCalc P((AtAxisCoreWidget));

/*
 *   Calculate the tic_interval, given the size of the axis.
 *   Assume the min/max has been rounded.
 */

static float CalcLinTicInterval(AtXYAxisWidget aw, float min, float max)
{
     AtAxisCorePart *ac = &aw->axiscore;
     AtXYAxisPart   *ax = &aw->axis;
     int len = ac->vertical ? ac->y1 - ac->y2 : ac->x2 - ac->x1;
     int th, nt, nst;
     float mag, flr, d, sizeticratio;
     int mult;
     float ret;
     AtPlotterPart *pp = & (((AtPlotterWidget)XtParent(aw))->plotter);

     if (ax->subtics_per_tic < 0) ax->subtics_per_tic = 0;
     if (ax->subtics_per_tic > 9) ax->subtics_per_tic = 9;
/*     if (min >= max) {
       printf("CalcLinTicInterval - min <= max\n");
       return(1);
       } */
     if (min==max) {
       /*printf("CalcLinTicInterval - min==max\n");*/
       return(1);
       }
     if ((abs(len)) <= 10) {
       /*printf("CalcLinTicInterval y1 %d, y2 %d\n", ac->y1, ac->y2);*/
       return(1);
       } /* Hasn't been set yet */

     /*
      * Make some assumptions about the height of the labels for
      * vertical axes or the width for horizontal axes.  For vertical
      * ones, we assume that any label will be the same height, so we
      * use that, else we assume a 4:1 aspect ratio if we know the max
      * width, else we punt.For horizontal labels, the maxwidth is
      * what we want, else we guess on the width of label[0], else we
      * punt.
      */
     if (ac->vertical) {
	  if (ac->max_num_height > 0) th = ac->max_num_height;
	  else if (ac->num_tics > 0 && pp->axis_fs) 
	    th = pp->axis_fs->ascent + pp->axis_fs->descent;
            else th = 10;
     } else {
	  /*if (ac->actual_num_width)
	       th = ac->actual_num_width;*/
	  if (ac->max_num_height > 0)
	       th = ac->max_num_height;
	  else if (ac->num_tics > 0 && pp->axis_fs) {
	       th = ac->tic_label_string[0] ?
		 XTextWidth(pp->axis_fs, ac->tic_label_string[0],
		   strlen(ac->tic_label_string[0])) : 0;
	  } else th = 20;
     }
     if (th<=1) {
       /*printf("CalcLinTicInterval - num_width<=1\n");*/
       th = 10;
       }

     /*nt = len / (th * 0.75 * 8);*/ 
     if (ac->vertical) nt = len / (th * 2.5);
     else nt = len / (th * 1.5);
     if (nt < 1)
	  nt = 1;
     if (nt < 2 && min * max < 0)
	  nt = 2;                  /* Stops an infinite loop... */
     nst = (len / nt) / 8;

     mag = log10(fabs(max - min));
     flr = floor(mag);
     sizeticratio = pow(10.0, mag - flr) / nt;

     /*
      * The ratio thresholds were calculated to split the difference
      * in the resulting number of ticks
      */
     d = 1.0;
     while(1){
	  if (sizeticratio > 2.857*d){
	       mult = 5;
	       break;
	  }
	  if (sizeticratio > 1.333*d){
	       mult = 2;
	       break;
	  }
	  if (sizeticratio > 0.6666*d){
	       mult = 1;
	       break;
	  }
	  d /= 10.0;
     }
     ret = ax->auto_tics ? mult * d * pow(10.0, flr) : ax->tic_interval;

     /*
      * Now figure out subtics.
      * If it makes sense to do 5 or 10 subdivision, do it.
      * Otherwise do a power of 2.
      * Respect the length in pixels between tics.
      */
#ifdef aaaaa
     if(ac->num_tics > 1)
	  len = len / (ac->num_tics - 1);
     if (mult == 2) {
	  if      (nst >= 8  && len > 80) nst = 7;
	  else if (nst >= 4  && len > 40) nst = 3;
	  else if (nst >= 2  && len >  10) nst = 1;
	  else                            nst = 0;
     }
     else {    /* 1 or 5 */
	  if      (nst >= 10 && len > 72) nst = 9;
	  else if (nst >= 5  && len > 32) nst = 4;
	  else if (nst >= 2  && len >  8) nst = 1;
	  else                            nst = 0;
     }
     if (ax->tic_dft) ax->subtics_per_tic = nst;
#endif
     if (ax->tic_dft) ax->subtics_per_tic = 1;
     else ax->subtics_per_tic = (ac->subtic_interval <= 1) ?
	1 : (ret / ac->subtic_interval - 1);
     if (ret<=0) {
       ret = 1;
       }
     return ret;
}

static void LinAxisRange(acw, minp, maxp, tip, nwp)
AtAxisCoreWidget acw;
float *minp, *maxp, *tip;
int *nwp;
{
     AtXYAxisWidget  aw = (AtXYAxisWidget)acw;
     AtAxisCorePart *ac = &aw->axiscore;
     AtXYAxisPart   *ax = &aw->axis;
     float nti, ti, min, max, mn, mx, sti, f;
     int c;

#if 0
     if (ax->auto_scale) {         /* Accept given values */
	  min = mn = *minp;
	  max = mx = *maxp;
     }
     else {                        /* Accept min/max resources */
	  min = mn = *minp = ax->min;
	  max = mx = *maxp = ax->max;
     }
#endif

     if (ac->max_dft) max = mx = *maxp;
     else max = mx = *maxp = ax->max;
     if (ac->min_dft) min = mn = *minp;
     else min = mn = *minp = ax->min;
     /*ac->max_bak = max; ac->min_bak = min;*/    

     /* Calculate tic interval */
  if (ax->num_dft) {
     c = 0;
     nti = CalcLinTicInterval(aw, mn, mx);
     do {
	  ti = nti;
	  if (mx - mn ==nti) break;
	  mn = floor(min / ti) * ti;
	  mx =  ceil(max / ti) * ti;
	  nti = CalcLinTicInterval(aw, mn, mx);
	  if (++c > 2)    /* Stop tic calculation! Originaly 5 times*/
	       ti = nti;
     } while (nti != ti);
   /*  if (!ax->tic_dft) ax->subtics_per_tic = (ac->subtic_interval <= 0) ?
	1 : (ti / ac->subtic_interval - 1);*/
  } else {
    if (ac->tic_interval<=0) {
      /*printf("WARNING tic_interval <= 0!\n");*/
      ac->tic_interval = 1;
      }
    nti = ti = ac->tic_interval;
    mn = floor(min / ti) * ti;
    mx =  ceil(max / ti) * ti;
    ax->subtics_per_tic = ((ax->tic_dft) || (ac->subtic_interval <=0)) ? 1 : 
	 (ac->tic_interval / ac->subtic_interval - 1);
    }

     /* Preserve tic endpoints */
     ax->tmin = mn;
     ax->tmax = mx;

     /* Round down/up min/max to subtic interval */
     if ( !ax->round_endpoints) {
	  sti = ti / (ax->subtics_per_tic + 1);
	  for (f = mn; f <= min; f += sti) {
	       if (f <= min)
		    mn = f;
	       else
		    break;
	  }
	  for (f = mx; f >= max; f -= sti) {
	       if (f >= max)
		    mx = f;
	       else
		    break;
	  }
     }
     *tip = ti;
     *minp = mn;
     *maxp = mx;

     /*ac->min = *minp * 1.01;       * Force recalc *
     ac->max = *maxp * 0.99;*/ 
     ac->min = *minp;  
     ac->max = *maxp; 
}

static void LinAxisCalc(acw)
AtAxisCoreWidget acw;
{
  AtXYAxisWidget  aw = (AtXYAxisWidget)acw;
  AtAxisCorePart *ac = &aw->axiscore;
  AtXYAxisPart   *ax = &aw->axis;
  float min, max, tmin, tmax, sti;
  float l, h, f, sh, sf;
  int nti, nsti;
  char lbl[256];

  /* Get endpoints */
  tmin = ax->tmin;
  tmax = ax->tmax;
  min = ac->min;
  max = ac->max;

  /* Calc subtic interval, upper and lower bounds */
  sti = ac->tic_interval / (ax->subtics_per_tic + 1);
  l = min + 0.5 * sti;
  h = max - 0.5 * sti;

  /* Count tics and subtics between the endpoints */
  for(nti = nsti = 2, f = tmin; f < tmax; f += ac->tic_interval) {
    if(f > l && f < h) {
       ++nti;
       ++nsti;
    }
    sh = f + ac->tic_interval - 0.5 * sti;
    for (sf = f + sti; sf < sh; sf += sti) {
      if (sf > l  && sf < h)
        ++nsti;
      }
    }

  ac->num_tics    = nti;
  ac->num_subtics = nsti;
  ac->tic_values =
	  (float *) XtMalloc(sizeof(float) * ac->num_tics);
  ac->tic_label_string =
	  (String *) XtMalloc(sizeof(String) * ac->num_tics);
  ac->subtic_values =
	  (float *) XtMalloc(sizeof(float) * ac->num_subtics);

  /* Set tics and labels at and between endpoints */
  ac->tic_values[0] = ac->subtic_values[0] = min;
  if (!ac->vertical && (ac->anno_method==PLOT_ANNO_TIME_LABELS)) {
      char str[TMSTR_MAXLEN];
      struct tm *xtm;
      time_t first, next;
      int inc, unit;
      AtPlotterPart *pp=&(((AtPlotterWidget)XtParent((Widget)acw))->plotter);

      switch (pp->time_unit) {
	case PLOT_TMUNIT_SECONDS: unit = 1;
	  break;
	case PLOT_TMUNIT_MINUTES: unit = 60;
	  break;
	case PLOT_TMUNIT_HOURS: unit = 3600;
	  break;
	case PLOT_TMUNIT_DAYS: unit = 24*3600;
	  break;
	case PLOT_TMUNIT_WEEKS: unit = 7*24*3600;
	  break;
	case PLOT_TMUNIT_MONTHS: unit = 30*24*3600; 
	  break;
	case PLOT_TMUNIT_YEARS: unit = 365*24*3600;
	default : unit = 1;
	}
      first = next = pp->time_base;
      xtm = gmtime(&next);
      inc = (int)(ac->tic_values[0] - pp->raw_bounding_box.xmin);
      if (inc==0) {
	strftime(str, TMSTR_MAXLEN, pp->time_format, xtm);
        ac->tic_label_string[0] = XtNewString(str);
	}
      else {
	switch (pp->time_unit) {
	  case PLOT_TMUNIT_MONTHS: 
	             xtm->tm_year += inc / 12;
		     xtm->tm_mon += inc % 12;
                     if (xtm->tm_mon>=12) {
		       xtm->tm_mon = 0;
		       xtm->tm_year++;
	               }
		     break;
          case PLOT_TMUNIT_YEARS: 
                     xtm->tm_year+=inc;
                     break;
          default:
                     first = pp->time_base + unit * inc;
	             xtm = gmtime(&(first));
          }
        strftime(str, TMSTR_MAXLEN, pp->time_format, xtm);
        ac->tic_label_string[0] = XtNewString(str);
        }
      
      for (nti = nsti = 0, f = tmin; f < tmax; f += ac->tic_interval) {
	  if (f > l && f < h) {
	       ++nti;
	       ++nsti;
	       ac->tic_values[nti] = ac->subtic_values[nsti] = f;
               inc = (int)(ac->tic_values[nti] - ac->tic_values[nti-1]);
	       if (inc <= 0) ac->tic_label_string[nti] = NULL;
	       else {
                 switch (pp->time_unit) {
	           case PLOT_TMUNIT_MONTHS: 
	             xtm->tm_year += inc / 12;
		     xtm->tm_mon += inc % 12;
                     if (xtm->tm_mon>=12) {
		       xtm->tm_mon = 0;
		       xtm->tm_year++;
	               }
                     break;
                   case PLOT_TMUNIT_YEARS: 
                     xtm->tm_year+=inc;
                     break;
                   default:
                     next = first + unit * 
		     ((int) (ac->tic_values[nti] - ac->tic_values[0]));
	             xtm = gmtime(&(next));
                     }
                 strftime(str, TMSTR_MAXLEN, pp->time_format, xtm);
	         ac->tic_label_string[nti] = XtNewString(str);
	         }
	  }
	  sh = f + ac->tic_interval - 0.5 * sti;
	  for (sf = f + sti; sf < sh; sf += sti) {
	       if (sf > l && sf < h) {
		    ++nsti;
		    ac->subtic_values[nsti] = sf;
	       }
	  }
     }
     ++nti;
     ++nsti;
     ac->tic_values[nti] = ac->subtic_values[nsti] = max;
     switch (pp->time_unit) {
           case PLOT_TMUNIT_MONTHS: 
	     xtm->tm_year += inc / 12;
	     xtm->tm_mon += inc % 12;
             if (xtm->tm_mon>=12) {
               xtm->tm_mon = 0;
               xtm->tm_year++;
               }
             break;
           case PLOT_TMUNIT_YEARS: 
             xtm->tm_year+=inc;
             break;
           default:
             next = first + unit *
		     ((int) (ac->tic_values[nti] - ac->tic_values[0]));
	     xtm = gmtime(&(next));
         }
     strftime(str, TMSTR_MAXLEN, pp->time_format, xtm);
     ac->tic_label_string[nti] = XtNewString(str);
    }
  else {
      sprintf(lbl, ax->lintic_format, min);
      ac->tic_label_string[0] = XtNewString(lbl);
        for (nti = nsti = 0, f = tmin; f < tmax; f += ac->tic_interval) {
	  if (f > l && f < h) {
	       ++nti;
	       ++nsti;
	       ac->tic_values[nti] = ac->subtic_values[nsti] = f;
	       sprintf(lbl, ax->lintic_format, f);
	       ac->tic_label_string[nti] = XtNewString(lbl);
	  }
	  sh = f + ac->tic_interval - 0.5 * sti;
	  for (sf = f + sti; sf < sh; sf += sti) {
	       if (sf > l && sf < h) {
		    ++nsti;
		    ac->subtic_values[nsti] = sf;
	       }
	  }
     }
     ++nti;
     ++nsti;
     ac->tic_values[nti] = ac->subtic_values[nsti] = max;
     sprintf(lbl, ax->lintic_format, max);
     ac->tic_label_string[nti] = XtNewString(lbl);
   } /*end of else */
}

/*
 *   Internal calc and range procs for logarithmic axes
 */

static void LogAxisRange P((AtAxisCoreWidget, float *, float *,
			    float *, int *));
static void LogAxisCalc P((AtAxisCoreWidget));

static void LogAxisRange(acw, minp, maxp, tip, nwp)
AtAxisCoreWidget acw;
float *minp, *maxp, *tip;
int *nwp;
{
     AtXYAxisWidget  aw = (AtXYAxisWidget) acw;
     AtAxisCorePart *ac = &aw->axiscore;
     AtXYAxisPart   *ax = &aw->axis;
     int len = ac->vertical ? ac->y1 - ac->y2 : ac->x2 - ac->x1;
     float lmin, lmax, min, max;
     float ls, lx, lm, lv;
     int nl, nst;

     /* Axis min/max boundaries on full decades */
#if 0
     if (ax->auto_scale) {              /* Accept given values */
	  min = *minp;
	  max = *maxp;
     }
     else {                             /* Accept min/max resources */
	  min = ax->min;
	  max = ax->max;
     }
#endif

     if (ac->min_dft) min = *minp;
     else min = ax->min;
     if (ac->max_dft) max = *maxp;
     else max = ax->max;
     /*ac->max_bak = max;
     ac->min_bak = min;*/

     lmin = floor(log10(min));
     lmax =  ceil(log10(max));

     /* Calc number of subtics dependent on number of pixels/decade */
     nl = len / (int) (lmax - lmin);
     if(nl > 80)
	  nst = 8;
     else if(nl > 40)
	  nst = 4;
     else if(nl > 8)
	  nst = 1;
     else
	  nst = 0;

     /* Preserve end points and number of subtics */
     ax->subtics_per_tic = nst;
     ax->tmin = lmin;
     ax->tmax = lmax;

     /* Calculate the real axis min/max boundaries */
     if (ax->round_endpoints) {         /* Min/max on decades */
	  *minp = pow(10.0, lmin);
	  *maxp = pow(10.0, lmax);
     }
     else {                             /* Min/max on subdecades */
	  ls = 1.0;                     /* Calc the lower boundary */
	  lx = 1.0;
	  lm = pow(10.0, lmin);
	  lv = min;
	  while (lm <= min) {
	       lv = lm;
	       lm  = pow(10.0, lmin + log10(ls));
	       ls += lx;
	  }
	  *minp = lv;
	  ls = 10.0;                    /* Calc the upper boundary */
	  lx =  1.0;
	  lm = pow(10.0, lmax);
	  lv = max;
	  lmax -= 1.0;
	  while (lm >= max) {
	       lv = lm;
	       lm  = pow(10.0, lmax + log10(ls));
	       ls -= lx;
	  }
	  *maxp = lv;
     }
     *tip  = 1.0;                       /* Always one tic per decade */

     ac->min = *minp * 1.01;           /*  Force recalc */
     ac->max = *maxp * 0.99;
}

static void LogAxisCalc(acw)
AtAxisCoreWidget acw;
{
#define Inrange(f)  (f >= ac->min && f <= ac->max)
     AtXYAxisWidget  aw = (AtXYAxisWidget)acw;
     AtAxisCorePart *ac = &aw->axiscore;
     AtXYAxisPart   *ax = &aw->axis;
     int nst = ax->subtics_per_tic;
     float lmin, lmax, min, max, lf, ld, lc, ls, lx;
     int nti, nsti, i;
     char lbl[256];

     lmin = ax->tmin;
     lmax = ax->tmax;

  if ((lmax-lmin)<1) {
     printf("LogAxisCalc - lmax-lmin<1\n");
     ac->num_tics    = 2;
     ac->num_subtics = nst;
     }
  else {
     ac->num_tics    = 1   + (int) (lmax - lmin);
     ac->num_subtics = nst * (int) (lmax - lmin);
     }
     ac->tic_values =
	  (float *) XtMalloc(sizeof(float) * ac->num_tics);
     ac->tic_label_string =
	  (String *) XtMalloc(sizeof(String) * ac->num_tics);
     ac->subtic_values =
	  (float *) XtMalloc(sizeof(float) * ac->num_subtics);

     nti = nsti = 0;
     for (ld = lmin; ld <= lmax; ld += 1.0) {
	  lf = pow(10.0, ld);
	  assert(nti < ac->num_tics);
	  if (Inrange(lf)) {
	       ac->tic_values[nti] = lf;
	       sprintf(lbl, aw->axis.logtic_format, lf);
	       ac->tic_label_string[nti] = XtNewString(lbl);
	       ++nti;
	  }
	  else {
	       if (nti == 0 && ac->min > pow(10.0, lmin)) {
		    ac->tic_values[nti] = ac->min;
		    sprintf(lbl, aw->axis.logtic_format, ac->min);
		    ac->tic_label_string[nti] = XtNewString(lbl);
		    ++nti;
	       }
	       else if (ac->max < pow(10.0, lmax)) {
		    ac->tic_values[nti] = ac->max;
		    sprintf(lbl, aw->axis.logtic_format, ac->max);
               	    ac->tic_label_string[nti] = XtNewString(lbl);
		    ++nti;
	       }
	  }
	  if (nst > 0 && ld < lmax) {
	       if (nst == 8) {
		    ls = 2.0;
		    lx = 1.0;
	       }
	       else if (nst == 4) {
		    ls = 2.0;
		    lx = 2.0;
	       }
	       else {    /* nst == 1 */
		    ls = 5.0;
		    lx = 5.0;
	       }
	       for (i = 0; i < nst; i++) {
		    lc = pow(10.0, ld + log10(ls));
		    if (Inrange(lc)) {
			 assert(nsti < ac->num_subtics);
			 ac->subtic_values[nsti] = lc;
			 ++nsti;
		    }
		    ls += lx;
	       }

	  }

     }

     if (ac->num_tics != nti) {
	  printf("LogAxisCalc: ac->num_tics %d != nti %d\n", ac->num_tics,nti);
	  ac->tic_values =
	       (float *) XtRealloc((char *) ac->tic_values,
				    sizeof(float) * nti);
	  ac->tic_label_string =
	       (String *) XtRealloc((char *) ac->tic_label_string,
				     sizeof(String) * nti);
	  ac->num_tics = nti;
     }
     if (ac->num_subtics != nsti) {
  printf("LogAxisCalc: ac->num_subtics %d != nsti %d\n", ac->num_subtics,nsti);
	  ac->subtic_values =
	       (float *) XtRealloc((char *) ac->subtic_values,
				    sizeof(float) * nsti);
	  ac->num_subtics = nsti;
     }

#undef Inrange
}

/*
 *   The XYAxis member procs
 */

/*
 *   The range proc
 */

static void RangeProc(acw, minp, maxp, tip, nwp)
AtAxisCoreWidget acw;
float *minp, *maxp, *tip;
int *nwp;
{
     AtXYAxisWidget  aw = (AtXYAxisWidget) acw;
     AtAxisCorePart *ac = &aw->axiscore;
     AtXYAxisPart   *ax = &aw->axis;
     char lbl[40];
     AtPlotterPart *pp=&(((AtPlotterWidget)XtParent((Widget)acw))->plotter);

     if (ax->axis_transform == AtTransformLOGARITHMIC) {
	  /*if ((ax->auto_scale && *minp <= 0.0) ||
	      ( !ax->auto_scale && ax->min <= 0.0)) */
	  if ((ac->min_dft && *minp <= 0.0) ||
	      ( !ac->min_dft && ax->min <= 0.0)) {
	       XtAppWarning(XtWidgetToApplicationContext(XtParent((Widget)aw)),
			    "Can't create logarithmic axis for min <= 0.0");
	       ax->axis_transform = AtTransformLINEAR;
	       LinAxisInitialize(aw);
	  }
	  else {
	       LogAxisRange(acw, minp, maxp, tip, nwp);
	       return;
	  }
     }
     LinAxisRange(acw, minp, maxp, tip, nwp);
}

/*
 *   The calc proc
 */

static void CalcProc(acw)
AtAxisCoreWidget acw;
{
     AtXYAxisWidget aw = (AtXYAxisWidget) acw;

     if (aw->axis.axis_transform == AtTransformLOGARITHMIC)
	  LogAxisCalc(acw);
     else      /* AtTransformLINEAR */
	  LinAxisCalc(acw);
}

/*
 *   The resource converter
 */

void AtCvtStringToAxisTransform(args, num_args, from, to)
XrmValue *args;
Cardinal num_args;
XrmValue *from, *to;
{
     AtTransform transform;

     transform = AtTransformINVALID;

     if (strcasecmp(from->addr, "linear") == 0)
	  transform = AtTransformLINEAR;
     else if (strcasecmp(from->addr, "logarithmic") == 0)
	  transform = AtTransformLOGARITHMIC;

     if (transform == AtTransformINVALID)
	  XtStringConversionWarning(from->addr, XtRTransform);
     else {
	  to->addr = (caddr_t) &transform;
	  to->size = sizeof(AtTransform);
     }
}

void AtRegisterAxisTransformConverter()
{
     static Boolean registered = False;

     if (!registered) {
	  XtAddConverter(XtRString, XtRTransform,
			 (XtConverter) AtCvtStringToAxisTransform, NULL,0);
	  registered = True;
     }
}
